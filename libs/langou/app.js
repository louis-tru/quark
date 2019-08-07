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
import 'langou/display_port';
import { NativeNotification } from 'langou/event';
import ViewController from 'langou/ctr';

var _langou = requireNative('_langou');
var Root = _langou.Root;
var cur = null;
var cur_root_ctr = null;

/**
 * @class GUIApplication
 * @bases NativeGUIApplication,NativeNotification
 */
export class GUIApplication extends _langou.NativeGUIApplication {
	
	event onLoad;
	event onUnload;
	event onBackground;
	event onForeground;
	event onPause;
	event onResume;
	event onMemoryWarning;
	
	/**
	 * @constructor([options])
	 * @arg [options] {Object} { anisotropic {bool}, multisample {0-4} }
	 */
	constructor(options) {
		super(options);
		cur = this;
	}
	
	/**
	 * @func start(vdom)
	 * @arg vdom {Object}
	 */
	start(vdom) {
		util.assert(vdom.isViewController(), 'The "ViewController" must be used to start the application');

		function handleMounted(e) {
			util.assert(cur_root_ctr.dom instanceof Root, 'Root view controller first children must be Root view');
		}

		if ( this.isLoaded ) {
			_langou.lock(()=>{
				cur_root_ctr = ViewController.render(vdom);
				cur_root_ctr.onMounted.on(handleMounted);
			});
		} else {
			this.onLoad.on(()=>{
				cur_root_ctr = ViewController.render(vdom);
				cur_root_ctr.onMounted.on(handleMounted);
			});
		}

		return this;
	}

	//@end
}

util.extendClass(GUIApplication, NativeNotification);

export {

	/**
	 * @get currend {GUIApplication} 
	 */
	get current() { return cur },

	/**
	 * @get root {Root} 
	 */
	get root() { return cur_root_ctr.dom },

	/**
	 * @get rootCtr {ViewController}
	 */
	get rootCtr() { return cur_root_ctr },
};
