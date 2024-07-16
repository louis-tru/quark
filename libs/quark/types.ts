/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright (c) 2015, blue.chu
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

const _font = __binding__('_font');

type Reference = (ref: any[])=>any[];

function getReference(ref: any[], plus?: Reference) {
	return plus ? plus(ref): ref;
}

function get_help(reference?: any[], ref?: Reference, enum_value?: any) {
	let message = '';
	if ( reference ) {
		for ( let val of getReference(reference, ref) ) {
			if ( val ) message += ', ' + JSON.stringify(val);
		}
	}
	if ( enum_value ) {
		for (let i of Object.values(enum_value)) {
			if (typeof i == 'string') {
				message += ', ' + i.toLowerCase();
			}
		}
	}
	return message;
}

function error(value: any, desc?: string, reference?: any[], ref?: Reference, enum_value?: any) {
	let err: string;
	let msg = String(desc || '%s').replace(/\%s/g, '`' + value + '`');
	let help = get_help(reference, ref, enum_value);
	if (help) {
		err = `Bad argument. \`${msg}\`. Examples: ${help}`;
	} else {
		err = `Bad argument. \`${msg}\`.`;
	}
	return new Error(err);
}

export type N = number;

interface Constructor<T> {
	new(...args: any[]): T;
}

type RemoveField<Type, S> = {
	[Property in keyof Type as Exclude<Property, S>]: Type[Property]
};

type RemoveToStringField<Type> = RemoveField<Type, 'toString'>;

export function initDefaults<T>(cls: Constructor<T>, ext: RemoveToStringField<Partial<T>>) {
	Object.assign(cls.prototype, ext);
}

function newBase<T>(cls: Constructor<T>, ext: RemoveToStringField<Partial<T>>) {
	(ext as any).__proto__ = cls.prototype
	return ext as T;
}

function hexStr(num: N) {
	if (num < 16) {
		return '0' + num.toString(16);
	} else {
		return num.toString(16);
	}
}

function toCapitalize<T extends string>(str: T): Capitalize<T> {
	return str[0].toUpperCase() + str.substring(1) as Capitalize<T>;
}

// -------------------------------------------------------------------------------------

export enum Repeat {
	Repeat,
	RepeatX,
	RepeatY,
	RepeatNo,
};

export enum FillPositionKind {
	Value,     /* 明确值  rem */
	Ratio,     /* 百分比  % */
	Start,     /* 开始 start */
	End,       /* 结束 end */
	Center,    /* 居中 center */
};

export enum FillSizeKind {
	Auto,      /* 自动值  auto */
	Value,     /* 明确值  rem */
	Ratio,     /* 百分比  % */
};
export type BoxOriginKind = FillSizeKind;
export const BoxOriginKind = FillSizeKind;

export enum Direction {
	Row,
	RowReverse,
	Column,
	ColumnReverse,
};

// 项目在主轴上的对齐方式
export enum ItemsAlign {
	Start, // 左对齐
	Center, // 居中
	End, // 右对齐
	SpaceBetween, // 两端对齐，项目之间的间隔都相等
	SpaceAround, // 每个项目两侧的间隔相等。所以，项目之间的间隔比项目与边框的间隔大一倍
	SpaceEvenly, // 每个项目两侧的间隔相等,这包括边框的间距
};

// 项目在交叉轴内如何对齐
export enum CrossAlign {
	Start, // 与交叉轴内的起点对齐
	Center, // 与交叉轴内的中点对齐
	End, // 与交叉轴内的终点对齐
};

// 主轴溢出包裹，开启后当主轴溢出时分裂成多根交叉轴
export enum Wrap {
	NoWrap, // 只有一根交叉轴线
	Wrap, // 溢出后会有多根交叉轴线
	WrapReverse, // 多根交叉轴线反向排列
};

// 多根交叉轴线的对齐方式。如果项目只有一根交叉轴，该属性不起作用
export enum WrapAlign {
	Start, // 与交叉轴的起点对齐
	Center, // 与交叉轴的中点对齐
	End, // 与交叉轴的终点对齐
	SpaceBetween, // 与交叉轴两端对齐,轴线之间的间隔平均分布
	SpaceAround, // 每根轴线两侧的间隔都相等,所以轴线之间的间隔比轴线与边框的间隔大一倍
	SpaceEvenly, // 每根轴线两侧的间隔都相等,这包括边框的间距
	Stretch, // 轴线占满整个交叉轴，平均分配剩余的交叉轴空间
};

// view align
export enum Align {
	Auto,
	Start,
	Center,
	End,
	Baseline = Auto, // box vertical align in text
	Top, // box vertical align in text
	Middle, // box vertical align in text
	Bottom, // box vertical align in text
	LeftTop = Start,
	CenterTop,
	RightTop,
	LeftMiddle,
	CenterMiddle,
	RightMiddle,
	LeftBottom,
	CenterBottom,
	RightBottom,
};

export enum BoxSizeKind {
	None,    /* Do not use value */
	Auto,    /* 包裹内容 wrap content */
	Match,   /* 匹配父视图 match parent */
	Value,     /* 明确值  rem */
	Ratio,   /* 百分比 value % */
	Minus,   /* 减法(parent-value) value ! */
};

export enum TextValueKind {
	Inherit, Default, Value,
};

export enum TextAlign {
	Inherit, // inherit
	Left,           /* 左对齐 */
	Center,         /* 居中 */
	Right,          /* 右对齐 */
	Default = Left,
};

export enum TextDecoration {
	Inherit, // inherit
	None,           /* 没有 */
	Overline,       /* 上划线 */
	LineThrough,   /* 中划线 */
	Underline,      /* 下划线 */
	Default = None,
};

export enum TextOverflow {
	Inherit, // inherit
	Normal,          /* 不做任何处理 */
	Clip,            /* 剪切 */
	Ellipsis,        /* 剪切并显示省略号 */
	EllipsisCenter, /* 剪切并居中显示省略号 */
	Default = Normal,
};

export enum TextWhiteSpace {
	Inherit,       // inherit
	Normal,       /* 合并空白序列,使用自动wrap */
	NoWrap,       /* 合并空白序列,不使用自动wrap */
	Pre,          /* 保留所有空白,不使用自动wrap */
	PreWrap,      /* 保留所有空白,使用自动wrap */
	PreLine,      /* 合并空白符序列,但保留换行符,使用自动wrap */
	Default = Normal,
};

export enum TextWordBreak {
	Inherit,  // inherit
	Normal,   /* 保持单词在同一行 */
	BreakWord,/* 保持单词在同一行,除非单词长度超过一行才截断 */
	BreakAll, /* 以字为单位行空间不足换行 */
	KeepAll,  /* 所有连续的字符都当成一个单词,除非出现空白符、换行符、标点符 */
	Default = Normal,
};

