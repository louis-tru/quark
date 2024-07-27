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

import { LOG, Pv, Mv } from './tool'
import qk, { Application, Window } from 'quark'
import * as types from 'quark/types';

const app = new Application();
const win = new Window({
	frame: types.newRect(0,0,500,500),
}).activate();

LOG('\nTest Application:\n')
Pv(qk, 'app', app)
Pv(win, 'root', win.root)
Pv(win, 'rootCtr', win.rootCtr)

Mv(app.onUnload, 'on', [function() { LOG('---- onunload') }])
Mv(app.onBackground, 'on', [function() { LOG('---- onbackground') }])
Mv(app.onForeground, 'on', [function() { LOG('---- onforeground') }])
Mv(app.onPause, 'on', [function() { LOG('---- onpause') }])
Mv(app.onResume, 'on', [function() { LOG('---- onresume') }])
Mv(app.onMemoryWarning, 'on', [function() { LOG('---- onmemorywarning') }])

Mv(app, 'clear', []);
Pv(app, 'isLoaded', false);
Pv(app, 'screen', app.screen)
Pv(win, 'root', e=>e.isFocus)
Pv(win, 'focusView', win.root)

// defaultTextOptions
Pv(app.defaultTextOptions, 'textBackgroundColor', e=>e.toString()=='#ffffff',
	e=>e.textBackgroundColor=types.parseTextColor('#fff'))

Pv(app.defaultTextOptions, 'textColor', e=>e.toString()=='#ff0000',
	e=>e.textColor=types.parseTextColor('#f00'))

Pv(app.defaultTextOptions, 'textSize', e=>e.value==16,
	e=>e.textSize=types.parseTextSize(16))

Pv(app.defaultTextOptions, 'textWeight', e=>e==types.TextWeight.Thin,
	e=>e.textWeight=types.parseTextWeight('thin'))

Pv(app.defaultTextOptions, 'textSlant', e=>e==types.TextSlant.Italic,
	e=>e.textSlant=types.parseTextSlant('italic'))

Pv(app.defaultTextOptions, 'textFamily', e=>!!e.familys())

Pv(app.defaultTextOptions, 'textShadow', e=>e.value.color.toString()=='#0000ff'&&e.value.x==10,
	e=>e.textShadow=types.parseTextShadow('10 10 10 #00f'))

Pv(app.defaultTextOptions, 'textLineHeight', e=>e.value==0,
	e=>e.textLineHeight=types.parseTextLineHeight(0))

Pv(app.defaultTextOptions, 'textDecoration', e=>e==types.TextDecoration.LineThrough,
	e=>e.textDecoration=types.parseTextDecoration('lineThrough'))

Pv(app.defaultTextOptions, 'textOverflow', e=>e==types.TextOverflow.Clip,
	e=>e.textOverflow=types.parseTextOverflow('clip'))

Pv(app.defaultTextOptions, 'textWhiteSpace', e=>e==types.TextWhiteSpace.NoWrap,
	e=>e.textWhiteSpace=types.parseTextWhiteSpace('noWrap'))
