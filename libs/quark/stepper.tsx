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

import {Jsx,ViewController} from './ctr';
import {mainScreenScale} from './screen';
import {createCss,StyleSheets} from './css';

const px = 1 / mainScreenScale();

createCss({
	'.qk_stepper': {
		width: 45 * 2 + 3,
	},
	'.qk_stepper .qk_minus,\
	.qk_stepper .qk_plus': {
		width: 45,
		height: 28,
		textLineHeight: 1,
		textSize: 18,
		textFamily: 'iconfont',
		textColor: '#727272',
		border: `${px} #727272`,
		borderColor: '#727272',
		backgroundColor: '#fff',
		textAlign: "center",
	},
	'.qk_stepper .qk_minus': {
		borderRadiusLeftTop: 6,
		borderRadiusLeftBottom: 6,
	},
	'.qk_stepper .qk_plus': {
		borderWidthLeft: 0,
		borderRadiusRightTop: 6,
		borderRadiusRightBottom: 6,
	},
	'.qk_stepper .qk_minus:active,\
	.qk_stepper .qk_plus:active': {
		backgroundColor: '#eee',
	},
});

/**
 * @class Stepper
 * @extends ViewController
*/
export class Stepper extends ViewController<{
	class?: string,
	style?: StyleSheets,
	min?: number,
	max?: number,
	step?: number,
	initValue?: number,
	onChange?:(value:number, sender: Stepper)=>void,
}> {
	private _value = this.props.initValue || 0;

	/**
	 * @getset value:number
	*/
	get value() {
		return this._value;
	}

	set value(value) {
		if ( typeof value == 'number') {
			value = Math.min(this.props.max || Infinity, Math.max(this.props.min || 0, value));
			if ( value != this._value ) {
				this._value = value;
				if (this.isMounted)
					this.triggerChange();
			}
		}
	}

	protected triggerUpdate() {
		this.value = this._value;
	}

	protected triggerChange() {
		this.props.onChange?.call(null, this._value, this);
	}

	private _MinusClickHandle = ()=>{
		this.value = this._value - (this.props.step || 1);
	};
	
	private _PlusClickHandle = ()=>{
		this.value = this._value + (this.props.step || 1);
	};

	render() {
		return (
			<box class={['qk_stepper',this.props.class||'']} style={this.props.style}>
				<button class="qk_minus" onClick={this._MinusClickHandle} value={"\ued5e"}/>
				<button class="qk_plus" onClick={this._PlusClickHandle} value={"\ued5d"}/>
			</box>
		);
	}
}
