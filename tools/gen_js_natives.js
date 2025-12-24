/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright (c) 2015-2016, Louis.chu
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

var fs = require('fs');
var path = require('path');
var inputs = process.argv.slice(2);
var output_cc = inputs.pop();
var output_h = inputs.pop();
var is_wrap = inputs.pop() == 'wrap';
var prefix = inputs.pop();
var pkgname = inputs.pop();
var Buffer = require('buffer').Buffer;
var check_file_is_change = require('./check').check_file_is_change;
var host_os = process.platform == 'darwin' ? 'osx': process.platform;

// console.log(inputs);
/*
console.log(process.argv);
console.log(prefix);
console.log(output_h);
console.log(output_cc);
console.log(pkgname);
*/

function format_string() {
	var rev = arguments[0];
	for (var i = 1, len = arguments.length; i < len; i++)
		rev = rev.replace(new RegExp('\\{' + (i - 1) + '\\}', 'g'), arguments[i]);
	return rev;
}

/*
 extern const int CORE_native_js_code_value_count_;
 extern const unsigned char CORE_native_js_code_value_[];
 extern count int CORE_native_js_code_util_count_;
 extern const unsigned char CORE_native_js_code_util_[];
 extern const int CORE_native_js_count_;
 extern const NativeJSCode CORE_native_js_[];
*/
/*
const int CORE_native_js_code_value_count_ = 6;
const unsigned char CORE_native_js_code_value_[] = { 0, 1, 2, 3, 4, 5 };
count int CORE_native_js_code_util_count_ = 6;
const unsigned char CORE_native_js_code_util_[] = { 0, 1, 2, 3, 4, 5 };
const int CORE_native_js_count_ = 2;
const NativeJSCode CORE_native_js_[] = {
	{ 6, CORE_native_js_code_value_, "value", ".js" },
	{ 6, CORE_native_js_code_util_, "util", ".js" },
};
*/

var wrap_s = Buffer.from('(function(exports,module,global){').toJSON().data;
var wrap_json_s = Buffer.from('(function(exports,module){module.exports=').toJSON().data;
var wrap_e = Buffer.from('})').toJSON().data;
var wrap_len = is_wrap ? wrap_s.length + wrap_e.length : 0;
var wrap_json_len = is_wrap ? wrap_json_s.length + wrap_e.length : 0;

function write(fp) {
	for (var i = 1; i < arguments.length; i++) {
		fs.writeSync(fp, arguments[i], 'utf-8');
		fs.writeSync(fp, '\n', 'utf-8');
	}
}

function write_no_line_feed(fp) {
	for (var i = 1; i < arguments.length; i++) {
		fs.writeSync(fp, arguments[i], 'utf-8');
	}
}

/**
 * Remove byte order marker. This catches EF BB BF (the UTF-8 BOM)
 * because the buffer-to-string conversion in `fs.readFileSync()`
 * translates it to FEFF, the UTF-16 BOM.
 */
function stripBOM(content) {
	if (content.charCodeAt(0) === 0xFEFF) {
		content = content.slice(1);
	}
	return content;
}

function readSource(pathname) {
	// var ext = path.extname(pathname);
	// if (/*pathname.indexOf('value.js') == -1 && */(ext == '.js' || ext == '.jsx')) {
	// 	console.log('jsa-shell', pathname);
	// 	syscall.syscall(`${__dirname}/../libs/qkmake/bin/${host_os}-jsa-shell ` +
	// 									`${pathname} ${pathname}~ --clean-comment`);
	// 	var result = fs.readFileSync(pathname + '~').toJSON().data;
	// 	// if (pathname.indexOf('value.js') != -1) {
	// 	// 	console.log(fs.readFileSync(pathname + '~', 'utf-8'));
	// 	// } else {
	// 	fs.unlinkSync(pathname + '~');
	// 	// }
	// 	return result;
	// } else {
	return fs.readFileSync(pathname).toJSON().data;
	// }
}

function write_file_item(filename, fd_h, fd_cc, pkgname, read) {
	var extname = path.extname(filename);
	var basename = path.basename(filename).replace(/\..*$/gm, '').replace(/-/gm, '_');
	var pathname = pkgname ? `${pkgname}/${basename}`: basename;
	var name_suffix = pathname.replace(/\//gm, '_');
	var name = format_string('{0}_native_js_code_{1}_', prefix, name_suffix);
	var count_name = format_string('{0}_native_js_code_{1}_count_', prefix, name_suffix);
	var arr = read(filename);
	var isJson = extname == '.json';
	var length = arr.length + (isJson ? wrap_json_len: wrap_len);

	var r = {
		name,
		count: length,
		pathname,
		extname,
		basename,
	};

	// h
	write(fd_h, format_string('extern const int {0};', count_name));
	write(fd_h, format_string('extern const unsigned char {0}[];', name));
	// cc
	write(fd_cc, format_string('const int {0} = {1};', count_name, length));
	write(fd_cc, format_string('const unsigned char {0}[] = {', name));

	if (is_wrap) {
		write_no_line_feed(fd_cc, (isJson ? wrap_json_s: wrap_s).join(','), ',');
	}
	write_no_line_feed(fd_cc, arr.join(','));
	if (is_wrap) {
		write_no_line_feed(fd_cc, ',', Buffer.from('})').toJSON().data.join(',') );
	}
	write(fd_cc, ',0', '};');

	return r;
}

function main() {

	if ( !check_file_is_change(inputs.concat([__filename]), [output_h, output_cc]) ) {
		return;
	}

	var fd_h = fs.openSync(output_h, 'w');
	var fd_cc = fs.openSync(output_cc, 'w');

	if (!fd_h || !fd_cc) {
		throw "Output error";
	}

	var h_name = output_h.replace(/[-\.\/]/gm, '_');

	write(fd_h,
				'#ifndef __native__js__' + h_name + '__',
				'#define __native__js__' + h_name + '__',
				'namespace native_js {',
				'struct ' + prefix + '_NativeJSCode {',
				' int count;',
				' const char* code;',
				' const char* name;',
				' const char* ext;',
				'};'
	);

	write(fd_cc,
				'#include "' + output_h.replace(/^.+\/([^\/]+)$/, '$1') + '"',
				'namespace native_js {'
	);

	var js = [];

	for (var i = 0; i < inputs.length; i++) {
		js.push( write_file_item(inputs[i], fd_h, fd_cc, pkgname, readSource) );
	}

	write(fd_h, format_string('extern const int {0}_native_js_count_;', prefix));
	write(fd_h, format_string('extern const {0}_NativeJSCode {0}_native_js_[];', prefix));
	write(fd_cc, format_string('const int {0}_native_js_count_ = {1};', prefix, js.length));
	write(fd_cc, format_string('const {0}_NativeJSCode {0}_native_js_[] = {', prefix));

	for (var { count, name, pathname, extname } of js) {
		write(fd_cc, `{ ${count}, (const char*)${name}, "${pathname}", "${extname}" },`);
	}

	write(fd_h, '}\n#endif');
	write(fd_cc, '};\n}');
	fs.closeSync(fd_h);
	fs.closeSync(fd_cc);
}

main();
