/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright (c) 2015, xuewen.chu
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of xuewen.chu nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL xuewen.chu BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * ***** END LICENSE BLOCK ***** */

var fs = require('ngui-utils/fs');
var path = require('path');
var syscall = require('ngui-utils/syscall').syscall;
var copy_header = require('./cp-header').copy_header;
var large_file_cut = require('ngui-utils/large_file_cut').cut;
var read_ngui_version = require('./read_version').read_ngui_version
var argv = process.argv.slice(2);
var os = argv.shift();
var framework_dir = path.resolve(argv.shift() + '/ngui.framework');
var source = __dirname + '/..';

if ( argv.length == 0 ) {
	throw new Error('Bad argument.');
}

function read_ngui_version_str() {
	var versions = read_ngui_version();
	var a = versions[0];
	var b = versions[1] < 10 ? '0' + versions[1] : versions[1];
	var c = versions[2] < 10 ? '0' + versions[2] : versions[2];
	return `${a}.${b}.${c}`;
}

function read_plist_and_replace_version(version) {
	var buf = fs.readFileSync(__dirname + '/framework.plist.binary');
	var reg = new RegExp(new Buffer('1.11.11').toString('hex'), 'gm');
	var hex_str = buf.toString('hex').replace(reg, new Buffer(String(version)).toString('hex'));
	var buf2 = new Buffer(hex_str, 'hex');
	return buf2;
	// var buf = fs.readFileSync(__dirname + '/ngui.framework.plist');
	// return buf.toString('utf8').replace(/{version}/mg, function() {
	// 	return version;
	// });
}

fs.mkdir_p_sync(framework_dir);

var version = read_ngui_version_str();

// write plist
fs.writeFileSync(framework_dir + '/Info.plist', read_plist_and_replace_version(version));
// copy header
copy_header(source + '/ngui', framework_dir + '/Headers');
// Merge dynamic library
syscall(`lipo -create ${argv.join(' ')} -output ${framework_dir}/ngui`);

if ( fs.statSync(`${framework_dir}/ngui`).size > 1024 * 1024 * 50 ) { // > 50mb
	large_file_cut(`${framework_dir}/ngui`, 4);
	fs.rm_r(`${framework_dir}/ngui`);
}

