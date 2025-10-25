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
	let msg = String(desc || '%s').replace(/\%s/g, value);
	let help = get_help(reference, ref, enum_value);
	if (help) {
		err = `Bad argument. \`${msg}\`. @example ${help}`;
	} else {
		err = `Bad argument. \`${msg}\`.`;
	}
	return new Error(err);
}

/**
 * @type N:number
 */
export type N = number;

interface Constructor<T> {
	new(...args: any[]): T;
}

export type RemoveField<T, S> = {
	[K in keyof T as Exclude<K, S>]: T[K]
}

export type RemoveReadonly<T> = {
	-readonly [P in keyof T]: T[P];
}

// export type RemoveToStringField<Type> = RemoveField<Type, 'toString'>;

export type OnlyDataFields<T> = {
	// [K in keyof T as T[K] extends (...args: any[]) => any ? never : K]: T[K]
	[K in keyof T as NonNullable<T[K]> extends Function ? never : K]: T[K]
}

export function initDefaults<T>(cls: Constructor<T>, ext: OnlyDataFields<Partial<T>>) {
	Object.assign(cls.prototype, ext);
}

function newBase<T>(cls: Constructor<T>, ext: OnlyDataFields<Partial<T>>) {
	(ext as any).__proto__ = cls.prototype;
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
	if (str.charCodeAt(0) > 96) { // a-z
		return str[0].toUpperCase() + str.substring(1) as Capitalize<T>;
	} else {
		return '' as Capitalize<T>;
	}
}

// -------------------------------------------------------------------------------------

/**
 * @enum Repeat
*/
export enum Repeat {
	Repeat,
	RepeatX, // only repeat x
	RepeatY, // only repeat y
	NoRepeat,
	MirrorRepeat, // mirror repeat
	MirrorRepeatX, // only mirror repeat x
	MirrorRepeatY, // only mirror repeat y
};

/**
 * @enum FillPositionKind
*/
export enum FillPositionKind {
	Start,     //!< Starting position
	Center,    //!< Centered position
	End,       //!< Ending position
	Value,     //!< Explicit Value
	Ratio,     //!< Percentage value
};

/**
 * @enum FillSizeKind
*/
export enum FillSizeKind {
	Auto,      //!< Auto value
	Value,     //!< Explicit Value
	Ratio,     //!< Percentage value
};

/**
 * @type BoxOriginKind
*/
export type BoxOriginKind = FillSizeKind;
export const BoxOriginKind = FillSizeKind;

/**
 * @enum Direction
 * 
 * Flex Layout direction
*/
export enum Direction {
	None, //!<
	Row, //!<
	RowReverse, //!<
	Column, //!<
	ColumnReverse, //!<
	Left = Row, //!<
	Right = RowReverse, //!<
	Top = Column, //!<
	Bottom = ColumnReverse, //!<
};

/**
 * @enum ItemsAlign
 * 
 * 项目在主轴上的对齐方式
 */
export enum ItemsAlign {
	Start, //!< 左对齐
	Center, //!< 居中
	End, //!< 右对齐
	SpaceBetween, //!< 两端对齐，项目之间的间隔都相等
	SpaceAround, //!< 每个项目两侧的间隔相等。所以，项目之间的间隔比项目与边框的间隔大一倍
	SpaceEvenly, //!< 每个项目两侧的间隔相等,这包括边框的间距
	CenterCenter, //!< 把除两端以外的所有项目尽可能的居中对齐
};

/**
 * @enum CrossAlign
 * 
 * 项目在交叉轴内如何对齐
 */
export enum CrossAlign {
	Start = 1, // 与交叉轴内的起点对齐
	Center, // 与交叉轴内的中点对齐
	End, // 与交叉轴内的终点对齐
	Both, // 与交叉轴内的两端对齐
};

/**
 * @enum Wrap
 * 
 * 主轴溢出包裹，开启后当主轴溢出时分裂成多根交叉轴
 */
export enum Wrap {
	NoWrap, //!< 只有一根交叉轴线
	Wrap, //!< 溢出后会有多根交叉轴线
	WrapReverse, //!< 多根交叉轴线反向排列
};

/**
 * @enum WrapAlign
 * 
 * 多根交叉轴线的对齐方式。如果项目只有一根交叉轴，该属性不起作用
 */
export enum WrapAlign {
	Start, //!< 与交叉轴的起点对齐
	Center, //!< 与交叉轴的中点对齐
	End, //!< 与交叉轴的终点对齐
	SpaceBetween, //!< 与交叉轴两端对齐,轴线之间的间隔平均分布
	SpaceAround, //!< 每根轴线两侧的间隔都相等,所以轴线之间的间隔比轴线与边框的间隔大一倍
	SpaceEvenly, //!< 每根轴线两侧的间隔都相等,这包括边框的间距
	Stretch = 7, //!< {7} 轴线占满整个交叉轴，平均分配剩余的交叉轴空间
};

/**
 * View align
 */
export enum Align {
	Normal, //!<
	Start, //!<
	Center, //!<
	End, //!<
	Both, //!<
	NewStart = Both, //!< {Both} New independent line and left align
	NewCenter, //!< New independent line and center align
	NewEnd, //!< New independent line and right align
	FloatStart, //!< Try not to wrap until the maximum limit and left align
	FloatCenter, //!< Try not to wrap until the maximum limit and center align
	FloatEnd, //!< Try not to wrap until the maximum limit and right align
	Baseline = Normal, //!< {Normal} box vertical align in text
	Top, //!< box vertical align in text
	Middle, //!< box vertical align in text
	Bottom, //!< box vertical align in text
	LeftTop = Start, //!< {Start}
	CenterTop, //!<
	RightTop, //!<
	LeftMiddle, //!<
	CenterMiddle, //!<
	RightMiddle, //!<
	LeftBottom, //!<
	CenterBottom, //!<
	RightBottom, //!<
};

/**
 * @enum BoxSizeKind
 * 
 * The width/height value kind for the Box
*/
export enum BoxSizeKind {
	None,    /* Do not use value */
	Auto,    /* wrap content or auto value */
	Match,   /* match parent */
	Value,   /* Density-independent Pixel, dp = px * window.scale */
	Dp = Value,/* dp, alias of Value */
	// Pt,    /* pt, Points, pt = px * window.defaultScale */
	// Px,   /* px, Pixel */
	Ratio,   /* value % */
	Minus,   /* (parent-value) value ! */
};

/**
 * @enum TextValueKind
 * 
 * Basic type for the Text attributes
*/
export enum TextValueKind {
	Inherit, //!<
	Default, //!<
	Value, //!<
};

/**
 * @enum TextAlign
*/
export enum TextAlign {
	Inherit,        //!< inherit
	Left,           //!< 左对齐
	Center,         //!< 居中
	Right,          //!< 右对齐
	Default = Left, //!< {Left}
};

/**
 * @enum TextDecoration
*/
export enum TextDecoration {
	Inherit,        //!< inherit
	None,           //!< 没有
	Overline,       //!< 上划线
	LineThrough,    //!< 中划线
	Underline,      //!< 下划线
	Default = None, //!< {None}
};

/**
 * @enum TextOverflow
*/
export enum TextOverflow {
	Inherit,         //!< inherit
	Normal,          //!< 不做任何处理
	Clip,            //!< 剪切
	Ellipsis,        //!< 剪切并显示省略号
	EllipsisCenter,  //!< 剪切并居中显示省略号
	Default = Normal,//!< {Normal}
};

/**
 * @enum TextWhiteSpace
*/
export enum TextWhiteSpace {
	Inherit,      //!< inherit
	Normal,       //!< 合并空白序列,使用自动wrap
	NoWrap,       //!< 合并空白序列,不使用自动wrap
	Pre,          //!< 保留所有空白,不使用自动wrap
	PreWrap,      //!< 保留所有空白,使用自动wrap
	PreLine,      //!< 合并空白符序列,但保留换行符,使用自动wrap
	Default = Normal, //!< {Normal}
};

/**
 * @enum TextWordBreak
*/
export enum TextWordBreak {
	Inherit,  //!< inherit
	Normal,   //!< 保持单词在同一行
	BreakWord,//!< 保持单词在同一行,除非单词长度超过一行才截断
	BreakAll, //!< 以字为单位行空间不足换行
	KeepAll,  //!< 所有连续的字符都当成一个单词,除非出现空白符、换行符、标点符
	Default = Normal, //!< {Normal}
};

/**
 * @enum TextWeight
*/
export enum TextWeight {
	Inherit      = 0,   //!<
	Thin         = 100, //!<
	Ultralight   = 200, //!<
	Light        = 300, //!<
	Regular      = 400, //!<
	Medium       = 500, //!<
	Semibold     = 600, //!<
	Bold         = 700, //!<
	Heavy        = 800, //!<
	Black        = 900, //!<
	ExtraBlack   = 1000, //!<
	Default      = Regular, //!< {Regular}
};

