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

var enum_keys = [
  [ 'AUTO',               'auto' ],
  [ 'FULL',               'full' ],
  [ 'PIXEL',              'pixel' ],
  [ 'PERCENT',            'percent' ],
  [ 'MINUS',              'minus' ],
  [ 'INHERIT',            'inherit' ],
  [ 'VALUE',              'value' ],
  [ 'THIN',               'thin'],              // 100
  [ 'ULTRALIGHT',         'ultralight'],        // 200
  [ 'LIGHT',              'light'],             // 300
  [ 'REGULAR',            'regular'],           // 400
  [ 'MEDIUM',             'medium'],            // 500
  [ 'SEMIBOLD',           'semibold'],          // 600
  [ 'BOLD',               'bold'],              // 700
  [ 'HEAVY',              'heavy'],             // 800
  [ 'BLACK',              'black'],             // 900
  [ 'THIN_ITALIC',        'thin_italic'],       // 100
  [ 'ULTRALIGHT_ITALIC',  'ultralight_italic'], // 200
  [ 'LIGHT_ITALIC',       'light_italic'],      // 300
  [ 'ITALIC',             'italic'],            // 400
  [ 'MEDIUM_ITALIC',      'medium_italic'],     // 500
  [ 'SEMIBOLD_ITALIC',    'semibold_italic'],   // 600
  [ 'BOLD_ITALIC',        'bold_italic'],       // 700
  [ 'HEAVY_ITALIC',       'heavy_italic'],      // 800
  [ 'BLACK_ITALIC',       'black_italic'],      // 900
  [ 'OTHER',              'other' ],
  [ 'NONE',               'none' ],
  [ 'OVERLINE',           'overline' ],
  [ 'LINE_THROUGH',       'line_through' ],
  [ 'UNDERLINE',          'underline' ],
  [ 'LEFT',               'left' ],
  [ 'CENTER',             'center' ],
  [ 'RIGHT',              'right' ],
  [ 'LEFT_REVERSE',       'left_reverse' ],
  [ 'CENTER_REVERSE',     'center_reverse' ],
  [ 'RIGHT_REVERSE',      'right_reverse' ],
  [ 'TOP',                'top' ],
  [ 'BOTTOM',             'bottom' ],
  [ 'MIDDLE',             'middle' ],
  [ 'REPEAT',             'repeat' ],
  [ 'REPEAT_X',           'repeat_x' ],
  [ 'REPEAT_Y',           'repeat_y' ],
  [ 'MIRRORED_REPEAT',    'mirrored_repeat' ],
  [ 'MIRRORED_REPEAT_X',  'mirrored_repeat_x' ],
  [ 'MIRRORED_REPEAT_Y',  'mirrored_repeat_x' ],
  [ 'NORMAL',             'normal' ],
  [ 'CLIP',               'clip' ],
  [ 'ELLIPSIS',           'ellipsis' ],
  [ 'CENTER_ELLIPSIS',    'center_ellipsis' ],
  [ 'NO_WRAP',            'no_wrap' ],
  [ 'NO_SPACE',           'no_space' ],
  [ 'PRE',                'pre' ],
  [ 'PRE_LINE',           'pre_line' ],
  // keyboard type
  [ 'ASCII',              'ascii' ],
  [ 'NUMBER',             'number' ],
  [ 'URL',                'url' ],
  [ 'NUMBER_PAD',         'number_pad' ],
  [ 'PHONE',              'phone' ],
  [ 'NAME_PHONE',         'name_phone' ],
  [ 'EMAIL',              'email' ],
  [ 'DECIMAL',            'decimal' ],
  [ 'TWITTER',            'twitter' ],
  [ 'SEARCH',             'search' ],
  [ 'ASCII_NUMBER',       'ascii_numner' ],
  // keyboard return type
  [ 'GO',                 'go' ],
  [ 'JOIN',               'join' ],
  [ 'NEXT',               'next' ],
  [ 'ROUTE',              'route' ],
  // [ 'SEARCH',             'search' ],
  [ 'SEND',               'send' ],
  [ 'DONE',               'done' ],
  [ 'EMERGENCY_CALL',     'emergency' ],
  [ 'CONTINUE',           'continue' ],
];

var enum_object = { };
var value_type = [];
var background_position_type = [];
var background_size_type = [];
var text_arrts_type = [];
var text_style = [];
var text_decoration = [];
var text_align = [];
var text_overflow = [];
var align = [];
var content_align = [];
var repeat = [];
var direction = [];
var text_overflow = [];
var text_white_space = [];
var keyboard_type = [];
var keyboard_return_type = [];

enum_keys.forEach(function(names, index) {
  enum_object[names[1]] = index;
  exports[names[0]] = index;
});

// ValueType
value_type[enum_object.auto] = 1;
value_type[enum_object.full] = 1;
value_type[enum_object.pixel] = 1;
value_type[enum_object.percent] = 1;
value_type[enum_object.minus] = 1;
// BackgroundPositionType
background_position_type[enum_object.pixel] = 1;
background_position_type[enum_object.percent] = 1;
background_position_type[enum_object.left] = 1;
background_position_type[enum_object.right] = 1;
background_position_type[enum_object.center] = 1;
background_position_type[enum_object.top] = 1;
background_position_type[enum_object.bottom] = 1;
// BackgroundSizeType
background_size_type[enum_object.auto] = 1;
background_size_type[enum_object.pixel] = 1;
background_size_type[enum_object.percent] = 1;
// TextArrtsType
text_arrts_type[enum_object.inherit] = 1;
text_arrts_type[enum_object.value] = 1;
// TextStyle
text_style[enum_object.thin] = 1;              // 100
text_style[enum_object.ultralight] = 1;        // 200
text_style[enum_object.light] = 1;             // 300
text_style[enum_object.regular] = 1;           // 400
text_style[enum_object.medium] = 1;            // 500
text_style[enum_object.semibold] = 1;          // 600
text_style[enum_object.bold] = 1;              // 700
text_style[enum_object.heavy] = 1;             // 800
text_style[enum_object.black] = 1;             // 900
text_style[enum_object.thin_italic] = 1;       // 100
text_style[enum_object.ultralight_italic] = 1; // 200
text_style[enum_object.light_italic] = 1;      // 300
text_style[enum_object.italic] = 1;            // 400
text_style[enum_object.medium_italic] = 1;     // 500
text_style[enum_object.semibold_italic] = 1;   // 600
text_style[enum_object.bold_italic] = 1;       // 700
text_style[enum_object.heavy_italic] = 1;      // 800
text_style[enum_object.black_italic] = 1;      // 900
text_style[enum_object.other] = 1;
// TextDecoration
text_decoration[enum_object.none] = 1;
text_decoration[enum_object.overline] = 1;
text_decoration[enum_object.line_through] = 1;
text_decoration[enum_object.underline] = 1;
// TextAlign
text_align[enum_object.left] = 1;
text_align[enum_object.center] = 1;
text_align[enum_object.right] = 1;
text_align[enum_object.left_reverse] = 1;
text_align[enum_object.center_reverse] = 1;
text_align[enum_object.right_reverse] = 1;
// TextOverflow
text_overflow[enum_object.normal] = 1;
text_overflow[enum_object.clip] = 1;
text_overflow[enum_object.ellipsis] = 1;
text_overflow[enum_object.center_ellipsis] = 1;
// TextWhiteSpace
text_white_space[enum_object.normal] = 1;
text_white_space[enum_object.no_wrap] = 1;
text_white_space[enum_object.no_space] = 1;
text_white_space[enum_object.pre] = 1;
text_white_space[enum_object.pre_line] = 1;
// Align
align[enum_object.left] = 1;
align[enum_object.right] = 1;
align[enum_object.center] = 1;
align[enum_object.top] = 1;
align[enum_object.bottom] = 1;
align[enum_object.none] = 1;
// ContentAlign
content_align[enum_object.left] = 1;
content_align[enum_object.right] = 1;
content_align[enum_object.top] = 1;
content_align[enum_object.bottom] = 1;
// Repeat
repeat[enum_object.none] = 1;
repeat[enum_object.repeat] = 1;
repeat[enum_object.repeat_x] = 1;
repeat[enum_object.repeat_y] = 1;
repeat[enum_object.mirrored_repeat] = 1;
repeat[enum_object.mirrored_repeat_x] = 1;
repeat[enum_object.mirrored_repeat_y] = 1;
// Direction
direction[enum_object.none] = 1;
direction[enum_object.left] = 1;
direction[enum_object.right] = 1;
direction[enum_object.top] = 1;
direction[enum_object.bottom] = 1;
// KeyboardType
keyboard_type[enum_object.normal] = 1;
keyboard_type[enum_object.ascii] = 1;
keyboard_type[enum_object.number] = 1;
keyboard_type[enum_object.url] = 1;
keyboard_type[enum_object.number_pad] = 1;
keyboard_type[enum_object.phone] = 1;
keyboard_type[enum_object.name_phone] = 1;
keyboard_type[enum_object.email] = 1;
keyboard_type[enum_object.decimal] = 1;
keyboard_type[enum_object.twitter] = 1;
keyboard_type[enum_object.search] = 1;
keyboard_type[enum_object.ascii_numner] = 1;
// KeyboardReturnType
keyboard_return_type[enum_object.normal] = 1;
keyboard_return_type[enum_object.go] = 1;
//keyboard_return_type[enum_object.google] = 1;
keyboard_return_type[enum_object.join] = 1;
keyboard_return_type[enum_object.next] = 1;
keyboard_return_type[enum_object.route] = 1;
keyboard_return_type[enum_object.search] = 1;
keyboard_return_type[enum_object.send] = 1;
//keyboard_return_type[enum_object.yahoo] = 1;
keyboard_return_type[enum_object.done] = 1;
keyboard_return_type[enum_object.emergency] = 1;
keyboard_return_type[enum_object['continue']] = 1;

