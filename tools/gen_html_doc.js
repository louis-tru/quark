#!/usr/bin/env node
/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright (c) 2015-2016, blue.chu
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of blue.chu nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL blue.chu BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * ***** END LICENSE BLOCK ***** */

var fs = require('qktool/node/fs');
var path = require('path');
var marked_html = require('qkmake/marked/html');
var read_quark_version = require('./read_version').read_quark_version;
var gen_readme = require('./gen_readme');
var argv = process.argv.slice(2);
var template = null;
var indexeds = {};

if ( argv < 2 ) {
	throw new Error('Bad argument.');
}

var source = path.resolve(argv[0]);
var output_md = path.resolve(argv[1], 'md');
var output_html = path.resolve(argv[1], 'html');

var getBasenamePrefix = gen_readme.getBasenamePrefix;
var escapingFilePath = gen_readme.escapingFilePath;

function genReadme(src, target) {
	if (!fs.existsSync(target)) {
		fs.mkdirSync(target);
	}

	var files =
		fs.readdirSync(source)
		.filter(e=>(e[0]!='.'&&['.ts','.tsx'].indexOf(path.extname(e))>=0));

	for (let name of files) {
		var src2 = src + '/' + name;
		var stat = fs.statSync(src2);

		if (stat.isFile()) {
			gen_readme.startExec(src2, `${target}/${getBasenamePrefix(name)}.md`);
		} 
		else if (stat.isDirectory()) {
			genReadme(src2, target + '/' + name);
		}
	}
}

function getIndexCode() {
	if (fs.existsSync(source + '/index.md')) {
		return fs.readFileSync(source + '/index.md').toString();
	} else {
		var files =
			fs.readdirSync(source)
			.filter(e=>(['errno.ts'].indexOf(e)==-1&&['.ts','.tsx'].indexOf(path.extname(e))>=0))
			.sort((a,b)=>a.localeCompare(b));
		// console.log(files);
		var lastFiles = [];
		files = files.filter(e=>e[0]=='_'?(lastFiles.push(e),false):true)

		return [
			'# [`Quark`]',
			'',
			'* [`About`](README.md)',
			'* [`Tools`](https://www.npmjs.com/package/qkmake)',
			'* [`Examples`](https://github.com/louis-tru/quark/tree/master/examples)',
			'',
			'# Modules',
			'',
			...files.concat(lastFiles).map(e=>{
				let name = getBasenamePrefix(e);
				return `* [\`quark/${name}\`](${escapingFilePath(name)}.md)`;
			}),
			'',
			'[`Quark`]: http://quarks.cc/',
		].join('\n');
	}
}

function makeIndexed() {
	var md = getIndexCode().replace(/\.(md|mdown)(\#|\))/img, '.html$2');
	var r = marked_html.gen_html(md, '', '__placeholder_body__');
	template = fs.readFileSync(__dirname + '/doc_template.html').toString();
	template = template.replace('__placeholder_version__', 'v' + read_quark_version().join('.'));
	template = template.replace('__placeholder_index__', r.html);
}

function genHtml(src, target) {
	var extname = path.extname(src).toLowerCase();
	if ( extname == '.md' || extname == '.mdown' ) {
		var md = fs.readFileSync(output_md + src).toString();
		var save = target.substring(0, target.length - extname.length) + '.html';
		md = md.replace(/\.(md|mdown)(\#|\))/img, '.html$2');
		var tmp = template.replace('__placeholder_src__', src.substring(1).replace(/.(md|mdown)/i, '.html'));
		tmp = tmp.replace('__placeholder_relative__', new Array(src.split('/').length - 1).join('../'));
		var r = marked_html.gen_html(md, indexeds[src] || 'Quark API Documentation', tmp);
		fs.writeFileSync(save, r.html);
	}
}

function eachGenHtml(src, target) {
	if (!fs.existsSync(target)) { // 创建目录
		fs.mkdirSync(target);
	}
	for (let name of fs.readdirSync(output_md + src)) {
		if (name[0] != '.') {
			var src2 = src + '/' + name;
			var target2 = target + '/' + escapingFilePath(name);
			var stat = fs.statSync(output_md + src2);

			if (stat.isFile()) {
				genHtml(src2, target2);
			} 
			else if (stat.isDirectory()) {
				eachGenHtml(src2, target2);
			}
		}
	}
}

fs.mkdir_p_sync(output_html);
fs.cp_sync(require.resolve('qkmake') + '/../marked/assets', output_html + '/assets');

genReadme(source, output_md);

fs.copyFileSync(`${__dirname}/../README.md`, `${output_md}/README.md`);
fs.copyFileSync(`${__dirname}/../README-cn.md`, `${output_md}/README-cn.md`);

makeIndexed();
eachGenHtml('', output_html);