/**
 * @enum TextWidth
*/
export enum TextWidth {
	Inherit          = 0, //!<
	UltraCondensed   = 1, //!<
	ExtraCondensed   = 2, //!<
	Condensed        = 3, //!<
	SemiCondensed    = 4, //!<
	Normal           = 5, //!<
	SemiExpanded     = 6, //!<
	Expanded         = 7, //!<
	ExtraExpanded    = 8, //!<
	UltraExpanded    = 9, //!<
	Default          = Normal, //!< {Normal}
};

/**
 * @enum TextSlant
*/
export enum TextSlant {
	Inherit, //!<
	Normal,  //!<
	Italic,  //!<
	Oblique, //!<
	Default = Normal, //!< {Normal}
};

/**
 * @type TextStyle
 */
export type TextStyle = number;

/**
 * @enum KeyboardType
*/
export enum KeyboardType {
	Normal, //!<
	Ascii,  //!<
	Number, //!<
	Url,    //!<
	NumberPad, //!<
	Phone,  //!<
	NamePhone, //!<
	Email,  //!<
	Decimal, //!<
	Search, //!<
	AsciiNumber, //!<
};

/**
 * @enum KeyboardReturnType
*/
export enum KeyboardReturnType {
	Normal, //!<
	Go, //!<
	Join, //!<
	Next, //!<
	Route, //!<
	Search, //!<
	Send, //!<
	Done, //!<
	Emergency, //!<
	Continue, //!<
};

/**
 * @enum CursorStyle
*/
export enum CursorStyle {
	Normal, //!<
	None, //!<
	NoneUntilMouseMoves, //!<
	Arrow, //!<
	Ibeam, //!<
	PointingHand, //!<
	Pointer = PointingHand, //!<
	ClosedHand, //!<
	OpenHand, //!<
	ResizeLeft, //!<
	ResizeRight, //!<
	ResizeLeftRight, //!<
	ResizeUp, //!<
	ResizeDown, //!<
	ResizeUpDown, //!<
	Crosshair, //!<
	DisappearingItem, //!<
	OperationNotAllowed, //!<
	DragLink, //!<
	DragCopy, //!<
	ContextualMenu, //!<
	IbeamForVertical, //!<
};

/**
 * Color changes will affect sub views
 */
export enum CascadeColor {
	None, // No change
	Alpha, // Only change Alpha for sub views
	Color, // Only change RGB for sub views
	Rgb = Color, // alias of Color
	Both, // Change RGB and Alpha for sub views
};

// -------------------------------------------------------------------------------------

/**
 * @type RepeatIn:"repeat"|"repeatX"|"repeatY"|"noRepeat"|"mirrorRepeat"|"mirrorRepeatX"|"mirrorRepeatY"|Repeat
*/
export type RepeatIn = Uncapitalize<keyof typeof Repeat> | Repeat;
/**
 * @type DirectionIn:"row"|"rowReverse"|"column"|"columnReverse"|Direction
*/
export type DirectionIn = Uncapitalize<keyof typeof Direction> | Direction;
/**
 * @type ItemsAlignIn:"start"|"center"|"end"|"spaceBetween"|"spaceAround"|"spaceEvenly"|\
"centerCenter"|ItemsAlign
*/
export type ItemsAlignIn = Uncapitalize<keyof typeof ItemsAlign> | ItemsAlign;
/**
 * @type CrossAlignIn:"start"|"center"|"end"|"both"|CrossAlign
*/
export type CrossAlignIn = Uncapitalize<keyof typeof CrossAlign> | CrossAlign;
/**
 * @type WrapIn:"noWrap"|"wrap"|"wrapReverse"|Wrap
*/
export type WrapIn = Uncapitalize<keyof typeof Wrap> | Wrap;
/**
 * @type WrapAlignIn:"start"|"center"|"end"|"spaceBetween"|"spaceAround"|"spaceEvenly"|"stretch"|\
WrapAlign
*/
export type WrapAlignIn = Uncapitalize<keyof typeof WrapAlign> | WrapAlign;
/**
 * @type AlignIn:"normal"|"start"|"center"|"end"|"both"|"newStart"|"newCenter"|"newEnd"|\
"floatStart"|"floatCenter"|"floatEnd"|"baseline"|"top"|"middle"|"bottom"|"leftTop"|"centerTop"|\
"rightTop"|"leftMiddle"|"centerMiddle"|"rightMiddle"|"leftBottom"|"centerBottom"|"rightBottom"|Align
 */
export type AlignIn = Uncapitalize<keyof typeof Align> | Align;
/**
 * @type TextAlignIn:"center"|"inherit"|"left"|"right"|"default"|TextAlign
*/
export type TextAlignIn = Uncapitalize<keyof typeof TextAlign> | TextAlign;
/**
 * @type TextDecorationIn:"inherit"|"default"|"none"|"overline"|"lineThrough"|"underline"|TextDecoration
*/
export type TextDecorationIn = Uncapitalize<keyof typeof TextDecoration> | TextDecoration;
/**
 * @type TextOverflowIn:"normal"|"inherit"|"default"|"clip"|"ellipsis"|"ellipsisCenter"|TextOverflow
*/
export type TextOverflowIn = Uncapitalize<keyof typeof TextOverflow> | TextOverflow;
/**
 * @type TextWhiteSpaceIn:"noWrap"|"normal"|"inherit"|"default"|"pre"|"preWrap"|"preLine"|TextWhiteSpace
*/
export type TextWhiteSpaceIn = Uncapitalize<keyof typeof TextWhiteSpace> | TextWhiteSpace;
/**
 * @type TextWordBreakIn:"normal"|"inherit"|"default"|"breakWord"|"breakAll"|"keepAll"|TextWordBreak
*/
export type TextWordBreakIn = Uncapitalize<keyof typeof TextWordBreak> | TextWordBreak;
/**
 * @type TextWeightIn:"inherit"|"default"|"thin"|"ultralight"|"light"|"regular"|"medium"|"semibold"\
|"bold"|"heavy"|"black"|"extraBlack"|TextWeight
*/
export type TextWeightIn = Uncapitalize<keyof typeof TextWeight> | TextWeight;
/**
 * @type TextWidthIn:"normal"|"inherit"|"default"|"ultraCondensed"|"extraCondensed"|"condensed"|\
"semiCondensed"|"semiExpanded"|"expanded"|"extraExpanded"|"ultraExpanded"|TextWidth
 */
export type TextWidthIn = Uncapitalize<keyof typeof TextWidth> | TextWidth;
/**
 * @type TextSlantIn:"normal"|"inherit"|"default"|"italic"|"oblique"|TextSlant
*/
export type TextSlantIn = Uncapitalize<keyof typeof TextSlant> | TextSlant; 
/**
 * @type KeyboardTypeIn:"number"|"normal"|"ascii"|"url"|"numberPad"|"phone"|"namePhone"|\
"email"|"decimal"|"search"|"asciiNumber"|KeyboardType
*/
export type KeyboardTypeIn = Uncapitalize<keyof typeof KeyboardType> | KeyboardType; 
/**
 * @type KeyboardReturnTypeIn:"normal"|"search"|"go"|"join"|"next"|"route"|"send"|"done"|\
"emergency"|"continue"|KeyboardReturnType
*/
export type KeyboardReturnTypeIn = Uncapitalize<keyof typeof KeyboardReturnType> | KeyboardReturnType; 
/**
 * @type CursorStyleIn:"normal"|"none"|"noneUntilMouseMoves"|"arrow"|"ibeam"|"pointingHand"|\
"pointer"|"closedHand"|"openHand"|"resizeLeft"|"resizeRight"|"resizeLeftRight"|"resizeUp"|\
"resizeDown"|"resizeUpDown"|"crosshair"|"disappearingItem"|"operationNotAllowed"|"dragLink"|\
"dragCopy"|"contextualMenu"|"ibeamForVertical"|CursorStyle
*/
export type CursorStyleIn = Uncapitalize<keyof typeof CursorStyle> | CursorStyle;

/**
 * @type CascadeColorIn:"none"|"alpha"|"color"|"both"|CascadeColor
*/
export type CascadeColorIn = Uncapitalize<keyof typeof CascadeColor> | CascadeColor;

/***/
export class Base<T> {
	toString() { return '[types base]' } //!<
	toStringStyled(indent?: number) { //!<
		return (indent ? new Array(indent+1).join(' '): '') + this.toString();
	}
	constructor(opts?: OnlyDataFields<Partial<T>>) {
		Object.assign(this, opts)
	}
}

export const PI2 = Math.PI * 2; // 360deg
export const PIHalf = Math.PI * 0.5; // 90deg
export const PIHalfHalf = Math.PI * 0.25; // 45deg
export const PIDegree = PI2 / 360; // 1deg

/**
 * @class Vec2 2D vector
*/
export class Vec2 extends Base<Vec2> {
	/**
	 * x component
	*/
	readonly x: N;

	/**
	 * y component
	*/
	readonly y: N;

