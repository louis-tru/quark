/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright (c) 2015, Louis.chu
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of Louis.chu nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL Louis.chu BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * ***** END LICENSE BLOCK ***** */

import {createCss,StyleSheets} from './css';
import {Jsx,ViewController,link} from './ctr';

createCss({
	'.qk_checkbox': {
		width: 20,
		height: 20,
		backgroundColor: '#fff0',
		border: '2 #aaa',
		borderRadius: 100,
		textAlign: 'center',
		textLineHeight: 1,
		textFamily: 'iconfont',
		textColor: '#fff',
		textSize: 14,
		opacity: 1,
	},
	'.qk_checkbox:hover': {
		opacity: 0.7,
	},
	'.qk_checkbox:active': {
		opacity: 0.35,
	},
	'.qk_checkbox:hover, .qk_checkbox:active, .qk_checkbox.on': {
		backgroundColor: '#0079ff',
		borderColor: '#0079ff',
	},
	'.qk_checkbox .qk_mark': {
		visible: false,
	},
	'.qk_checkbox.on .qk_mark': {
		visible: true,
	},
	// switch
	'.qk_switch': {
		width: 50,
		height: 31,
		borderRadius: 18,
	},
	'.qk_switch.off': {
		backgroundColor: '#ddd',
		time: 300,
	},
	'.qk_switch.on': {
		backgroundColor: '#4dd865',
		time: 300,
	},
	// switch button
	'.qk_switch .qk_button': {
		borderRadius: 16,
		backgroundColor: '#fff',
		height: 27,
		y: 2,
	},
	'.qk_switch:normal .qk_button': {
		width: 27,
		time: 200,
	},
	'.qk_switch:hover .qk_button, .qk_switch:active .qk_button': {
		width: 33,
		time: 200,
	},
	'.qk_switch.off .qk_button': {
		x: 2,
		time: 200,
	},
	'.qk_switch.on .qk_button': {
		x: 20,
		time: 200,
	},
	'.qk_switch.on:hover .qk_button, .qk_switch.on:active .qk_button': {
		x: 14,
		time: 200,
	},
	// switch background
	'.qk_switch .qk_background': {
		backgroundColor: '#eee',
		borderRadius: 16,
		align: 'centerMiddle',
	},
	'.qk_switch:normal .qk_background': {
		width: 46,
		height: 27,
		opacity: 1,
		time: 300,
	},
	'.qk_switch:hover .qk_background, .qk_switch:active .qk_background, \
	.qk_switch.on .qk_background': {
		width: 0,
		height: 0,
		opacity: 0,
		time: 300,
	},
});

/**
 * @class Basic
 * @extends ViewController
*/
export class Basic<P={},S={}> extends ViewController<{
	class?: string,
	style?: StyleSheets,
	disable?: boolean,
	initSelected?: boolean,
	onChange?:(value:boolean, sender: Basic)=>void,
}&P,S> {
	private _selected = !!this.props.initSelected;

	/**
	 * @getset disable:boolean Is it disable?
	*/
	@link disable = false;

	protected triggerMounted() {
		this.asDom().onClick.on(()=>{
			if ( !this.disable )
				this.selected = !this.selected;
		}, '1');
	}

	protected triggerChange(value: boolean) {
		this.props.onChange?.call(null, value, this);
	}

	/**
	 * @getset selected:boolean Is it selected?
	*/
	get selected() {
		return this._selected;
	}

	set selected(value: boolean) {
		if(value != this._selected) {
			this._selected = !!value;
			this.update();
			this.triggerChange(value);
		}
	}
}

/**
 * @class Checkbox
 * @extends Basic
 * 
 * the checkbox and the radio button UI component
*/
export class Checkbox<P={},S={}> extends Basic<P,S> {
	protected render() {
		return (
			<button class={['qk_checkbox',this.props.class||'',this.selected?'on':'off']} style={this.props.style}>
				<label class="qk_mark" value={"\ued71"} />
			</button>
		)
	}
}

/**
 * @class Switch
 * @extends Basic
 * 
 * The switch UI component
*/
export class Switch<P={},S={}> extends Basic<P,S> {
	protected render() {
		return (
			<free class={['qk_switch',this.props.class||'',this.selected?'on':'off']} style={this.props.style}>
				<free class="qk_background" />
				<morph class="qk_button" />
			</free>
		);
	}
}

let test = ()=>(
	<Checkbox disable={false} initSelected={true}>
	</Checkbox>
);