// ----------------------------

var { Background, BackgroundImage, BackgroundGradient, _priv } = exports;

function check_uinteger(value) {
  return Number.isInteger(value) ? value >= 0 : false;
}

function check_integer_ret(value) {
  if (!check_uinteger(value)) {
    throw new Error('Bad argument.');
  }
  return value;
}

function check_unsigned_number(value) {
  return Number.isFinite(value) ? value >= 0 : false;
}

function check_number(value) {
  return Number.isFinite(value);
}

function check_number_ret(value) {
  if (!check_number(value)) {
    throw new Error('Bad argument.');
  }
  return value;
}

function check_unsigned_number_ret(value) {
  if (!check_unsigned_number(value)) {
    throw new Error('Bad argument.');
  }
  return value;
}

function check_is_null_ret(value) {
  if (value === null) {
    throw new Error('Bad argument.');
  }
  return value;
}

function check_enum(enum_obj, value) {
  return Number.isInteger(value) && enum_obj[value];
}

function check_enum_ret(enum_obj, value) {
  if (Number.isInteger(value) && enum_obj[value]) {
    return value;
  } else {
    throw new Error('Bad argument.');
  }
}

function check_string(value) {
  return typeof value == 'string'; 
}

function check_string_ret(value) {
  if (typeof value == 'string') {
    return value;
  } else {
    throw new Error('Bad argument.');
  }
}

function get_error_msg(reference, enum_value) {
  var message = '';
  if ( reference ) {
    for ( var val of reference ) {
      if ( val ) message += ', ' + JSON.stringify(val);
    }
  }
  if ( enum_value ) {
    for ( var i = 0; i < enum_value.length; i++ ) {
      if ( enum_value[i] ) message += ', ' + JSON.stringify(enum_keys[i][1]);
    }
  }
  return message;
}

class Base {
  toString() {
    return '[object]';
  }
}

class Enum extends Base {
  //_enum: null;
  //_value: enum_object.auto;
  get value() {
    return this._value;
  }
  set value(val) {
    this._value = check_enum_ret(this._enum, val);
  }
  constructor(value) {
    super();
    if (check_enum(this._enum, value)) {
      this._value = value;
    }
  }
  toString() {
    return enum_keys[this._value][1];
  }
}
Enum.prototype._enum = null;
Enum.prototype._value = enum_object.auto;

class TextAlign extends Enum {
  //_enum: text_align;
  //_value: enum_object.left;
  static help(str) {
    return get_error_msg(0, text_align);
  }
}
TextAlign.prototype._enum = text_align;
TextAlign.prototype._value = enum_object.left;

class Align extends Enum {
  //_enum: align;
  //_value: enum_object.left;
  static help(str) {
    return get_error_msg(0, align);
  }
}
Align.prototype._enum = align;
Align.prototype._value = enum_object.left;

class ContentAlign extends Enum {
  // _enum: content_align;
  // _value: enum_object.left;
  static help(str) {
    return get_error_msg(0, content_align);
  }
}
ContentAlign.prototype._enum = content_align;
ContentAlign.prototype._value = enum_object.left;

class Repeat extends Enum {
  // _enum: repeat;
  // _value: enum_object.none;
  static help(str) {
    return get_error_msg(0, repeat);
  }
}
Repeat.prototype._enum = repeat;
Repeat.prototype._value = enum_object.none;

class Direction extends Enum {
  // _enum: direction;
  // _value: enum_object.left;
  static help(str) {
    return get_error_msg(0, direction);
  }
}
Direction.prototype._enum = direction;
Direction.prototype._value = enum_object.left;

class KeyboardType extends Enum {
  // _enum: keyboard_type;
  // _value: enum_object.normal;
  static help(str) {
    return get_error_msg(0, keyboard_type);
  }
}
KeyboardType.prototype._enum = keyboard_type;
KeyboardType.prototype._value = enum_object.normal;

class KeyboardReturnType extends Enum {
  // _enum: keyboard_return_type;
  // _value: enum_object.normal;
  static help(str) {
    return get_error_msg(0, keyboard_return_type);
  }
}
KeyboardReturnType.prototype._enum = keyboard_return_type;
KeyboardReturnType.prototype._value = enum_object.normal;

class Border extends Base {
  // _width: 0;
  // _color: null;
  get width() { return this._width; }
  get color() { return this._color; }
  get r() { return this._color._r; }
  get g() { return this._color._g; }
  get b() { return this._color._b; }
  get a() { return this._color._a; }
  set width(value) {
    this._width = check_number_ret(value);
  }
  set color(value) { 
    if (value instanceof Color) {
      this._color = value; 
    } else if (typeof value == 'string') { // 解析字符串
      this._color = check_is_null_ret(parse_color(value));
    } else {
      throw new Error('Bad argument.');
    }
  }
  constructor(width, color) {
    super();
    if (arguments.length > 0) {
      if (check_number(width)) this._width = width;
      this._color = color instanceof Color ? color : new Color();
    } else {
      this._color = new Color();
    }
  }
  toString() {
    return `${this._width} ${this._color}`;
  }
  static help(str) {
    return get_error_msg(['10 #ff00aa', '10 rgba(255,255,0,255)']);
  }
}
Border.prototype._width = 0;
Border.prototype._color = null;

class Shadow extends Base {
  // _offset_x: 0;
  // _offset_y: 0;
  // _size: 0;
  // _color: null;
  get offsetX() { return this._offset_x; }
  get offsetY() { return this._offset_y; }
  get size() { return this._size; }
  get color() { return this._color; }
  get r() { return this._color._r; }
  get g() { return this._color._g; }
  get b() { return this._color._b; }
  get a() { return this._color._a; }
  set offsetX(value) {  this._offset_x = check_number_ret(value); }
  set offsetY(value) { this._offset_y = check_number_ret(value); }
  set size(value) { this._size = check_unsigned_number(value); }
  set color(value) {
    if (value instanceof Color) {
      this._color = value; 
    } else if (typeof value == 'string') { // 解析字符串
      this._color = check_is_null_ret(parse_color(value));
    } else {
      throw new Error('Bad argument.');
    }
  }
  constructor(offset_x, offset_y, size, color) {
    super();
    if (arguments.length > 0) {
      if (check_number(offset_x)) this._offset_x = offset_x;
      if (check_number(offset_y)) this._offset_y = offset_y;
      if (check_unsigned_number(size)) this._size = size;
      this._color = color instanceof Color ? color : new Color();
    } else {
      this._color = new Color();
    }
  }
  toString() {
    return `${this._offset_x} ${this._offset_y} ${this._size} ${this._color}`;
  }
  static help(str) {
    return get_error_msg(['10 10 2 #ff00aa', '10 10 2 rgba(255,255,0,255)']);
  }
}
Shadow.prototype._offset_x = 0;
Shadow.prototype._offset_y = 0;
Shadow.prototype._size = 0;
Shadow.prototype._color = null;