export enum TextWeight {
	Inherit      = 0,
	Thin         = 100,
	Ultralight   = 200,
	Light        = 300,
	Regular      = 400,
	Medium       = 500,
	Semibold     = 600,
	Bold         = 700,
	Heavy        = 800,
	Black        = 900,
	ExtraBlack   = 1000,
	Default      = Regular, // default
};

export enum TextWidth {
	Inherit          = 0, // inherit
	UltraCondensed   = 1,
	ExtraCondensed   = 2,
	Condensed        = 3,
	SemiCondensed    = 4,
	Normal           = 5,
	SemiExpanded     = 6,
	Expanded         = 7,
	ExtraExpanded    = 8,
	UltraExpanded    = 9,
	Default          = Normal,
};

export enum TextSlant {
	Inherit, // inherit
	Normal, // 正常
	Italic, // 斜体
	Oblique,  // 倾斜
	Default = Normal,
};

export type TextStyle = number;

export enum KeyboardType {
	Normal,
	Ascii,
	Number,
	Url,
	NumberPad,
	Phone,
	NamePhone,
	Email,
	Decimal,
	Search,
	AsciiNumber,
};

export enum KeyboardReturnType {
	Normal,
	Oo,
	Join,
	Next,
	Route,
	Search,
	Send,
	Done,
	Emergency,
	Continue,
};

export enum CursorStyle {
	Normal,
	None,
	NoneUntilMouseMoves,
	Arrow,
	Ibeam,
	PointingHand,
	ClosedHand,
	OpenHand,
	ResizeLeft,
	ResizeRight,
	ResizeLeftRight,
	ResizeUp,
	ResizeDown,
	ResizeUpDown,
	Crosshair,
	DisappearingItem,
	OperationNotAllowed,
	DragLink,
	DragCopy,
	ContextualMenu,
	IbeamForVertical,
};

export enum FindDirection {
	None,
	Left,
	Top,
	Right,
	Bottom,
};

// -------------------------------------------------------------------------------------

export type RepeatIn = Uncapitalize<keyof typeof Repeat> | Repeat;
export type DirectionIn = Uncapitalize<keyof typeof Direction> | Direction;
export type ItemsAlignIn = Uncapitalize<keyof typeof ItemsAlign> | ItemsAlign;
export type CrossAlignIn = Uncapitalize<keyof typeof CrossAlign> | CrossAlign;
export type WrapIn = Uncapitalize<keyof typeof Wrap> | Wrap;
export type WrapAlignIn = Uncapitalize<keyof typeof WrapAlign> | WrapAlign;
export type AlignIn = Uncapitalize<keyof typeof Align> | Align;
export type TextAlignIn = Uncapitalize<keyof typeof TextAlign> | TextAlign;
export type TextDecorationIn = Uncapitalize<keyof typeof TextDecoration> | TextDecoration;
export type TextOverflowIn = Uncapitalize<keyof typeof TextOverflow> | TextOverflow;
export type TextWhiteSpaceIn = Uncapitalize<keyof typeof TextWhiteSpace> | TextWhiteSpace;
export type TextWordBreakIn = Uncapitalize<keyof typeof TextWordBreak> | TextWordBreak;
export type TextWeightIn = Uncapitalize<keyof typeof TextWeight> | TextWeight;
export type TextWidthIn = Uncapitalize<keyof typeof TextWidth> | TextWidth;
export type TextSlantIn = Uncapitalize<keyof typeof TextSlant> | TextSlant;
export type KeyboardTypeIn = Uncapitalize<keyof typeof KeyboardType> | KeyboardType;
export type KeyboardReturnTypeIn = Uncapitalize<keyof typeof KeyboardReturnType> | KeyboardReturnType;
export type CursorStyleIn = Uncapitalize<keyof typeof CursorStyle> | CursorStyle;
export type FindDirectionIn = Uncapitalize<keyof typeof FindDirection> | FindDirection;

export class Base<T> {
	toString() { return '[types base]' }
	toStringStyled(indent?: number) {
		return (indent ? new Array(indent+1).join(' '): '') + this.toString();
	}
	constructor(opts?: RemoveToStringField<Partial<T>>) {
		Object.assign(this, opts)
	}
}

export class Vec2 extends Base<Vec2> {
	x: N;
	y: N;
	toString(): `vec2(${N},${N})` { return `vec2(${this.x},${this.y})` }
}
initDefaults(Vec2, { x: 0, y: 0 });
export type Vec2In = `${number} ${number}` | `vec2(${N},${N})` | N | [N,N] | Vec2;

export class Vec3 extends Base<Vec3> {
	x: N;
	y: N;
	z: N;
	toString(): `vec3(${N},${N},${N})` { return `vec3(${this.x},${this.y},${this.z})` }
}
initDefaults(Vec3, { x: 0, y: 0, z: 0 });
export type Vec3In = `${number} ${number} ${number}` | `vec3(${N},${N},${N})` | N | [N,N,N] | Vec3;

export class Vec4 extends Base<Vec4> {
	x: N;
	y: N;
	z: N;
	w: N;
	toString(): `vec4(${N},${N},${N},${N})` {
		return `vec4(${this.x},${this.y},${this.z},${this.w})`;
	}
}
initDefaults(Vec4, { x: 0, y: 0, z: 0, w: 0 });
export type Vec4In = `${number} ${number} ${number} ${number}` |
	`vec4(${N},${N},${N},${N})` | N | [N,N,N,N] | Vec4;

export class Curve extends Base<Curve> {
	p1: Vec2;
	p2: Vec2;
	get p1x() { return this.p1.x; }
	get p1y() { return this.p1.y; }
	get p2x() { return this.p2.x; }
	get p2y() { return this.p2.y; }
	toString(): `curve(${N},${N},${N},${N})` {
		return `curve(${this.p1.x},${this.p1.y},${this.p2.x},${this.p2.y})`;
	}
}
initDefaults(Curve, { p1: new Vec2({x:0,y:0}), p2: new Vec2({x:1,y:1}) });
export type CurveIn = 'linear' | 'ease' | 'easeIn' | 'easeOut' | 'easeInOut' | 
	`curve(${N},${N},${N},${N})` | [number,number,number,number] | Curve;

export class Rect extends Base<Rect> {
	origin: Vec2;
	size: Vec2;
	get x() { return this.origin.x; }
	get y() { return this.origin.y; }
	get width() { return this.size.x; }
	get height() { return this.size.y; }
	toString(): `rect(${N},${N},${N},${N})` {
		return `rect(${this.x},${this.y},${this.width},${this.height})`;
	}
}
initDefaults(Rect, { origin: new Vec2, size: new Vec2 });
export type RectIn = `rect(${N},${N},${N},${N})` | `rect(${N},${N})` |
	[number,number,number,number] | Rect;