	/**
	 * Get string value
	*/
	toString(): `vec2(${N},${N})` { return `vec2(${this.x},${this.y})` }

	/** Check vector is zero */
	isZero() {
		return this.x === 0 && this.y === 0;
	}

	/**
	 * Check two vectors are equal
	*/
	equal(b: Vec2): boolean {
		return this.x === b.x && this.y === b.y;
	}

	/**
	 * Add two vectors
	*/
	add(b: Vec2): Vec2 {
		return newBase(Vec2, { x: this.x + b.x, y: this.y + b.y });
	}

	/**
	 * Subtract two vectors
	*/
	sub(b: Vec2): Vec2 {
		return newBase(Vec2, { x: this.x - b.x, y: this.y - b.y });
	}

	/**
	 * Multiply vector by a scalar
	 */
	mul(b: N): Vec2 {
		return newBase(Vec2, { x: this.x * b, y: this.y * b });
	}

	/**
	 * Divide vector by a scalar
	 */
	div(b: N): Vec2 {
		return newBase(Vec2, { x: this.x / b, y: this.y / b });
	}

	/**
	 * Get the length of the vector
	*/
	length(): N {
		return Math.sqrt(this.x * this.x + this.y * this.y);
	}

	/**
	 * Returns vector length squared
	 * @return The length squared
	 */
	lengthSq(): N {
		return this.x * this.x + this.y * this.y;
	}

	/**
	 * Get the dot product of two vectors
	*/
	dot(b: Vec2): N {
		return this.x * b.x + this.y * b.y;
	}

	/**
	 * returns scalar outer product
	*/
	det(b: Vec2) {
		return this.x * b.y - this.y * b.x;
	}

	/**
	 * Get the normalized vector
	*/
	normalized(): Vec2 {
		let len = this.length();
		if (len === 0)
			return newVec2(0, 0);
		let invLen = 1 / len; // optimize to avoid division
		return newVec2(this.x * invLen, this.y * invLen);
	}

	/**
	 * Rotate the vector by 90 degrees clockwise
	 */
	rotate90z() {
		return newVec2(this.y, -this.x);
	}

	/**
	 * Rotate the vector by 270 degrees clockwise (90 degrees counterclockwise)
	 */
	rotate270z() { // ccw rotate 90
		return newVec2(-this.y, this.x);
	}

	/**
	 * Default to use Cartesian coordinate system
	 * Returns zero when the previous is on the same side and on the same line as the next.
	 * Clockwise direction inward, screen coordinates outward
	 */
	normalline(prev: Vec2, next: Vec2) {
		let toNext   = newVec2(next.x - this.x, next.y - this.y).normalized().rotate90z();
		let fromPrev = newVec2(this.x - prev.x, this.y - prev.y).normalized().rotate90z();
		// Returns zero when the previous is on the same side and on the same line as the next
		return toNext.add(fromPrev).normalized();
	}

	/**
	 * Get the angle of the point in radians
	 * @return angle in radians
	*/
	angle(): N {
		return Math.atan2(this.y, this.x);
	}

	/**
	 * Get the angle of the point in degrees
	 */
	angleInDegrees(): N {
		return this.angle() / PIDegree;
	}

	/**
	 * Get the angle to another point in radians
	 * @param p The other point
	 * @return angle in radians
	*/
	angleTo(p: Vec2): N {
		return newVec2(p.y - this.y, p.x - this.x).angle();
	}

	/**
	 * Rotate the vector by radians
	 * @param radians The angle in radians
	 * @return The rotated vector
	*/
	rotate(radians: N): Vec2 {
		const c = Math.cos(radians);
		const s = Math.sin(radians);
		return newVec2(
			this.x * c - this.y * s,
			this.x * s + this.y * c
		);
	}

	/**
	 * Reverse the y axis of the vector
	*/
	reverseY(): Vec2 {
		return newVec2(this.x, -this.y);
	}

	/** 
	 * Get the direction of the vector
	 * @param accuracy The accuracy of angle, the smaller the more accurate, default is 1
	 * @return Return to clockwise direction
	 */
	direction(accuracy: number = 1): Direction {
		if (this.x === 0 && this.y === 0)
			return Direction.None;
		let angle = this.angle();
		angle = (angle + PIHalfHalf) % PI2;
		if (angle < 0)
			angle += PI2;
		if (accuracy < 1) { // filter angle accuracy
			const threshold = (1 - accuracy) * 0.5; // 0 ~ 0.5
			const normalizedAngle = angle % PIHalf / PIHalf - threshold; // -threshold ~ 1-threshold
			if (normalizedAngle < 0 || normalizedAngle > accuracy)
				return Direction.None;
		}
		// Return to clockwise direction
		return [Direction.Right, Direction.Bottom,
			Direction.Left, Direction.Top][Math.floor(angle / PIHalf)];
	}

	/**
	 * Get the midpoint between two vectors
	*/
	mid(b: Vec2): Vec2 {
		return newVec2((this.x + b.x) * 0.5, (this.y + b.y) * 0.5);
	}

	/**
	 * Get the midpoint of multiple vectors
	 * @static
	 * @param vec:Vec2[] The vectors array
	 * @returns {Vec2} The midpoint vector
	 */
	static mid(vec: Vec2[]): Vec2 {
		if (vec.length === 0)
			return newVec2(0, 0);
		let x = 0;
		let y = 0;
		let n = vec.length;
		for (let i = 0; i < n; i++) {
			x += vec[i].x;
			y += vec[i].y;
		}
		return newVec2(x / n, y / n);
	}

	/**
	 * @static
	 * Create a new Vec2 instance
	*/
	static new(x: N, y: N) {
		return newVec2(x, y);
	}

	/**
	 * @static
	 * Create a zero vector
	*/
	static zero() {
		return newVec2(0, 0);
	}
}

initDefaults(Vec2, { x: 0, y: 0 });
/**
 * @type Vec2In:'0　0'|'vec2(0,0)'|N|[0,0]|Vec2
*/
export type Vec2In = `${number} ${number}` | `vec2(${N},${N})` | N | [N,N] | Vec2;

/**
 * @class Vec3
*/
export class Vec3 extends Base<Vec3> {
	readonly x: N; //!<
	readonly y: N; //!<
	readonly z: N; //!<
	toString(): `vec3(${N},${N},${N})` { return `vec3(${this.x},${this.y},${this.z})` }
}
initDefaults(Vec3, { x: 0, y: 0, z: 0 });
/**
 * @type Vec3In:'0　0　1'|'vec3(0,0,1)'|N|[0,0,1]|Vec3
*/
export type Vec3In = `${number} ${number} ${number}` | `vec3(${N},${N},${N})` | N | [N,N,N] | Vec3;

/**
 * @class Vec4
*/
export class Vec4 extends Base<Vec4> {
	readonly x: N; //!<
	readonly y: N; //!<
	readonly z: N; //!<
	readonly w: N; //!<
	toString(): `vec4(${N},${N},${N},${N})` {
		return `vec4(${this.x},${this.y},${this.z},${this.w})`;
	}
}
initDefaults(Vec4, { x: 0, y: 0, z: 0, w: 0 });
/**
 * @type Vec4In:'0　0　1　1'|'vec4(0,0,1,1)'|N|[0,0,1,1]|Vec4
*/
export type Vec4In = `${number} ${number} ${number} ${number}` |
	`vec4(${N},${N},${N},${N})` | N | [N,N,N,N] | Vec4;

/**
 * @class Curve
*/
export class Curve extends Base<Curve> {
	readonly p1: Vec2; //!<
	readonly p2: Vec2; //!<
	get p1x() { return this.p1.x; } //!< {N}
	get p1y() { return this.p1.y; } //!< {N}
	get p2x() { return this.p2.x; } //!< {N}
	get p2y() { return this.p2.y; } //!< {N}
	toString(): `curve(${N},${N},${N},${N})` {
		return `curve(${this.p1.x},${this.p1.y},${this.p2.x},${this.p2.y})`;
	}
}
initDefaults(Curve, { p1: new Vec2({x:0,y:0}), p2: new Vec2({x:1,y:1}) });
/**
 * @type CurveIn:'linear'|'ease'|'easeIn'|'easeOut'|'easeInOut'|'curve(0,0,1,1)'|[0,0,1,1]|Curve
*/
export type CurveIn = 'linear' | 'ease' | 'easeIn' | 'easeOut' | 'easeInOut' |
	`curve(${N},${N},${N},${N})` | [number,number,number,number] | Curve;

/**
 * @class Rect
*/
export class Rect extends Base<Rect> {
	readonly begin: Vec2; //!<
	readonly size: Vec2; //!<
	get x() { return this.begin.x; } //!< {N}
	get y() { return this.begin.y; } //!< {N}
	get width() { return this.size.x; } //!< {N}
	get height() { return this.size.y; } //!< {N}
	toString(): `rect(${N},${N},${N},${N})` {
		return `rect(${this.x},${this.y},${this.width},${this.height})`;
	}
}
initDefaults(Rect, { begin: new Vec2, size: new Vec2 });
export type RectIn = `rect(${N},${N},${N},${N})` | `rect(${N},${N})` | //!< {'rect(0,0,100,100)'|'rect(0,0)'|[0,0,100,100]|Rect}
	[number,number,number,number] | Rect;