function to_hex_string(num) {
  if (num < 16) {
    return '0' + num.toString(16);
  } else {
    return num.toString(16);
  }
}

class Color extends Base {
// @private:
  // _r: 0;
  // _g: 0;
  // _b: 0;
  // _a: 255;
// @public:
  get r() { return this._r; }
  get g() { return this._g; }
  get b() { return this._b; }
  get a() { return this._a; }
  
  set r(value) {
    this._r = check_number_ret(value) % 256;
  }
  set g(value) {
    this._g = check_number_ret(value) % 256;
  }
  set b(value) {
    this._b = check_number_ret(value) % 256;
  }
  set a(value) {
    this._a = check_number_ret(value) % 256;
  }
  
  constructor(r, g, b, a) {
    super();
    if (arguments.length > 0) {
      if (check_uinteger(r)) this._r = r % 256;
      if (check_uinteger(g)) this._g = g % 256;
      if (check_uinteger(b)) this._b = b % 256;
      if (check_uinteger(a)) this._a = a % 256;
    }
  }
  
  reverse() {
    return new Color(255 - this._r, 255 - this._g, 255 - this._b, this._a);
  }
  
  toRGBString() {
    return `rgb(${this._r}, ${this._g}, ${this._b})`;
  }
  
  toRGBAString() {
    return `rgba(${this._r}, ${this._g}, ${this._b}, ${this._a})`;
  }
  
  toString() {
    return `#${to_hex_string(this._r)}${to_hex_string(this._g)}${to_hex_string(this._b)}`;
  }
  
  toHex32String() {
    return `#${to_hex_string(this._r)}${to_hex_string(this._g)}${to_hex_string(this._b)}${to_hex_string(this._a)}`;
  }
  static help(str) {
    return get_error_msg(['rgba(255,255,255,255)', '#ff0', '#ff00', '#ff00ff', '#ff00ffff']);
  }
}
Color.prototype._r = 0;
Color.prototype._g = 0;
Color.prototype._b = 0;
Color.prototype._a = 255;

class Vec2 extends Base {
// @private:
  // _x: 0;
  // _y: 0;
// @public:  
  get x() { return this._x; }
  get y() { return this._y; }
  set x(value) { this._x = check_number_ret(value); }
  set y(value) { this._y = check_number_ret(value); }
  constructor(x, y) {
    super();
    if (arguments.length > 0) {
      if (check_number(x)) this._x = x;
      if (check_number(y)) this._y = y;
    }
  }
  toString() {
    return `vec2(${this._x}, ${this._y})`;
  }
  static help(str) {
    return get_error_msg(['vec2(1,1)', '1 1']);
  }
}
Vec2.prototype._x = 0;
Vec2.prototype._y = 0;

class Vec3 extends Base {
// @private:
  // _x: 0;
  // _y: 0;
  // _z: 0;
// @public:
  get x() { return this._x; }
  get y() { return this._y; }
  get z() { return this._x; }
  set x(value) { this._x = check_number_ret(value); }
  set y(value) { this._y = check_number_ret(value); }
  set z(value) { this._z = check_number_ret(value); }
  constructor(x, y, z) {
    super();
    if (arguments.length > 0) {
      if (check_number(x)) this._x = x;
      if (check_number(y)) this._y = y;
      if (check_number(z)) this._z = z;
    }
  }
  toString() {
    return `vec3(${this._x}, ${this._y}, ${this._z})`;
  }
  static help(str) {
    return get_error_msg(['vec3(0,0,1)', '0 0 1']);
  }
}
Vec3.prototype._x = 0;
Vec3.prototype._y = 0;
Vec3.prototype._z = 0;

class Vec4 extends Base {
 // @private:
  // _x: 0;
  // _y: 0;
  // _z: 0;
  // _w: 0;
// @public:
  get x() { return this._x; }
  get y() { return this._y; }
  get z() { return this._x; }
  get w() { return this._w; }
  set x(value) { this._x = check_number_ret(value); }
  set y(value) { this._y = check_number_ret(value); }
  set z(value) { this._z = check_number_ret(value); }
  set w(value) { this._w = check_number_ret(value); }
  constructor(x, y, z, w) {
    super();
    if (arguments.length > 0) {
      if (check_number(x)) this._x = x;
      if (check_number(y)) this._y = y;
      if (check_number(z)) this._z = z;
      if (check_number(w)) this._w = w;
    }
  }
  toString() {
    return `vec4(${this._x}, ${this._y}, ${this._z}, ${this._w})`;
  }
  static help(str) {
    return get_error_msg(['vec4(0,0,1,1)', '0 0 1 1']);
  }
}
Vec4.prototype._x = 0;
Vec4.prototype._y = 0;
Vec4.prototype._z = 0;
Vec4.prototype._w = 0;

class Curve extends Base {
  // _p1_x: 0;
  // _p1_y: 0;
  // _p2_x: 1;
  // _p2_y: 1;
  constructor(p1_x, p1_y, p2_x, p2_y) {
    super();
    if (arguments.length > 0) {
      if (check_number(p1_x)) this._p1_x = p1_x;
      if (check_number(p1_y)) this._p1_y = p1_y;
      if (check_number(p2_x)) this._p2_x = p2_x;
      if (check_number(p2_y)) this._p2_y = p2_y;
    }
  }
  get point1X() { return this._p1_x; }
  get point1Y() { return this._p1_y; }
  get point2X() { return this._p2_x; }
  get point2Y() { return this._p2_y; }
  toString() {
    return `curve(${this.p1_x}, ${this.p1_y}, ${this.p2_x}, ${this.p2_y})`;
  }
  static help(str) {
    return get_error_msg(['curve(0,0,1,1)', '0 0 1 1']);
  }
}
Curve.prototype._p1_x = 0;
Curve.prototype._p1_y = 0;
Curve.prototype._p2_x = 1;
Curve.prototype._p2_y = 1;

class Rect extends Base {
// @private:
  // _x: 0;
  // _y: 0;
  // _width: 0;
  // _height: 0;
// @public:
  get x() { return this._x; }
  get y() { return this._y; }
  get width() { return this._width; }
  get height() { return this._height; }
  set x(value) { this._x = check_number_ret(value); }
  set y(value) { this._y = check_number_ret(value); }
  set width(value) { this._width = check_number_ret(value); }
  set height(value) { this._height = check_number_ret(value); }
  constructor(x, y, width, height) {
    super();
    if (arguments.length > 0) {
      if (check_number(x)) this._x = x;
      if (check_number(y)) this._y = y;
      if (check_number(width)) this._width = width;
      if (check_number(height)) this._height = height;
    }
  }
  toString() {
    return `rect(${this._x}, ${this._y}, ${this._width}, ${this._height})`;
  }
  static help(str) {
    return get_error_msg(['rect(0,0,-100,200)', '0 0 -100 200']);
  }
}
Rect.prototype._x = 0;
Rect.prototype._y = 0;
Rect.prototype._width = 0;
Rect.prototype._height = 0;
 
class Mat extends Base {
  // _value: null;
  get m0() { this._value[0]; }
  get m1() { this._value[1]; }
  get m2() { this._value[2]; }
  get m3() { this._value[3]; }
  get m4() { this._value[4]; }
  get m5() { this._value[5]; }
  set m0(value) { this._value[0] = check_number_ret(value); }
  set m1(value) { this._value[1] = check_number_ret(value); }
  set m2(value) { this._value[2] = check_number_ret(value); }
  set m3(value) { this._value[3] = check_number_ret(value); }
  set m4(value) { this._value[4] = check_number_ret(value); }
  set m5(value) { this._value[5] = check_number_ret(value); }
  constructor(m0, m1, m2, m3, m4, m5) {
    super();
    var value = [1, 0, 0, 0, 1, 0];
    if (arguments.length > 0) {
      if (arguments.length == 1) {
        if (check_number(m0)) {
          value[0] = m0;
          value[4] = m0;
        }
      } else {
        if (check_number(m0)) value[0] = m0;
        if (check_number(m1)) value[1] = m1;
        if (check_number(m2)) value[2] = m2;
        if (check_number(m3)) value[3] = m3;
        if (check_number(m4)) value[4] = m4;
        if (check_number(m5)) value[5] = m5;
      }
    }
    this._value = value;
  }
  toString() {
    var value = this._value;
    return `mat(${value[0]}, ${value[1]}, ${value[2]}, ${value[3]}, ${value[4]}, ${value[5]})`;
  }
  static help(str) {
    return get_error_msg(['mat(1,0,0,1,0,1)']);
  }
}
Mat.prototype._value = null;

