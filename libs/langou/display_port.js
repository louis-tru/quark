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

import 'langou/util';
import 'langou/app';
import { NativeNotification } from 'langou/event';

export requireNative('_display_port');

/**
 * @class DisplayPort
 */
class DisplayPort extends NativeNotification {
	event onChange;
	event onOrientation;
}

util.extendClass(exports.DisplayPort, DisplayPort);

export {

	/**
	 * @get current {DisplayPort}
	 */
	get current() { return app.current.displayPort; },

	/**
	 * @get atomPixel {float}
	 */
	get atomPixel() {
		return app.current ? app.current.displayPort.atomPixel: exports.defaultAtomPixel;
	},

	/**
	 * @get statusBarHeight {float}
	 */
	get statusBarHeight() {
		return app.current ? app.current.displayPort.statusBarHeight(): exports.defaultStatusBarHeight;
	},

	/**
	 * @func nextFrame(cb)
	 * @arg cb {Function}
	 */
	nextFrame: function(cb) {
		if ( app.current ) {
			app.current.displayPort.nextFrame(cb);
		} else {
			throw new Error("GUIApplication has not been created");
		}
	},
}
