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

export default async function(_: any) {
	LOG('\nTEST Types:\n')
	Mv(v, 'newColor', [255,255,255,255])
	Mv(v, 'newVec2', [1,1])
	Mv(v, 'newVec3', [1,1,1])
	Mv(v, 'newVec4', [1,1,1,1])
	Mv(v, 'newRect', [1,1,1,1])
	Mv(v, 'newMat', [1,0,0,0,1,0])
	Mv(v, 'newMat4', [1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1])
	Mv(v, 'newCurve', [0,0,1,1])
	Mv(v, 'newShadow', [1,1,1,255,255,255,255])
	Mv(v, 'newBoxBorder', [1,1,1,1,1])
	Mv(v, 'newFillPosition', [v.FillPositionKind.Center, 1])
	Mv(v, 'newFillSize', [v.FillSizeKind.Auto, 0])
	Mv(v, 'newBoxOrigin', [v.BoxOriginKind.Ratio, 0])
	Mv(v, 'newBoxSize', [v.BoxSizeKind.Match, 0])
	Mv(v, 'newTextColor', [v.TextValueKind.Value, 1,1,1,1])
	Mv(v, 'newTextSize', [v.TextValueKind.Inherit, 0])
	Mv(v, 'newTextLineHeight', [v.TextValueKind.Inherit, 0])
	Mv(v, 'newTextShadow', [v.TextValueKind.Value, 1,1,1,1,1,1,1])
	Mv(v, 'newTextFamily', [v.TextValueKind.Inherit, v.EmptyFFID])

	LOG('\nTEST Parse Function:\n')
	Mv(v, 'parseRepeat', ['noRepeat'], v.Repeat.NoRepeat)
	Mv(v, 'parseDirection', ['row'], v.Direction.Row)
	Mv(v, 'parseItemsAlign', ['center'], v.ItemsAlign.Center)
	Mv(v, 'parseCrossAlign', ['start'], v.CrossAlign.Start)
	Mv(v, 'parseWrap', ['noWrap'], v.Wrap.NoWrap)
	Mv(v, 'parseWrapAlign', ['spaceAround'], v.WrapAlign.SpaceAround)
	Mv(v, 'parseAlign', ['center'], v.Align.Center)
	Mv(v, 'parseTextAlign', ['left'], v.TextAlign.Left)
	Mv(v, 'parseTextDecoration', ['overline'], v.TextDecoration.Overline)
	Mv(v, 'parseTextOverflow', ['ellipsis'], v.TextOverflow.Ellipsis)
	Mv(v, 'parseTextWhiteSpace', ['pre'], v.TextWhiteSpace.Pre)
	Mv(v, 'parseTextWordBreak', ['breakAll'], v.TextWordBreak.BreakAll)
	Mv(v, 'parseTextWeight', ['thin'], v.TextWeight.Thin)
	Mv(v, 'parseTextWidth', ['extraExpanded'], v.TextWidth.ExtraExpanded)
	Mv(v, 'parseTextSlant', ['italic'], v.TextSlant.Italic)
	Mv(v, 'parseKeyboardType', ['numberPad'], v.KeyboardType.NumberPad)
	Mv(v, 'parseKeyboardReturnType', ['join'], v.KeyboardReturnType.Join)
	Mv(v, 'parseCursorStyle', ['ibeam'], v.CursorStyle.Ibeam)
	Mv(v, 'parseFindDirection', ['right'], v.FindDirection.Right)
	Mv(v, 'parseVec2', [`1 1`], e=>e.x==1&&e.y==1)
	Mv(v, 'parseVec3', [`1 1 1`], e=>e.x==1&&e.y==1&&e.z==1)
	Mv(v, 'parseVec4', [`1 1 1 1`], e=>e.x==1&&e.y==1&&e.z==1&&e.w==1)
	Mv(v, 'parseCurve', ['linear'], e=>e.p1x==0&&e.p1y==0&&e.p2x==1&&e.p2y==1)
	Mv(v, 'parseRect', [`rect(0,0,1,1)`], e=>e.x==0&&e.y==0&&e.width==1&&e.height==1)
	Mv(v, 'parseMat', ['mat(1,0,0,0,1,0)'],e=>e.toString()=='mat(1,0,0,0,1,0)')
	Mv(v, 'parseMat4', ['mat4(1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1)'],e=>e.toString()=='mat4(1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1)')
	Mv(v, 'parseColor', ['rgba(0,0,100,1)'],e=>e.toString()=='#000064');
	Mv(v, 'parseShadow', ['10 10 5 #f00'],e=>e.toString()=='10 10 5 #ff0000')
	Mv(v, 'parseBoxBorder', [`1 #f00`], e=>e.toString()=='1 #ff0000')
	Mv(v, 'parseFillPosition', ['start'], e=>e.kind==v.FillPositionKind.Start)
	Mv(v, 'parseFillSize', ['20%'], e=>e.kind==v.FillSizeKind.Ratio&&e.value==0.2)
	Mv(v, 'parseBoxOrigin', ['30%'], e=>e.kind==v.BoxOriginKind.Ratio&&e.value==0.3)
	Mv(v, 'parseBoxSize', ['match'], e=>e.kind==v.BoxSizeKind.Match)
	Mv(v, 'parseTextColor', ['#f00'], e=>e.value.toString()=='#ff0000')
	Mv(v, 'parseTextSize', ['inherit'], e=>e.toString()=='inherit')
	Mv(v, 'parseTextLineHeight', [24], e=>e.value==24)
	Mv(v, 'parseTextShadow', ['10 10 10 #00f'], e=>e.toString()=='10 10 10 #0000ff')
	Mv(v, 'parseTextFamily', ['inherit'], e=>e.toString()=='inherit');
	//
	Mv(v, 'parseBoxFilter', ['image(res/image.png, auto 100%, x=start, y=20%, repeat)'])
	Mv(v, 'parseBoxFilter', ['radial(#ff00ff 0%, #ff0 50%, #00f 100%)'])
	Mv(v, 'parseBoxFilter', ['linear(90, #ff00ff 0%, #ff0 50%, #00f 100%)'])
	Mv(v, 'parseBoxShadow', [`10 10 2 rgba(255,255,0,1)`])
}