export class Mat extends Base<Mat> {
	m0: N;
	m1: N;
	m2: N;
	m3: N;
	m4: N;
	m5: N;
	get value() {
		return [this.m0,this.m1,this.m2,this.m3,this.m4,this.m5];
	}
	toString(): `mat(${N},${N},${N},${N},${N},${N})` {
		let _ = this;
		return `mat(${_.m0},${_.m1},${_.m2},${_.m3},${_.m4},${_.m5})`;
	}
}
initDefaults(Mat, { m0: 1, m1: 0, m2: 0, m3: 0, m4: 1, m5: 0 });
export type MatIn = N | Mat | ReturnType<typeof Mat.prototype.toString>;

export class Mat4 extends Base<Mat4> {
	m0: N;
	m1: N;
	m2: N;
	m3: N;
	m4: N;
	m5: N;
	m6: N;
	m7: N;
	m8: N;
	m9: N;
	m10: N;
	m11: N;
	m12: N;
	m13: N;
	m14: N;
	m15: N;
	get value() { return [
		this.m0,this.m1,this.m2,this.m3,this.m4,this.m5,this.m6,this.m7,
		this.m8,this.m9,this.m10,this.m11,this.m12,this.m13,this.m14,this.m15];
	}
	toString():
		`mat4(${N},${N},${N},${N},${N},${N},${N},${N},${N},${N},${N},${N},${N},${N},${N},${N})`
	{
		let _ = this;
		return `mat4(\
${_.m0},${_.m1},${_.m2},${_.m3},${_.m4},${_.m5},${_.m6},${_.m7},\
${_.m8},${_.m9},${_.m10},${_.m11},${_.m12},${_.m13},${_.m14},${_.m15})`;
	}
}
initDefaults(Mat4, {
	m0: 1, m1: 0, m2: 0, m3: 0,m4: 0, m5: 1, m6: 0, m7: 0,
	m8: 0, m9: 0, m10: 1, m11: 0,m12: 0, m13: 0, m14: 0, m15: 1
});
export type Mat4In = N | Mat4 | ReturnType<typeof Mat4.prototype.toString>;

export class Color extends Base<Color> {
	r: N;
	g: N;
	b: N;
	a: N;
	constructor(r: N, g: N, b: N, a: N) {
		super();
		this.r = r;
		this.g = g;
		this.b = b;
		this.a = a;
	}
	reverse() {
		return new Color(255 - this.r, 255 - this.g, 255 - this.b, this.a);
	}
	toRGBString(): `rgb(${N},${N},${N})` {
		return `rgb(${this.r},${this.g},${this.b})`;
	}
	toRGBAString(): `rgba(${N},${N},${N},${N})` {
		return `rgba(${this.r},${this.g},${this.b},${this.a})`;
	}
	toString(): `#${string}` {
		return `#${hexStr(this.r)}${hexStr(this.g)}${hexStr(this.b)}`;
	}
	toHex32String(): `#${string}` {
		return `#${hexStr(this.r)}${hexStr(this.g)}${hexStr(this.b)}${hexStr(this.a)}`;
	}
}
initDefaults(Color, { r: 0, g: 0, b: 0, a: 255 });
export type ColorStrIn = `#${string}` | `rgb(${N},${N},${N})` | `rgba(${N},${N},${N},${N})`;
export type ColorIn = ColorStrIn | N | Color;

export class Shadow extends Base<Shadow> {
	x: N;
	y: N;
	size: N;
	color: Color;
	get r() { return this.color.r; }
	get g() { return this.color.g; }
	get b() { return this.color.b; }
	get a() { return this.color.a; }
	toString(): `${N} ${N} ${N} #${string}` {
		return `${this.x} ${this.y} ${this.size} ${this.color.toString()}`
	}
}
initDefaults(Shadow, { x: 0, y: 0, size: 0, color: new Color(0,0,0,255) });
export type ShadowIn = `${N} ${N} ${N} ${ColorStrIn}` | Shadow;

export class BoxBorder extends Base<BoxBorder> {
	width: N;
	color: Color;
	get r() { return this.color.r; }
	get g() { return this.color.g; }
	get b() { return this.color.b; }
	get a() { return this.color.a; }
	toString(): `${N} #${string}` {
		return `${this.width} ${this.color.toString()}`
	}
}
initDefaults(BoxBorder, { width: 0, color: new Color(0,0,0,255) });
export type BoxBorderIn = `${N} ${ColorStrIn}` | BoxBorder;

export class FillPosition extends Base<FillPosition> {
	value: N;
	kind: FillPositionKind;
};
initDefaults(FillPosition, { value: 0, kind: FillPositionKind.Value });
type FillPositionKindStr = 
	Uncapitalize<keyof RemoveField<typeof FillPositionKind, 'Value'|'Ratio'|number>>;
export type FillPositionIn = N | FillPosition | `${number}%` | FillPositionKindStr;

export class FillSize extends Base<FillSize> {
	value: N;
	kind: FillSizeKind;
};
initDefaults(FillSize, { value: 0, kind: FillSizeKind.Auto });
type FillSizeKindStr = Uncapitalize<keyof RemoveField<typeof FillSizeKind, 'Value'|'Ratio'|number>>;
export type FillSizeIn = N | FillSize | `${number}%` | FillSizeKindStr;

export class BoxSize extends Base<BoxSize> {
	value: N;
	kind: BoxSizeKind;
};
initDefaults(BoxSize, { value: 0, kind: BoxSizeKind.Value });
type BoxSizeKindStr = 
	Uncapitalize<keyof RemoveField<typeof BoxSizeKind, 'Value'|'Ratio'|'Minus'|number>>
export type BoxSizeIn = N | BoxSize | `${number}%` | `${number}!` | BoxSizeKindStr;

export class BoxOrigin extends Base<BoxSize> {
	value: N;
	kind: BoxOriginKind;
};
type BoxOriginKindStr = FillSizeKindStr;
initDefaults(BoxOrigin, { value: 0, kind: FillSizeKind.Value });
export type BoxOriginIn = N | BoxOrigin | `${number}%` | BoxOriginKindStr;

const TextBase_toString = [
	()=>'inherit',
	()=>'default',
	(v: any)=>v+'',
	()=>'unknown',
];
class TextBase<Derived,Value> extends Base<Derived> {
	value: Value;
	kind: TextValueKind;
	toString() {
		return (TextBase_toString[this.kind] || TextBase_toString[3])(this.value);
	}
}
export class TextColor extends TextBase<TextColor,Color> {
	get r() { return this.value.r; }
	get g() { return this.value.g; }
	get b() { return this.value.b; }
	get a() { return this.value.a; }
}
initDefaults(TextColor, { value: new Color(0,0,0,255), kind: TextValueKind.Inherit });
type TextValueKindInStr = Uncapitalize<keyof RemoveField<typeof TextValueKind, 'Value'|number>>;
export type TextColorIn = TextValueKindInStr | ColorIn | TextColor;

