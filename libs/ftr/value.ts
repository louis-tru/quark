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

export enum Types {
	AUTO,
	FULL,
	PIXEL,
	PERCENT,
	MINUS,
	INHERIT,
	VALUE,
	THIN,              // 100
	ULTRALIGHT,        // 200
	LIGHT,             // 300
	REGULAR,           // 400
	MEDIUM,            // 500
	SEMIBOLD,          // 600
	BOLD,              // 700
	HEAVY,             // 800
	BLACK,             // 900
	THIN_ITALIC,       // 100
	ULTRALIGHT_ITALIC, // 200
	LIGHT_ITALIC,      // 300
	ITALIC,            // 400
	MEDIUM_ITALIC,     // 500
	SEMIBOLD_ITALIC,   // 600
	BOLD_ITALIC,       // 700
	HEAVY_ITALIC,      // 800
	BLACK_ITALIC,      // 900
	OTHER,
	NONE,
	OVERLINE,
	LINE_THROUGH,
	UNDERLINE,
	LEFT,
	CENTER,
	RIGHT,
	LEFT_REVERSE,
	CENTER_REVERSE,
	RIGHT_REVERSE,
	TOP,
	BOTTOM,
	MIDDLE,
	REPEAT,
	REPEAT_X,
	REPEAT_Y,
	MIRRORED_REPEAT,
	MIRRORED_REPEAT_X,
	MIRRORED_REPEAT_Y,
	NORMAL,
	CLIP,
	ELLIPSIS,
	CENTER_ELLIPSIS,
	NO_WRAP,
	NO_SPACE,
	PRE,
	PRE_LINE,
	WRAP,
	// keyboard type
	ASCII,
	NUMBER,
	URL,
	NUMBER_PAD,
	PHONE,
	NAME_PHONE,
	EMAIL,
	DECIMAL,
	TWITTER,
	SEARCH,
	ASCII_NUMBER,
	// keyboard return type
	GO,
	JOIN,
	NEXT,
	ROUTE,
	// SEARCH,
	SEND,
	DONE,
	EMERGENCY,
	CONTINUE,
}

export enum ValueType {
	AUTO = Types.AUTO,
	FULL = Types.FULL,
	PIXEL = Types.PIXEL,
	PERCENT = Types.PERCENT,
	MINUS = Types.MINUS,
}

export enum TextValueType {
	INHERIT = Types.INHERIT,
	VALUE = Types.VALUE,
}

export enum BackgroundPositionType {
	PIXEL = Types.PIXEL,
	PERCENT = Types.PERCENT,
	LEFT = Types.LEFT,
	RIGHT = Types.RIGHT,
	CENTER = Types.CENTER,
	TOP = Types.TOP,
	BOTTOM = Types.BOTTOM,
}

export enum BackgroundSizeType {
	AUTO = Types.AUTO,
	PIXEL = Types.PIXEL,
	PERCENT = Types.PERCENT,
}

export enum TextStyleEnum {
	THIN = Types.THIN,              // 100
	ULTRALIGHT = Types.ULTRALIGHT,  // 200
	LIGHT = Types.LIGHT,            // 300
	REGULAR = Types.REGULAR,        // 400
	MEDIUM = Types.MEDIUM,          // 500
	SEMIBOLD = Types.SEMIBOLD,      // 600
	BOLD = Types.BOLD,              // 700
	HEAVY = Types.HEAVY,            // 800
	BLACK = Types.BLACK,            // 900
	THIN_ITALIC = Types.THIN_ITALIC,       // 100
	ULTRALIGHT_ITALIC = Types.ULTRALIGHT_ITALIC, // 200
	LIGHT_ITALIC = Types.LIGHT_ITALIC,     // 300
	ITALIC = Types.ITALIC,                 // 400
	MEDIUM_ITALIC = Types.MEDIUM_ITALIC,   // 500
	SEMIBOLD_ITALIC = Types.SEMIBOLD_ITALIC,   // 600
	BOLD_ITALIC = Types.BOLD_ITALIC,       // 700
	HEAVY_ITALIC = Types.HEAVY_ITALIC,     // 800
	BLACK_ITALIC = Types.BLACK_ITALIC,     // 900
	OTHER = Types.OTHER,
}

export enum TextDecorationEnum {
	NONE = Types.NONE,
	OVERLINE = Types.OVERLINE,
	LINE_THROUGH = Types.LINE_THROUGH,
	UNDERLINE = Types.UNDERLINE,
}

export enum TextAlign {
	LEFT = Types.LEFT,
	CENTER = Types.CENTER,
	RIGHT = Types.RIGHT,
	LEFT_REVERSE = Types.LEFT_REVERSE,
	CENTER_REVERSE = Types.CENTER_REVERSE,
	RIGHT_REVERSE = Types.RIGHT_REVERSE,
}

export enum TextOverflowEnum {
	NORMAL = Types.NORMAL,
	CLIP = Types.CLIP,
	ELLIPSIS = Types.ELLIPSIS,
	CENTER_ELLIPSIS = Types.CENTER_ELLIPSIS,
}

export enum Align {
	LEFT = Types.LEFT,
	RIGHT = Types.RIGHT,
	CENTER = Types.CENTER,
	TOP = Types.TOP,
	BOTTOM = Types.BOTTOM,
	NONE = Types.NONE,
}

export enum ContentAlign {
	LEFT = Types.LEFT,
	RIGHT = Types.RIGHT,
	TOP = Types.TOP,
	BOTTOM = Types.BOTTOM,
}

export enum Repeat {
	NONE = Types.NONE,
	REPEAT = Types.REPEAT,
	REPEAT_X = Types.REPEAT_X,
	REPEAT_Y = Types.REPEAT_Y,
	MIRRORED_REPEAT = Types.MIRRORED_REPEAT,
	MIRRORED_REPEAT_X = Types.MIRRORED_REPEAT_X,
	MIRRORED_REPEAT_Y = Types.MIRRORED_REPEAT_Y,
}

export enum Direction {
	NONE = Types.NONE,
	LEFT = Types.LEFT,
	RIGHT = Types.RIGHT,
	TOP = Types.TOP,
	BOTTOM = Types.BOTTOM,
}

export enum TextWhiteSpaceEnum {
	NORMAL = Types.NORMAL,
	NO_WRAP = Types.NO_WRAP,
	NO_SPACE = Types.NO_SPACE,
	PRE = Types.PRE,
	PRE_LINE = Types.PRE_LINE,
	WRAP = Types.WRAP,
}

export enum KeyboardType {
	NORMAL = Types.NORMAL,
	ASCII = Types.ASCII,
	NUMBER = Types.NUMBER,
	URL = Types.URL,
	NUMBER_PAD = Types.NUMBER_PAD,
	PHONE = Types.PHONE,
	NAME_PHONE = Types.NAME_PHONE,
	EMAIL = Types.EMAIL,
	DECIMAL = Types.DECIMAL,
	TWITTER = Types.TWITTER,
	SEARCH = Types.SEARCH,
	ASCII_NUMBER = Types.ASCII_NUMBER,
}

export enum KeyboardReturnType {
	NORMAL = Types.NORMAL,
	GO = Types.GO,
	JOIN = Types.JOIN,
	NEXT = Types.NEXT,
	ROUTE = Types.ROUTE,
	SEARCH = Types.SEARCH,
	SEND = Types.SEND,
	DONE = Types.DONE,
	EMERGENCY = Types. EMERGENCY,
	CONTINUE = Types.CONTINUE,
}

const enum_object: Dict<number> = {};

Object.entries(Types).forEach(function([key,val]) {
	if (typeof val == 'number') {
		enum_object[key.toLowerCase()] = val;
	}
});

export declare class Background {
	next: Background | null;
}

export interface BackgroundOptions {
	image?: {
		src?: string;
		repeat?: Repeat;
		position?: BackgroundPositionCollection;
		positionX?: BackgroundPosition;
		positionY?: BackgroundPosition;
		size?: BackgroundSizeCollection;
		sizeX?: BackgroundSize;
		sizeY?: BackgroundSize;
	};
	gradient?: {}
}