/**
 * @class Range
*/
export class Range extends Base<Range> {
	readonly begin: Vec2; //!< top-left corner
	readonly end: Vec2; //!< bottom-right corner
	get p0() { return this.begin.x; }
	get p1() { return this.begin.y; }
	get p2() { return this.end.x; }
	get p3() { return this.end.y; }
}
initDefaults(Range, { begin: new Vec2, end: new Vec2 });
export type RangeIn = Range;

/**
 * @class Region
*/
export class Region extends Base<Region> {
	readonly begin: Vec2; //!< top-left corner
	readonly end: Vec2; //!< bottom-right corner
	readonly origin: Vec2; //!< origin point
	get p0() { return this.begin.x; }
	get p1() { return this.begin.y; }
	get p2() { return this.end.x; }
	get p3() { return this.end.y; }
	get p4() { return this.origin.x; }
	get p5() { return this.origin.y; }
}
initDefaults(Region, { begin: new Vec2, end: new Vec2, origin: new Vec2 });
export type RegionIn = Region;

/**
 * @class BorderRadius
*/
export class BorderRadius extends Base<BorderRadius> {
	readonly leftTop: Vec2 //!<
	readonly rightTop: Vec2 //!<
	readonly rightBottom: Vec2; //!<
	readonly leftBottom: Vec2; //!<
	get p0() { return this.leftTop.x; }
	get p1() { return this.leftTop.y; }
	get p2() { return this.rightTop.x; }
	get p3() { return this.rightTop.y; }
	get p4() { return this.rightBottom.x; }
	get p5() { return this.rightBottom.y; }
	get p6() { return this.leftBottom.x; }
	get p7() { return this.leftBottom.y; }
}
initDefaults(BorderRadius, {
	leftTop: new Vec2, rightTop: new Vec2, rightBottom: new Vec2, leftBottom: new Vec2
});
export type BorderRadiusIn = BorderRadius;

/**
 * @class Mat
*/
export class Mat extends Base<Mat> {
	readonly m0: N; //!<
	readonly m1: N; //!<
	readonly m2: N; //!<
	readonly m3: N; //!<
	readonly m4: N; //!<
	readonly m5: N; //!<
	get value() {
		return [this.m0,this.m1,this.m2,this.m3,this.m4,this.m5];
	}
	toString(): `mat(${N},${N},${N},${N},${N},${N})` {
		let _ = this;
		return `mat(${_.m0},${_.m1},${_.m2},${_.m3},${_.m4},${_.m5})`;
	}
}
initDefaults(Mat, { m0: 1, m1: 0, m2: 0, m3: 0, m4: 1, m5: 0 });
export type MatIn = N | Mat | ReturnType<typeof Mat.prototype.toString>; //!< {N|Mat|'mat(0,0,0,0,0,0)'}

/**
 * @class Mat4
*/
export class Mat4 extends Base<Mat4> {
	readonly m0: N; //!<
	readonly m1: N; //!<
	readonly m2: N; //!<
	readonly m3: N; //!<
	readonly m4: N; //!<
	readonly m5: N; //!<
	readonly m6: N; //!<
	readonly m7: N; //!<
	readonly m8: N; //!<
	readonly m9: N; //!<
	readonly m10: N; //!<
	readonly m11: N; //!<
	readonly m12: N; //!<
	readonly m13: N; //!<
	readonly m14: N; //!<
	readonly m15: N; //!<
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

/**
 * @class Color
*/
export class Color extends Base<Color> {
	readonly r: N; //!< red 0 ~ 255
	readonly g: N; //!< green 0 ~ 255
	readonly b: N; //!< blue 0 ~ 255
	readonly a: N; //!< alpha 0 ~ 255
	get alpha() { //!< {N} alpha 0.0 ~ 1.0
		return this.a / 255;
	}
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
		return `rgba(${this.r},${this.g},${this.b},${this.a/255})`;
	}
	toString(): `#${string}` {
		return `#${hexStr(this.r)}${hexStr(this.g)}${hexStr(this.b)}`;
	}
	toHex32String(): `#${string}` {
		return `#${hexStr(this.r)}${hexStr(this.g)}${hexStr(this.b)}${hexStr(this.a)}`;
	}
}
initDefaults(Color, { r: 0, g: 0, b: 0, a: 255 });
export type ColorStrIn = `#${string}` | `rgb(${N},${N},${N})` | `rgba(${N},${N},${N},${N})`; //!< {'#fff'|'rgb(0,0,0)'|'rgba(0,0,0,1)'}
export type ColorIn = ColorStrIn | N | Color; //!<

/**
 * @class Shadow
*/
export class Shadow extends Base<Shadow> {
	readonly x: N; //!< {N}
	readonly y: N; //!< {N}
	readonly size: N; //!< {N}
	readonly color: Color; //!< {Color}
	get r() { return this.color.r; } //!< {N}
	get g() { return this.color.g; } //!< {N}
	get b() { return this.color.b; } //!< {N}
	get a() { return this.color.a; } //!< {N}
	toString(): `${N} ${N} ${N} #${string}` {
		return `${this.x} ${this.y} ${this.size} ${this.color.toString()}`
	}
}
initDefaults(Shadow, { x: 0, y: 0, size: 0, color: new Color(0,0,0,255) });
export type ShadowIn = `${N} ${N} ${N} ${ColorStrIn}` | Shadow; //!< {'2　2　2　rgb(100,100,100)'|Shadow}

/**
 * @class Border
*/
export class Border extends Base<Border> {
	readonly width: N; //!<
	readonly color: Color; //!<
	get r() { return this.color.r; } //!< {N}
	get g() { return this.color.g; } //!< {N}
	get b() { return this.color.b; } //!< {N}
	get a() { return this.color.a; } //!< {N}
	toString(): `${N} #${string}` {
		return `${this.width} ${this.color.toString()}`
	}
}
initDefaults(Border, { width: 0, color: new Color(0,0,0,255) });
export type BorderIn = `${N} ${ColorStrIn}` | Border; //!< {'1　rgb(100,100,100)'|Border}

/**
 * @class FillPosition
*/
export class FillPosition extends Base<FillPosition> {
	readonly value: N; //!<
	readonly kind: FillPositionKind; //!<
};
initDefaults(FillPosition, { value: 0, kind: FillPositionKind.Value });
type FillPositionKindStr = //!< {'start'|'end'|'center'}
	Uncapitalize<keyof RemoveField<typeof FillPositionKind, 'Value'|'Ratio'|number>>;
/** @type FillPositionIn:N|FillPosition|'50%'|FillPositionKindStr */
export type FillPositionIn = N | FillPosition | `${number}%` | FillPositionKindStr;

/**
 * @class FillSize
*/
export class FillSize extends Base<FillSize> {
	readonly value: N; //!<
	readonly kind: FillSizeKind; //!<
};
initDefaults(FillSize, { value: 0, kind: FillSizeKind.Auto });
type FillSizeKindStr = Uncapitalize<keyof RemoveField<typeof FillSizeKind, 'Value'|'Ratio'|number>>; //!< {'auto'}
export type FillSizeIn = N | FillSize | `${number}%` | FillSizeKindStr; //!< {N|FillSize|'50%'|FillSizeKindStr}

/**
 * @class BoxSize
*/
export class BoxSize extends Base<BoxSize> {
	readonly value: N; //!<
	readonly kind: BoxSizeKind; //!<
};
initDefaults(BoxSize, { value: 0, kind: BoxSizeKind.Value });
type BoxSizeKindStr = //!< {'auto'|'none'|'match'}
	Uncapitalize<keyof RemoveField<typeof BoxSizeKind, 'Value'|'Ratio'|'Minus'|number>>
export type BoxSizeIn = N | BoxSize | `${number}%` | `${number}!` | BoxSizeKindStr; //!< {N|BoxSize|'50%'|'100!'|BoxSizeKindStr}

/**
 * @class BoxOrigin
*/
export class BoxOrigin extends Base<BoxSize> {
	readonly value: N; //!<
	readonly kind: BoxOriginKind; //!<
};
type BoxOriginKindStr = FillSizeKindStr; //!<
initDefaults(BoxOrigin, { value: 0, kind: FillSizeKind.Value });
export type BoxOriginIn = N | BoxOrigin | `${number}%` | BoxOriginKindStr; //!< {N|BoxOrigin|'50%'|BoxOriginKindStr}

const TextBase_toString = [
	()=>'inherit',
	()=>'default',
	(v: any)=>v+'',
	()=>'unknown',
];

