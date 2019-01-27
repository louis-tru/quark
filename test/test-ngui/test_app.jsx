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
import { GUIApplication, Root } from 'shark';
import 'shark/font' as f;
import 'shark/url';
import 'shark/app';

new GUIApplication().start(<Root/>).onLoad = function() {
	const a = this;

	LOG('\nTest GUIApplication:\n')

	P(app, 'current')
	P(app, 'root')
	P(app, 'rootCtr')
	M(a.onUnload, 'on', [function() { LOG('---- onunload') }])
	M(a.onBackground, 'on', [function() { LOG('---- onbackground') }])
	M(a.onForeground, 'on', [function() { LOG('---- onforeground') }])
	M(a.onPause, 'on', [function() { LOG('---- onpause') }])
	M(a.onResume, 'on', [function() { LOG('---- onresume') }])
	M(a.onMemorywarning, 'on', [function() { LOG('---- onmemorywarning') }])
	M(a, 'clear');
	P(a, 'isLoad');
	P(a, 'displayPort')
	P(a, 'root')
	P(a, 'focusView')
	P(a, 'defaultTextBackgroundColor')
	P(a, 'defaultTextBackgroundColor', '#f0f')
	P(a, 'defaultTextBackgroundColor')
	P(a, 'defaultTextColor');
	P(a, 'defaultTextColor', '#f00');
	P(a, 'defaultTextColor');
	P(a, 'defaultTextSize');
	P(a, 'defaultTextSize', 24);
	P(a, 'defaultTextSize');
	P(a, 'defaultTextStyle');
	P(a, 'defaultTextStyle', 'bold');
	P(a, 'defaultTextStyle');
	P(a, 'defaultTextFamily');
	P(a, 'defaultTextFamily', 'Helvetica');
	P(a, 'defaultTextFamily');
	P(a, 'defaultTextShadow');
	P(a, 'defaultTextShadow', '10 10 10 #00f');
	P(a, 'defaultTextShadow');
	P(a, 'defaultTextLineHeight');
	P(a, 'defaultTextLineHeight', 'auto');
	P(a, 'defaultTextLineHeight');
	P(a, 'defaultTextDecoration');
	P(a, 'defaultTextDecoration', 'overline');
	P(a, 'defaultTextDecoration');
	P(a, 'defaultTextOverflow');
	P(a, 'defaultTextOverflow', 'clip');
	P(a, 'defaultTextOverflow');
	P(a, 'defaultTextWhiteSpace');
	P(a, 'defaultTextWhiteSpace', 'no_space');
	P(a, 'defaultTextWhiteSpace');

};