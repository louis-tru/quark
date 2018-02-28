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

var util = require('./util');
var fs = require('fs');

/**
 * 解析行缩进
 */
function parse_indent(self, code) {

	var indent = 0;
	var space = 0;

	for (var i = 0; i < code.length; i++) {
		var char = code[i];

		if (char == ' ') {
			space++;
		}
		else if (char == '\t') {
			indent++;
		}
		else {
			break;
		}
	}

  // 使用2空格缩进
	if (space % 2 !== 0) {
		throw error(self, 'Keys data indent error');
	}

	return { indent: indent + space / 2, content: code.substr(i) };
}

// 读取一行代码
function read_line_code(self) {
  if (self.input.length > self.index) {
    var code = self.input[self.index];
    self.index++;
    return code;
  }
  return null;
}

// 解析接续多行值
function parse_continuous(self, str) { // str,
  if(str[str.length - 1] == '\\'){ // 连续的
    var ls = [str.substr(0, str.length - 1)];

    while(true) {
			str = read_line_code(self);
			if (str) {
				if (str[str.length - 1] == '\\') {
					ls.push(str.substr(0, str.length - 1));
				} else {
					ls.push(str);
					break;
				}
			} else {
				break;
			}
		}
		return ls.join('');
  }
  return str;
}

function parse_string_to(str) {
  return str.split(/[\s\t]+/).map(function (value) {
    var mat = value.match(/^((-?\d+(\.\d+)?((e|E)\d+)?)|(true)|(false)|(null))$/);
    if (mat) {
      return mat[2] ? parseFloat(value) : mat[6] ? true : mat[7] ? false : null;
    }
    return value;
  });
}

// 分割普通值
function parse_and_split_value(value) {

	var ls = []; // 结果
	var prev_end = 0;
	var index = 0;
	var c;

	// 处理字符串引号
	while (true) {
		var i;
		if ((i = value.indexOf("'", index)) != -1) {        // ' 查找字符串引号开始
			c = "'";
		} else if ((i = value.indexOf('"', index)) != -1) { // " 开始
			c = '"';
		} else { // 没找着字符串引号的开始
			break;
		}
		index = i;

		if (index === 0 ||
				value[index - 1] == ' ' ||
				value[index - 1] == '\t') { // 是否真的开始了

			if (prev_end != index) {
				var s = value.substring(prev_end, index).trim();
				if (s) {
					ls = ls.concat(parse_string_to(s));
				}
			}

			index++;

			var end = index;
			var str = [];

			// 查找结束
			while ((end = value.indexOf(c, end)) != -1) {
				if (value[end - 1] == '\\') { // 字符转义
          str.push(value.substring(index, end - 1) + c);
					end += 1; // 继续找
					index = end;
			  } else {    // 不是转义,字符串引号结束
					ls.push(value.substring(index, end));
					index = prev_end = end + 1; // 设置上一个结束的位置
					break;
				}
			}

			if (end == -1) { // 没找着'|",结束
				ls.push(value.substr(index));
				prev_end = value.length;
				break;
			}
		} else {
			index++; // 在下一个位置继续查找
		}
	}

	if (prev_end === 0) {
		ls = parse_string_to(value);
	}
	else if (prev_end != value.length) {
		var s = value.substr(prev_end).trim();
		if (s) {
			ls = ls.concat( parse_string_to(s.split(/[\s\t]+/)) );
		}
	}

	return ls;
}

// 解析多行数组
function parse_multi_row_array(self, indent) {

  var ls = [];
  var code = read_line_code(self);
  while(code !== null){
		if(/^[\s\t]*@end[\s\t]*$/.test(code)){ // 查询结束关键字
			// 开始缩进与结束缩进必需相同,否则异常
			if(parse_indent(self, code).indent == indent){
			  return ls;
			}
			else{
				throw error(self, '@end desc data indent error');
			}
		}
		ls.push(parse_continuous(self, code));
    code = read_line_code(self); // 继续查询end
  }
  return ls;
}

// 读取一对普通 key/value
function read_key_value_item(self) {
  var code;

  while (true) {
    code = read_line_code(self);
    if (code === null) {
      return null;
    }
    else if(code) {
      if(code.trim() !== ''){
        break;
      }
    }
  }

  var item = parse_indent(self, code);
  var content = item.content;
  var mat = content.match(/\@?[^\s\@,]+|,/); // 查询key

  if (!mat) {
    throw error(self, 'Key Illegal characters');
  }

  var key = mat[0];
  var value = '';

  if(key.length < content.length) {
    var char = content[key.length]; //content.substr(key.length, 1);

    switch (char) {
      case ':':
        // 多行描叙数组,所以这一行后面不能在出现非空格的字符
        // TODO : 后面可以随意写无需遵循缩进格式,直到文档结束或遇到@end
        if(/[^\s\t]/.test(content.substr(key.length + 1))){ // 查询非空格字符
          throw error(self, 'Parse multi row array Illegal characters');
        }
        value = parse_multi_row_array(self, item.indent); // 解析多行数组
        break;
      case ' ':
      case '\t':

        value = content.substr(key.length + 1).trim();
        if(value) {
          value = parse_and_split_value(parse_continuous(self, value) // 解析连续的字符
          															); // 解析分割普通值
          if (value.length == 1) {
            value = value[0];
          }
        }
        break;
      default:
        throw error(self, 'Key Illegal characters');
    }
  }

  item.key = key;
  item.value = value;
  return item;
}