/**
 * @class TextBase
*/
class TextBase<Derived,Value> extends Base<Derived> {
	readonly value: Value; //!<
	readonly kind: TextValueKind; //!<
	toString() {
		return (TextBase_toString[this.kind] || TextBase_toString[3])(this.value);
	}
}

/**
 * @class TextColor
*/
export class TextColor extends TextBase<TextColor,Color> {
	get r() { return this.value.r; } //!< {N}
	get g() { return this.value.g; } //!< {N}
	get b() { return this.value.b; } //!< {N}
	get a() { return this.value.a; } //!< {N}
}
initDefaults(TextColor, { value: new Color(0,0,0,255), kind: TextValueKind.Inherit });
type TextValueKindInStr = Uncapitalize<keyof RemoveField<typeof TextValueKind, 'Value'|number>>; //!< {'inherit'|'default'}
export type TextColorIn = TextValueKindInStr | ColorIn | TextColor; //!<

/**
 * @class
*/
export class TextSize extends TextBase<TextSize,N> {}
initDefaults(TextSize, { value: 0, kind: TextValueKind.Inherit });
export type TextSizeIn = TextValueKindInStr | N | TextSize; //!<
export type TextLineHeightIn = TextSizeIn; //!<
export type TextLineHeight = TextSize; //!<
export const TextLineHeight = TextSize;

/**
 * @class
*/
export class TextShadow extends TextBase<TextShadow,Shadow> {
	get x() { return this.value.x } //!< {N}
	get y() { return this.value.y } //!< {N}
	get size() { return this.value.size } //!< {N}
	get color() { return this.value.color } //!< {Color}
	get r() { return this.color.r; } //!< {N}
	get g() { return this.color.g; } //!< {N}
	get b() { return this.color.b; } //!< {N}
	get a() { return this.color.a; } //!< {N}
}
initDefaults(TextShadow, { value: new Shadow, kind: TextValueKind.Inherit });
export type TextShadowIn = TextValueKindInStr | ShadowIn | TextShadow; //!<

/**
 * @class
*/
export class TextStroke extends TextBase<TextStroke,Border> {
	get width() { return this.value.width } //!< {N}
	get color() { return this.value.color } //!< {Color}
	get r() { return this.color.r; } //!< {N}
	get g() { return this.color.g; } //!< {N}
	get b() { return this.color.b; } //!< {N}
	get a() { return this.color.a; } //!< {N}
}
initDefaults(TextStroke, { value: new Border, kind: TextValueKind.Inherit });
export type TextStrokeIn = TextValueKindInStr | BorderIn | TextStroke; //!<

export type FFID = Uint8Array; //!<
/**
 * @class
*/
export class TextFamily extends TextBase<TextFamily,FFID> {
	toString() {
		if (this.kind == TextValueKind.Value) {
			// let _ = this.value;
			// let high = _[0] << 24 | _[1] << 16 | _[2] << 8 | _[3];
			// let low = _[4] << 24 | _[5] << 16 | _[6] << 8 | _[7];
			// return `ffid(${high},${low})`;
			return this.families();
		} else {
			return (TextBase_toString[this.kind] || TextBase_toString[3])(this.value);
		}
	}
	/**
	 * @method families()
	 * @return {string}
	*/
	families(): string {
		let isNullptr = this.value.every(e=>!e);
		return isNullptr ? '': _font.getFamiliesName(this.value);
	}
}
export const EmptyFFID = new Uint8Array([0,0,0,0,0,0,0,0]); //!< {FFID}
initDefaults(TextFamily, { value: EmptyFFID, kind: TextValueKind.Inherit });
export type TextFamilyIn = TextValueKindInStr | string | TextFamily; //!<

/**
 * @class BoxFilter
 * @abstract
*/
export declare abstract class BoxFilter {
	readonly type: N; //!<
	next: BoxFilter | null; //!<
}

type BoxFilterInStr = `image(${string})`|`radial(${string})`|`linear(${string})`;

/**
 * @type BoxFilterIn:string|string[]|BoxFilter
 * @example
 * ```
 * image(res/image.png, auto 100%, x=start, y=20%, repeat)
 * 
 * radial(#ff00ff 0%, #ff0 50%, #00f 100%)
 * 
 * linear(90, #ff00ff 0%, #ff0 50%, #00f 100%)
 * ```
 */
export type BoxFilterIn = BoxFilterInStr | BoxFilterInStr[] | BoxFilter | null;

/**
 * @class FillImage
*/
export declare class FillImage extends BoxFilter {
	readonly src: string; //!<
	readonly width: FillSize; //!<
	readonly height: FillSize; //!<
	readonly x: FillPosition; //!<
	readonly y: FillPosition; //!<
	readonly repeat: Repeat; //!<
	/**
	 * @method constructor(src,init?)
	 * @param src:string
	 * @param init?:object
	*/
	constructor(src: string, init?: {
		width?: FillSize, height?: FillSize,
		x?: FillPosition, y?: FillPosition, repeat?: Repeat,
	});
}

/**
 * @class FillGradientRadial
*/
export declare class FillGradientRadial extends BoxFilter {
	readonly positions: N[]; //!<
	readonly colors: Color[]; //!<
	constructor(pos: N[], colors: Color[]); //!<
}

/**
 * @class FillGradientLinear
 * @extends FillGradientRadial
*/
export declare class FillGradientLinear extends FillGradientRadial {
	readonly angle: N; //!<
	constructor(pos: N[], coloes: Color[], angle: N); //!<
}

/**
 * @class BoxShadow
*/
export declare class BoxShadow extends BoxFilter {
	readonly value: Shadow; //!<
	constructor(value: Shadow); //!<
}

/**
 * @example
 *```
 * 10 10 2 #ff00aa
 * 
 * ['10 10 2 #ff00aa']
 * 
 * 10 10 2 rgba(255,255,0,1)
 *```
 */
export type BoxShadowIn = ShadowIn | ShadowIn[] | BoxShadow;

/**
 * @class SkeletonData
*/
export declare abstract class SkeletonData {
	static Make(skelPath: string, atlasPath?: string, scale?: Float): SkeletonData; //!<
	static Make(skel: Uint8Array, atlasPath: string, scale?: Float): SkeletonData; //!<
	static Make(skel: Uint8Array, atlas: Uint8Array, dir: string, scale?: Float): SkeletonData; //!<
}

/**
 * @example
 * ```
 * skel(res/alien/image.skel, res/alien/image.atlas, scale=1)
 * ```
 */
export type SkeletonDataIn = `skel(${string})` | SkeletonData | null;

// -------------------------------------------------------------------------------------

/**
 * @method newColor(r:N,g:N,b:N,a:N)Color
*/
export function newColor(r: N, g: N, b: N, a: N) {
	return newBase(Color, {r,g,b,a});
}

/**
 * @method newVec2(x:N,y:N)Vec2
*/
export function newVec2(x: N, y: N) {
	return newBase(Vec2, {x,y});
}

/**
 * @method newVec3(x:N,y:N,z:N)Vec3
*/
export function newVec3(x: N, y: N, z: N) {
	return newBase(Vec3, { x, y, z });
}

/**
 * @method newVec4(x:N,y:N,z:N,w:N)Vec4
*/
export function newVec4(x: N, y: N, z: N, w: N) {
	return newBase(Vec4, { x, y, z, w});
}

/**
 * @method newRect(x:N,y:N,width:N,height:N)Rect
*/
export function newRect(x: N, y: N, width: N, height: N) {
	return newBase(Rect, {begin: newVec2(x,y),size: newVec2(width,height)});
}

/**
 * @method newRange(x:N,y:N,width:N,height:N)Range
*/
export function newRange(x: N, y: N, x1: N, y1: N) {
	return newBase(Range, {begin: newVec2(x,y), end: newVec2(x1,y1)});
}

/**
 * @method newRegion(x:N,y:N,width:N,height:N)Region
*/
export function newRegion(x: N, y: N, x1: N, y1: N, ox: N = 0, oy: N = 0) {
	return newBase(Region, {begin: newVec2(x,y), end: newVec2(x1,y1), origin: newVec2(ox,oy)});
}

/**
 * @method newBorderRadius(p0:N,p1:N,p2:N,p3:N,p4:N,p5:N,p6:N,p7:N)BorderRadius
*/
export function newBorderRadius(
	p0: N, p1: N,
	p2: N,p3: N,
	p4: N, p5: N,
	p6: N,p7: N
) {
	return newBase(BorderRadius, {
		leftTop: newVec2(p0, p1),
		rightTop: newVec2(p2, p3),
		rightBottom: newVec2(p4, p5),
		leftBottom: newVec2(p6, p7)
	});
}

/**
 * @method newMat(...value)Mat
 * @param value:N[]
*/
export function newMat(...value: N[]) {
	let mat = newBase(Mat, {});
	for (let i: 0 = 0; i < 6; i++)
		(mat as RemoveReadonly<Mat>)[`m${i}`] = value[i];
	return mat;
}

