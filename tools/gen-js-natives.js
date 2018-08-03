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

var fs = require('fs');
var path = require('path');
var inputs = process.argv.slice(2);
var output_cc = inputs.pop();
var output_h = inputs.pop();
var is_wrap = inputs.pop() == 'wrap';
var type = inputs.pop();
var Buffer = require('buffer').Buffer;
var check_file_is_change = require('./check').check_file_is_change;

/*
console.log(inputs);
console.log(type);
console.log(output_h);
console.log(output_cc);
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
	{ 6, CORE_native_js_code_value_, "value" },
	{ 6, CORE_native_js_code_util_, "util" },
};
*/

var wrap_s = new Buffer('(function(exports, global){').toJSON().data;
var wrap_e = new Buffer('})').toJSON().data;
var wrap_len = is_wrap ? wrap_s.length + wrap_e.length : 0;

function write(fd) {
	for (var i = 1; i < arguments.length; i++) {
		fs.writeSync(fd, arguments[i], 'utf-8');
		//if ( !no_line_feed )
			fs.writeSync(fd, '\n', 'utf-8');
	}
}

function write_no_line_feed(fd) {
	for (var i = 1; i < arguments.length; i++) {
		fs.writeSync(fd, arguments[i], 'utf-8');
	}
}

function main() {

	if ( !check_file_is_change(inputs, [output_h, output_cc]) ) {
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
				'struct ' + type + '_NativeJSCode {',
				' int count;',
				' const char* code;',
				' const char* name;',
				'};'
				);
	
	write(fd_cc,
				'#include "' + output_h.replace(/^.+\/([^\/]+)$/, '$1') + '"',
				'namespace native_js {'
				);
	
	var js = [];
	
	for (var i = 0; i < inputs.length; i++) {
		
		var filename = inputs[i];
		var basename = path.basename(filename).replace(/\..*$/gm, '').replace(/-/gm, '_');
		var name = format_string('{0}_native_js_code_{1}_', type, basename);
		var count_name = format_string('{0}_native_js_code_{1}_count_', type, basename);
		var arr = fs.readFileSync(filename).toJSON().data;
		var length = arr.length + wrap_len;
		
		js.push({ name: name, count: length, basename: basename });
		
		// h
		write(fd_h, format_string('extern const int {0};', count_name));
		write(fd_h, format_string('extern const unsigned char {0}[];', name));
		// cc
		write(fd_cc, format_string('const int {0} = {1};', count_name, length));
		write(fd_cc, format_string('const unsigned char {0}[] = {', name));
		
		if (is_wrap) {
			write_no_line_feed(fd_cc, wrap_s.join(','), ',');
		}
		write_no_line_feed(fd_cc, arr.join(','));
		if (is_wrap) {
			write_no_line_feed(fd_cc, ',', new Buffer('})').toJSON().data.join(',') );
		}
		write(fd_cc, ',0', '};');
	}
	
	write(fd_h, format_string('extern const int {0}_native_js_count_;', type));
	write(fd_h, format_string('extern const {0}_NativeJSCode {0}_native_js_[];', type));
	write(fd_cc, format_string('const int {0}_native_js_count_ = {1};', type, js.length));
	write(fd_cc, format_string('const {0}_NativeJSCode {0}_native_js_[] = {', type));
	
	for (var i = 0; i < js.length; i++) {
		write(fd_cc, '{ ' + js[i].count + ', (const char*)' + js[i].name + 
										', "' + js[i].basename + '" },');
	}
	
	write(fd_h, '}\n#endif');
	write(fd_cc, '};\n}');
	fs.closeSync(fd_h);
	fs.closeSync(fd_cc);
}

main();
