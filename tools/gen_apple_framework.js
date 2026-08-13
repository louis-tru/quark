/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright (c) 2015, Louis.chu
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of Louis.chu nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL Louis.chu BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * ***** END LICENSE BLOCK ***** */

// console.log(process.argv)

var fs = require('qktool/node/fs');
var path = require('path');
var syscall = require('qktool/node/syscall').syscall;
var copy_header = require('./cp_header').copy_header;
var large_file_cut = require('qktool/node/large_file_cut').cut;
var read_quark_version = require('./read_version').read_quark_version
var argv = process.argv.slice(2);
var os = argv.shift();
var name = argv.shift();
var cut = argv.shift(); // is cut
var inc = argv.shift(); // copy include files
var out_dir = argv.shift();
var framework_dir = path.resolve(`${out_dir}/${name}.framework`);
var framework_content_dir = os == 'mac' ? `${framework_dir}/Versions/A`: framework_dir;
var framework_resources_dir = os == 'mac' ? `${framework_content_dir}/Resources`: framework_dir;
var source = __dirname + '/..';

if ( argv.length == 0 ) {
	throw new Error('Bad argument.');
}

function read_quark_version_str() {
	var versions = read_quark_version();
	var a = versions[0] < 10 ? '0' + versions[0] : versions[0];
	var b = versions[1] < 10 ? '0' + versions[1] : versions[1];
	var c = versions[2] < 10 ? '0' + versions[2] : versions[2];
	return `${a}.${b}.${c}`;
}

function read_plist_and_replace_version() {
	var version = read_quark_version_str();
	var en = 'utf-8';
	var str = fs.readFileSync(`${__dirname}/${os}-framework.plist`, en);//.toString(en);
	str = str.replace(new RegExp(Buffer.from('11.11.11').toString(en), 'gm'),
										Buffer.from(version).toString(en));
	str = str.replace(new RegExp(Buffer.from('xxxxxxxxxx').toString(en), 'gm'), 
										Buffer.from(name).toString(en));
	return Buffer.from(str, en);
}

// macOS frameworks use a versioned (non-shallow) bundle. iOS frameworks keep
// the shallow layout expected on that platform. Recreate the bundle so an old
// shallow macOS Info.plist cannot remain at the framework root.
fs.rm_r_sync(framework_dir);
fs.mkdir_p_sync(framework_resources_dir);

// write plist
var plist = `${framework_resources_dir}/Info.plist`;
fs.writeFileSync(plist, read_plist_and_replace_version());
syscall(`plutil -convert binary1 ${plist}`); // convert binary

// copy header
if (inc != 'no-inc') {
	for (var src of (inc || source + '/src').split(/\s+/)) {
		copy_header(src, framework_content_dir + '/Headers');
	}
}
// Merge dynamic library
var binary = `${framework_content_dir}/${name}`;
syscall(`lipo -create ${argv.join(' ')} -output ${binary}`);

if (os == 'mac') {
	fs.symlinkSync('A', `${framework_dir}/Versions/Current`);
	fs.symlinkSync('Versions/Current/Headers', `${framework_dir}/Headers`);
	fs.symlinkSync('Versions/Current/Resources', `${framework_dir}/Resources`);
	fs.symlinkSync(`Versions/Current/${name}`, `${framework_dir}/${name}`);
}

function sign() { // SING:
	var XCODEDIR = syscall('xcode-select --print-path').stdout[0];
	// Signing Identity: "iPhone Developer: xuewen chu (6RGZX563Q6)"
	//    533B431519212ED8D9723111DAA1BFD5280AED85
	syscall(
	`CODESIGN_ALLOCATE=${XCODEDIR}/Toolchains/XcodeDefault.xctoolchain/usr/bin/codesign_allocate \
	codesign --force --sign 533B431519212ED8D9723111DAA1BFD5280AED85 --timestamp=none ${framework_dir}
	`);
}

// sign();

if (cut === 'cut') {
	if ( fs.statSync(binary).size > 1024 * 1024 * 50 ) { // > 50mb
		large_file_cut(binary, 4);
		fs.rm_r(binary);
	}
}