export class TextSize extends TextBase<TextSize,N> {}
initDefaults(TextSize, { value: 0, kind: TextValueKind.Inherit });
export type TextSizeIn = TextValueKindInStr | N | TextSize;
export type TextLineHeight = TextSize;
export const TextLineHeight = TextSize;
export type TextLineHeightIn = TextSizeIn;

export class TextShadow extends TextBase<TextShadow,Shadow> {}
initDefaults(TextShadow, { value: new Shadow, kind: TextValueKind.Inherit });
export type TextShadowIn = TextValueKindInStr | ShadowIn | TextShadow;

export type FFID = Uint8Array;
export class TextFamily extends TextBase<TextFamily,FFID> {
	toString() {
		if (this.kind == TextValueKind.Value) {
			// let _ = this.value;
			// let high = _[0] << 24 | _[1] << 16 | _[2] << 8 | _[3];
			// let low = _[4] << 24 | _[5] << 16 | _[6] << 8 | _[7];
			// return `ffid(${high},${low})`;
			return this.familys();
		} else {
			return (TextBase_toString[this.kind] || TextBase_toString[3])(this.value);
		}
	}
	familys(): string {
		let isNullptr = this.value.every(e=>!e);
		return isNullptr ? '': _font.getFamilysName(this.value);
	}
}
const EmptyFFID = new Uint8Array([0,0,0,0,0,0,0,0]);
initDefaults(TextFamily, { value: EmptyFFID, kind: TextValueKind.Inherit });
export type TextFamilyIn = TextValueKindInStr | string | TextFamily;



export declare class BoxFilter {
	readonly type: N;
	next: BoxFilter | null;
}

/**
Examples:
```
src(res/image.png, auto 100%, x=start, y=20%, repeat)

radial(#ff00ff 0%, #ff0 50%, #00f 100%)

linear(90, #ff00ff 0%, #ff0 50%, #00f 100%)
```
*/
export type BoxFilterIn = string | string[] | BoxFilter;

export declare class FillImage extends BoxFilter {
	src: string;
	width: FillSize;
	height: FillSize;
	x: FillPosition;
	y: FillPosition;
	repeat: Repeat;
	constructor(src: string, init?: {
		width?: FillSize, height?: FillSize,
		x?: FillPosition, y?: FillPosition, repeat?: Repeat,
	});
}

export declare class FillGradientRadial extends BoxFilter {
	readonly positions: Array<N>;
	readonly colors: Array<Color>;
	constructor(pos: Array<N>, coloes: Array<Color>);
}

export declare class FillGradientLinear extends FillGradientRadial {
	angle: N;
	constructor(pos: Array<N>, coloes: Array<Color>, angle: N);
}

export declare class BoxShadow extends BoxFilter {
	value: Shadow;
	constructor(value: Shadow);
}

/**
Examples:
 ```
 10 10 2 #ff00aa

 ['10 10 2 #ff00aa']

 10 10 2 rgba(255,255,0,255)
 ```
 */
export type BoxShadowIn = ShadowIn | ShadowIn[] | BoxShadow;

// -------------------------------------------------------------------------------------

export function newColor(r: N, g: N, b: N, a: N) {
	return newBase(Color, {r,g,b,a});
}

export function newVec2(x: N, y: N) {
	return newBase(Vec2, {x,y});
}

export function newVec3(x: N, y: N, z: N) {
	return newBase(Vec3, { x, y, z });
}

export function newVec4(x: N, y: N, z: N, w: N) {
	return newBase(Vec4, { x, y, z, w});
}

export function newRect(x: N, y: N, width: N, height: N) {
	return newBase(Rect, {origin: newVec2(x,y),size: newVec2(width,height)});
}

export function newMat(...value: N[]) {
	let mat = newBase(Mat, {});
	for (let i: 0 = 0; i < 6; i++)
		mat[`m${i}`] = value[i];
	return mat;
}

export function newMat4(...value: N[]) {
	let mat = newBase(Mat4, {});
	for (let i: 0 = 0; i < 16; i++)
		mat[`m${i}`] = value[i];
	return mat;
}

export function newCurve(p1x: N, p1y: N, p2x: N, p2y: N) {
	return newBase(Curve, { p1: newVec2(p1x,p1y), p2: newVec2(p2x,p2y) });
}

export function newShadow(
	x: N, y: N, size: N,
	r: N, g: N, b: N, a: N
) {
	return newBase(Shadow, {x,y,size,color: newColor(r, g, b, a)});
}

export function newBoxBorder(
	width: N, r: N, g: N, b: N, a: N
) {
	return newBase(BoxBorder, {width,color: newColor(r, g, b, a)});
}

export function newFillPosition(kind: FillPositionKind, value: N) {
	return newBase(FillPosition, {kind,value});
}

export function newFillSize(kind: FillSizeKind, value: N) {
	return newBase(FillSize, {kind,value});
}

export function newBoxOrigin(kind: BoxOriginKind, value: N) {
	return newBase(BoxOrigin, {kind,value});
}

export function newBoxSize(kind: BoxSizeKind, value: N) {
	return newBase(BoxSize, { kind, value });
}

export function bewBoxOrigin(kind: BoxOriginKind, value: N) {
	return newBase(BoxOrigin, { kind, value });
}

export function newTextColor(kind: TextValueKind, r: N, g: N, b: N, a: N) {
	return newBase(TextColor, { kind, value: newColor(r, g, b, a) });
}

export function newTextSize(kind: TextValueKind, value: N) {
	return newBase(TextSize, { kind, value });
}

export const newTextLineHeight = newTextSize;

export function newTextShadow(
	kind: TextValueKind, offset_x: N, offset_y: N,
	size: N, r: N, g: N, b: N, a: N
) {
	return newBase(TextShadow,{kind,value: newShadow(offset_x,offset_y,size,r,g,b,a)});
}

export function newTextFamily(kind: TextValueKind, ffid: Uint8Array) {
	return newBase(TextFamily,{kind,value:ffid});
}

// parse
// -------------------------------------------------------------------------------------

export function parseRepeat(val: RepeatIn, desc?: string): Repeat {
	return typeof val == 'string' ?
		Repeat[toCapitalize(val)] || 0: val in Repeat ? val: 0;
	// throw error(str, desc, undefined, Repeat);
}

export function parseDirection(val: DirectionIn, desc?: string): Direction {
	return typeof val == 'string' ?
		Direction[toCapitalize(val)] || 0: val in Direction ? val: 0;
}

export function parseItemsAlign(val: ItemsAlignIn, desc?: string): ItemsAlign {
	return typeof val == 'string' ?
		ItemsAlign[toCapitalize(val)] || 0:
		val in ItemsAlign ? val: 0;
}

export function parseCrossAlign(val: CrossAlignIn, desc?: string): CrossAlign {
	return typeof val == 'string' ?
		CrossAlign[toCapitalize(val)] || 0 : val in CrossAlign ? val: 0;
}