class Mat4 extends Base {
  // _value: null;
  get m0() { this._value[0]; }
  get m1() { this._value[1]; }
  get m2() { this._value[2]; }
  get m3() { this._value[3]; }
  get m4() { this._value[4]; }
  get m5() { this._value[5]; }
  get m6() { this._value[6]; }
  get m7() { this._value[7]; }
  get m8() { this._value[8]; }
  get m9() { this._value[9]; }
  get m10() { this._value[10]; }
  get m11() { this._value[11]; }
  get m12() { this._value[12]; }
  get m13() { this._value[13]; }
  get m14() { this._value[14]; }
  get m15() { this._value[15]; }
  set m0(value) { this._value[0] = check_number_ret(value); }
  set m1(value) { this._value[1] = check_number_ret(value); }
  set m2(value) { this._value[2] = check_number_ret(value); }
  set m3(value) { this._value[3] = check_number_ret(value); }
  set m4(value) { this._value[4] = check_number_ret(value); }
  set m5(value) { this._value[5] = check_number_ret(value); }
  set m6(value) { this._value[6] = check_number_ret(value); }
  set m7(value) { this._value[7] = check_number_ret(value); }
  set m8(value) { this._value[8] = check_number_ret(value); }
  set m9(value) { this._value[9] = check_number_ret(value); }
  set m10(value) { this._value[10] = check_number_ret(value); }
  set m11(value) { this._value[11] = check_number_ret(value); }
  set m12(value) { this._value[12] = check_number_ret(value); }
  set m13(value) { this._value[13] = check_number_ret(value); }
  set m14(value) { this._value[14] = check_number_ret(value); }
  set m15(value) { this._value[15] = check_number_ret(value); }
  constructor(m0, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11, m12, m13, m14, m15) {
    super();
    var value = [1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1];
    if (arguments.length > 0) {
      if (arguments.length == 1) {
        if (check_number(m0)) {
          value[0] = m0;
          value[5] = m0;
          value[10] = m0;
          value[15] = m0;
        }
      } else {
        if (check_number(m0)) value[0] = m0;
        if (check_number(m1)) value[1] = m1;
        if (check_number(m2)) value[2] = m2;
        if (check_number(m3)) value[3] = m3;
        if (check_number(m4)) value[4] = m4;
        if (check_number(m5)) value[5] = m5;
        if (check_number(m6)) value[6] = m6;
        if (check_number(m7)) value[7] = m7;
        if (check_number(m8)) value[8] = m8;
        if (check_number(m9)) value[9] = m9;
        if (check_number(m10)) value[10] = m10;
        if (check_number(m11)) value[11] = m11;
        if (check_number(m12)) value[12] = m12;
        if (check_number(m13)) value[13] = m13;
        if (check_number(m14)) value[14] = m14;
        if (check_number(m15)) value[15] = m15;
      }
    }
    this._value = value;
  }
  toString() {
    var value = this._value;
    return `mat4(\
${value[0]}, ${value[1]}, ${value[2]}, ${value[3]}, \
${value[4]}, ${value[5]}, ${value[6]}, ${value[7]}, \
${value[8]}, ${value[9]}, ${value[10]}, ${value[11]}, \
${value[12]}, ${value[13]}, ${value[14]}, ${value[15]})`;
  }
  static help(str) {
    return get_error_msg(['mat4(1,0,0,1,0,1,0,1,0,0,1,1,0,0,0,1)']);
  }
}
Mat4.prototype._value = null;

class Value extends Base {
  // _type: enum_object.auto;
  // _value: 0;
  get type() { return this._type; }
  get value() { return this._value; }
  set type(value) { this._type = check_enum_ret(value_type, value); }
  set value(val) { this._value = check_number_ret(val); }
  constructor(type, value) {
    super();
    if (arguments.length > 0) {
      if (check_enum(value_type, type)) {
        this._type = type;
      }
      if (check_number(value)) {
        this._value = value;
      }
    }
  }
  toString() {
    switch (this._type) {
      case enum_object.auto: return 'auto';
      case enum_object.full: return 'full';
      case enum_object.pixel: return this._value.toString();
      case enum_object.percent: return this._value * 100 + '%';
      default: return this._value + '!';
    }
  }
  static help(str) {
    return get_error_msg(['auto', 'full', 10, '20%', '60!']);
  }
}
Value.prototype._type = enum_object.auto;
Value.prototype._value = 0;

class BackgroundPosition extends Base {
  // _type: enum_object.pixel;
  // _value: 0;
  get type() { return this._type; }
  get value() { return this._value; }
  set type(value) { this._type = check_enum_ret(background_position_type, value); }
  set value(val) { this._value = check_number_ret(val); }
  constructor(type, value) {
    super();
    if (arguments.length > 0) {
      if (check_enum(background_position_type, type)) {
        this._type = type;
      }
      if (check_number(value)) {
        this._value = value;
      }
    }
  }
  toString() {
    switch (this._type) {
      case enum_object.pixel: return this._value.toString();
      case enum_object.percent: return this._value * 100 + '%';
      case enum_object.left: return 'left';
      case enum_object.right: return 'right';
      case enum_object.top: return 'top';
      case enum_object.bottom: return 'bottom';
      default: return 'center';
    }
  }
  static help(str) {
    return get_error_msg(['left', 'right', 'center', 'top', 'bottom', 10, '20%']);
  }
}
BackgroundPosition.prototype._type = enum_object.pixel;
BackgroundPosition.prototype._value = 0;

class BackgroundSize extends Base {
  // _type: enum_object.auto;
  // _value: 0;
  get type() { return this._type; }
  get value() { return this._value; }
  set type(value) { this._type = check_enum_ret(background_size_type, value); }
  set value(val) { this._value = check_number_ret(val); }
  constructor(type, value) {
    super();
    if (arguments.length > 0) {
      if (check_enum(background_size_type, type)) {
        this._type = type;
      }
      if (check_number(value)) {
        this._value = value;
      }
    }
  }
  toString() {
    switch (this._type) {
      case enum_object.auto: return 'auto';
      case enum_object.pixel: return this._value.toString();
      default: return this._value * 100 + '%';
    }
  }
  static help(str) {
    return get_error_msg(['auto', 10, '20%']);
  }
}
BackgroundSize.prototype._type = enum_object.auto;
BackgroundSize.prototype._value = 0;

class TextAttrsValue extends Base {
  // _type: enum_object.inherit;
  get type() { return this._type; }
  set type(value) {
    this._type = check_enum_ret(text_arrts_type, value);
  }
  constructor(type) {
    super();
    if ( check_enum(text_arrts_type, type) ) {
      this._type = type;
    }
  }
}
TextAttrsValue.prototype._type = enum_object.inherit;

class TextAttrsEnumValue extends TextAttrsValue {
  // _value: null;
  // _enum: null;
  get value() { return this._value; }
  set value(val) { this._value = check_enum_ret(this._enum, val); }
  constructor(type, value) {
    if (arguments.length > 0) {
      super(type);
      if (check_enum(this._enum, value)) {
        this._value = value;
      }
    } else {
      super();
    }
  }
  toString() {
    return this._type == enum_object.inherit ? 'inherit' : enum_keys[this._value][1];
  }
}
TextAttrsEnumValue.prototype._value = null;
TextAttrsEnumValue.prototype._enum = null;

