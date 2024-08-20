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

import { Application, Window, Box } from 'quark'
import * as types from 'quark/types'
import util from 'quark/util'

const app = new Application();
const win = new Window({
	fps: 0x0,
	frame: types.newRect(0,0,500,500),
	backgroundColor: types.newColor(0,0,255,255),
}).activate();

async function test_index(win: Window) {
	const box = new Box(win);
	box.width = types.newBoxSize(types.BoxSizeKind.Match, 0);
	box.height = types.newBoxSize(types.BoxSizeKind.Ratio, 0.5);
	box.backgroundColor = types.newColor(255,0,0,255);
	win.root.backgroundColor = types.newColor(0,255,0,100);
	win.root.append(box);
}

// import test from './test_action'
// import test from './test_app'
// import test from './test_buf'
// import test from './test_css'
// import test from './test_event'
// import test from './test_font'
// import test from './test_fs'
// import test from './test_gui'
import test from './test_http'
// import test from './test_os'
// import test from './test_path'
// import test from './test_reader'
// import test from './test_storage'
// import test from './test_types'
// import test from './test_util'
// import test from './test_view' #
// import test from './test_window'

(async function() {
	await test(win);

	console.log('\n------------------- Test End -------------------\n');
	// util.exit()
})()