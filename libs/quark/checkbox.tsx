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

import quark, { ViewController, Button, Indep, TextNode, _CVD } from './index';
import event, {EventNoticer,Event} from './event';

quark.css({

	// checkbox

	'.x_checkbox': {
		width: 20,
		height: 20,
		backgroundColor: '#fff',
		border: '1 #aaa',
		borderRadius: 20,
		opacity: 1,
	},
	
	'.x_checkbox:hover': {
		backgroundColor: '#0079ff',
		border: '1 #0079ff',
		opacity: 0.7,
	},
	
	'.x_checkbox:down': {
		backgroundColor: '#0079ff',
		border: '1 #0079ff',
		opacity: 0.35,
	},
		
	'.x_checkbox.on': {
		backgroundColor: '#0079ff',
		border: '1 #0079ff',
	},
		
	'.x_checkbox .mark': {
		visible: false,
		textFamily: 'iconfont',
		textColor: '#fff',
		textSize: 14,
		textLineHeight: 20,
		opacity: 1,
	},
	
	'.x_checkbox.on .mark': {
		visible: true,
	},
		
	// switch
	
	'.x_switch': {
		width: 50,
		height: 31,
		backgroundColor: '#ddd',
		borderRadius: 16,
		time: 300,
	},
	
	'.x_switch.on': {
		backgroundColor: '#4dd865',
		time: 300,
	},
		
	'.x_switch .background': {
		backgroundColor: '#eee',
		borderRadius: 16,
		alignX: 'center',
		alignY: 'center',
		width: 46,
		height: 27,
		opacity: 1,
		time: 300,
	},

	'.x_switch.on .background, \
	.x_switch:down .background, \
	.x_switch:hover .background': {
		width: 0,
		height: 0,
		opacity: 0.2,
		time: 300,
	},
	
	'.x_switch .button': {
		borderRadius: 16,
		backgroundColor: '#fff',
		width: 27,
		height: 27,
		x: 2,
		y: 2,
		time: 200,
	},
	
	'.x_switch:down .button, .x_switch:hover .button': {
		width: 33,
		time: 200,
	},

	'.x_switch.on .button': {
		x: 20,
		time: 200,
	},

	'.x_switch.on:down .button, .x_switch.on:hover .button': {
		x: 14,
		time: 200,
	},
	
});

class Basic extends ViewController {
	private m_selected = false;
	private m_disable = false;

	@event readonly onChange: EventNoticer<Event<boolean, Basic>>;

	triggerMounted() {
		this.domAs().receive = !this.m_disable;
		this.domAs().onClick.on(()=>{
			if ( !this.m_disable )
				this.selected = !this.selected;
		}, '1');
		return super.triggerMounted();
	}

	protected triggerChange(value: boolean) {
		return this.trigger('Change', value);
	}

	get disable() {
		return this.m_disable;
	}

	set disable(value) {
		if (this.isMounted)
			this.domAs().receive = !value;
		this.m_disable = !!value;
	}

	get selected() {
		return this.m_selected;
	}

	set selected(value: boolean) {
		value = !!value;
		if(value != this.m_selected) {
			this.markRerender();
			this.m_selected = value;
			if (this.isMounted)
				this.triggerChange(value);
		}
	}
}

/**
 * @class Checkbox
 */
export class Checkbox extends Basic {

	render() {
		return (
			<Button class={`x_checkbox ${this.selected ? 'on': ''}`} style={this.style} defaultHighlighted={0}>
				<TextNode class="mark" value={"\ued71"} />
			</Button>
		)
	}
}

/**
 * @class Switch
 */
export class Switch extends Basic {

	render() {
		return (
			<Button class={`x_switch ${this.selected ? 'on': ''}`} style={this.style} defaultHighlighted={0}>
				<Indep class="background" />
				<Indep class="button" />
			</Button>
		);
	}
}
