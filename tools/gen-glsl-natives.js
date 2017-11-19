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

var fs          = require('fs');
var path        = require('path');
var inputs      = process.argv.slice(2);
var output_cpp  = inputs.pop();
var output      = inputs.pop();
var tag         = inputs.pop();
var placeholder = inputs.pop();
var Buffer      = require('buffer').Buffer;
var check_file_is_change = require('./check').check_file_is_change;

if ( placeholder ) {
  placeholder = placeholder.split(',');
}

// tag = '';

function format_string() {
  var rev = arguments[0];
  for (var i = 1, len = arguments.length; i < len; i++)
    rev = rev.replace(new RegExp('\\{' + (i - 1) + '\\}', 'g'), arguments[i]);
  return rev;
}

function read_file_data(input, paths) {
  if (paths[input]) {
    return '';
  }
  paths[input] = true;
  
  var code = fs.readFileSync(input).toString('utf8')
              .replace(/\/\/.*$/mg, '');  // 删除注释
  
  return code.replace(/^#include\s+"([^"]+)"/gm, function (all, a) {
    return  a.match(/\.(vp|fp)\.glsl$/i) ?
            '' : read_file_data(path.resolve(path.dirname(input), a), paths);
  });
}

function main() {

  if ( !check_file_is_change(inputs, [output, output_cpp]) ) {
    return;
  }
  
  var date = new Date();
  var include = [
                 '#ifndef __shader_natives_' + date.valueOf() + '__',
                 '#define __shader_natives_' + date.valueOf() + '__',
                 '#include "ngui/ogl/ogl.h"',
                 'namespace ngui {',
                 'namespace shader {',
                 ];
  
  var natives = [ ];
  
  var shaders_struct = [
                        '#pragma pack(push,4)',
                        format_string('struct {0}GLShaders {', tag),
                        ];
  
  var mat = output.match(/[\/\\]?([^\/\\]+\.(h|hpp))$/i);
  
  if ( ! mat ) {
    throw "Output error";
  }
  
  var cpp = [
             format_string('#include "{0}"', mat[1]),
             'namespace ngui {',
             'namespace shader {',
             ];
  
  for (var i = 0, address = 0; i < inputs.length; i++) {
    var input = inputs[i];
    var mat = input.match(/[\/\\](_)?([^\/\\]+?)(_)?\.glsl$/i);
    if ( mat && !mat[1] && !mat[3] ) {
      
      var name = mat[2].replace(/[\.-]/gm, '_');
      var codes = read_file_data(input, { }).split('#frag');

      // 

      if ( codes.length != 2 ) {
        continue;
      }
      
      var cur_address = address;  // shader相对地址
      address += 2;
      var uniforms = [];
      var uniform_blocks = [];
      var attributes = [];
      
      codes.forEach(function(glsl_code, index) {
        
        // glsl_code = '#version 300 es \n' + glsl_code;
                    
        var type_vp = !index;
        var id = name + '_' + (type_vp ? 'vp' : 'fp');
        
        var buff = new Buffer(glsl_code).toJSON().data;
        var length = buff.length;
        
        // TODO parser glsl code
        var glsl_code_reg =
        /^\s*(?:layout\s*\(\s*location\s*=\s*(\d+)\s*\)\s+)?(uniform|attribute|in)\s+((lowp|mediump|highp)\s+)?[a-zA-Z][a-zA-Z2-4]{2,}\s+([a-zA-Z0-9\_]+)\s*(\[\s*\d+\s*\])?;\s*$/mg;
        var glsl_code_mat = glsl_code_reg.exec(glsl_code);
        
        while ( glsl_code_mat ) {
          
          if (glsl_code_mat[2] == 'uniform') {
            // 剔除重复的名称
            if (uniforms.indexOf(glsl_code_mat[5]) == -1) {
              uniforms.push(glsl_code_mat[5]);
              address++;
            }
          } else if (glsl_code_mat[2] == 'in') {
            if ( type_vp ) {
              attributes.push(glsl_code_mat[5]);
              address++;
            }
          } else {
            attributes.push(glsl_code_mat[5]);
            address++;
          }
          
          glsl_code_mat = glsl_code_reg.exec(glsl_code);
        }
        
        glsl_code_reg = /^\s*uniform\s+([a-zA-Z0-9\$_]+)\s*\{/mg;
        glsl_code_mat = glsl_code_reg.exec(glsl_code);
          
        while ( glsl_code_mat ) { // 剔除重复的名称
          if (uniform_blocks.indexOf(glsl_code_mat[1]) == -1) {
            uniform_blocks.push(glsl_code_mat[1]);
            address++;
          }
          glsl_code_mat = glsl_code_reg.exec(glsl_code);
        }
        
        buff.push(0);
        
        var s1 = 'extern const unsigned char {0}[];';
        var s2 = 'extern const unsigned long {0}_len;';
        var s3 = 'extern const unsigned char {0}[] = { {1} };';
        var s4 = 'extern const unsigned long {0}_len = {1};';
        
        include.push(format_string(s1, id));
        include.push(format_string(s2, id));
        cpp.push(format_string(s3, id, buff.join(',')));
        cpp.push(format_string(s4, id, length));
        
        if ( ! type_vp ) { // frag end          
          s1 = '{ "{0}", {0}_vp, {0}_fp, {0}_vp_len, {0}_fp_len, {1}, "{2}", "{3}", "{4}" }';
          natives.push(format_string(s1, name, cur_address,
                                     uniforms.join(','),
                                     uniform_blocks.join(','),
                                     attributes.join(',')));
          
          shaders_struct.push(format_string('GLShader {0};', name));
          shaders_struct.push(uniforms.map(function(item) {
            return format_string('int {0}_uniform_{1} = 0;', name, item);
          }).join('\n'));
          if ( uniform_blocks.length ) {
            shaders_struct.push(uniform_blocks.map(function(item) {
              return format_string('int {0}_uniform_{1} = 0;', name, item);
            }).join('\n'));
          }
          if ( placeholder ) {
          // placeholder attribute identifier
            shaders_struct.push(placeholder.map(function(item, i) {
              address++;
              return format_string('unsigned int {0}_in_{1} = {2};', name, item, i);
            }).join('\n'));
          }
          if ( attributes.length ) {
            shaders_struct.push(attributes.map(function(item, i) {
              return format_string('unsigned int {0}_in_{1} = {2};', name, item, i);
            }).join('\n'));
          }
          shaders_struct.push('');
        }
        // console.log(input);
      });
    }
  }
  
  include.push(format_string('static const struct NativeGLSL {0}natives[] = {', tag));
  include.push(natives.join(',\n'));
  include.push('};');
  include.push('}');
  
  include.push(shaders_struct.join('\n'));
  include.push('};');
  include.push('#pragma pack(pop)');
  
  include.push('}');
  include.push('#endif');
  cpp.push('}');
  cpp.push('}');
  
  fs.writeFileSync(output, include.join('\n'));
  fs.writeFileSync(output_cpp, cpp.join('\n'));
}

main();