class TextColor extends TextAttrsValue {
  // _value: null;
  get value() { return this._value; }
  get r() { return this._value._r; }
  get g() { return this._value._g; }
  get b() { return this._value._b; }
  get a() { return this._value._a; }
  set value(val) {
    if (val instanceof Color) {
      this._value = val; 
    } else if (typeof val == 'string') { // 解析字符串
      this._value = check_is_null_ret(parse_color(val));
    } else {
      throw new Error('Bad argument.');
    }
  }
  constructor(type, value) {
    if (arguments.length > 0) {
      super(type);
      this._value = value instanceof Color ? value : new Color();
    } else {
      super();
      this._value = new Color();
    }
  }
  toString() {
    return this._type == enum_object.inherit ? 'inherit' : this._value.toString();
  }
  static help(str) {
    return get_error_msg(['inherit', 'rgba(255,255,255,255)', '#ff0', '#ff00', '#ff00ff', '#ff00ffff']);
  }
}
TextColor.prototype._value = null;

class TextSize extends TextAttrsValue {
  // _value: 12;
  get value() { return this._value; }
  set value(val) { this._value = check_unsigned_number_ret(val); }
  constructor(type, value) {
    if (arguments.length > 0) {
      super(type);
      if (check_unsigned_number(value)) {
        this._value = value;
      }
    } else {
      super();
    }
  }
  toString() {
    return this._type == enum_object.inherit ? 'inherit' : this._value.toString();
  }
  static help(str) {
    return get_error_msg(['inherit', 12]);
  }
}
TextSize.prototype._value = 12;

class TextFamily extends TextAttrsValue {
  // _value: '';
  get value() { return this._value; }
  set value(val) { this._value = check_string_ret(val); }
  constructor(type, value) {
    if (arguments.length > 0) {
      super(type);
      if (check_string(value)) {
        this._value = value;
      }
    } else {
      super();
    }
  }
  toString() {
    return this._type == enum_object.inherit ? 'inherit' : this._value;
  }
  static help(str) {
    return get_error_msg(['inherit', 'Ubuntu Mono']);
  }
}
TextFamily.prototype._value = '';

class TextStyle extends TextAttrsEnumValue {
  // _value: enum_object.regular;
  // _enum: text_style;
  static help(str) {
    return get_error_msg(['inherit'], text_style);
  }
}
TextStyle.prototype._value = enum_object.regular;
TextStyle.prototype._enum = text_style;

class TextShadow extends TextAttrsValue {
  // _value: null;
  get value() { return this._value; }
  get offsetX() { return this._value._offset_x; }
  get offsetY() { return this._value._offset_y; }
  get size() { return this._value._size; }
  get color() { return this._value._color; }
  get r() { return this._value._color._r; }
  get g() { return this._value._color._g; }
  get b() { return this._value._color._b; }
  get a() { return this._value._color._a; }
  set value(val) {
    if (val instanceof Shadow) {
      this._value = val;
    } else if (typeof val == 'string') {
      this._value = check_is_null_ret(parse_shadow(val));
    } else {
      throw new Error('Bad argument.');
    }
  }
  constructor(type, value) {
    if (arguments.length > 0) {
      super(type);
      if (value instanceof Shadow) {
        this._value = value;
      } else {
        this._value = new Shadow();
      }
    } else {
      super();
      this._value = new Shadow();
    }
  }
  toString() {
    return this._type == enum_object.inherit ? 'inherit' : this._value.toString();
  }
  static help(str) {
    return get_error_msg(['inherit', '10 10 2 #ff00aa', '10 10 2 rgba(255,255,0,255)']);
  }
}
TextShadow.prototype._value = null;

class TextLineHeight extends TextAttrsValue {
  // _is_auto: true;
  // _height: 0;
  get isAuto() { return this._height <= 0; }
  get height() { return this._height; }
  set height(value) {
    this._height = check_unsigned_number_ret(value);
  }
  constructor(type, height) {
    if (arguments.length > 0) {
      super(type);
      if (arguments.length > 1) {
        if (check_unsigned_number(height)) this._height = height;
      }
    } else {
      super();
    }
  }
  toString() {
    if (this._type == enum_object.inherit) {
      return 'inherit';
    } else if (this._height <= 0) {
      return 'auto';
    } else {
      return this._height.toString();
    }
  }
  static help(str) {
    return get_error_msg(['inherit', 24]);
  }
}
TextLineHeight.prototype._height = 0;

class TextDecoration extends TextAttrsEnumValue {
  // _value: enum_object.none;
  // _enum: text_decoration;
  static help(str) {
    return get_error_msg(['inherit'], text_decoration);
  }
}
TextDecoration.prototype._value = enum_object.none;
TextDecoration.prototype._enum = text_decoration;

class TextOverflow extends TextAttrsEnumValue {
  // _value: enum_object.normal;
  // _enum: text_overflow;
  static help(str) {
    return get_error_msg(['inherit'], text_overflow);
  }
}
TextOverflow.prototype._value = enum_object.normal;
TextOverflow.prototype._enum = text_overflow;

class TextWhiteSpace extends TextAttrsEnumValue {
  // _value: enum_object.normal;
  // _enum: text_white_space;
  static help(str) {
    return get_error_msg(['inherit'], text_white_space);
  }
}
TextWhiteSpace.prototype._value = enum_object.normal
TextWhiteSpace.prototype._enum = text_white_space

// List

Background.help = function(str) {
  return get_error_msg(['url(image.png)']);
};

class _Values {
  static help(str) {
    return get_error_msg(['auto', 'full', 10, '20%', '60!']);
  }
}

class _Floats {
  static help(str) {
    return get_error_msg([10, '10 20 30 40']);
  }
}

class _Repeats {
  static help(str) {
    return get_error_msg([10, '10 20 30 40']);
  }
}

class _BackgroundPositions {
  static help(str) {
    return get_error_msg([10, '20%', 'left', 'right', 'center', 'top', 'bottom']);
  }
}

class _BackgroundSizes {
  static help(str) {
    return get_error_msg(['auto', 10, '50%']);
  }
}

// ----------------------------

function _text_align(value) { 
  var rev = new TextAlign();
  rev._value = value;
  return rev;
}
function _align(value) { 
  var rev = new Align();
  rev._value = value;
  return rev;
}
function _content_align(value) {
  var rev = new ContentAlign();
  rev._value = value;
  return rev;
}
function _repeat(value) {
  var rev = new Repeat();
  rev._value = value;
  return rev;
}
function _direction(value) {
  var rev = new Direction();
  rev._value = value;
  return rev;
}
function _keyboard_type(value) {
  var rev = new KeyboardType();
  rev._value = value;
  return rev;
}
function _keyboard_return_type(value) {
  var rev = new KeyboardReturnType();
  rev._value = value;
  return rev;
}
function _border(width, r, g, b, a) {
  var rev = new Border();
  rev._width = width;
  rev._color = _color(r, g, b, a);
  return rev;
}
function _shadow(offset_x, offset_y, size, r, g, b, a) {
  var rev = new Shadow();
  rev._offset_x = offset_x;
  rev._offset_y = offset_y;
  rev._size = size;
  rev._color = _color(r, g, b, a);
  return rev;
}
function _color(r, g, b, a) {
  var rev = new Color();
  rev._r = r;
  rev._g = g;
  rev._b = b;
  rev._a = a;
  return rev;
}
function _vec2(x, y) {
  var rev = new Vec2();
  rev._x = x;
  rev._y = y;
  return rev;
}
function _vec3(x, y, z) {
  var rev = new Vec3();
  rev._x = x;
  rev._y = y;
  rev._z = z;
  return rev;
}
function _vec4(x, y, z, w) {
  var rev = new Vec4();
  rev._x = x;
  rev._y = y;
  rev._z = z;
  rev._w = w;
  return rev;
}
function _curve(p1_x, p1_y, p2_x, p2_y) {
  var rev = new Curve();
  rev._p1_x = p1_x;
  rev._p1_y = p1_y;
  rev._p2_x = p2_x;
  rev._p2_y = p2_y;
  return rev;
}
function _rect(x, y, width, height) {
  var rev = new Rect();
  rev._x = x;
  rev._y = y;
  rev._width = width;
  rev._height = height;
  return rev;
}
function _mat(...value) {
  var rev = new Mat();
  rev._value = Array.toArray(value);
  return rev;
}
function _mat4(...value) {
  var rev = new Mat4();
  rev._value = Array.toArray(value);
  return rev;
}
function _value(type, value) { 
  var rev = new Value();
  rev._type = type;
  rev._value = value;
  return rev;
}
function _background_position(type, value) { 
  var rev = new BackgroundPosition();
  rev._type = type;
  rev._value = value;
  return rev;
}
function _background_size(type, value) { 
  var rev = new BackgroundSize();
  rev._type = type;
  rev._value = value;
  return rev;
}
function _text_color(type, r, g, b, a) {
  var rev = new TextColor();
  rev._type = type;
  rev._value = _color(r, g, b, a);
  return rev;
}
function _text_size(type, value) {
  var rev = new TextStyle();
  rev._type = type;
  rev._value = value;
  return rev;
}
function _text_family(type, value) {
  var rev = new TextFamily();
  rev._type = type;
  rev._value = value;
  return rev;
}
function _text_style(type, value) {
  var rev = new TextStyle();
  rev._type = type;
  rev._value = value;
  return rev;
}
function _text_shadow(type, offset_x, offset_y, size, r, g, b, a) {
  var rev = new TextShadow();
  rev._type = type;
  rev._value = _shadow(offset_x, offset_y, size, r, g, b, a);
  return rev;
}
function _text_line_height(type, height) {
  var rev = new TextLineHeight();
  rev._type = type;
  rev._height = height;
  return rev;
}
function _text_decoration(type, value) {
  var rev = new TextDecoration();
  rev._type = type;
  rev._value = value;
  return rev;
}
function _text_overflow(type, value) {
  var rev = new TextOverflow();
  rev._type = type;
  rev._value = value;
  return rev;
}
function _text_white_space(type, value) {
  var rev = new TextWhiteSpace();
  rev._type = type;
  rev._value = value;
  return rev;
}

