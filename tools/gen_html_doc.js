#!/usr/bin/env node
/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright (c) 2015-2016, xuewen.chu
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

var fs = require('ngui-util/fs');
var path = require('ngui-util/path');
var marked_html = require('ngui-tools/marked/html');
var marked = require('ngui-tools/node_modules/marked/index');
var read_ngui_version = require('./read_version').read_ngui_version;
var argv = process.argv.slice(2);
var template = null;
var indexs = { };

if ( argv < 2 ) {
	throw new Error('Bad argument.');
}

var source = path.format(argv[0]);
var output = path.format(argv[1]);

if ( fs.existsSync(source + '/index.md') ) { // 存在索引
	var md = fs.readFileSync(source + '/index.md').toString();
	md = md.replace(/\.(md|mdown)(\#|\))/img, '.html$2');
	var r = marked_html.gen_html(md, '', '__placeholder_body__');
	template = fs.readFileSync(__dirname + '/doc_template.html').toString();
	template = template.replace('__placeholder_version__', 'v' + read_ngui_version().join('.'));
	template = template.replace('__placeholder_index__', r.html);
}

function gen(src, target) {
	var extname = path.extname(src).toLowerCase();
	if ( extname == '.md' || extname == '.mdown' ) {		
		var md = fs.readFileSync(source + src).toString();
		var save = target.substr(0, target.length - extname.length) + '.html';
		md = md.replace(/\.(md|mdown)(\#|\))/img, '.html$2');
		var tmp = template.replace('__placeholder_src__', src.substr(1).replace(/.(md|mdown)/i, '.html'));
		tmp = tmp.replace('__placeholder_relative__', new Array(src.split('/').length - 1).join('../'));
		var r = marked_html.gen_html(md, indexs[src] || 'Ngui API Documentation', tmp);
		fs.writeFileSync(save, r.html);
	}
}

function each_dir(src, target) {

	if (!fs.existsSync(target)) { // 创建目录
    fs.mkdirSync(target);
  }

  var ls = fs.readdirSync(source + src);
  
  for (var i = 0; i < ls.length; i++) {
    var name = ls[i];
    if (name[0] != '.') {

	    var src2 = src + '/' + name;
	    var target2 = target + '/' + name;
	    var stat = fs.statSync(source + src2);
	    
	    if (stat.isFile()) {
	      gen(src2, target2);
	    } 
	    else if (stat.isDirectory()) {
	      each_dir(src2, target2);
	    }
  	}
  }
}

fs.mkdir_p_sync(output);
fs.cp_sync(__dirname + '/../node_modules/ngui-tools/marked/assets', output + '/assets');

each_dir('', output);