function error(self, message) {
  var err = new Error(message + ', row: ' + (self.index - 1));
  err.row = self.index - 1;
  return err;
}

/**
 * push data
 */
function push_data(self, data, key, value) {
	if(data instanceof Array){
		data.push(value);
	} else {
    if (key in data && typeof data[key] != 'funciton') { // key 重复
      throw error(self, 'Key repeated');
    }
    data[key] = value;
	}
}

/**
 * keys 解析器
 * @class Parser
 */
function Parser(str) {
	this.index = 0;
	this.input = 	str.replace(/\#.*$/mg, '') 		// 删除注释
								.split(/\r?\n/);					//
}

/**
 * parse
 */
Parser.prototype.parse = function() {
	var item = read_key_value_item(this);
	if (!item)
		return { };

	var output = item.key == ',' ? [ ] : { }; // 数组或key/value

	var stack = [output];

	while (true) {
		var indent = item.indent;
		var key = item.key;
		var value = item.value;

		var data = stack[indent];
		if (!data) {
			throw error(this, 'Keys data indent error'); // 缩进只能为两个空格或tab
		}
		stack.splice(indent + 1); // 出栈

		var next = read_key_value_item(this);
		if (next) {
			if(next.indent == stack.length){ // 子对像

				if (value === '') { // 如果有子对像,这个值必需为 ''
					value = next.key == ',' ? [ ] : { };
					stack.push(value); // 压栈
				} else {
					throw error(this, 'Keys data indent error');
				}
			}
			push_data(this, data, key, value);
			item = next;
		} else {
			push_data(this, data, key, value);
			break; // 已经没有更多key/value,结束
		}
	}
	return output;
};

function parse_keys(str) {
  return new Parser(str).parse();
};

// -------------------------------------------------------------------------------------------

function write_data(self, value) {
  self.m_out.push(
    new Array(self.m_indent * 2 + 1).join(' ') + 
    self.m_cur_name + ' ' + value);
}

function stringify_obj(self, value) {
  for (var name in value) {
    util.assert(/^[^\s]+$/.test(name), 'Key Illegal characters, `{0}`', name);
    self.m_cur_name = name;
    stringify(self, value[name]);
  }
}

function stringify_arr(self, value) {
  for (var i = 0; i < value.length; i++) {
    self.m_cur_name = ',';
    stringify(self, value[i]);
  }
}

function stringify (self, value) {
  var m = { '\n': '\\n', "'": "\\'" };
  var is_space = false;
  
  switch (typeof value) {
    case 'string':
      value = value.replace(/[\n\'\s]/g, function (a) {
        var rev = m[a];
        if (rev) return rev;
        is_space = true;
        return a;
      });
      if (is_space) {
        write_data(self, "'" + value + "'");
      } else {
        write_data(self, value);
      }
      break;
    case 'number': 
      write_data(self, isFinite(value) ? String(value) : 'null');
      break;
    case 'boolean': 
      write_data(self, value.toString());
      break;
    case 'object': 
      if (!value) {
        write_data(self, 'null');
        break;
      }
      
      if (value instanceof Date) {
        var year = value.getUTCFullYear();
        var month = value.getUTCMonth() + 1;
        var date = value.getUTCDate();
        var hours = value.getUTCHours();
        var minutes = value.getUTCMinutes();
        var seconds = value.getUTCSeconds();
        var milliseconds = value.getUTCMilliseconds();
        write_data(self, year + '-' +
          (month < 10 ? '0' : '') + month + '-' +
          (date < 10 ? '0' : '') + date + 'T' +
          (hours < 10 ? '0' : '') + hours + ':' +
          (minutes < 10 ? '0' : '') + minutes + ':' +
          (seconds < 10 ? '0' : '') + seconds + '.' +
          milliseconds + "Z");
        break;
      }
      
      write_data(self, '');
      self.m_indent++;
      
      if (util.is_array(value)) {
        stringify_arr(self, value);
      } else {
        stringify_obj(self, value);
      }
      self.m_indent--;
      break;
    default:
      write_data(self, 'null');
      break;
  }
}

/**
 * @class StringParser
 * @private
 */
var StringParser = util.class('StringParser', {

  m_indent: 0,
  m_out: null,
  m_cur_name: '',

  stringify: function(value) {
    
    util.assert(value && typeof value == 'object', 'Data must be object or array');
    
    this.m_indent = 0;
    this.m_out = [ ];
    
    if (util.is_array(value)) {
      stringify_arr(this, value);
    } else {
      stringify_obj(this, value);
    }
    return this.m_out.join('\n');
  }
});

/**
 * @fun parse_file # 解析文件
 * @ret {Object}
 */
exports.parse_file = function(path) {
  return parse_keys( fs.readFileSync(path).toString('utf8') );
};

/**
 * @fun parse # 解析keys字符串
 * @ret {Object}
 */
exports.parse = function (str) {
  return parse_keys(str);
};

/**
 * @fun stringify # 转换对像为keys格式字符串
 * @arg value {Object}
 * @ret {String}
 */
exports.stringify = function(value) {
  return new StringParser().stringify(value);
};