/**
 * @method newMat4(...value)Mat4
 * @param value:N[]
*/
export function newMat4(...value: N[]) {
	let mat = newBase(Mat4, {});
	for (let i: 0 = 0; i < 16; i++)
		(mat as RemoveReadonly<Mat4>)[`m${i}`] = value[i];
	return mat;
}

/**
 * @method newCurve(p1x:N,p1y:N,p2x:N,p2y:N)Curve
*/
export function newCurve(p1x: N, p1y: N, p2x: N, p2y: N) {
	return newBase(Curve, { p1: newVec2(p1x,p1y), p2: newVec2(p2x,p2y) });
}

/**
 * @method newShadow(x:N,y:N,size:N,r:N,g:N,b:N,a:N)Shadow
*/
export function newShadow(
	x: N, y: N, size: N,
	r: N, g: N, b: N, a: N
) {
	return newBase(Shadow, {x,y,size,color: newColor(r, g, b, a)});
}

/**
 * @method newBorder(width:N,r:N,b:N,a:N)Border
*/
export function newBorder(
	width: N, r: N, g: N, b: N, a: N
) {
	return newBase(Border, {width,color: newColor(r, g, b, a)});
}

/**
 * @method newFillPosition(kink:FillPositionKind,value:N)FillPosition
*/
export function newFillPosition(kind: FillPositionKind, value: N) {
	return newBase(FillPosition, {kind,value});
}

/**
 * @method newFillSize(kink:FillSizeKind,value:N)FillSize
*/
export function newFillSize(kind: FillSizeKind, value: N) {
	return newBase(FillSize, {kind,value});
}

/**
 * @method newBoxOrigin(kink:BoxOriginKind,value:N)BoxOrigin
*/
export function newBoxOrigin(kind: BoxOriginKind, value: N) {
	return newBase(BoxOrigin, {kind,value});
}

/**
 * @method newBoxSize(kink:BoxSizeKind,value:N)BoxSize
*/
export function newBoxSize(kind: BoxSizeKind, value: N) {
	return newBase(BoxSize, { kind, value });
}

/**
 * @method newTextColor(kink:TextValueKind,r:N,g:N,b:N,a:N)TextColor
*/
export function newTextColor(kind: TextValueKind, r: N, g: N, b: N, a: N) {
	return newBase(TextColor, { kind, value: newColor(r, g, b, a) });
}

/**
 * @method newTextSize(kink:TextValueKind,value:N)TextSize
*/
export function newTextSize(kind: TextValueKind, value: N) {
	return newBase(TextSize, { kind, value });
}

/**
 * @method newTextLineHeight(kink:TextValueKind,value:N)TextLineHeight
*/
export const newTextLineHeight = newTextSize;

/**
 * @method newTextShadow(kind:TextValueKind,offset_x:N,offset_y:N,size:N,r:N,g:N,b:N,a:N)TextShadow
*/
export function newTextShadow(
	kind: TextValueKind, offset_x: N, offset_y: N, size: N, r: N, g: N, b: N, a: N
) {
	return newBase(TextShadow,{kind,value: newShadow(offset_x,offset_y,size,r,g,b,a)});
}

/**
 * @method newTextStroke(kind:TextValueKind,width:N,r:N,g:N,b:N,a:N)TextStroke
*/
export function newTextStroke(
	kind: TextValueKind, width: N, r: N, g: N, b: N, a: N
) {
	return newBase(TextStroke,{kind,value: newBorder(width,r,g,b,a)});
}

/**
 * @method newTextFamily(kind:TextValueKind,ffid?:FFID)TextFamily
*/
export function newTextFamily(kind: TextValueKind, ffid: FFID = EmptyFFID) {
	return newBase(TextFamily,{kind,value:ffid});
}

// parse
// -------------------------------------------------------------------------------------

export function parseRepeat(val: RepeatIn, desc?: string): Repeat { //!<
	return typeof val === 'string' ?
		Repeat[toCapitalize(val)] || 0: val in Repeat ? val: 0;
	// throw error(str, desc, undefined, Repeat);
}

export function parseDirection(val: DirectionIn, desc?: string): Direction { //!<
	return typeof val === 'string' ?
		Direction[toCapitalize(val)] || 0: val in Direction ? val: 0;
}

export function parseItemsAlign(val: ItemsAlignIn, desc?: string): ItemsAlign { //!<
	return typeof val === 'string' ?
		ItemsAlign[toCapitalize(val)] || 0:
		val in ItemsAlign ? val: 0;
}

export function parseCrossAlign(val: CrossAlignIn, desc?: string): CrossAlign { //!<
	return typeof val === 'string' ?
		CrossAlign[toCapitalize(val)] || 1 : val in CrossAlign ? val: 1;
}

export function parseWrap(val: WrapIn, desc?: string): Wrap { //!<
	return typeof val === 'string' ?
		Wrap[toCapitalize(val)] || 0 : val in Wrap ? val: 0;
}

export function parseWrapAlign(val: WrapAlignIn, desc?: string): WrapAlign { //!<
	return typeof val === 'string' ?
		WrapAlign[toCapitalize(val)] || 0 : val in WrapAlign ? val: 0;
}

export function parseAlign(val: AlignIn, desc?: string): Align { //!<
	return typeof val === 'string' ?
		Align[toCapitalize(val)] || 0 : val in Align ? val: 0;
}

export function parseTextAlign(val: TextAlignIn, desc?: string): TextAlign { //!<
	return typeof val === 'string' ?
		TextAlign[toCapitalize(val)] || 0 : val in TextAlign ? val: 0;
}

export function parseTextDecoration(val: TextDecorationIn, desc?: string): TextDecoration { //!<
	return typeof val === 'string' ?
		TextDecoration[toCapitalize(val)] || 0 : val in TextDecoration ? val: 0;
}

export function parseTextOverflow(val: TextOverflowIn, desc?: string): TextOverflow { //!<
	return typeof val === 'string' ?
		TextOverflow[toCapitalize(val)] || 0 : val in TextOverflow ? val: 0;
}

export function parseTextWhiteSpace(val: TextWhiteSpaceIn, desc?: string): TextWhiteSpace { //!<
	return typeof val === 'string' ?
		TextWhiteSpace[toCapitalize(val)] || 0 : val in TextWhiteSpace ? val: 0;
}

export function parseTextWordBreak(val: TextWordBreakIn, desc?: string): TextWordBreak { //!<
	return typeof val === 'string' ?
		TextWordBreak[toCapitalize(val)] || 0 : val in TextWordBreak ? val: 0;
}

export function parseTextWeight(val: TextWeightIn, desc?: string): TextWeight { //!<
	return typeof val === 'string' ?
		TextWeight[toCapitalize(val)] || 0 : val in TextWeight ? val: 0;
}

export function parseTextWidth(val: TextWidthIn, desc?: string): TextWidth { //!<
	return typeof val === 'string' ?
		TextWidth[toCapitalize(val)] || 0 : val in TextWidth ? val: 0;
}

export function parseTextSlant(val: TextSlantIn, desc?: string): TextSlant { //!<
	return typeof val === 'string' ?
		TextSlant[toCapitalize(val)] || 0 : val in TextSlant ? val: 0;
}

export function parseKeyboardType(val: KeyboardTypeIn, desc?: string): KeyboardType { //!<
	return typeof val === 'string' ?
		KeyboardType[toCapitalize(val)] || 0 : val in KeyboardType ? val: 0;
}

export function parseKeyboardReturnType(val: KeyboardReturnTypeIn, desc?: string): KeyboardReturnType { //!<
	return typeof val === 'string' ?
		KeyboardReturnType[toCapitalize(val)] || 0 : val in KeyboardReturnType ? val: 0;
}

export function parseCursorStyle(val: CursorStyleIn, desc?: string): CursorStyle { //!<
	return typeof val === 'string' ?
		CursorStyle[toCapitalize(val)] || 0 : val in CursorStyle ? val: 0;
}

export function parseCascadeColor(val: CascadeColorIn, desc?: string): CascadeColor { //!<
	// if (typeof val === 'string')
	// 	val = CascadeColor[toCapitalize(val)];
	// return val in CascadeColor ? val: CascadeColor.Alpha;
	return typeof val === 'string' ?
		CascadeColor[toCapitalize(val)] || 0 : val in CascadeColor ? val: 0;
}

