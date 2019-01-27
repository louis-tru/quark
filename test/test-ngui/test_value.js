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

import { P, M, LOG, VM, VP } from './test'
import 'shark/value' as v;

LOG('\nTest Enum Value:\n')

P(v, 'AUTO');
P(v, 'FULL');
P(v, 'PIXEL');
P(v, 'PERCENT');
P(v, 'MINUS');
P(v, 'INHERIT');
P(v, 'VALUE');
P(v, 'LIGHT');
P(v, 'REGULAR');
P(v, 'BOLD');
P(v, 'OTHER');
P(v, 'NONE');
P(v, 'OVERLINE');
P(v, 'LINE_THROUGH');
P(v, 'UNDERLINE');
P(v, 'LEFT');
P(v, 'CENTER');
P(v, 'RIGHT');
P(v, 'LEFT_REVERSE');
P(v, 'CENTER_REVERSE');
P(v, 'RIGHT_REVERSE');
P(v, 'TOP');
P(v, 'BOTTOM');
P(v, 'MIDDLE');
P(v, 'REPEAT');
P(v, 'REPEAT_X');
P(v, 'REPEAT_Y');
P(v, 'MIRRORED_REPEAT');
P(v, 'MIRRORED_REPEAT_X');
P(v, 'MIRRORED_REPEAT_Y');
P(v, 'NORMAL');
P(v, 'CLIP');
P(v, 'ELLIPSIS');
P(v, 'CENTER_ELLIPSIS');
P(v, 'NO_WRAP');
P(v, 'NO_SPACE');
P(v, 'PRE');
P(v, 'PRE_LINE');
// keyboard type
P(v, 'ASCII');
P(v, 'NUMBER');
P(v, 'URL');
P(v, 'NUMBER_PAD');
P(v, 'PHONE_PAD');
P(v, 'NAME_PHONE_PAD');
P(v, 'EMAIL');
P(v, 'DECIMAL_PAD');
P(v, 'TWITTER');
P(v, 'WEB_SEARCH');
P(v, 'ASCII_NUMBER_PAD');
// keyboard return type
P(v, 'GO');
P(v, 'JOIN');
P(v, 'NEXT');
P(v, 'ROUTE');
P(v, 'SEARCH');
P(v, 'SEND');
P(v, 'DONE');
P(v, 'EMERGENCY_CALL');
P(v, 'CONTINUE');

LOG('\nTEST Constructor:\n')

LOG(new v.TextAlign(v.LEFT));
LOG(new v.Align(v.CENTER));
LOG(new v.ContentAlign(v.RIGHT));
LOG(new v.Repeat(v.REPEAT));
LOG(new v.Direction(v.BOTTOM));
LOG(new v.KeyboardType(v.NUMBER));
LOG(new v.KeyboardReturnType(v.NEXT));
LOG(new v.Border(10, new v.Color(255, 100, 200, 255)));
LOG(new v.Shadow(10, 10, 5, new v.Color(255, 0, 0, 255)));
LOG(new v.Color(0, 0, 0, 255));
LOG(new v.Vec2(10, 90));
LOG(new v.Vec3(0,0,1));
LOG(new v.Vec4(0,0,0,3));
LOG(new v.Curve(0,0,1,1));
LOG(new v.Rect(0,0,100,100));
LOG(new v.Mat());
LOG(new v.Mat4());
LOG(new v.Value(v.PIXEL, 10));
LOG(new v.TextColor(v.VALUE, new v.Color()));
LOG(new v.TextSize(v.VALUE, 16));
LOG(new v.TextFamily(v.VALUE, 'AAA'));
LOG(new v.TextStyle(v.VALUE, v.BOLD));
LOG(new v.TextShadow(v.VALUE, new v.Shadow(10, 10, 5, new v.Color(255, 0, 0, 255))));
LOG(new v.TextLineHeight(v.VALUE, true));
LOG(new v.TextDecoration(v.VALUE, v.LINE_THROUGH));
LOG(new v.TextOverflow(v.VALUE, v.CLIP));
LOG(new v.TextWhiteSpace(v.VALUE, v.PRE));

LOG('\nTEST Parse Function:\n')

M(v, 'parseTextAlign', ['left']);
M(v, 'parseAlign', ['center']);
M(v, 'parseContentAlign', ['right']);
M(v, 'parseRepeat', ['none']);
M(v, 'parseDirection', ['top']);
M(v, 'parseKeyboardType', ['ascii']);
M(v, 'parseKeyboardReturnType', ['route']);
M(v, 'parseBorder', ['10 #f00a']);
M(v, 'parseShadow', ['10 10 5 #f00']);
M(v, 'parseColor', ['rgba(0,0,100,255)']);
M(v, 'parseVec2', ['10 9']);
M(v, 'parseVec3', ['10 0 4']);
M(v, 'parseVec4', ['vec4(4,5,6,7)']);
M(v, 'parseCurve', ['0 0 1 1']);
M(v, 'parseRect', ['0 0 100 200']);
M(v, 'parseMat', ['mat(1,0,0,0,1,0)']);
M(v, 'parseMat4', ['mat4(1,0,0,300.909,0,1,0,0,200,0,1,100,0,0,0,1)']);
M(v, 'parseValue', ['full']);
M(v, 'parseValues', ['full auto 100! 50% 50.8']);
M(v, 'parseFloatValues', ['10 20 30 40']);
M(v, 'parseTextColor', ['#f00']);
M(v, 'parseTextSize', ['inherit']);
M(v, 'parseTextFamily', ['inherit']);
M(v, 'parseTextStyle', ['light']);
M(v, 'parseTextShadow', ['10 10 10 #00f']);
M(v, 'parseTextLineHeight', ['24']);
M(v, 'parseTextDecoration', ['overline']);
M(v, 'parseTextOverflow', ['center_ellipsis']);
M(v, 'parseTextWhiteSpace', ['no_wrap']);