export function parseWrap(val: WrapIn, desc?: string): Wrap {
	return typeof val == 'string' ?
		Wrap[toCapitalize(val)] || 0 : val in Wrap ? val: 0;
}

export function parseWrapAlign(val: WrapAlignIn, desc?: string): WrapAlign {
	return typeof val == 'string' ?
		WrapAlign[toCapitalize(val)] || 0 : val in WrapAlign ? val: 0;
}

export function parseAlign(val: AlignIn, desc?: string): Align {
	return typeof val == 'string' ?
		Align[toCapitalize(val)] || 0 : val in Align ? val: 0;
}

export function parseTextAlign(val: TextAlignIn, desc?: string): TextAlign {
	return typeof val == 'string' ?
		TextAlign[toCapitalize(val)] || 0 : val in TextAlign ? val: 0;
}

export function parseTextDecoration(val: TextDecorationIn, desc?: string): TextDecoration {
	return typeof val == 'string' ?
		TextDecoration[toCapitalize(val)] || 0 : val in TextDecoration ? val: 0;
}

export function parseTextOverflow(val: TextOverflowIn, desc?: string): TextOverflow {
	return typeof val == 'string' ?
		TextOverflow[toCapitalize(val)] || 0 : val in TextOverflow ? val: 0;
}

export function parseTextWhiteSpace(val: TextWhiteSpaceIn, desc?: string): TextWhiteSpace {
	return typeof val == 'string' ?
		TextWhiteSpace[toCapitalize(val)] || 0 : val in TextWhiteSpace ? val: 0;
}

export function parseTextWordBreak(val: TextWordBreakIn, desc?: string): TextWordBreak {
	return typeof val == 'string' ?
		TextWordBreak[toCapitalize(val)] || 0 : val in TextWordBreak ? val: 0;
}

export function parseTextWeight(val: TextWeightIn, desc?: string): TextWeight {
	return typeof val == 'string' ?
		TextWeight[toCapitalize(val)] || 0 : val in TextWeight ? val: 0;
}

export function parseTextWidth(val: TextWidthIn, desc?: string): TextWidth {
	return typeof val == 'string' ?
		TextWidth[toCapitalize(val)] || 0 : val in TextWidth ? val: 0;
}

export function parseTextSlant(val: TextSlantIn, desc?: string): TextSlant {
	return typeof val == 'string' ?
		TextSlant[toCapitalize(val)] || 0 : val in TextSlant ? val: 0;
}

export function parseKeyboardType(val: KeyboardTypeIn, desc?: string): KeyboardType {
	return typeof val == 'string' ?
		KeyboardType[toCapitalize(val)] || 0 : val in KeyboardType ? val: 0;
}

export function parseKeyboardReturnType(val: KeyboardReturnTypeIn, desc?: string): KeyboardReturnType {
	return typeof val == 'string' ?
		KeyboardReturnType[toCapitalize(val)] || 0 : val in KeyboardReturnType ? val: 0;
}

export function parseCursorStyle(val: CursorStyleIn, desc?: string): CursorStyle {
	return typeof val == 'string' ?
		CursorStyle[toCapitalize(val)] || 0 : val in CursorStyle ? val: 0;
}

export function parseFindDirection(val: FindDirectionIn, desc?: string): FindDirection {
	return typeof val == 'string' ?
		FindDirection[toCapitalize(val)] || 0 : val in FindDirection ? val: 0;
}

const vec2Reg = [
	/^\s*(-?(?:\d+)?\.?\d+)\s+(-?(?:\d+)?\.?\d+)\s*$/,
	/^\s*vec2\(\s*(-?(?:\d+)?\.?\d+)\s*,\s*(-?(?:\d+)?\.?\d+)\s*\)\s*$/,
];
export function parseVec2(val: Vec2In, desc?: string): Vec2 {
	if (typeof val == 'string') {
		let m = val.match(vec2Reg[0]) || val.match(vec2Reg[1]);
		if (m) {
			return newVec2(parseFloat(m[1]), parseFloat(m[2]));
		}
	} else if (typeof val == 'number') {
		return newVec2(val, val);
	} else if (Array.isArray(val)) {
		return newVec2(val[0], val[1]);
	} else if (val instanceof Vec2) {
		return val;
	}
	throw error(val, desc, ['1 1','vec2(1,1)']);
}

const vec3Reg = [
	/^\s*(-?(?:\d+)?\.?\d+)\s+(-?(?:\d+)?\.?\d+)\s+(-?(?:\d+)?\.?\d+)\s*$/,
	/^\s*vec3\(\s*(-?(?:\d+)?\.?\d+)\s*,\s*(-?(?:\d+)?\.?\d+)\s*,\s*(-?(?:\d+)?\.?\d+)\s*\)\s*$/,
];
export function parseVec3(val: Vec3In, desc?: string): Vec3 {
	if (typeof val == 'string') {
		let m = val.match(vec3Reg[0]) || val.match(vec3Reg[1]);
		if (m)
			return newVec3(parseFloat(m[1]), parseFloat(m[2]), parseFloat(m[3]));
	} else if (typeof val == 'number') {
		return newVec3(val, val, val);
	} else if (Array.isArray(val)) {
		return newVec3(val[0], val[1], val[2]);
	} else if (val instanceof Vec3) {
		return val;
	} 
	throw error(val, desc, ['0 0 1', 'vec3(0,0,1)']);
}

const vec4Reg = [
	/^\s*(-?(?:\d+)?\.?\d+)\s+(-?(?:\d+)?\.?\d+)\s+(-?(?:\d+)?\.?\d+)\s+(-?(?:\d+)?\.?\d+)\s*$/,
	/^ *vec4\(\s*(-?(?:\d+)?\.?\d+)\s*,\s*(-?(?:\d+)?\.?\d+)\s*,\s*(-?(?:\d+)?\.?\d+)\s*,\s*(-?(?:\d+)?\.?\d+)\s*\)\s*$/,
];
export function parseVec4(val: Vec4In, desc?: string): Vec4 {
	if (typeof val == 'string') {
		let m = val.match(vec4Reg[0]) || val.match(vec4Reg[1]);
		if (m)
			return newVec4(parseFloat(m[1]), parseFloat(m[2]), parseFloat(m[3]), parseFloat(m[4]));
	} else if (typeof val == 'number') {
		return newVec4(val, val, val, val);
	} else if (val instanceof Vec4) {
		return val;
	} else if (Array.isArray(val)) {
		return newVec4(val[0],val[1],val[2],val[3]);
	}
	throw error(val, desc, ['0 0 1 1', 'vec4(0,0,1,1)']);
}

