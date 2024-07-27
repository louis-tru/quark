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

import { Mv, LOG } from './tool'
import * as v from 'quark/types';

LOG('\nTEST Types:\n')
// export declare function newColor(r: N, g: N, b: N, a: N): Color;
// export declare function newVec2(x: N, y: N): Vec2;
// export declare function newVec3(x: N, y: N, z: N): Vec3;
// export declare function newVec4(x: N, y: N, z: N, w: N): Vec4;
// export declare function newRect(x: N, y: N, width: N, height: N): Rect;
// export declare function newMat(...value: N[]): Mat;
// export declare function newMat4(...value: N[]): Mat4;
// export declare function newCurve(p1x: N, p1y: N, p2x: N, p2y: N): Curve;
// export declare function newShadow(x: N, y: N, size: N, r: N, g: N, b: N, a: N): Shadow;
// export declare function newBoxBorder(width: N, r: N, g: N, b: N, a: N): BoxBorder;
// export declare function newFillPosition(kind: FillPositionKind, value: N): FillPosition;
// export declare function newFillSize(kind: FillSizeKind, value: N): FillSize;
// export declare function newBoxOrigin(kind: BoxOriginKind, value: N): BoxOrigin;
// export declare function newBoxSize(kind: BoxSizeKind, value: N): BoxSize;
// export declare function newTextColor(kind: TextValueKind, r: N, g: N, b: N, a: N): TextColor;
// export declare function newTextSize(kind: TextValueKind, value: N): TextSize;
// export declare const    newTextLineHeight: typeof newTextSize;
// export declare function newTextShadow(kind: TextValueKind, offset_x: N, offset_y: N, size: N, r: N, g: N, b: N, a: N): TextShadow;
// export declare function newTextFamily(kind: TextValueKind, ffid: Uint8Array): TextFamily;

LOG('\nTEST Parse Function:\n')
// export declare function parseRepeat(val: RepeatIn, desc?: string): Repeat;
// export declare function parseDirection(val: DirectionIn, desc?: string): Direction;
// export declare function parseItemsAlign(val: ItemsAlignIn, desc?: string): ItemsAlign;
// export declare function parseCrossAlign(val: CrossAlignIn, desc?: string): CrossAlign;
// export declare function parseWrap(val: WrapIn, desc?: string): Wrap;
// export declare function parseWrapAlign(val: WrapAlignIn, desc?: string): WrapAlign;
// export declare function parseAlign(val: AlignIn, desc?: string): Align;
// export declare function parseTextAlign(val: TextAlignIn, desc?: string): TextAlign;
// export declare function parseTextDecoration(val: TextDecorationIn, desc?: string): TextDecoration;
// export declare function parseTextOverflow(val: TextOverflowIn, desc?: string): TextOverflow;
// export declare function parseTextWhiteSpace(val: TextWhiteSpaceIn, desc?: string): TextWhiteSpace;
// export declare function parseTextWordBreak(val: TextWordBreakIn, desc?: string): TextWordBreak;
// export declare function parseTextWeight(val: TextWeightIn, desc?: string): TextWeight;
// export declare function parseTextWidth(val: TextWidthIn, desc?: string): TextWidth;
// export declare function parseTextSlant(val: TextSlantIn, desc?: string): TextSlant;
// export declare function parseKeyboardType(val: KeyboardTypeIn, desc?: string): KeyboardType;
// export declare function parseKeyboardReturnType(val: KeyboardReturnTypeIn, desc?: string): KeyboardReturnType;
// export declare function parseCursorStyle(val: CursorStyleIn, desc?: string): CursorStyle;
// export declare function parseFindDirection(val: FindDirectionIn, desc?: string): FindDirection;
// export declare function parseVec2(val: Vec2In, desc?: string): Vec2;
// export declare function parseVec3(val: Vec3In, desc?: string): Vec3;
// export declare function parseVec4(val: Vec4In, desc?: string): Vec4;
// export declare function parseCurve(val: CurveIn, desc?: string): Curve;
// export declare function parseRect(val: RectIn, desc?: string): Rect;
// export declare function parseMat(val: MatIn, desc?: string): Mat;
// export declare function parseMat4(val: Mat4In, desc?: string): Mat4;
// export declare function parseColor(val: ColorIn, desc?: string, ref?: Reference): Color;
// export declare function parseShadow(val: ShadowIn, desc?: string, ref?: Reference): Shadow;
// export declare function parseBoxBorder(val: BoxBorderIn, desc?: string): BoxBorder;
// export declare function parseFillPosition(val: FillPositionIn, desc?: string): FillPosition;
// export declare function parseFillSize(val: FillSizeIn, desc?: string): FillSize;
// export declare function parseBoxOrigin(val: BoxOriginIn, desc?: string): BoxOrigin;
// export declare function parseBoxSize(val: BoxSizeIn, desc?: string): BoxSize;
// export declare function parseTextColor(val: TextColorIn, desc?: string): TextColor;
// export declare function parseTextSize(val: TextSizeIn, desc?: string): TextSize;
// export declare function parseTextLineHeight(val: TextLineHeightIn, desc?: string): TextSize;
// export declare function parseTextShadow(val: TextShadowIn, desc?: string): TextShadow;
// export declare function parseTextFamily(val: TextFamilyIn, desc?: string): TextFamily;
// export declare function parseBoxFilter(val: BoxFilterIn, desc?: string): BoxFilter;
// export declare function parseBoxShadow(val: BoxShadowIn, desc?: string): BoxShadow;

Mv(v, 'parseRepeat', ['repeatNo'], v.Repeat.RepeatNo);
Mv(v, 'parseDirection', ['row'], v.Direction.Row);
Mv(v, 'parseTextAlign', ['left'], v.TextAlign.Left);
Mv(v, 'parseAlign', ['center'], v.Align.Center);
Mv(v, 'parseKeyboardType', ['ascii'], v.KeyboardType.Ascii);
Mv(v, 'parseKeyboardReturnType', ['route'], v.KeyboardReturnType.Route);
Mv(v, 'parseShadow', ['10 10 5 #f00']);
Mv(v, 'parseColor', ['rgba(0,0,100,255)']);
Mv(v, 'parseVec2', ['10 9']);
Mv(v, 'parseVec3', ['10 0 4']);
Mv(v, 'parseVec4', ['vec4(4,5,6,7)']);
Mv(v, 'parseCurve', ['curve(0,0,1,1)']);
Mv(v, 'parseRect', ['rect(0,0,100,200)']);
Mv(v, 'parseMat', ['mat(1,0,0,0,1,0)']);
Mv(v, 'parseMat4', ['mat4(1,0,0,300.909,0,1,0,0,200,0,1,100,0,0,0,1)']);
Mv(v, 'parseBoxSize', ['match']);
Mv(v, 'parseTextColor', ['#f00']);
Mv(v, 'parseTextSize', ['inherit']);
Mv(v, 'parseTextFamily', ['inherit']);
Mv(v, 'parseTextWeight', ['light']);
Mv(v, 'parseTextWidth', ['ultraExpanded']);
Mv(v, 'parseTextSlant', ['italic']);
Mv(v, 'parseTextShadow', ['10 10 10 #00f']);
Mv(v, 'parseTextLineHeight', [24]);
Mv(v, 'parseTextDecoration', ['overline']);
Mv(v, 'parseTextOverflow', ['ellipsisCenter']);
Mv(v, 'parseTextWhiteSpace', ['noWrap']);