// parse

function parse_text_align(str) { 
  if (typeof str == 'string') {
    var value = enum_object[str];
    if (check_enum(text_align, value)) {
      var rev = new TextAlign();
      rev._value = value;
      return rev;
    }
  }
  return null;
}

function parse_align(str) { 
  if (typeof str == 'string') {
    var value = enum_object[str];
    if (check_enum(align, value)) {
      var rev = new Align();
      rev._value = value;
      return rev;
    }
  }
  return null;
}

function parse_content_align(str) {
  if (typeof str == 'string') {
    var value = enum_object[str];
    if (check_enum(content_align, value)) {
      var rev = new ContentAlign();
      rev._value = value;
      return rev;
    }
  }
  return null;
}

function parse_repeat(str) {
  if (typeof str == 'string') {
    var value = enum_object[str];
    if (check_enum(repeat, value)) {
      var rev = new Repeat();
      rev._value = value;
      return rev;
    }
  }
  return null;
}

function parse_direction(str) {
  if (typeof str == 'string') {
    var value = enum_object[str];
    if (check_enum(direction, value)) {
      var rev = new Direction();
      rev._value = value;
      return rev;
    }
  }
  return null;
}

function parse_keyboard_type(str) {
  if (typeof str == 'string') {
    var value = enum_object[str];
    if (check_enum(keyboard_type, value)) {
      var rev = new KeyboardType();
      rev._value = value;
      return rev;
    }
  }
  return null;
}

function parse_keyboard_return_type(str) {
  if (typeof str == 'string') {
    var value = enum_object[str];
    if (check_enum(keyboard_return_type, value)) {
      var rev = new KeyboardReturnType();
      rev._value = value;
      return rev;
    }
  }
  return null;
}

function parse_border(str) { 
  if (typeof str == 'string') {
    // 10 #ff00ff
    var m = str.match(/^ *((?:\d+)?\.?\d+)/);
    if (m) {
      var rev = new Border();
      rev._width = parseFloat(m[1]);
      var color = parse_color(str.substr(m[0].length + 1));
      if (color) {
        rev._color = color;
      }
      return rev;
    }
  }
  return null;
}

function parse_shadow(str) { 
  if (typeof str == 'string') {
    // 10 10 2 #ff00aa
    var m = str.match(/^ *(-?(?:\d+)?\.?\d+) +(-?(?:\d+)?\.?\d+) +((?:\d+)?\.?\d+)/);
    if (m) {
      var rev = new Shadow();
      rev._offset_x = parseFloat(m[1]);
      rev._offset_y = parseFloat(m[2]);
      rev._size = parseFloat(m[3]);
      var color = parse_color(str.substr(m[0].length + 1));
      if (color) {
        rev._color = color;
      }
      return rev;
    }
  }
  return null;
}