export declare class BackgroundImage extends Background {
	src: string;
	// readonly hasBase64: boolean;
	repeat: Repeat;
	position: BackgroundPositionCollection;
	positionX: BackgroundPosition;
	positionY: BackgroundPosition;
	size: BackgroundSizeCollection;
	sizeX: BackgroundSize;
	sizeY: BackgroundSize;
}

// export declare class BackgroundGradient extends Background {
// }

export class BackgroundPositionCollection {
	x: BackgroundPosition;
	y: BackgroundPosition;
}

export class BackgroundSizeCollection {
	x: BackgroundSize;
	y: BackgroundSize;
}

function check_uinteger(value: any) {
	return Number.isInteger(value) ? value >= 0 : false;
}

function check_unsigned_number(value: any) {
	return Number.isFinite(value) ? value >= 0 : false;
}

function check_number(value: any) {
	return Number.isFinite(value);
}

function check_enum(enum_obj: any, value: any) {
	return Number.isInteger(value) && enum_obj[value];
}

function check_string(value: any) {
	return typeof value == 'string'; 
}

function check_integer_ret(value: any) {
	if (!check_uinteger(value)) {
		throw new Error('Bad argument.');
	}
	return value;
}

function check_number_ret(value: any) {
	if (!check_number(value)) {
		throw new Error('Bad argument.');
	}
	return value;
}

function check_unsigned_number_ret(value: any) {
	if (!check_unsigned_number(value)) {
		throw new Error('Bad argument.');
	}
	return value;
}

function check_is_null_ret(value: any) {
	if (value === null) {
		throw new Error('Bad argument.');
	}
	return value;
}

function check_enum_ret(enum_obj: any, value: any) {
	if (Number.isInteger(value) && enum_obj[value]) {
		return value;
	} else {
		throw new Error('Bad argument.');
	}
}

function check_string_ret(value: any) {
	if (typeof value == 'string') {
		return value;
	} else {
		throw new Error('Bad argument.');
	}
}

function get_help(reference?: any[], enum_value?: any) {
	var message = '';
	if ( reference ) {
		for ( var val of reference ) {
			if ( val ) message += ', ' + JSON.stringify(val);
		}
	}
	if ( enum_value ) {
		for (var i of Object.values(enum_value)) {
			if (typeof i == 'string') {
				message += ', ' + i.toLowerCase();
			}
		}
	}
	return message;
}

class Base {
	toString() {
		return '[object]';
	}
}

export class Border extends Base {
	private _width: number;
	private _color: Color;
	get width() { return this._width; }
	get color() { return this._color; }
	get r() { return (this._color as any)._r; }
	get g() { return (this._color as any)._g as number; }
	get b() { return (this._color as any)._b as number; }
	get a() { return (this._color as any)._a as number; }
	set width(value) {
		this._width = check_number_ret(value);
	}
	set color(value) { 
		if (value instanceof Color) {
			this._color = value; 
		} else if (typeof value == 'string') { // 解析字符串
			this._color = check_is_null_ret(parseColor(value));
		} else {
			throw new Error('Bad argument.');
		}
	}
	constructor(width?: number, color?: Color) {
		super();
		if (arguments.length > 0) {
			if (check_number(width)) this._width = width as number;
			this._color = color instanceof Color ? color : new Color();
		} else {
			this._color = new Color();
		}
	}
	toString() {
		return `${this._width} ${this._color}`;
	}
}
(Border.prototype as any)._width = 0;
(Border.prototype as any)._color = null;

export class Shadow extends Base {
	private _offset_x: number;
	private _offset_y: number;
	private _size: number;
	private _color: Color;
	get offsetX() { return this._offset_x; }
	get offsetY() { return this._offset_y; }
	get size() { return this._size; }
	get color() { return this._color; }
	get r() { return (this._color as any)._r; }
	get g() { return (this._color as any)._g; }
	get b() { return (this._color as any)._b; }
	get a() { return (this._color as any)._a; }
	set offsetX(value) {  this._offset_x = check_number_ret(value); }
	set offsetY(value) { this._offset_y = check_number_ret(value); }
	set size(value) { this._size = check_unsigned_number_ret(value); }
	set color(value) {
		if (value instanceof Color) {
			this._color = value; 
		} else if (typeof value == 'string') { // 解析字符串
			this._color = check_is_null_ret(parseColor(value));
		} else {
			throw new Error('Bad argument.');
		}
	}
	constructor(offset_x?: number, offset_y?: number, size?: number, color?: Color) {
		super();
		if (arguments.length > 0) {
			if (check_number(offset_x)) this._offset_x = offset_x as number;
			if (check_number(offset_y)) this._offset_y = offset_y as number;
			if (check_unsigned_number(size)) this._size = size as number;
			this._color = color instanceof Color ? color : new Color();
		} else {
			this._color = new Color();
		}
	}
	toString() {
		return `${this._offset_x} ${this._offset_y} ${this._size} ${this._color}`;
	}
}
(Shadow.prototype as any)._offset_x = 0;
(Shadow.prototype as any)._offset_y = 0;
(Shadow.prototype as any)._size = 0;
(Shadow.prototype as any)._color = null;

function to_hex_string(num: number) {
	if (num < 16) {
		return '0' + num.toString(16);
	} else {
		return num.toString(16);
	}
}

export class Color extends Base {
	private _r: number;
	private _g: number;
	private _b: number;
	private _a: number;

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
	