const CurveConsts = {
	linear: [0, 0, 1, 1],
	ease: [0.25, 0.1, 0.25, 1],
	easeIn: [0.42, 0, 1, 1],
	easeOut: [0, 0, 0.58, 1],
	easeInOut: [0.42, 0, 0.58, 1],
};
const CurveReg = [
	/^\s*(-?(?:\d+)?\.?\d+)\s+(-?(?:\d+)?\.?\d+)\s+(-?(?:\d+)?\.?\d+)\s+(-?(?:\d+)?\.?\d+)\s*$/,
	/^\s*curve\(\s*(-?(?:\d+)?\.?\d+)\s*,\s*(-?(?:\d+)?\.?\d+)\s*,\s*(-?(?:\d+)?\.?\d+)\s*,\s*(-?(?:\d+)?\.?\d+)\s*\)\s*$/
];
export function parseCurve(val: CurveIn, desc?: string): Curve {
	if (typeof val == 'string') {
		let s = CurveConsts[val as 'linear'];
		if (s) {
			return newCurve(s[0], s[1], s[2], s[3]);
		}
		let m = val.match(CurveReg[0]) || val.match(CurveReg[1]);
		if (m) {
			return newCurve(parseFloat(m[1]), parseFloat(m[2]), parseFloat(m[3]), parseFloat(m[4]));
		}
	} else if (Array.isArray(val)) {
		return newCurve(val[0], val[1], val[2], val[3]);
	} else if (val instanceof Curve) {
		return val;
	}
	throw error(val, desc, ['curve(0,0,1,1)', '0 0 1 1', 'linear', 'ease', 'easeIn', 'easeOut', 'easeInOut']);
}

const RectReg = [
	/^\s*(-?(?:\d+)?\.?\d+)\s+(-?(?:\d+)?\.?\d+)\s+(-?(?:\d+)?\.?\d+)\s+(-?(?:\d+)?\.?\d+)\s*$/,
	/^\s*rect\(\s*(-?(?:\d+)?\.?\d+)\s*,\s*(-?(?:\d+)?\.?\d+)\s*,\s*(-?(?:\d+)?\.?\d+)\s*,\s*(-?(?:\d+)?\.?\d+)\s*\)\s*$/
];
export function parseRect(val: RectIn, desc?: string): Rect {
	if (typeof val == 'string') {
		let m = val.match(RectReg[0]) || val.match(RectReg[1]);
		if (m) {
			return newRect(parseFloat(m[1]), parseFloat(m[2]), parseFloat(m[3]), parseFloat(m[4]));
		}
	} else if (Array.isArray(val)) {
		return newRect(val[0], val[1], val[2], val[3]);
	} else if (val instanceof Rect) {
		return val;
	}
	throw error(val, desc, ['rect(0,0,-100,200)', '0 0 -100 200']);
}

const matReg = new RegExp(`^\s*mat\\(\s*${new Array(6).join(
	'(-?(?:\\d+)?\\.?\\d+)\s*,\s*')}(-?(?:\\d+)?\\.?\\d+)\s*\\)\s*$`
);
export function parseMat(val: MatIn, desc?: string): Mat {
	if (typeof val == 'string') {
		let m = val.match(matReg);
		if (m) {
			return newMat(...[
				parseFloat(m[1]), parseFloat(m[2]), parseFloat(m[3]),
				parseFloat(m[4]), parseFloat(m[5]), parseFloat(m[6]),
			]);
		}
	} else if (typeof val == 'number') {
		return newMat(val,0,0,0,val,0);
	} else if (val instanceof Mat) {
		return val;
	} 
	throw error(val, desc, ['mat(1,0,0,1,0,1)']);
}

const mat4Reg = new RegExp(`^\s*mat4\\(\s*${new Array(16).join(
	'(-?(?:\\d+)?\\.?\\d+)\s*,\s*')}(-?(?:\\d+)?\\.?\\d+)\s*\\)\s*$`
);
export function parseMat4(val: Mat4In, desc?: string): Mat4 {
	if (typeof val == 'string') {
		let m = mat4Reg.exec(val);
		if (m) {
			return newMat4(...[
				parseFloat(m[1]), parseFloat(m[2]), parseFloat(m[3]), parseFloat(m[4]),
				parseFloat(m[5]), parseFloat(m[6]), parseFloat(m[7]), parseFloat(m[8]),
				parseFloat(m[9]), parseFloat(m[10]), parseFloat(m[11]), parseFloat(m[12]),
				parseFloat(m[13]), parseFloat(m[14]), parseFloat(m[15]), parseFloat(m[16]),
			]);
		}
	} else if (typeof val == 'number') {
		return newMat4(val,0,0,0,0,val,0,0,0,0,val,0,0,0,0,val);
	} else if (val instanceof Mat4) {
		return val;
	}
	throw error(val, desc, ['mat4(1,0,0,1,0,1,0,1,0,0,1,1,0,0,0,1)']);
}

// type ColorStrIn = `#${string}` | `rgb(${N},${N},${N})` | `rgba(${N},${N},${N},${N})`;
// export type ColorIn = ColorStrIn | N | Color;
const ColorReg = [
	/^#([0-9a-f]{3}([0-9a-f])?([0-9a-f]{2})?([0-9a-f]{2})?)$/i,
	/^\s*rgb(a)?\(\s*(\d{1,3})\s*,\s*(\d{1,3})\s*,\s*(\d{1,3})(\s*,\s*(\d{1,3}))?\s*\)\s*$/,
];
function parseColorNumber(val: number) {
	return newColor(val >> 24 & 255, // r
	val >> 16 & 255, // g
	val >> 8 & 255, // b
	val >> 0 & 255); // a	
}
export function parseColor(val: ColorIn, desc?: string, ref?: Reference): Color {
	if (typeof val == 'string') {
		let m = val.match(ColorReg[0]);
		if (m) {
			if (m[4]) { // 8
				return newColor(parseInt(m[1].substring(0, 2), 16),
											parseInt(m[1].substring(2, 4), 16),
											parseInt(m[1].substring(4, 6), 16),
											parseInt(m[1].substring(6, 8), 16));
			} else if (m[3]) { // 6
				return newColor(parseInt(m[1].substring(0, 2), 16),
											parseInt(m[1].substring(2, 4), 16),
											parseInt(m[1].substring(4, 6), 16), 255);
			} else if (m[2]) { // 4
				return newColor(parseInt(m[1].substring(0, 1), 16) * 17,
											parseInt(m[1].substring(1, 2), 16) * 17,
											parseInt(m[1].substring(2, 3), 16) * 17,
											parseInt(m[1].substring(3, 4), 16) * 17);
			} else { // 3
				return newColor(parseInt(m[1].substring(0, 1), 16) * 17,
											parseInt(m[1].substring(1, 2), 16) * 17,
											parseInt(m[1].substring(2, 3), 16) * 17, 255);
			}
		}
		m = val.match(ColorReg[1]);
		if (m) {
			if (m[1] == 'a') { // rgba
				if (m[5]) { // a
					return newColor(
						parseInt(m[2]) % 256, parseInt(m[3]) % 256, parseInt(m[4]) % 256, parseInt(m[6]) % 256
					);
				}
			} else { // rgb
				if (!m[5]) {
					return newColor(
						parseInt(m[2]) % 256, parseInt(m[3]) % 256, parseInt(m[4]) % 256, 255
					);
				}
			}
		}
	} else if (typeof val == 'number') {
		return parseColorNumber(val);
	} else if (val instanceof Color) {
		return val;
	}
	throw error(val, desc, [
		'rgba(255,255,255,255)', 'rgb(255,255,255)', '#ff0', '#ff00', '#ff00ff', '#ff00ffff'
	], ref);
}