const vec2Reg = [
	/^\s*(-?(?:\d+)?\.?\d+)\s+(-?(?:\d+)?\.?\d+)\s*$/,
	/^\s*vec2\(\s*(-?(?:\d+)?\.?\d+)\s*,\s*(-?(?:\d+)?\.?\d+)\s*\)\s*$/,
];
export function parseVec2(val: Vec2In, desc?: string): Vec2 { //!<
	if (typeof val === 'string') {
		let m = val.match(vec2Reg[0]) || val.match(vec2Reg[1]);
		if (m) {
			return newVec2(parseFloat(m[1]), parseFloat(m[2]));
		}
	} else if (typeof val === 'number') {
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
export function parseVec3(val: Vec3In, desc?: string): Vec3 { //!<
	if (typeof val === 'string') {
		let m = val.match(vec3Reg[0]) || val.match(vec3Reg[1]);
		if (m)
			return newVec3(parseFloat(m[1]), parseFloat(m[2]), parseFloat(m[3]));
	} else if (typeof val === 'number') {
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
export function parseVec4(val: Vec4In, desc?: string): Vec4 { //!<
	if (typeof val === 'string') {
		let m = val.match(vec4Reg[0]) || val.match(vec4Reg[1]);
		if (m)
			return newVec4(parseFloat(m[1]), parseFloat(m[2]), parseFloat(m[3]), parseFloat(m[4]));
	} else if (typeof val === 'number') {
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
export function parseCurve(val: CurveIn, desc?: string): Curve { //!<
	if (typeof val === 'string') {
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
export function parseRect(val: RectIn, desc?: string): Rect { //!<
	if (typeof val === 'string') {
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

export function parseRange(val: RangeIn, desc?: string): Range { //!<
	if (val instanceof Range) {
		return val;
	}
	throw error(val, desc);
}

export function parseRegion(val: RegionIn, desc?: string): Region { //!<
	if (val instanceof Region) {
		return val;
	}
	throw error(val, desc);
}

export function parseBorderRadius(val: BorderRadiusIn, desc?: string): BorderRadius { //!<
	if (val instanceof BorderRadius) {
		return val;
	}
	throw error(val, desc);
}

const matReg = new RegExp(`^\s*mat\\(\s*${new Array(6).join(
	'(-?(?:\\d+)?\\.?\\d+)\s*,\s*')}(-?(?:\\d+)?\\.?\\d+)\s*\\)\s*$`
);
export function parseMat(val: MatIn, desc?: string): Mat { //!<
	if (typeof val === 'string') {
		let m = val.match(matReg);
		if (m) {
			return newMat(...[
				parseFloat(m[1]), parseFloat(m[2]), parseFloat(m[3]),
				parseFloat(m[4]), parseFloat(m[5]), parseFloat(m[6]),
			]);
		}
	} else if (typeof val === 'number') {
		return newMat(val,0,0,0,val,0);
	} else if (val instanceof Mat) {
		return val;
	} 
	throw error(val, desc, ['mat(1,0,0,1,0,1)']);
}

const mat4Reg = new RegExp(`^\s*mat4\\(\s*${new Array(16).join(
	'(-?(?:\\d+)?\\.?\\d+)\s*,\s*')}(-?(?:\\d+)?\\.?\\d+)\s*\\)\s*$`
);
export function parseMat4(val: Mat4In, desc?: string): Mat4 { //!<
	if (typeof val === 'string') {
		let m = mat4Reg.exec(val);
		if (m) {
			return newMat4(...[
				parseFloat(m[1]), parseFloat(m[2]), parseFloat(m[3]), parseFloat(m[4]),
				parseFloat(m[5]), parseFloat(m[6]), parseFloat(m[7]), parseFloat(m[8]),
				parseFloat(m[9]), parseFloat(m[10]), parseFloat(m[11]), parseFloat(m[12]),
				parseFloat(m[13]), parseFloat(m[14]), parseFloat(m[15]), parseFloat(m[16]),
			]);
		}
	} else if (typeof val === 'number') {
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
	/^\s*rgb(a)?\(\s*(\d{1,3})\s*,\s*(\d{1,3})\s*,\s*(\d{1,3})(\s*,\s*(0?\.\d+|1))?\s*\)\s*$/,
];
function parseColorNumber(val: number) {
	return newColor(val >> 24 & 255, // r
	val >> 16 & 255, // g
	val >> 8 & 255, // b
	val >> 0 & 255); // a
}
export function parseColor(val: ColorIn, desc?: string, ref?: Reference): Color { //!<
	if (typeof val === 'string') {
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
						parseInt(m[2]) % 256, parseInt(m[3]) % 256, parseInt(m[4]) % 256, parseFloat(m[6]) * 255
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
	}
	else if (typeof val === 'number') {
		return parseColorNumber(val);
	}
	else if (val instanceof Color) {
		return val;
	}
	throw error(val, desc, [
		'rgba(255,255,255,1)', 'rgb(255,255,255)', '#ff0', '#ff00', '#ff00ff', '#ff00ffff'
	], ref);
}

const ShadowReg = /^\s*(-?(?:\d+)?\.?\d+)\s+(-?(?:\d+)?\.?\d+)\s+((?:\d+)?\.?\d+)/;
export function parseShadow(val: ShadowIn, desc?: string, ref?: Reference): Shadow { //!<
	if (typeof val === 'string') {
		// 10 10 2 #ff00aa
		let m = val.match(ShadowReg);
		if (m) {
			return new Shadow({
				x: parseFloat(m[1]),
				y: parseFloat(m[2]),
				size: parseFloat(m[3]),
				color: parseColor(val.substring(m[0].length + 1) as ColorStrIn, desc, r=>{
					return (ref||(r=>r))(['10 10 2 #ff00aa', '10 10 2 rgba(255,255,0,1)']);
				}),
			});
		}
	} else if (val instanceof Shadow) {
		return val;
	}
	throw error(val, desc, ['10 10 2 #ff00aa', '10 10 2 rgba(255,255,0,1)'], ref);
}

const BorderReg = /^\s*(-?(?:\d+)?\.?\d+)/;
export function parseBorder(val: BorderIn, desc?: string, ref?: Reference): Border { //!<
	if (typeof val === 'string') {
		// 10 #ff00aa
		let m = val.match(BorderReg);
		if (m) {
			return new Border({
				width: parseFloat(m[1]),
				color: parseColor(val.substring(m[0].length + 1) as ColorStrIn, desc, r=>{
					return (ref||(r=>r))(['10 #ff00aa', '10 rgba(255,255,0,1)']);
				}),
			});
		}
	} else if (val instanceof BoxShadow) {
		return val;
	}
	throw error(val, desc, ['10 #ff00aa', '10 rgba(255,255,0,1)']);
}

export function parseFillPosition(val: FillPositionIn, desc?: string): FillPosition { //!<
	if (typeof val === 'string') {
		let kind = FillPositionKind[toCapitalize(val as FillPositionKindStr)];
		if (kind !== undefined) {
			return newFillPosition(kind, 0);
		}
		let m = val.match(/^\s*(-?(?:\d+)?\.?\d+)(%)?\s*$/);
		if (m) {
			let kind = m[2] ? FillPositionKind.Ratio: FillPositionKind.Value;
			let val = parseFloat(m[1]);
			return newFillPosition(kind, m[2] ? val * 0.01: val);
		}
	}
	else if (typeof val === 'number') {
		return newFillPosition(FillPositionKind.Value, val);
	}
	else if (val instanceof FillPosition) {
		return val;
	}
	throw error(val, desc, ["start", "end", "center", 10, '20%']);
}

export function parseFillSize(val: FillSizeIn, desc?: string): FillSize { //!<
	if (typeof val === 'string') {
		let kind = FillSizeKind[toCapitalize(val as FillSizeKindStr)];
		if (kind !== undefined) {
			return newFillSize(kind, 0);
		}
		let m = val.match(/^\s*(-?(?:\d+)?\.?\d+)(%)?\s*$/);
		if (m) {
			let kind = m[2] ? FillSizeKind.Ratio: FillSizeKind.Value;
			let val = parseFloat(m[1]);
			return newFillSize(kind, m[2] ? val * 0.01: val);
		}
	}
	else if (typeof val === 'number') {
		return newFillSize(FillSizeKind.Value, val);
	}
	else if (val instanceof FillSize) {
		return val;
	}
	throw error(val, desc, ["auto", 10, '20%']);
}

export function parseBoxOrigin(val: BoxOriginIn, desc?: string): BoxOrigin { //!<
	if (typeof val === 'string') {
		let kind = BoxOriginKind[toCapitalize(val as BoxOriginKindStr)];
		if (kind !== undefined) {
			return newBoxOrigin(kind, 0);
		}
		let m = val.match(/^\s*(-?(?:\d+)?\.?\d+)(%)?\s*$/);
		if (m) {
			let kind = m[2] ? BoxOriginKind.Ratio: BoxOriginKind.Value;
			let val = parseFloat(m[1]);
			return newBoxOrigin(kind, m[2] ? val * 0.01: val);
		}
	}
	else if (typeof val === 'number') {
		return newBoxOrigin(BoxOriginKind.Value, val);
	}
	else if (val instanceof BoxOrigin) {
		return val;
	}
	throw error(val, desc, ["auto", 10, '20%']);
}

export function parseBoxSize(val: BoxSizeIn, desc?: string): BoxSize { //!<
	if (typeof val === 'string') {
		let kind = BoxSizeKind[toCapitalize(val as BoxSizeKindStr)];
		if (kind !== undefined) {
			return newBoxSize(kind, 0);
		}
		let m = val.match(/^\s*(-?(?:\d+)?\.?\d+)(%|!)?\s*$/);
		if (m) {
			let kind = m[2] ? m[2] == '%' ? BoxSizeKind.Ratio: BoxSizeKind.Minus: BoxSizeKind.Value;
			let val = parseFloat(m[1]);
			return newBoxSize(kind, m[2] == '%' ? val * 0.01: val);
		}
	}
	else if (typeof val === 'number') {
		return newBoxSize(BoxSizeKind.Value, val);
	}
	else if (val instanceof BoxSize) {
		return val;
	}
	throw error(val, desc, ["auto", "match", 10, '20%', '60!']);
}

export function parseTextColor(val: TextColorIn, desc?: string): TextColor { //!<
	if (typeof val === 'string') {
		let kind = TextValueKind[toCapitalize(val as TextValueKindInStr)];
		if (kind !== undefined) {
			return newTextColor(kind, 0,0,0,255);
		}
		return new TextColor({
			kind: TextValueKind.Value,
			value: parseColor(val as ColorStrIn, desc, (ref)=>['inherit', 'default', ...ref])
		});
	} else if (typeof val === 'number') {
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
		'rgba(255,255,255,1)', 'rgb(255,255,255)', '#ff0', '#ff00', '#ff00ff', '#ff00ffff'
	]);
}

const TextSizeReg = /^(?:\d+)?\.?\d+$/;
export function parseTextSize(val: TextSizeIn, desc?: string): TextSize { //!<
	if (typeof val === 'string') {
		let kind = TextValueKind[toCapitalize(val as TextValueKindInStr)];
		if (kind !== undefined) {
			return newTextSize(kind, 0);
		}
		if (TextSizeReg.test(val)) {
			return newTextSize(TextValueKind.Value, parseFloat(val));
		}
	} else if (typeof val === 'number') {
		return newTextSize(TextValueKind.Value, val);
	} else if (val instanceof TextSize) {
		return val;
	}
	throw error(val, desc, ['inherit', 'default', 16, '12']);
}

export function parseTextLineHeight(val: TextLineHeightIn, desc?: string): TextLineHeight { //!<
	return parseTextSize(val, desc);
}

export function parseTextShadow(val: TextShadowIn, desc?: string): TextShadow { //!<
	if (typeof val === 'string') {
		let kind = TextValueKind[toCapitalize(val as TextValueKindInStr)];
		if (kind !== undefined) {
			return newTextShadow(kind,0,0,0,0,0,0,0);
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
	throw error(val, desc, ['inherit', 'default', '10 10 2 #ff00aa', '10 10 2 rgba(255,255,0,1)']);
}

export function parseTextStroke(val: TextStrokeIn, desc?: string): TextStroke { //!<
	if (typeof val === 'string') {
		let kind = TextValueKind[toCapitalize(val as TextValueKindInStr)];
		if (kind !== undefined) {
			return newTextStroke(kind,0,0,0,0,0);
		}
		return new TextStroke({
			kind: TextValueKind.Value,
			value: parseBorder(val as BorderIn, desc, ref=>['inherit', 'default', ...ref]),
		});
	} else if (val instanceof Border) {
		return new TextStroke({
			kind: TextValueKind.Value,
			value: val,
		});
	} else if (val instanceof TextStroke) {
		return val;
	}
	throw error(val, desc, ['inherit', 'default', '1 #ff00aa', '2 rgba(255,255,0,1)']);
}

export function parseTextFamily(val: TextFamilyIn, desc?: string): TextFamily { ///!<
	if (typeof val === 'string') {
		let kind = TextValueKind[toCapitalize(val as TextValueKindInStr)];
		if (kind !== undefined) {
			return newTextFamily(kind,EmptyFFID);
		}
		return newTextFamily(TextValueKind.Value, _font.getFontFamilies(val));
	} else if (val instanceof TextFamily) {
		return val;
	}
	throw error(val, desc, ['inherit', 'default', 'Ubuntu Mono']);
}

const parseBoxFilterReference = [
	'image(res/image.png, auto 100%, x=start, y=20%, repeat)',
	'radial(#ff00ff 0%, #ff0 50%, #00f 100%)',
	'linear(90, #ff00ff 0%, #ff0 50%, #00f 100%)',
	// shadow(1, 1, 2, #ff00ff)
];
const parseCmdReg = [
	/^\s*([a-z]+)\s*\(\s*/i, // image(
	/^([^,\)]+)(,\s*|\))/,
	/^"(([^"\\]|\\")+)"\s*(,\s*|\))/,
	/^'(([^'\\]|\\')+)'\s*(,\s*|\))/,
	/\s+/,
	/^([a-z][a-z0-9\_]*)=[^=]?/i
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
				let m2 = arg.match(parseCmdReg[5]);
				if (m2) { // is k/v
					let k = m2[1];
					let v = arg.substring(k.length + 1);
					// if (k.match(parseCmdReg[5])) {
					if (k in kv) {
						kv[k].push(v);
					} else {
						kv[k] = [v];
					}
					// }
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
	args.map(([c,p])=>{
		colors.push(parseColor(c as ColorStrIn, desc, ()=>parseBoxFilterReference));
		pos.push(p ? (Number(p.substring(0, p.length - 1)) || 0) * 0.01: 0);
	});
	return {pos,colors};
}

function parseBoxFilterItem(val: string, desc?: string): BoxFilter {
	const FillImage_ = exports.FillImage as typeof FillImage;
	const FillGradientLinear_ = exports.FillGradientLinear as typeof FillGradientLinear;
	const FillGradientRadial_ = exports.FillGradientRadial as typeof FillGradientRadial;
	if (typeof val === 'string') {
		let cmd = parseCmd(val);
		if (cmd) {
			let {val,args,kv} = cmd;
			switch(val) {
				case 'image': {
					let width, height, repeat;
					if (args[1]) {
						var [w,h] = args[1];
						width = parseFillSize(w as FillSizeIn, desc);
						if (h)
							height = parseFillSize(h as FillSizeIn, desc);
						if (args[2])
							repeat = parseRepeat(args[2][0] as RepeatIn, desc);
					}
					return new FillImage_(String(args[0][0]), {
						width, height, repeat,
						x: kv.x && parseFillPosition(kv.x[0], desc),
						y: kv.y && parseFillPosition(kv.y[0], desc),
					});
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

export function parseBoxFilter(val: BoxFilterIn, desc?: string): BoxFilter | null { ///!<
	const BoxFilter_ = exports.BoxFilter as typeof BoxFilter;
	if (val instanceof BoxFilter_) {
		return val;
	} else if (Array.isArray(val)) {
		val = val.filter(e=>e);
		if (val.length)
			return linkFilter(val, val.map(e=>parseBoxFilterItem(e)), desc, ()=>parseBoxFilterReference);
	} else if (val) {
		return parseBoxFilterItem(val, desc);
	}
	return null;
}

const parseBoxShadowRef = [
	'10 10 2 #ff00aa', ['10 10 2 #ff00aa'], '10 10 2 rgba(255,255,0,1)'
];
export function parseBoxShadow(val: BoxShadowIn, desc?: string): BoxShadow { ///!<
	const BoxShadow_ = exports.BoxShadow as typeof BoxShadow;
	if (val instanceof BoxShadow_) {
		return val;
	} else if (Array.isArray(val)) {
		return linkFilter(val,
			val.map(e=>new BoxShadow_(parseShadow(e, desc, ()=>parseBoxShadowRef))),
			desc, ()=>parseBoxShadowRef
		);
	} else {
		let s = parseShadow(val, desc, ()=>parseBoxShadowRef);
		return new BoxShadow_(s);
	}
}

const parseSkeletonDataRef = [
	'skel(res/alien/image.skel, res/alien/image.atlas, scale=1)'
];
export function parseSkeletonData(val: SkeletonDataIn, desc?: string): SkeletonData | null { ///!<
	const SkeletonData_ = exports.SkeletonData as typeof SkeletonData;
	if (val instanceof SkeletonData_) {
		return val;
	} else if (typeof val === 'string') {
		let cmd = parseCmd(val);
		if (cmd && cmd.val == 'skel') {
			let {args:[a,b],kv:{scale}} = cmd;
			return SkeletonData_.Make(a[0], b ? b[0]: '', scale ? scale[0]: 1);
		}
	} else if (!val) {
		return null;
	}
	throw error(val, desc, parseSkeletonDataRef);
}

/**
 * @type parseBoxFilterPtr:parseBoxFilter
*/
export const parseBoxFilterPtr = parseBoxFilter;

/**
 * @type parseBoxShadowPtr:parseBoxShadow
*/
export const parseBoxShadowPtr = parseBoxShadow;

/**
 * @type parseSkeletonDataPtr:parseSkeletonData
*/
export const parseSkeletonDataPtr = parseSkeletonData;