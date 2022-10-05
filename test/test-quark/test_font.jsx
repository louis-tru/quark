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

import { P, M, LOG, VM, VP } from './test';
import { Application, Root } from 'quark';
import 'quark/font' as f;
import 'fs';
import 'quark/url';

new Application().start(<Root/>).onLoad = function() {

	LOG('\nTest Font:\n')

	M(f, 'setDefaultFonts', ['Helvetica, PingFang HK, Thonburi']);
	M(f, 'setDefaultFonts', [['Helvetica', 'PingFang HK', 'Thonburi']]);
	M(f, 'defaultFontNames', []);
	M(f, 'familyNames', []);
	M(f, 'fontNames', ['Helvetica']);
	M(f, 'fontNames', ['PingFang HK']);
	M(f, 'test', ['PingFang HK']);
	M(f, 'test', ['.PingFangHK-Regular']);
	M(f, 'test', ['.HelveticaLight-Oblique']);
	M(f, 'registerFont', [fs.readFileSync(url.resources('res/font/lateef.ttf')), 'A']);
	M(f, 'test', ['A']);
	M(f, 'registerFontFile', [url.resources('res/font/SF-UI-Display-Black.otf'), 'B']);
	M(f, 'test', ['B']);
	M(f, 'setFamilyAlias', ['B', 'B-']);
	M(f, 'test', ['B-']);
};