const ShadowReg = /^\s*(-?(?:\d+)?\.?\d+)\s+(-?(?:\d+)?\.?\d+)\s+((?:\d+)?\.?\d+)/;
export function parseShadow(val: ShadowIn, desc?: string, ref?: Reference): Shadow {
	if (typeof val == 'string') {
		// 10 10 2 #ff00aa
		let m = val.match(ShadowReg);
		if (m) {
			return new Shadow({
				x: parseFloat(m[1]),
				y: parseFloat(m[2]),
				size: parseFloat(m[3]),
				color: parseColor(val.substring(m[0].length + 1) as ColorStrIn, desc, r=>{
					return ref ? ref(['10 10 2 #ff00aa', '10 10 2 rgba(255,255,0,255)']): r;
				}),
			});
		}
	} else if (val instanceof Shadow) {
		return val;
	}
	throw error(val, desc, ['10 10 2 #ff00aa', '10 10 2 rgba(255,255,0,255)'], ref);
}

const BoxBorderReg = /^\s*(-?(?:\d+)?\.?\d+)/;
export function parseBoxBorder(val: BoxBorderIn, desc?: string): BoxBorder {
	if (typeof val == 'string') {
		// 10 #ff00aa
		let m = val.match(BoxBorderReg);
		if (m) {
			return new BoxBorder({
				width: parseFloat(m[1]),
				color: parseColor(val.substring(m[0].length + 1) as ColorStrIn, desc,
					r=>['10 #ff00aa', '10 rgba(255,255,0,255)']),
			});
		}
	} else if (val instanceof BoxShadow) {
		return val;
	}
	throw error(val, desc, ['10 #ff00aa', '10 rgba(255,255,0,255)']);
}

export function parseFillPosition(val: FillPositionIn, desc?: string): FillPosition {
	if (typeof val == 'string') {
		let kind = FillPositionKind[toCapitalize(val as FillPositionKindStr)];
		if (kind !== undefined) {
			return newFillPosition(kind, 0);
		}
		let m = val.match(/^\s*(-?(?:\d+)?\.?\d+)(%)?\s*$/);
		if (m) {
			let kind = m[3] ? FillPositionKind.Ratio: FillPositionKind.Value;
			return newFillPosition(kind, parseFloat(m[1]));
		}
	} else if (val instanceof FillPosition) {
		return val;
	}
	throw error(val, desc, ["start", "end", "center", 10, '20%']);
}

export function parseFillSize(val: FillSizeIn, desc?: string): FillSize {
	if (typeof val == 'string') {
		let kind = FillSizeKind[toCapitalize(val as FillSizeKindStr)];
		if (kind !== undefined) {
			return newFillSize(kind, 0);
		}
		let m = val.match(/^\s*(-?(?:\d+)?\.?\d+)(%)?\s*$/);
		if (m) {
			let kind = m[3] ? FillSizeKind.Ratio: FillSizeKind.Value;
			return newFillSize(kind, parseFloat(m[1]));
		}
	} else if (val instanceof FillSize) {
		return val;
	}
	throw error(val, desc, ["auto", 10, '20%']);
}

export function parseBoxOrigin(val: BoxOriginIn, desc?: string): BoxOrigin {
	if (typeof val == 'string') {
		let kind = BoxOriginKind[toCapitalize(val as BoxOriginKindStr)];
		if (kind !== undefined) {
			return newBoxOrigin(kind, 0);
		}
		let m = val.match(/^\s*(-?(?:\d+)?\.?\d+)(%)?\s*$/);
		if (m) {
			let kind = m[3] ? FillSizeKind.Ratio: FillSizeKind.Value;
			return newBoxOrigin(kind, parseFloat(m[1]));
		}
	} else if (val instanceof BoxOrigin) {
		return val;
	}
	throw error(val, desc, ["auto", 10, '20%']);
}

export function parseBoxSize(val: BoxSizeIn, desc?: string): BoxSize {
	if (typeof val == 'string') {
		let kind = BoxSizeKind[toCapitalize(val as BoxSizeKindStr)];
		if (kind !== undefined) {
			return newBoxSize(kind, 0);
		}
		let m = val.match(/^\s*(-?(?:\d+)?\.?\d+)(%|!)?\s*$/);
		if (m) {
			let kind = m[3] ? m[3] == '%' ? BoxSizeKind.Ratio: BoxSizeKind.Minus: BoxSizeKind.Value;
			return newBoxSize(kind, parseFloat(m[1]));
		}
	} else if (val instanceof BoxSize) {
		return val;
	}
	throw error(val, desc, ["auto", "match", 10, '20%', '60!']);
}

export function parseTextColor(val: TextColorIn, desc?: string): TextColor {
	if (typeof val == 'string') {
		let kind = TextValueKind[toCapitalize(val as TextValueKindInStr)];
		if (kind !== undefined) {
			return newTextColor(kind, 0,0,0,255);
		}
		return new TextColor({
			kind: TextValueKind.Value,
			value: parseColor(val as ColorStrIn, desc, (ref)=>['inherit', 'default', ...ref])
		});
	} else if (typeof val == 'number') {
		return new TextColor({
			kind: TextValueKind.Value,
			value: parseColorNumber(val),
		});
	} else if (val instanceof Color) {
		return new TextColor({
			kind: TextValueKind.Value,
			value: val,
		});
	} else if (val instanceof TextColor) {
		return val;
	}
	throw error(val, desc, ['inherit', 'default',
		'rgba(255,255,255,255)', 'rgb(255,255,255)', '#ff0', '#ff00', '#ff00ff', '#ff00ffff'
	]);
}

const TextSizeReg = /^(?:\d+)?\.?\d+$/;
export function parseTextSize(val: TextSizeIn, desc?: string) {
	if (typeof val == 'string') {
		let kind = TextValueKind[toCapitalize(val as TextValueKindInStr)];
		if (kind !== undefined) {
			return newTextSize(kind, 0);
		}
		if (TextSizeReg.test(val)) {
			return newTextSize(TextValueKind.Value, parseFloat(val));
		}
	} else if (typeof val == 'number') {
		return newTextSize(TextValueKind.Value, val);
	} else if (val instanceof TextSize) {
		return val;
	}
	throw error(val, desc, ['inherit', 'default', 16, '12']);
}