function parse_color(str) {
  if (typeof str == 'string') {
    if (/^ *none *$/.test(str)) {
      return _color(0, 0, 0, 0);
    }
    var m = str.match(/^#([0-9a-f]{3}([0-9a-f])?([0-9a-f]{2})?([0-9a-f]{2})?)$/i);
    if (m) {
      if (m[4]) { // 8
        return _color(parseInt(m[1].substr(0, 2), 16),
                      parseInt(m[1].substr(2, 2), 16),
                      parseInt(m[1].substr(4, 2), 16),
                      parseInt(m[1].substr(6, 2), 16));
      } else if (m[3]) { // 6
        return _color(parseInt(m[1].substr(0, 2), 16),
                      parseInt(m[1].substr(2, 2), 16),
                      parseInt(m[1].substr(4, 2), 16), 255);
      } else if (m[2]) { // 4
        return _color(parseInt(m[1].substr(0, 1), 16) * 17,
                      parseInt(m[1].substr(1, 1), 16) * 17,
                      parseInt(m[1].substr(2, 1), 16) * 17,
                      parseInt(m[1].substr(3, 1), 16) * 17);
      } else { // 3
        return _color(parseInt(m[1].substr(0, 1), 16) * 17,
                      parseInt(m[1].substr(1, 1), 16) * 17,
                      parseInt(m[1].substr(2, 1), 16) * 17, 255);
      }
    }
    var m = str.match(/^ *rgb(a)?\( *(\d{1,3}) *, *(\d{1,3}) *, *(\d{1,3})( *, *(\d{1,3}))? *\) *$/);
    if (m) {
      if (m[1] == 'a') { // rgba
        if (m[5]) { // a
          return _color(parseInt(m[2]) % 256, 
                        parseInt(m[3]) % 256,
                        parseInt(m[4]) % 256,
                        parseInt(m[6]) % 256);
        }
      } else { // rgb
        if (!m[5]) {
          return _color(parseInt(m[2]) % 256, 
                        parseInt(m[3]) % 256,
                        parseInt(m[4]) % 256, 255);
        }
      }
    }

  }
  return null;
}

function parse_vec2(str) {
  if (typeof str == 'string') {
    var m = str.match(/^ *(-?(?:\d+)?\.?\d+) +(-?(?:\d+)?\.?\d+) *$/) ||
            str.match(/^ *vec2\( *(-?(?:\d+)?\.?\d+) *, *(-?(?:\d+)?\.?\d+) *\) *$/);
    if (m) {
      return _vec2(parseFloat(m[1]), parseFloat(m[2]));
    }
  }
  return null;
}

function parse_vec3(str) {
  if (typeof str == 'string') {
    var m = str.match(/^ *(-?(?:\d+)?\.?\d+) +(-?(?:\d+)?\.?\d+) +(-?(?:\d+)?\.?\d+) *$/) ||
            str.match(/^ *vec3\( *(-?(?:\d+)?\.?\d+) *, *(-?(?:\d+)?\.?\d+) *, *(-?(?:\d+)?\.?\d+) *\) *$/);
    if (m) {
      return _vec3(parseFloat(m[1]), parseFloat(m[2]), parseFloat(m[3]));
    }
  }
  return null;
}

function parse_vec4(str) {
  if (typeof str == 'string') {
    var m = str.match(/^ *(-?(?:\d+)?\.?\d+) +(-?(?:\d+)?\.?\d+) +(-?(?:\d+)?\.?\d+) +(-?(?:\d+)?\.?\d+) *$/) ||
    str.match(/^ *vec4\( *(-?(?:\d+)?\.?\d+) *, *(-?(?:\d+)?\.?\d+) *, *(-?(?:\d+)?\.?\d+) *, *(-?(?:\d+)?\.?\d+) *\) *$/);
    if (m) {
      return _vec4(parseFloat(m[1]), parseFloat(m[2]), parseFloat(m[3]), parseFloat(m[4]));
    }
  }
  return null;
}

function parse_curve(str) {
  if (typeof str == 'string') {
    var m = str.match(/^ *(-?(?:\d+)?\.?\d+) +(-?(?:\d+)?\.?\d+) +(-?(?:\d+)?\.?\d+) +(-?(?:\d+)?\.?\d+) *$/) ||
    str.match(/^ *curve\( *(-?(?:\d+)?\.?\d+) *, *(-?(?:\d+)?\.?\d+) *, *(-?(?:\d+)?\.?\d+) *, *(-?(?:\d+)?\.?\d+) *\) *$/);
    if (m) {
      return _curve(parseFloat(m[1]), parseFloat(m[2]), parseFloat(m[3]), parseFloat(m[4]));
    }
  }
  return null;
}

function parse_rect(str) {
  if (typeof str == 'string') {
    var m = str.match(/^ *(-?(?:\d+)?\.?\d+) +(-?(?:\d+)?\.?\d+) +(-?(?:\d+)?\.?\d+) +(-?(?:\d+)?\.?\d+) *$/) ||
            str.match(/^ *rect\( *(-?(?:\d+)?\.?\d+) *, *(-?(?:\d+)?\.?\d+) *, *(-?(?:\d+)?\.?\d+) *, *(-?(?:\d+)?\.?\d+) *\) *$/);
    if (m) {
      return _rect(parseFloat(m[1]), parseFloat(m[2]), parseFloat(m[3]), parseFloat(m[4]));
    }
  }
  return null;
}

var parse_mat_reg = new RegExp(`^ *mat\\( *${new Array(6).join('(-?(?:\\d+)?\\.?\\d+) *, *')}(-?(?:\\d+)?\\.?\\d+) *\\) *$`);
var parse_mat4_reg = new RegExp(`^ *mat4\\( *${new Array(16).join('(-?(?:\\d+)?\\.?\\d+) *, *')}(-?(?:\\d+)?\\.?\\d+) *\\) *$`);
function parse_mat(str) {
  if (typeof str == 'string') {
    var m = parse_mat_reg.exec(str);
    if (m) {
      var value = [
        parseFloat(m[1]),
        parseFloat(m[2]),
        parseFloat(m[6]),
        parseFloat(m[4]),
        parseFloat(m[5]),
        parseFloat(m[6]),
      ];
      return _mat(value);
    }
  }
  return null;
}

function parse_mat4(str) {
  if (typeof str == 'string') {
    var m = parse_mat4_reg.exec(str);
    if (m) {
        var value = [
        parseFloat(m[1]),
        parseFloat(m[2]),
        parseFloat(m[6]),
        parseFloat(m[4]),
        parseFloat(m[5]),
        parseFloat(m[6]),
        parseFloat(m[7]),
        parseFloat(m[8]),
        parseFloat(m[9]),
        parseFloat(m[10]),
        parseFloat(m[11]),
        parseFloat(m[12]),
        parseFloat(m[13]),
        parseFloat(m[14]),
        parseFloat(m[15]),
        parseFloat(m[16]),
      ];
      return _mat4(value);
    }
  }
  return null;
}

function parse_value(str) { 
  if (typeof str == 'string') {
    // auto | full | 10.1 | 20% | 60!
    var m = str.match(/^((auto)|(full)|(-?(?:\d+)?\.?\d+)(%|!)?)$/);
    if (m) {
      if (m[2]) { // auto
        return _value(enum_object.auto, 0);
      } else if (m[3]) { // full
        return _value(enum_object.full, 0);
      } else { //
        var type = enum_object.pixel;
        var value = parseFloat(m[4]);
        if (m[5]) {
          if ( m[5] == '%' ) {
            type = enum_object.percent;
            value /= 100; // %
          } else { // 10!
            type = enum_object.minus;
          }
        }
        return _value(type, value);
      }
    }
  }
  return null;
}

var background_position_type_2 = [];
background_position_type_2[enum_object.left] = 1;
background_position_type_2[enum_object.right] = 1;
background_position_type_2[enum_object.center] = 1;
background_position_type_2[enum_object.top] = 1;
background_position_type_2[enum_object.bottom] = 1;

function parse_background_position(str) {
  if (typeof str == 'string') {
    // left | right | center | top | bottom | 10.1 | 20% 
    var type = enum_object[str];
    var value = 0;
    if (check_enum(background_position_type_2, type)) {
      return _background_position(type, value);
    }
    var m = str.match(/^(-?(?:\d+)?\.?\d+)(%)?$/);
    if (m) {
      type = enum_object.pixel;
      value = parseFloat(m[1]);
      if (m[2]) { // %
        type = enum_object.percent;
        value /= 100; // %
      }
      return _background_position(type, value);
    }
  }
  return null;
}

function parse_background_size(str) { 
  if (typeof str == 'string') {
    // auto | 10.1 | 20% 
    var m = str.match(/^((auto)|(-?(?:\d+)?\.?\d+)(%)?)$/);
    if (m) {
      if (m[2]) { // auto
        return _background_size(enum_object.auto, 0);
      } else {
        var type = enum_object.pixel;
        var value = parseFloat(m[3]);
        if (m[4]) { // %
          type = enum_object.percent;
          value /= 100; // %
        }
        return _background_size(type, value);
      }
    }
  }
  return null;
}

function parse_background_gradient(str) {
  // m = str.match(/^\s*gradient\(([^\(]+)\)\s*$/);
  // if (m) {}
  return null;
}

function parse_background_1(str) {
  var m;
  // background-repeat    // none | repeat | repeat_x | mirrored_repeat | mirrored_repeat_x | mirrored_repeat_y
  // background-position  // left | right | center | top | bottom | 10.1 | 20% 
  // background-size      // auto | 10.1 | 20% 
  m = str.match(/^\s*url\(([^\(]+)\)(?:\s+(.+))?\s*$/);
  if (m) {
    var attributes = { 
      src: m[1],
      // repeat
      // positionX
      // positionY
      // sizeX
      // sizeY
    };
    if (m[2]) { // attributes 
      var val;
      for (var i of m[2].split(/\s+/)) {
        if (val = parse_repeat(i)) { // repeat
          attributes.repeat = val;
        } else if (val = parse_background_position(i)) { // position
          if ('positionX' in attributes) {
            if ('positionY' in attributes) {
              if (val.type == enum_object.pixel || // px
                val.type == enum_object.percent    // %
              ) {
                val = _background_size(val.type, val.value);
                if ('sizeX' in attributes) {
                  if ('sizeY' in attributes) { // error
                    return null;
                  } else {
                    attributes.sizeY = val;
                  }
                } else {
                  attributes.sizeX = val;
                }
              } else { // error
                return null;
              }
            } else {
              attributes.positionY = val;
            }
          } else {
            attributes.positionX = val;
          }
        } else if (val = parse_background_size(i)) { // size
          if ('sizeX' in attributes) {
            if ('sizeY' in attributes) { // error
              return null;
            } else {
              attributes.sizeY = val;
            }
          } else {
            attributes.sizeX = val;
          }
        } else {
          // console.log('***************', i);
          return null;
        }
      }
    }
    // console.log('-----------------', attributes);
    return Object.assign(new BackgroundImage(), attributes);
  }
  return parse_background_gradient(str);
}

function parse_background_2(str) { // parse background image
  var m;
  m = str.match(/^\s*url\(([^\(]+)\)\s*$/);
  if (m) {
    return Object.assign(new BackgroundImage, { src: m[1] });
  } else {
    return parse_background_gradient(str);
  }
}

function parse_background(str, image) {
  var r = null;
  if (typeof str == 'string') {
    var prev;
    for (var i of str.split(/\s*,\s*/)) {
      var bg = image ? parse_background_2(i): parse_background_1(i);
      if (bg) {
        if (!r) {
          r = bg;
        }
        if (prev) {
          prev.next = bg;
        }
        prev = bg;
      } else {
        return null;
      }
    }
  }
  return r;
}

function parse_values(str) {
  if (typeof str == 'string') {
    var rev = [];
    for (var i of str.split(/\s+/)) {
      var val = parse_value(i);
      if (val) {
        rev.push(val);
      } else {
        return null;
      }
    }
    return rev;
  }
  return null;
}

function parse_floats(str) {
  if (typeof str == 'string') {
    var ls = str.split(/\s+/);
    var rev = [];
    for (var i of str.split(/\s+/)) {
      var mat = i.match(/^(-?(?:\d+)?\.?\d+)$/);
      if (!mat) {
        return null;
      }
      rev.push(parseFloat(mat[1]));
    }
    return rev;
  }
  return null;
}

function parse_repeats(str) {
  if (typeof str == 'string') {
    var rev = [];
    for (var i of str.split(/\s*,\s*/)) {
      var r = parse_repeat(i);
      if (r) {
        rev.push(r);
      } else {
        return null;
      }
    }
    return rev;
  }
  return null;
}

function parse_background_positions(str) { 
  if (typeof str == 'string') {
    var rev = [];
    for (var i of str.split(/\s*,\s*/)) {
      var items = [];
      for (var j of i.split(/\s+/)) {
        var r = parse_background_position(i);
        if (r) {
          items.push(r);
        } else {
          return null;
        }
      }
      if (items.length == 1) {
        items.push(_background_position(enum_object.center, 0));
      }
      r.push(items);
    }
    return rev;
  }
  return null;
}

function parse_background_sizes(str) { 
  if (typeof str == 'string') {
    var rev = [];
    for (var i of str.split(/\s*,\s*/)) {
      var items = [];
      for (var j of i.split(/\s+/)) {
        var r = parse_background_size(i);
        if (r) {
          items.push(r);
        } else {
          return null;
        }
      }
      if (items.length == 1) {
        items.push(_background_size(enum_object.auto, 0));
      }
      r.push(items);
    }
    return rev;
  }
  return null;
}

function parse_text_color(str) {
  if (typeof str == 'string') {
    if (str == 'inherit') {
      return new TextColor();
    } else {
      var value = parse_color(str);
      if (value) {
        return _text_color(enum_object.value, value._r, value._g, value._b, value._a);
      }
    }
  }
  return null;
}

function parse_text_size(str) {
  if (typeof str == 'string') {
    if (str == 'inherit') {
      return new TextSize();
    } else {
      if (/^(?:\d+)?\.?\d+$/.test(str)) {
        return _text_size(enum_object.value, parseFloat(str));
      }
    }
  }
  return null;
}

function parse_text_family(str) {
  if (typeof str == 'string') {
    if (str == 'inherit') {
      return new TextFamily();
    } else {
      return _text_family(enum_object.value, str);
    }
  }
  return null;
}

function parse_text_style(str) {
  if (typeof str == 'string') {
    if (str == 'inherit') {
      return new TextStyle();
    } else {
      var value = enum_object[str];
      if (check_enum(text_style, value)) {
        return _text_style(enum_object.value, value);
      }
    }
  }
  return null;
}

function parse_text_shadow(str) {
  if (typeof str == 'string') {
    if (str == 'inherit') {
      return new TextShadow();
    } else {
      var value = parse_shadow(str);
      if (value) {
        return _text_shadow(enum_object.value, value);
      }
    }
  }
  return null;
}

function parse_text_line_height(str) {
  if (typeof str == 'string') {
    if (str == 'inherit') {
      return new TextLineHeight();
    } else if (str == 'auto') {
      return _text_line_height(enum_object.value, 0);
    } else {
      if (/^(?:\d+)?\.?\d+$/.test(str)) {
        return _text_line_height(enum_object.value, parseFloat(str));
      }
    }
  }
  return null;
}

function parse_text_decoration(str) {
  if (typeof str == 'string') {
    if (str == 'inherit') {
      return new TextDecoration();
    } else {
      var value = enum_object[str];
      if (check_enum(text_decoration, value)) {
        return _text_decoration(enum_object.value, value);
      }
    }
  }
  return null;
}

function parse_text_overflow(str) {
  if (typeof str == 'string') {
    if (str == 'inherit') {
      return new TextDecoration();
    } else {
      var value = enum_object[str];
      if (check_enum(text_overflow, value)) {
        return _text_overflow(enum_object.value, value);
      }
    }
  }
  return null;
}

function parse_text_white_space(str) {
  if (typeof str == 'string') {
    if (str == 'inherit') {
      return new TextWhiteSpace();
    } else {
      var value = enum_object[str];
      if (check_enum(text_white_space, value)) {
        return _text_white_space(enum_object.value, value);
      }
    }
  }
  return null;
}

// constructor
exports.TextAlign = TextAlign;
exports.Align = Align;
exports.ContentAlign = ContentAlign;
exports.Repeat = Repeat;
exports.Direction = Direction;
exports.KeyboardType = KeyboardType;
exports.KeyboardReturnType = KeyboardReturnType;
exports.Border = Border;
exports.Shadow = Shadow;
exports.Color = Color;
exports.Vec2 = Vec2;
exports.Vec3 = Vec3;
exports.Vec4 = Vec4;
exports.Curve = Curve;
exports.Rect = Rect;
exports.Mat = Mat;
exports.Mat4 = Mat4;
exports.Value = Value;
exports.BackgroundPosition = BackgroundPosition;
exports.BackgroundSize = BackgroundSize;
exports.TextColor = TextColor;
exports.TextSize = TextSize;
exports.TextFamily = TextFamily;
exports.TextStyle = TextStyle;
exports.TextShadow = TextShadow;
exports.TextLineHeight = TextLineHeight;
exports.TextDecoration = TextDecoration;
exports.TextOverflow = TextOverflow;
exports.TextWhiteSpace = TextWhiteSpace;
 // parse
exports.parseTextAlign = parse_text_align;
exports.parseAlign = parse_align;
exports.parseContentAlign = parse_content_align;
exports.parseRepeat = parse_repeat;
exports.parseDirection = parse_direction;
exports.parseKeyboardType = parse_keyboard_type;
exports.parseKeyboardReturnType = parse_keyboard_return_type;
exports.parseBorder = parse_border;
exports.parseShadow = parse_shadow;
exports.parseColor = parse_color;
exports.parseVec2 = parse_vec2;
exports.parseVec3 = parse_vec3;
exports.parseVec4 = parse_vec4;
exports.parseCurve = parse_curve;
exports.parseRect = parse_rect;
exports.parseMat = parse_mat;
exports.parseMat4 = parse_mat4;
exports.parseValue = parse_value;
exports.parseBackground = parse_background;
exports.parseBackgroundPosition = parse_background_position;
exports.parseBackgroundSize = parse_background_size;
exports.parseTextColor = parse_text_color;
exports.parseTextSize = parse_text_size;
exports.parseTextFamily = parse_text_family;
exports.parseTextStyle = parse_text_style;
exports.parseTextShadow = parse_text_shadow;
exports.parseTextLineHeight = parse_text_line_height;
exports.parseTextDecoration = parse_text_decoration;
exports.parseTextOverflow = parse_text_overflow;
exports.parseTextWhiteSpace = parse_text_white_space;
exports.parseRepeats = parse_repeats;
exports.parseValues = parse_values;
exports.parseFloats = parse_floats;
exports.parseBackgroundPositions = parse_background_positions;
exports.parseBackgroundSizes = parse_background_sizes;
// _priv
_priv._isBase = function(val) { return val instanceof Base };
_priv._TextAlign = _text_align;
_priv._Align = _align;
_priv._ContentAlign = _content_align;
_priv._Repeat = _repeat;
_priv._Direction = _direction;
_priv._KeyboardType = _keyboard_type;
_priv._KeyboardReturnType = _keyboard_return_type;
_priv._Border = _border;
_priv._Shadow = _shadow;
_priv._Color = _color;
_priv._Vec2 = _vec2;
_priv._Vec3 = _vec3;
_priv._Vec4 = _vec4;
_priv._Curve = _curve;
_priv._Rect = _rect;
_priv._Mat = _mat;
_priv._Mat4 = _mat4;
_priv._Value = _value;
_priv._BackgroundPosition = _background_position;
_priv._BackgroundSize = _background_size;
_priv._TextColor = _text_color;
_priv._TextSize = _text_size;
_priv._TextFamily = _text_family;
_priv._TextStyle = _text_style;
_priv._TextShadow = _text_shadow;
_priv._TextLine_height = _text_line_height;
_priv._TextDecoration = _text_decoration;
_priv._TextOverflow = _text_overflow;
_priv._TextWhiteSpace = _text_white_space;
// priv class
_priv.Values = _Values;
_priv.Floats = _Floats;
_priv.Repeats = _Repeats;
_priv.BackgroundPositions = _BackgroundPositions;
_priv.BackgroundSizes = _BackgroundSizes;

Object.assign(_priv, exports);