	constructor(r?: number, g?: number, b?: number, a?: number) {
		super();
		if (arguments.length > 0) {
			if (check_uinteger(r)) this._r = r as number % 256;
			if (check_uinteger(g)) this._g = g as number % 256;
			if (check_uinteger(b)) this._b = b as number % 256;
			if (check_uinteger(a)) this._a = a as number % 256;
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
}
(Color.prototype as any)._r = 0;
(Color.prototype as any)._g = 0;
(Color.prototype as any)._b = 0;
(Color.prototype as any)._a = 255;

export class Vec2 extends Base {
	private _x: number;
	private _y: number;
	get x() { return this._x; }
	get y() { return this._y; }
	set x(value) { this._x = check_number_ret(value); }
	set y(value) { this._y = check_number_ret(value); }
	constructor(x?: number, y?: number) {
		super();
		if (arguments.length > 0) {
			if (check_number(x)) this._x = x as number;
			if (check_number(y)) this._y = y as number;
		}
	}
	toString() {
		return `vec2(${this._x}, ${this._y})`;
	}
}
(Vec2.prototype as any)._x = 0;
(Vec2.prototype as any)._y = 0;

export class Vec3 extends Base {
	private _x: number;
	private _y: number;
	private _z: number;
	get x() { return this._x; }
	get y() { return this._y; }
	get z() { return this._x; }
	set x(value) { this._x = check_number_ret(value); }
	set y(value) { this._y = check_number_ret(value); }
	set z(value) { this._z = check_number_ret(value); }
	constructor(x?: number, y?: number, z?: number) {
		super();
		if (arguments.length > 0) {
			if (check_number(x)) this._x = x as number;
			if (check_number(y)) this._y = y as number;
			if (check_number(z)) this._z = z as number;
		}
	}
	toString() {
		return `vec3(${this._x}, ${this._y}, ${this._z})`;
	}
}
(Vec3.prototype as any)._x = 0;
(Vec3.prototype as any)._y = 0;
(Vec3.prototype as any)._z = 0;

export class Vec4 extends Base {
	private _x: number;
	private _y: number;
	private _z: number;
	private _w: number;
	get x() { return this._x; }
	get y() { return this._y; }
	get z() { return this._x; }
	get w() { return this._w; }
	set x(value) { this._x = check_number_ret(value); }
	set y(value) { this._y = check_number_ret(value); }
	set z(value) { this._z = check_number_ret(value); }
	set w(value) { this._w = check_number_ret(value); }
	constructor(x?: number, y?: number, z?: number, w?: number) {
		super();
		if (arguments.length > 0) {
			if (check_number(x)) this._x = x as number;
			if (check_number(y)) this._y = y as number;
			if (check_number(z)) this._z = z as number;
			if (check_number(w)) this._w = w as number;
		}
	}
	toString() {
		return `vec4(${this._x}, ${this._y}, ${this._z}, ${this._w})`;
	}
}
(Vec4.prototype as any)._x = 0;
(Vec4.prototype as any)._y = 0;
(Vec4.prototype as any)._z = 0;
(Vec4.prototype as any)._w = 0;

export class Curve extends Base {
	private _p1_x: number;
	private _p1_y: number;
	private _p2_x: number;
	private _p2_y: number;
	constructor(p1_x?: number, p1_y?: number, p2_x?: number, p2_y?: number) {
		super();
		if (arguments.length > 0) {
			if (check_number(p1_x)) this._p1_x = p1_x as number;
			if (check_number(p1_y)) this._p1_y = p1_y as number;
			if (check_number(p2_x)) this._p2_x = p2_x as number;
			if (check_number(p2_y)) this._p2_y = p2_y as number;
		}
	}
	get point1X() { return this._p1_x; }
	get point1Y() { return this._p1_y; }
	get point2X() { return this._p2_x; }
	get point2Y() { return this._p2_y; }
	toString() {
		return `curve(${this._p1_x}, ${this._p1_y}, ${this._p2_x}, ${this._p2_y})`;
	}
}
(Curve.prototype as any)._p1_x = 0;
(Curve.prototype as any)._p1_y = 0;
(Curve.prototype as any)._p2_x = 1;
(Curve.prototype as any)._p2_y = 1;

export class Rect extends Base {
	private _x: number;
	private _y: number;
	private _width: number;
	private _height: number;
	get x() { return this._x; }
	get y() { return this._y; }
	get width() { return this._width; }
	get height() { return this._height; }
	set x(value) { this._x = check_number_ret(value); }
	set y(value) { this._y = check_number_ret(value); }
	set width(value) { this._width = check_number_ret(value); }
	set height(value) { this._height = check_number_ret(value); }
	constructor(x?: number, y?: number, width?: number, height?: number) {
		super();
		if (arguments.length > 0) {
			if (check_number(x)) this._x = x as number;
			if (check_number(y)) this._y = y as number;
			if (check_number(width)) this._width = width as number;
			if (check_number(height)) this._height = height as number;
		}
	}
	toString() {
		return `rect(${this._x}, ${this._y}, ${this._width}, ${this._height})`;
	}
}
(Rect.prototype as any)._x = 0;
(Rect.prototype as any)._y = 0;
(Rect.prototype as any)._width = 0;
(Rect.prototype as any)._height = 0;
 
export class Mat extends Base {
	private _value: number[];
	get value() { return this._value; }
	get m0() { return this._value[0]; }
	get m1() { return this._value[1]; }
	get m2() { return this._value[2]; }
	get m3() { return this._value[3]; }
	get m4() { return this._value[4]; }
	get m5() { return this._value[5]; }
	set m0(value) { this._value[0] = check_number_ret(value); }
	set m1(value) { this._value[1] = check_number_ret(value); }
	set m2(value) { this._value[2] = check_number_ret(value); }
	set m3(value) { this._value[3] = check_number_ret(value); }
	set m4(value) { this._value[4] = check_number_ret(value); }
	set m5(value) { this._value[5] = check_number_ret(value); }
	constructor(...m: number[]) {
		super();
		var value = [1, 0, 0, 0, 1, 0];
		if (m.length > 0) {
			if (m.length == 1) {
				var m0 = m[0];
				if (check_number(m0)) {
					value[0] = m0 as number;
					value[4] = m0 as number;
				}
			} else {
				var j = 0;
				for (var i of m) {
					if (check_number(i))
						value[j++] = i;
				}
			}
		}
		this._value = value;
	}
	toString() {
		var value = this._value;
		return `mat(${value[0]}, ${value[1]}, ${value[2]}, ${value[3]}, ${value[4]}, ${value[5]})`;
	}
}

export class Mat4 extends Base {
	private _value: number[];
	get value() { return this._value; }
	get m0() { return this._value[0]; }
	get m1() { return this._value[1]; }
	get m2() { return this._value[2]; }
	get m3() { return this._value[3]; }
	get m4() { return this._value[4]; }
	get m5() { return this._value[5]; }
	get m6() { return this._value[6]; }
	get m7() { return this._value[7]; }
	get m8() { return this._value[8]; }
	get m9() { return this._value[9]; }
	get m10() { return this._value[10]; }
	get m11() { return this._value[11]; }
	get m12() { return this._value[12]; }
	get m13() { return this._value[13]; }
	get m14() { return this._value[14]; }
	get m15() { return this._value[15]; }
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
	constructor(...m: number[]) {
		super();
		var value = [1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1];
		if (m.length > 0) {
			if (m.length == 1) {
				var m0 = m[0];
				if (check_number(m0)) {
					value[0] = m0;
					value[5] = m0;
					value[10] = m0;
					value[15] = m0;
				}
			} else {
				var j = 0;
				for (var i of m) {
					if (check_number(i))
						value[j++] = i;
				}
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
}

export class Value extends Base {
	private _type: ValueType;
	private _value: number;
	get type() { return this._type; }
	get value() { return this._value; }
	set type(value) { this._type = check_enum_ret(ValueType, value); }
	set value(val) { this._value = check_number_ret(val); }
	constructor(type?: ValueType, value?: number) {
		super();
		if (arguments.length > 0) {
			if (check_enum(ValueType, type)) this._type = type as ValueType;
			if (check_number(value)) this._value = value as number;
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
}
(Value.prototype as any)._type = ValueType.AUTO;
(Value.prototype as any)._value = 0;

export class BackgroundPosition extends Base {
	private _type: BackgroundPositionType;
	private _value: number;
	get type() { return this._type; }
	get value() { return this._value; }
	set type(value) { this._type = check_enum_ret(BackgroundPositionType, value); }
	set value(val) { this._value = check_number_ret(val); }
	constructor(type?: BackgroundPositionType, value?: number) {
		super();
		if (arguments.length > 0) {
			if (check_enum(BackgroundPositionType, type)) this._type = type as BackgroundPositionType;
			if (check_number(value)) this._value = value as number;
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
}
(BackgroundPosition.prototype as any)._type = BackgroundPositionType.PIXEL;
(BackgroundPosition.prototype as any)._value = 0;

export class BackgroundSize extends Base {
	private _type: BackgroundSizeType;
	private _value: number;
	get type() { return this._type; }
	get value() { return this._value; }
	set type(value) { this._type = check_enum_ret(BackgroundSizeType, value); }
	set value(val) { this._value = check_number_ret(val); }
	constructor(type?: BackgroundSizeType, value?: number) {
		super();
		if (arguments.length > 0) {
			if (check_enum(BackgroundSizeType, type)) this._type = type as BackgroundSizeType;
			if (check_number(value)) this._value = value as number;
		}
	}
	toString() {
		switch (this._type) {
			case enum_object.auto: return 'auto';
			case enum_object.pixel: return this._value.toString();
			default: return this._value * 100 + '%';
		}
	}
}
(BackgroundSize.prototype as any)._type = BackgroundSizeType.AUTO;
(BackgroundSize.prototype as any)._value = 0;

class TextValue extends Base {
	protected _type: TextValueType;
	get type() { return this._type; }
	set type(value) {
		this._type = check_enum_ret(TextValueType, value);
	}
	constructor(type?: TextValueType) {
		super();
		if ( check_enum(TextValueType, type) ) this._type = type as TextValueType;
	}
}
(TextValue.prototype as any)._type = TextValueType.INHERIT;

export class TextColor extends TextValue {
	private _value: Color;
	get value() { return this._value; }
	get r() { return (this._value as any)._r; }
	get g() { return (this._value as any)._g; }
	get b() { return (this._value as any)._b; }
	get a() { return (this._value as any)._a; }
	set value(val) {
		if (val instanceof Color) {
			this._value = val; 
		} else if (typeof val == 'string') { // 解析字符串
			this._value = check_is_null_ret(parseColor(val));
		} else {
			throw new Error('Bad argument.');
		}
	}
	constructor(type?: TextValueType, value?: Color) {
		super(type);
		this._value = arguments.length > 1 && value instanceof Color ? value : new Color();
	}
	toString() {
		return this._type == TextValueType.INHERIT ? 'inherit' : this._value.toString();
	}
}
(TextColor.prototype as any)._value = null;

export class TextSize extends TextValue {
	private _value: number;
	get value() { return this._value; }
	set value(val) { this._value = check_unsigned_number_ret(val); }
	constructor(type?: TextValueType, value?: number) {
		super(type);
		if (arguments.length > 1) {
			if (check_unsigned_number(value)) this._value = value as number;
		}
	}
	toString() {
		return this._type == TextValueType.INHERIT ? 'inherit' : this._value.toString();
	}
}
(TextSize.prototype as any)._value = 12;

export class TextFamily extends TextValue {
	private _value: string;
	get value() { return this._value; }
	set value(val) { this._value = check_string_ret(val); }
	constructor(type?: TextValueType, value?: string) {
		super(type);
		if (arguments.length > 1) {
			if (check_string(value)) this._value = value as string;
		}
	}
	toString() {
		return this._type == TextValueType.INHERIT ? 'inherit' : this._value;
	}
}
(TextFamily.prototype as any)._value = '';

export class TextStyle extends TextValue {
	private _value: TextStyleEnum;
	get value() { return this._value; }
	set value(val) { this._value = check_enum_ret(TextStyleEnum, val); }
	constructor(type?: TextValueType, value?: TextStyleEnum) {
		super(type);
		if (arguments.length > 1) {
			if (check_enum(TextStyleEnum, value)) this._value = value as TextStyleEnum;
		}
	}
	toString() {
		return this._type == TextValueType.INHERIT ? 'inherit' : TextStyleEnum[this._value].toLowerCase();
	}
}
(TextStyle.prototype as any)._value = TextStyleEnum.REGULAR;

export class TextShadow extends TextValue {
	private _value: Shadow;
	get value() { return this._value; }
	get offsetX() { return (this._value as any)._offset_x; }
	get offsetY() { return (this._value as any)._offset_y; }
	get size() { return (this._value as any)._size; }
	get color() { return (this._value as any)._color; }
	get r() { return (this as any)._value._color._r; }
	get g() { return (this as any)._value._color._g; }
	get b() { return (this as any)._value._color._b; }
	get a() { return (this as any)._value._color._a; }
	set value(val) {
		if (val instanceof Shadow) {
			this._value = val;
		} else if (typeof val == 'string') {
			this._value = check_is_null_ret(parseShadow(val));
		} else {
			throw new Error('Bad argument.');
		}
	}
	constructor(type?: TextValueType, value?: Shadow) {
		super(type);
		this._value = arguments.length > 1 && value instanceof Shadow ? value: new Shadow();
	}
	toString() {
		return this._type == TextValueType.INHERIT ? 'inherit' : this._value.toString();
	}
}
(TextShadow.prototype as any)._value = null;

export class TextLineHeight extends TextValue {
	private _height: number;
	get isAuto() { return this._height <= 0; }
	get height() { return this._height; }
	set height(value) {
		this._height = check_unsigned_number_ret(value);
	}
	constructor(type?: TextValueType, height?: number) {
		super(type);
		if (arguments.length > 1) {
			if (check_unsigned_number(height)) this._height = height as number;
		}
	}
	toString() {
		if (this._type == TextValueType.INHERIT) {
			return 'inherit';
		} else if (this._height <= 0) {
			return 'auto';
		} else {
			return this._height.toString();
		}
	}
}
(TextLineHeight as any).prototype._height = 0;

export class TextDecoration extends TextValue {
	private _value: TextDecorationEnum;
	get value() { return this._value; }
	set value(val) { this._value = check_enum_ret(TextDecoration, val); }
	constructor(type?: TextValueType, value?: TextDecorationEnum) {
		super(type);
		if (arguments.length > 1) {
			if (check_enum(TextDecorationEnum, value)) this._value = value as TextDecorationEnum;
		}
	}
	toString() {
		return this._type == TextValueType.INHERIT ? 'inherit' : TextDecorationEnum[this._value].toLowerCase();
	}
}
(TextDecoration as any).prototype._value = TextDecorationEnum.NONE;

export class TextOverflow extends TextValue {
	private _value: TextOverflowEnum;
	get value() { return this._value; }
	set value(val) { this._value = check_enum_ret(TextDecoration, val); }
	constructor(type?: TextValueType, value?: TextOverflowEnum) {
		super(type);
		if (arguments.length > 1) {
			if (check_enum(TextOverflowEnum, value)) this._value = value as TextOverflowEnum;
		}
	}
	toString() {
		return this._type == TextValueType.INHERIT ? 'inherit' : TextOverflowEnum[this._value].toLowerCase();
	}
}
(TextOverflow as any).prototype._value = TextOverflowEnum.NORMAL;

export class TextWhiteSpace extends TextValue {
	private _value: TextWhiteSpaceEnum;
	get value() { return this._value; }
	set value(val) { this._value = check_enum_ret(TextDecoration, val); }
	constructor(type?: TextValueType, value?: TextWhiteSpaceEnum) {
		super(type);
		if (arguments.length > 1) {
			if (check_enum(TextWhiteSpaceEnum, value)) this._value = value as TextWhiteSpaceEnum;
		}
	}
	toString() {
		return this._type == TextValueType.INHERIT ? 'inherit' : TextWhiteSpaceEnum[this._value].toLowerCase();
	}
}
(TextWhiteSpace as any).prototype._value = TextWhiteSpaceEnum.NORMAL;

// ----------------------------

function _text_align(value: any) {
	return value as TextAlign;
}
function _align(value: any) {
	return value as Align;
}
function _content_align(value: any) {
	return value as ContentAlign;
}
function _repeat(value: any) {
	return value as Repeat;
}
function _direction(value: any) {
	return value as Direction;
}
function _keyboard_type(value: any) {
	return value as KeyboardType;
}
function _keyboard_return_type(value: any) {
	return value as KeyboardReturnType;
}
function _border(width: number, r: number, g: number, b: number, a: number) {
	return {
		__proto__: Border.prototype,
		_width: width,
		_color: _color(r, g, b, a),
	} as unknown as Border;
}
function _shadow(offset_x: number, offset_y: number, size: number, r: number, g: number, b: number, a: number) {
	return {
		__proto__: Shadow.prototype,
		_offset_x: offset_x,
		_offset_y: offset_y,
		_size: size,
		_color: _color(r, g, b, a),
	} as unknown as Shadow;
}
function _color(r: number, g: number, b: number, a: number) {
	return {
		__proto__: Color.prototype,
		_r: r,
		_g: g,
		_b: b,
		_a: a,
	} as unknown as Color;
}
function _vec2(x: number, y: number) {
	return {
		__proto__: Vec2.prototype,
		_x: x,
		_y: y,
	} as unknown as Vec2;
}
function _vec3(x: number, y: number, z: number) {
	return {
		__proto__: Vec3.prototype,
		_x: x,
		_y: y,
		_z: z,
	} as unknown as Vec3;
}
function _vec4(x: number, y: number, z: number, w: number) {
	return {
		__proto__: Vec4.prototype,
		_x: x,
		_y: y,
		_z: z,
		_w: w,
	} as unknown as Vec4;
}
function _curve(p1_x: number, p1_y: number, p2_x: number, p2_y: number) {
	return {
		__proto__: Curve.prototype,
		_p1_x: p1_x,
		_p1_y: p1_y,
		_p2_x: p2_x,
		_p2_y: p2_y,
	} as unknown as Curve;
}
function _rect(x: number, y: number, width: number, height: number) {
	return {
		__proto__: Rect.prototype,
		_x: x,
		_y: y,
		_width: width,
		_height: height,
	} as unknown as Rect;
}
function _mat(...value: any[]) {
	return {
		__proto__: Mat.prototype,
		_value: value,
	} as unknown as Mat;
}
function _mat4(...value: any[]) {
	return {
		__proto__: Mat4.prototype,
		_value: value,
	} as unknown as Mat4;
}
function _value(type: ValueType, value: number) {
	return {
		__proto__: Value.prototype,
		_type: type,
		_value: value,
	} as unknown as Value;
}
function _background_position(type: BackgroundPositionType, value: number) { 
	return {
		__proto__: BackgroundPosition.prototype,
		_type: type,
		_value: value,
	} as unknown as BackgroundPosition;
}
function _background_size(type: BackgroundSizeType, value: number) { 
	return {
		__proto__: BackgroundSize.prototype,
		_type: type,
		_value: value,
	} as unknown as BackgroundSize;
}
function _background_position_collection(type: BackgroundPositionType, value: number, type_y: BackgroundPositionType, value_y: number) { 
	return {
		__proto__: BackgroundPositionCollection.prototype,
		x: _background_position(type, value),
		y: _background_position(type_y, value_y),
	} as BackgroundPositionCollection;
}
function _background_size_collection(type: BackgroundSizeType, value: number, type_y: BackgroundSizeType, value_y: number) { 
	return {
		__proto__: BackgroundSizeCollection.prototype,
		x: _background_size(type, value),
		y: _background_size(type_y, value_y),
	} as BackgroundSizeCollection;
}
function _text_color(type: TextValueType, r: number, g: number, b: number, a: number) {
	return {
		__proto__: TextColor.prototype,
		_type: type,
		_value: _color(r, g, b, a),
	} as unknown as TextColor;
}
function _text_size(type: TextValueType, value: number) {
	return {
		__proto__: TextSize.prototype,
		_type: type,
		_value: value,
	} as unknown as TextSize;
}
function _text_family(type: TextValueType, value: string) {
	return {
		__proto__: TextFamily.prototype,
		_type: type,
		_value: value,
	} as unknown as TextFamily;
}
function _text_style(type: TextValueType, value: TextStyleEnum) {
	return {
		__proto__: TextStyle.prototype,
		_type: type,
		_value: value,
	} as unknown as TextStyle;
}
function _text_shadow(type: TextValueType, offset_x: number, offset_y: number, size: number, r: number, g: number, b: number, a: number) {
	return {
		__proto__: TextShadow.prototype,
		_type: type,
		_value: _shadow(offset_x, offset_y, size, r, g, b, a),
	} as unknown as TextShadow;
}
function _text_line_height(type: TextValueType, height: number) {
	return {
		__proto__: TextLineHeight.prototype,
		_type: type,
		_height: height,
	} as unknown as TextLineHeight;
}
function _text_decoration(type: TextValueType, value: TextDecorationEnum) {
	return {
		__proto__: TextDecoration.prototype,
		_type: type,
		_value: value,
	} as unknown as TextDecoration;
}
function _text_overflow(type: TextValueType, value: TextOverflowEnum) {
	return {
		__proto__: TextOverflow.prototype,
		_type: type,
		_value: value,
	} as unknown as TextOverflow;
}
function _text_white_space(type: TextValueType, value: TextWhiteSpaceEnum) {
	return {
		__proto__: TextWhiteSpace.prototype,
		_type: type,
		_value: value,
	} as unknown as TextWhiteSpace;
}

// parse

export type TextAlignIn = string | TextAlign | number;
export type AlignIn = string | Align | number;
export type ContentAlignIn = string | ContentAlign | number;
export type RepeatIn = string | Repeat | number;
export type DirectionIn = string | Direction | number;
export type KeyboardTypeIn = string | KeyboardType | number;
export type KeyboardReturnTypeIn = string | KeyboardReturnType | number;
export type BorderIn = string | Border;
export type ShadowIn = string | Shadow;
export type ColorIn = string | Color | number;
export type Vec2In = string | Vec2 | number | [number, number?];
export type Vec3In = string | Vec3 | number | [number, number?, number?];
export type Vec4In = string | Vec4 | number | [number, number?, number?, number?];
export type CurveIn = 'linear' | 'ease' | 'ease_in' | 'ease_out' | 'ease_in_out' | string | Curve;
export type RectIn = string | Rect | number;
export type MatIn = string | Mat | number;
export type Mat4In = string | Mat4 | number;
export type ValueIn = 'auto' | 'full' | string | Value | number;
export type BackgroundPositionIn = 'left' | 'right' | 'center' | 'top' | 'bottom' | string | BackgroundPosition | number;
export type BackgroundSizeIn = 'auto' | string | BackgroundSize | number;
export type BackgroundIn = string | Background | BackgroundOptions;
export type ValuesIn = string | Value[] | Value | number;
export type AlignsIn = string | Align[] | Align | number;
export type FloatsIn = string | number[] | number;
export type BackgroundPositionCollectionIn = string | BackgroundPositionCollection | BackgroundPosition;
export type BackgroundSizeCollectionIn = string | BackgroundSizeCollection | BackgroundSize;
export type TextColorIn = string | TextColor | Color;
export type TextSizeIn = string | TextSize | number;
export type TextFamilyIn = string | TextFamily;
export type TextStyleIn = string | TextStyle | TextStyleEnum | number;
export type TextShadowIn = string | TextShadow | Shadow;
export type TextLineHeightIn = string | TextLineHeight | number;
export type TextDecorationIn = string | TextDecoration | TextDecorationEnum | number;
export type TextOverflowIn = string | TextOverflow | TextOverflowEnum | number;
export type TextWhiteSpaceIn = string | TextWhiteSpace | TextWhiteSpaceEnum | number;

function error(value: any, desc?: string, reference?: any[], enum_value?: any){
	var err: string;
	var msg = String(desc || '%s').replace(/\%s/g, '`' + value + '`');
	var help = get_help(reference, enum_value);
	if (help) {
		err = `Bad argument. \`${msg}\`. Examples: ${help}`;
	} else {
		err = `Bad argument. \`${msg}\`.`;
	}
	return new Error(err);
}

export function parseTextAlign(str: TextAlignIn, desc?: string) { 
	if (typeof str == 'string') {
		var value = enum_object[str];
		if (check_enum(TextAlign, value)) {
			return value as TextAlign;
		}
	} else if (check_enum(TextAlign, str)) {
		return str as TextAlign;
	}
	throw error(str, desc, undefined, TextAlign);
}

export function parseAlign(str: AlignIn, desc?: string) { 
	if (typeof str == 'string') {
		var value = enum_object[str];
		if (check_enum(Align, value)) {
			return value as Align;
		}
	} else if (check_enum(Align, str)) {
		return str as Align;
	}
	throw error(str, desc, undefined, Align);
}

export function parseContentAlign(str: ContentAlignIn, desc?: string) {
	if (typeof str == 'string') {
		var value = enum_object[str];
		if (check_enum(ContentAlign, value)) {
			return value as ContentAlign;
		}
	} else if (check_enum(ContentAlign, str)) {
		return str as ContentAlign;
	}
	throw error(str, desc, undefined, ContentAlign);
}

export function parseRepeat(str: RepeatIn, desc?: string) {
	if (typeof str == 'string') {
		var value = enum_object[str];
		if (check_enum(Repeat, value)) {
			return value as Repeat;
		}
	} else if (check_enum(Repeat, str)) {
		return str as Repeat;
	}
	throw error(str, desc, undefined, Repeat);
}

export function parseDirection(str: DirectionIn, desc?: string) {
	if (typeof str == 'string') {
		var value = enum_object[str];
		if (check_enum(Direction, value)) {
			return value as Direction;
		}
	} else if (check_enum(Direction, str)) {
		return str as Direction;
	}
	throw error(str, desc, undefined, Direction);
}

export function parseKeyboardType(str: KeyboardTypeIn, desc?: string) {
	if (typeof str == 'string') {
		var value = enum_object[str];
		if (check_enum(KeyboardType, value)) {
			return value as KeyboardType;
		}
	} else if (check_enum(KeyboardType, str)) {
		return str as KeyboardType;
	}
	throw error(str, desc, undefined, KeyboardType);
}

export function parseKeyboardReturnType(str: KeyboardReturnTypeIn, desc?: string) {
	if (typeof str == 'string') {
		var value = enum_object[str];
		if (check_enum(KeyboardReturnType, value)) {
			return value as KeyboardReturnType;
		}
	} else if (check_enum(KeyboardReturnType, str)) {
		return str as KeyboardReturnType;
	}
	throw error(str, desc, undefined, KeyboardReturnType);
}

export function parseBorder(str: BorderIn, desc?: string) { 
	if (typeof str == 'string') {
		// 10 #ff00ff
		var m = str.match(/^ *((?:\d+)?\.?\d+)/);
		if (m) {
			var rev = new Border();
			(rev as any)._width = parseFloat(m[1]);
			var color = parseColor(str.substr(m[0].length + 1));
			if (color) {
				(rev as any)._color = color;
			}
			return rev;
		}
	} else if (str instanceof Border) {
		return str;
	}
	throw error(str, desc, ['10 #ff00aa', '10 rgba(255,255,0,255)']);
}

export function parseShadow(str: ShadowIn, desc?: string) { 
	if (typeof str == 'string') {
		// 10 10 2 #ff00aa
		var m = str.match(/^ *(-?(?:\d+)?\.?\d+) +(-?(?:\d+)?\.?\d+) +((?:\d+)?\.?\d+)/);
		if (m) {
			var rev = new Shadow();
			(rev as any)._offset_x = parseFloat(m[1]);
			(rev as any)._offset_y = parseFloat(m[2]);
			(rev as any)._size = parseFloat(m[3]);
			var color = parseColor(str.substr(m[0].length + 1));
			if (color) {
				(rev as any)._color = color;
			}
			return rev;
		}
	} else if (str instanceof Shadow) {
		return str;
	}
	throw error(str, desc, ['10 10 2 #ff00aa', '10 10 2 rgba(255,255,0,255)']);
}

export function parseColor(str: ColorIn, desc?: string) {
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
	} else if (str instanceof Color) {
		return str;
	} else if (typeof str == 'number') {
		_color(str >> 24 & 255, // r
					str >> 16 & 255, // g
					str >> 8 & 255, // b
					str >> 0 & 255); // a
	}
	throw error(str, desc, ['rgba(255,255,255,255)', 'rgb(255,255,255)', '#ff0', '#ff00', '#ff00ff', '#ff00ffff']);
}

export function parseVec2(str: Vec2In, desc?: string) {
	if (typeof str == 'string') {
		var m = str.match(/^ *(-?(?:\d+)?\.?\d+) +(-?(?:\d+)?\.?\d+) *$/) ||
						str.match(/^ *vec2\( *(-?(?:\d+)?\.?\d+) *, *(-?(?:\d+)?\.?\d+) *\) *$/);
		if (m) {
			return _vec2(parseFloat(m[1]), parseFloat(m[2]));
		}
	} else if (str instanceof Vec2) {
		return str;
	} else if (typeof str == 'number') {
		return _vec2(str, str);
	} else if (Array.isArray(str)) {
		var x = Number(str[0]) || 0;
		var y = Number(str[1]) || x;
		return _vec2(x, y);
	}
	throw error(str, desc, ['vec2(1,1)', '1 1']);
}

export function parseVec3(str: Vec3In, desc?: string) {
	if (typeof str == 'string') {
		var m = str.match(/^ *(-?(?:\d+)?\.?\d+) +(-?(?:\d+)?\.?\d+) +(-?(?:\d+)?\.?\d+) *$/) ||
						str.match(/^ *vec3\( *(-?(?:\d+)?\.?\d+) *, *(-?(?:\d+)?\.?\d+) *, *(-?(?:\d+)?\.?\d+) *\) *$/);
		if (m) {
			return _vec3(parseFloat(m[1]), parseFloat(m[2]), parseFloat(m[3]));
		}
	} else if (str instanceof Vec3) {
		return str;
	} else if (typeof str == 'number') {
		return _vec3(str, str, str);
	} else if (Array.isArray(str)) {
		var x = Number(str[0]) || 0;
		var y = Number(str[1]) || x;
		var z = Number(str[2]) || y;
		return _vec3(x, y, z);
	}
	throw error(str, desc, ['vec3(0,0,1)', '0 0 1']);
}

export function parseVec4(str: Vec4In, desc?: string) {
	if (typeof str == 'string') {
		var m = str.match(/^ *(-?(?:\d+)?\.?\d+) +(-?(?:\d+)?\.?\d+) +(-?(?:\d+)?\.?\d+) +(-?(?:\d+)?\.?\d+) *$/) ||
		str.match(/^ *vec4\( *(-?(?:\d+)?\.?\d+) *, *(-?(?:\d+)?\.?\d+) *, *(-?(?:\d+)?\.?\d+) *, *(-?(?:\d+)?\.?\d+) *\) *$/);
		if (m) {
			return _vec4(parseFloat(m[1]), parseFloat(m[2]), parseFloat(m[3]), parseFloat(m[4]));
		}
	} else if (str instanceof Vec4) {
		return str;
	} else if (typeof str == 'number') {
		return _vec4(str, str, str, str);
	} else if (Array.isArray(str)) {
		var x = Number(str[0]) || 0;
		var y = Number(str[1]) || x;
		var z = Number(str[2]) || y;
		var w = Number(str[3]) || z;
		return _vec4(x, y, z, w);
	}
	throw error(str, desc, ['vec4(0,0,1,1)', '0 0 1 1']);
}

export function parseCurve(str: CurveIn, desc?: string) {
	if (typeof str == 'string') {
		var s = ({
			linear: [0, 0, 1, 1],
			ease: [0.25, 0.1, 0.25, 1],
			ease_in: [0.42, 0, 1, 1],
			ease_out: [0, 0, 0.58, 1],
			ease_in_out: [0.42, 0, 0.58, 1],
		} as Dict<number[]>)[str];
		if (s) {
			return _curve(s[0], s[1], s[2], s[3]);
		}
		var m = str.match(/^ *(-?(?:\d+)?\.?\d+) +(-?(?:\d+)?\.?\d+) +(-?(?:\d+)?\.?\d+) +(-?(?:\d+)?\.?\d+) *$/) ||
		str.match(/^ *curve\( *(-?(?:\d+)?\.?\d+) *, *(-?(?:\d+)?\.?\d+) *, *(-?(?:\d+)?\.?\d+) *, *(-?(?:\d+)?\.?\d+) *\) *$/);
		if (m) {
			return _curve(parseFloat(m[1]), parseFloat(m[2]), parseFloat(m[3]), parseFloat(m[4]));
		}
	} else if (str instanceof Curve) {
		return str;
	}
	throw error(str, desc, ['curve(0,0,1,1)', '0 0 1 1', 'linear', 'ease', 'ease_in', 'ease_out', 'ease_in_out']);
}

export function parseRect(str: RectIn, desc?: string) {
	if (typeof str == 'string') {
		var m = str.match(/^ *(-?(?:\d+)?\.?\d+) +(-?(?:\d+)?\.?\d+) +(-?(?:\d+)?\.?\d+) +(-?(?:\d+)?\.?\d+) *$/) ||
						str.match(/^ *rect\( *(-?(?:\d+)?\.?\d+) *, *(-?(?:\d+)?\.?\d+) *, *(-?(?:\d+)?\.?\d+) *, *(-?(?:\d+)?\.?\d+) *\) *$/);
		if (m) {
			return _rect(parseFloat(m[1]), parseFloat(m[2]), parseFloat(m[3]), parseFloat(m[4]));
		}
	} else if (str instanceof Rect) {
		return str;
	} else if (typeof str == 'number') {
		return _rect(str, str, str, str);
	}
	throw error(str, desc, ['rect(0,0,-100,200)', '0 0 -100 200']);
}

const parse_mat_reg = new RegExp(`^ *mat\\( *${new Array(6).join('(-?(?:\\d+)?\\.?\\d+) *, *')}(-?(?:\\d+)?\\.?\\d+) *\\) *$`);
const parse_mat4_reg = new RegExp(`^ *mat4\\( *${new Array(16).join('(-?(?:\\d+)?\\.?\\d+) *, *')}(-?(?:\\d+)?\\.?\\d+) *\\) *$`);

export function parseMat(str: MatIn, desc?: string) {
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
	} else if (str instanceof Mat) {
		return str;
	} else if (typeof str == 'number') {
		return new Mat(str);
	}
	throw error(str, desc, ['mat(1,0,0,1,0,1)']);
}

export function parseMat4(str: Mat4In, desc?: string) {
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
	} else if (str instanceof Mat4) {
		return str;
	} else if (typeof str == 'number') {
		return new Mat4(str);
	}
	throw error(str, desc, ['mat4(1,0,0,1,0,1,0,1,0,0,1,1,0,0,0,1)']);
}

export function parseValue(str: ValueIn, desc?: string) {
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
	} else if (str instanceof Value) {
		return str;
	} else if (typeof str == 'number') {
		return _value(enum_object.pixel, str);
	}
	throw error(str, desc, ['auto', 'full', 10, '20%', '60!']);
}

enum background_position_type_2 {
	LEFT = Types.LEFT,
	RIGHT = Types.RIGHT,
	CENTER = Types.CENTER,
	TOP = Types.TOP,
	BOTTOM = Types.BOTTOM,
}

export function parseBackgroundPosition(str: BackgroundPositionIn, desc?: string) {
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
	} else if (str instanceof BackgroundPosition) {
		return str;
	} else if (typeof str == 'number') {
		return _background_position(enum_object.pixel, str);
	}
	throw error(str, desc, ['left', 'right', 'center', 'top', 'bottom', 10, '20%']);
}

export function parseBackgroundSize(str: BackgroundSizeIn, desc?: string) { 
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
	} else if (str instanceof BackgroundSize) {
		return str;
	} else if (typeof str == 'number') {
		return _background_size(enum_object.pixel, str);
	}
	throw error(str, desc, ['auto', 10, '20%']);
}

enum BGToken {
	EOS, // 
	LPAREN, // (
	RPAREN, // )
	COMMA, // ,
	WHITESPACE, // \s\r\t\n
	OTHER, // 
}

interface BGScanner {
	code: string;
	value: string;
	index: number;
}

var background_help = `url(res/image.png) repeat(none,[repeat]) position(left,[20%]) size(auto,[10.1])`;

function scanner_background_token(scanner: BGScanner): BGToken {
	var token = BGToken.OTHER;
	var value = '';
	var code = scanner.code;
	var index = scanner.index;
	do {
		if (index >= code.length) {
			token = BGToken.EOS;
		} else {
			switch (code.charCodeAt(index)) {
				case 9: // \t
				case 10: // \n
				case 13: // \r
				case 32: // \s
					if (value)
						token = BGToken.WHITESPACE;
					break;
				case 40: // (
					token = BGToken.LPAREN; break;
				case 41: // )
					token = BGToken.RPAREN; break;
				case 44: // ,
					token = BGToken.COMMA; break;
				default:
					value += code.charAt(index); break;
			}
		}
		index++;
	} while (token == BGToken.OTHER);

	if (value) {
		index--;
		token = BGToken.OTHER;
	}

	scanner.index = index;
	scanner.value = value;
	return token;
}

function parse_background_paren(scanner: BGScanner, desc?: string): string[] {
	if (scanner_background_token(scanner) == BGToken.LPAREN) {
		var exp: any[] = [];
		var token: BGToken;
		while ((token = scanner_background_token(scanner)) != BGToken.EOS) {
			switch (token) {
				case BGToken.LPAREN:
					scanner.index--;
					exp.push( parse_background_paren(scanner, desc) );
					break;
				case BGToken.RPAREN: // )
					if (exp.length === 0)
						throw error(scanner.code, desc, [background_help]);
					return exp;
				case BGToken.OTHER:
					exp.push( scanner.value );
					if (scanner_background_token(scanner) != BGToken.COMMA) { // ,
						throw error(scanner.code, desc, [background_help]);
					}
					break;
			}
		}
	}
	throw error(scanner.code, desc, [background_help]);
}

export function parseBackground(str: BackgroundIn, desc?: string): Background {
	// url(res/image.png) repeat(none,[repeat]) position(left,[20%]) size(auto,[10.1])
	if (str instanceof Background) {
		return str;
	}
	var r: Background | null = null;
	if (typeof str == 'string') {
		var bgi: BackgroundImage | null = null;
		var prev: Background | null = null;
		var token: BGToken;
		var scanner: BGScanner = { code: str, value: '', index: 0 };

		while ( (token = scanner_background_token(scanner)) != BGToken.EOS ) {
			switch (token) {
				case BGToken.COMMA: {// ,
					bgi = null;
					break;
				}
				case BGToken.OTHER: {
					if (scanner.value == 'url' || scanner.value == 'image') {
						if (bgi)
							throw error(str, desc, [background_help]);
						bgi = new BackgroundImage();
						bgi.src = parse_background_paren(scanner)[0] as string;
						if (!r) r = bgi;
						if (prev) prev.next = bgi;
						prev = bgi;
					}
					// else if (scanner.value == 'gradient') { }
					else {
						if (!bgi) throw error(str, desc, [background_help]);

						switch (scanner.value) {
							case 'repeat': {
								if (scanner_background_token(scanner) == BGToken.LPAREN) { // repeat(none)
									scanner.index--;
									bgi.repeat = parse_background_paren(scanner).map(e=>parseRepeat(e, desc))[0];
								} else { // repeat
									bgi.repeat = Repeat.REPEAT;
								}
								break;
							}
							case 'position': { // position(10,top)
								var poss = parse_background_paren(scanner).map(e=>parseBackgroundPosition(e, desc));
								bgi.position = { x: poss[0], y: poss[1] || poss[0] };
								break;
							}
							case 'size': { // size(10,20%)
								var sizes = parse_background_paren(scanner).map(e=>parseBackgroundSize(e, desc));
								bgi.size = { x: sizes[0], y: sizes[1] || sizes[0] };
								break;
							}
							case 'positionX': { // positionX(10)
								bgi.positionX = parse_background_paren(scanner).map(e=>parseBackgroundPosition(e, desc))[0];
								break;
							}
							case 'positionY': { // positionY(10)
								bgi.positionY = parse_background_paren(scanner).map(e=>parseBackgroundPosition(e, desc))[0];
								break;
							}
							case 'sizeX': { // sizeX(10)
								bgi.sizeX = parse_background_paren(scanner).map(e=>parseBackgroundSize(e, desc))[0];
								break;
							}
							case 'sizeY': { // sizeY(10)
								bgi.sizeY = parse_background_paren(scanner).map(e=>parseBackgroundSize(e, desc))[0];
								break;
							}
							default:
								throw error(str, desc, [background_help]);
						}
					}
					break;
				}
				default:
					throw error(str, desc, [background_help]);
			}
		}
	} else {
		if (str.image) {
			return Object.assign(new BackgroundImage(), str.image);
		}
	}
	if (r)
		return r;
	throw error(str, desc, [background_help]);
}

export function parseValues(str: ValuesIn, desc?: string) {
	if (typeof str == 'string') {
		var rev: Value[] = [];
		for (var i of str.split(/\s+/)) {
			rev.push(parseValue(i, desc));
		}
		return rev;
	} else if (str instanceof Value) {
		return [str];
	} else if (Array.isArray(str)) {
		return str.map(e=>parseValue(e, desc));
	} else {
		return [parseValue(str)];
	}
	throw error(str, desc, ['auto', 'full', 10, '20%', '60!']);
}

export function parseAligns(str: AlignsIn, desc?: string) {
	if (typeof str == 'string') {
		var rev: Align[] = [];
		for (var i of str.split(/\s+/)) {
			rev.push(parseAlign(i));
		}
		return rev;
	} else if (Array.isArray(str)) {
		return str.map(e=>parseAlign(e));
	} else if (check_enum(Align, str)) {
		return [str as Align];
	}
	throw error(str, desc, ['left', 'top center']);
}

export function parseFloats(str: FloatsIn, desc?: string) {
	if (typeof str == 'string') {
		var ls = str.split(/\s+/);
		var rev: number[] = [];
		for (var i of str.split(/\s+/)) {
			var mat = i.match(/^(-?(?:\d+)?\.?\d+)$/);
			if (!mat) {
				throw error(str, desc, [10, '10 20 30 40']);
			}
			rev.push(parseFloat(mat[1]));
		}
		return rev;
	} else if (Array.isArray(str)) {
		return str.map(e=>Number(e) || 0);
	} else if (isFinite(str)) {
		return [str];
	}
	throw error(str, desc, [10, '10 20 30 40']);
}

export function parseBackgroundPositionCollection(str: BackgroundPositionCollectionIn, desc?: string) { 
	if (typeof str == 'string') {
		var items: BackgroundPosition[] = [];
		for (var j of str.split(/\s+/))
			items.push(parseBackgroundPosition(j, desc));
		var x = items[0] || new BackgroundPosition();
		var y = items[1] || x;
		return { __proto__: BackgroundPositionCollection.prototype, x, y } as BackgroundPositionCollection;
	} else if (str instanceof BackgroundPositionCollection) {
		return str;
	} else if (str instanceof BackgroundPosition) {
		return { __proto__: BackgroundPositionCollection.prototype, x: str, y: str } as BackgroundPositionCollection;
	}
	throw error(str, desc, [10, '20%', 'left', 'right', 'center', 'top', 'bottom']);
}

export function parseBackgroundSizeCollection(str: BackgroundSizeCollectionIn, desc?: string) {
	if (typeof str == 'string') {
		var items: BackgroundSize[] = [];
		for (var j of str.split(/\s+/))
			items.push(parseBackgroundSize(j, desc));
		var x = items[0] || new BackgroundSize();
		var y = items[1] || x;
		return { __proto__: BackgroundSizeCollection.prototype, x, y } as BackgroundSizeCollection;
	} else if (str instanceof BackgroundSizeCollection) {
		return str;
	} else if (str instanceof BackgroundSize) {
		return { __proto__: BackgroundSizeCollection.prototype, x: str, y: str } as BackgroundSizeCollection;
	}
	throw error(str, desc, ['auto', 10, '50%']);
}

export function parseTextColor(str: TextColorIn, desc?: string) {
	if (typeof str == 'string') {
		if (str == 'inherit') {
			return new TextColor();
		} else {
			var value = parseColor(str);
			if (value) {
				return new TextColor(TextValueType.VALUE, value);
			}
		}
	} else if (str instanceof TextColor) {
		return str;
	} else if (str instanceof Color) {
		return new TextColor(TextValueType.VALUE, str);
	}
	throw error(str, desc, ['inherit', 'rgba(255,255,255,255)', 'rgb(255,255,255)', '#ff0', '#ff00', '#ff00ff', '#ff00ffff']);
}

export function parseTextSize(str: TextSizeIn, desc?: string) {
	if (typeof str == 'string') {
		if (str == 'inherit') {
			return new TextSize();
		} else {
			if (/^(?:\d+)?\.?\d+$/.test(str)) {
				return _text_size(TextValueType.VALUE, parseFloat(str));
			}
		}
	} else if (str instanceof TextSize) {
		return str;
	} else if (typeof str == 'number') {
		return _text_size(TextValueType.VALUE, str);
	}
	throw error(str, desc, ['inherit', 12]);
}

export function parseTextFamily(str: TextFamilyIn, desc?: string) {
	if (typeof str == 'string') {
		if (str == 'inherit') {
			return new TextFamily();
		} else {
			return _text_family(TextValueType.VALUE, str);
		}
	} else if (str instanceof TextFamily) {
		return str;
	}
	throw error(str, desc, ['inherit', 'Ubuntu Mono']);
}

export function parseTextStyle(str: TextStyleIn, desc?: string) {
	if (typeof str == 'string') {
		if (str == 'inherit') {
			return new TextStyle();
		} else {
			var value = enum_object[str];
			if (check_enum(TextStyleEnum, value)) {
				return _text_style(TextValueType.VALUE, value);
			}
		}
	} else if (str instanceof TextStyle) {
		return str;
	} else if (check_enum(TextStyleEnum, str)) {
		return _text_style(TextValueType.VALUE, str as TextStyleEnum);
	}
	throw error(str, desc, ['inherit'], TextStyleEnum);
}

export function parseTextShadow(str: TextShadowIn, desc?: string) {
	if (typeof str == 'string') {
		if (str == 'inherit') {
			return new TextShadow();
		} else {
			var value = parseShadow(str);
			if (value) {
				return new TextShadow(TextValueType.VALUE, value);
			}
		}
	} else if (str instanceof TextShadow) {
		return str;
	} else if (str instanceof Shadow) {
		return new TextShadow(TextValueType.VALUE, str);
	}
	throw error(str, desc, ['inherit', '10 10 2 #ff00aa', '10 10 2 rgba(255,255,0,255)']);
}

export function parseTextLineHeight(str: TextLineHeightIn, desc?: string) {
	if (typeof str == 'string') {
		if (str == 'inherit') {
			return new TextLineHeight();
		} else if (str == 'auto') {
			return _text_line_height(TextValueType.VALUE, 0);
		} else {
			if (/^(?:\d+)?\.?\d+$/.test(str)) {
				return _text_line_height(TextValueType.VALUE, parseFloat(str));
			}
		}
	} else if (str instanceof TextLineHeight) {
		return str;
	} else if (typeof str == 'number') {
		return _text_line_height(TextValueType.VALUE, str);
	}
	throw error(str, desc, ['inherit', 24]);
}

export function parseTextDecoration(str: TextDecorationIn, desc?: string) {
	if (typeof str == 'string') {
		if (str == 'inherit') {
			return new TextDecoration();
		} else {
			var value = enum_object[str];
			if (check_enum(TextDecorationEnum, value)) {
				return _text_decoration(TextValueType.VALUE, value);
			}
		}
	} else if (str instanceof TextDecoration) {
		return str;
	} else if (check_enum(TextDecorationEnum, str)) {
		return _text_decoration(TextValueType.VALUE, str as TextDecorationEnum);
	}
	throw error(str, desc, ['inherit'], TextDecorationEnum);
}

export function parseTextOverflow(str: TextOverflowIn, desc?: string) {
	if (typeof str == 'string') {
		if (str == 'inherit') {
			return new TextOverflow();
		} else {
			var value = enum_object[str];
			if (check_enum(TextOverflowEnum, value)) {
				return _text_overflow(TextValueType.VALUE, value);
			}
		}
	} else if (str instanceof TextOverflow) {
		return str;
	} else if (check_enum(TextOverflowEnum, str)) {
		return _text_overflow(TextValueType.VALUE, str as TextOverflowEnum);
	}
	throw error(str, desc, ['inherit'], TextOverflowEnum);
}

export function parseTextWhiteSpace(str: TextWhiteSpaceIn, desc?: string) {
	if (typeof str == 'string') {
		if (str == 'inherit') {
			return new TextWhiteSpace();
		} else {
			var value = enum_object[str];
			if (check_enum(TextWhiteSpaceEnum, value)) {
				return _text_white_space(TextValueType.VALUE, value);
			}
		}
	} else if (str instanceof TextWhiteSpace) {
		return str;
	} else if (check_enum(TextWhiteSpaceEnum, str)) {
		return _text_white_space(TextValueType.VALUE, str as TextWhiteSpaceEnum);
	}
	throw error(str, desc, ['inherit'], TextWhiteSpaceEnum);
}

// _priv
const _priv = exports._priv;
delete exports._priv;
Object.assign(_priv, exports);

_priv._Base = Base;
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
_priv._BackgroundPositionCollection = _background_position_collection;
_priv._BackgroundSizeCollection = _background_size_collection;
_priv._TextColor = _text_color;
_priv._TextSize = _text_size;
_priv._TextFamily = _text_family;
_priv._TextStyle = _text_style;
_priv._TextShadow = _text_shadow;
_priv._TextLine_height = _text_line_height;
_priv._TextDecoration = _text_decoration;
_priv._TextOverflow = _text_overflow;
_priv._TextWhiteSpace = _text_white_space;