export function parseTextLineHeight(val: TextLineHeightIn, desc?: string) {
	return parseTextSize(val, desc);
}

export function parseTextShadow(val: TextShadowIn, desc?: string): TextShadow {
	if (typeof val == 'string') {
		let kind = TextValueKind[toCapitalize(val as TextValueKindInStr)];
		if (kind !== undefined) {
			return newTextShadow(kind,0,0,0,0,0,0,255);
		}
		return new TextShadow({
			kind: TextValueKind.Value,
			value: parseShadow(val as ShadowIn, desc, (ref)=>['inherit', 'default', ...ref]),
		});
	} else if (val instanceof Shadow) {
		return new TextShadow({
			kind: TextValueKind.Value,
			value: val,
		});
	} else if (val instanceof TextShadow) {
		return val;
	}
	throw error(val, desc, ['inherit', 'default', '10 10 2 #ff00aa', '10 10 2 rgba(255,255,0,255)']);
}

export function parseTextFamily(val: TextFamilyIn, desc?: string) {
	if (typeof val == 'string') {
		let kind = TextValueKind[toCapitalize(val as TextValueKindInStr)];
		if (kind !== undefined) {
			return newTextFamily(kind,EmptyFFID);
		}
		return newTextFamily(kind, _font.getFontFamilys(val));
	} else if (val instanceof TextFamily) {
		return val;
	}
	throw error(val, desc, ['inherit', 'default', 'Ubuntu Mono']);
}

const parseBoxFilterReference = [
	'src(res/image.png, auto 100%, x=start, y=20%, repeat)',
	'radial(#ff00ff 0%, #ff0 50%, #00f 100%)',
	'linear(90, #ff00ff 0%, #ff0 50%, #00f 100%)',
	// shadow(1, 1, 2, #ff00ff)
];
const parseCmdReg = [
	/^\s*([a-z]+)\s*\(\s*/i, // src(
	/^([^,\)]+)(,\s*|\))/,
	/^"(([^"\\]|\\")+)"\s*(,\s*|\))/,
	/^'(([^'\\]|\\')+)'\s*(,\s*|\))/,
	/\s+/,
];
const parseCmdNext = {
	'_': (str: string)=>str.match(parseCmdReg[1]),
	'"': (str: string)=>str.match(parseCmdReg[2]),
	"'": (str: string)=>str.match(parseCmdReg[3]),
};

function parseCmd(val: string, desc?: string) {
	let m = val.match(parseCmdReg[0]);
	if (m) {
		let cmd = m[1];
		let args = [] as string[][];
		let kv: Dict<any[]> = {};
		let index = m[0].length;
		let valLen = val.length;
		while(index < valLen && val[index-1] != ')') {
			let char = val[index];
			let m = (parseCmdNext[char as '"'] || parseCmdNext._)(val.substring(index));
			if (m) {
				let arg = m[1];
				let idx = arg.indexOf('=');
				if (idx > 0) {
					let k = arg.substring(0, idx);
					let v = arg.substring(idx + 1);
					if (kv[k]) {
						kv[k].push(v);
					} else {
						kv[k] = [v];
					}
				} else {
					args.push(arg.split(parseCmdReg[4]));
				}
				index += m[0].length;
			}
		}
		if (args.length)
			return {val: cmd, kv, args};
	}
	throw error(val, desc, parseBoxFilterReference);
}

function getColorsPos(args: string[][], desc?: string) {
	let pos = [] as number[];
	let colors = [] as Color[];
	args.slice(1).map(([c,p])=>{
		colors.push(parseColor(c as ColorStrIn, desc, ()=>parseBoxFilterReference));
		pos.push(p ? (Number(p.substring(0, p.length - 1)) || 0) * 0.01: 0);
	});
	return {pos,colors};
}

function parseBoxFilterItem(val: string, desc?: string): BoxFilter {
	const FillImage_ = exports.FillImage as typeof FillImage;
	const FillGradientLinear_ = exports.FillGradientLinear as typeof FillGradientLinear;
	const FillGradientRadial_ = exports.FillGradientRadial as typeof FillGradientRadial;
	if (typeof val == 'string') {
		let cmd = parseCmd(val);
		if (cmd) {
			let {val,args,kv} = cmd;
			switch(val) {
				case 'src': {
					let width, height, repeat;
					if (args[1]) {
						var [w,h] = args[1];
						width = parseFillSize(w as FillSizeIn, desc);
						if (h)
							height = parseFillSize(h as FillSizeIn, desc);
						if (args[2])
							repeat = parseRepeat(args[2][0] as RepeatIn, desc);
					}
					new FillImage_(String(args[0][0]), {
						width, height, repeat,
						x: kv.x && parseFillPosition(kv.x[0], desc),
						y: kv.y && parseFillPosition(kv.y[0], desc),
					});
					break;
				}
				case 'linear': {
					let angle = Number(args.shift()) || 0;
					let {pos,colors} = getColorsPos(args, desc)
					return new FillGradientLinear_(pos,colors, angle);
				}
				case 'radial': {
					let {pos,colors} = getColorsPos(args, desc)
					return new FillGradientRadial_(pos,colors);
				}
			}
		}
	}
	throw error(val, desc, parseBoxFilterReference);
}

function linkFilter<T extends BoxFilter>(val: any, filters: T[], desc?: string, ref?: Reference) {
	if (filters.length) {
		for (let i = 1; i < filters.length; i++)
			filters[i-1].next = filters[i];
		return filters[0];
	}
	throw error(val, desc, [], ref);
}

export function parseBoxFilter(val: BoxFilterIn, desc?: string): BoxFilter {
	const BoxFilter_ = exports.BoxFilter as typeof BoxFilter;
	if (val instanceof BoxFilter_) {
		return val;
	} else if (Array.isArray(val)) {
		return linkFilter(val, val.map(e=>parseBoxFilterItem(e)), desc, ()=>parseBoxFilterReference);
	} else {
		return parseBoxFilterItem(val, desc);
	}
}

const parseBoxShadowRef = [
	'10 10 2 #ff00aa', ['10 10 2 #ff00aa'], '10 10 2 rgba(255,255,0,255)'
];
export function parseBoxShadow(val: BoxShadowIn, desc?: string): BoxShadow {
	const BoxShadow_ = exports.BoxShadow as typeof BoxShadow;
	if (val instanceof BoxShadow_) {
		return val;
	} else if (Array.isArray(val)) {
		return linkFilter(val,
			val.map(e=>new BoxShadow_(parseShadow(e, desc, ()=>parseBoxShadowRef))),
			desc, ()=>parseBoxShadowRef
		);
	} else {
		return new BoxShadow_(parseShadow(val, desc, ()=>parseBoxShadowRef));
	}
}
