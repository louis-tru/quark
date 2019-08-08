/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright (c) 2015-2017, xuewen.chu
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
import { 
	CSS, Indep, Hybrid, Clip, Input, Span,
	LimitIndep, Button, atomPixel, langou, render
} from 'langou/langou';
import { Navigation } from 'langou/nav';

CSS({
	
	'.x_dialog': {
		minWidth: 380,
		maxWidth: '40!',
		maxHeight: '40!',
		alignX: 'center',
		alignY: 'center',
		backgroundColor: '#fff',
		borderRadius: 12,
	},
	
	'.x_dialog .title': {
		width: 'full',
		margin: 10,
		marginTop: 18,
		marginBottom: 0,
		textAlign: 'center',
		textStyle: 'bold',
		textSize: 18,
		textOverflow: 'ellipsis',
		textWhiteSpace: 'no_wrap',
	},
	
	'.x_dialog .content': {
		width: 'full',
		margin: 10,
		marginTop: 2,
		marginBottom: 20,
		textAlign: 'center',
		textSize: 14,
	},
	
	'.x_dialog .buttons': {
		width: 'full',
		borderRadiusLeftBottom: 12,
		borderRadiusRightBottom: 12,
	},
	
	'.x_dialog .button': {
		height: 43,
		borderTop: `${atomPixel*0.7} #9da1a0`,
		textSize: 18,
		textLineHeight: 43,
		textColor:"#0079ff",
	},
	
	'.x_dialog .button:normal': {
		backgroundColor: '#fff0', time: 180
	},
	
	'.x_dialog .button:hover': {
		backgroundColor: '#E1E4E455', time: 50
	},
	
	'.x_dialog .button:down': {
		backgroundColor: '#E1E4E4', time: 50
	},
	
	'.x_dialog .prompt': {
		marginTop: 10,
		width: "full",
		height: 30,
		backgroundColor: "#eee",
		borderRadius: 8,
	},
	
})

function compute_buttons_width(self) {
	
	var btns = self.buttons;
	
	if ( btns.length == 1 ) {
		btns[0].width = 'full';
	} else {
		var main_width = self.IDs.main.finalWidth;
		if ( main_width ) {
			for ( var btn of btns ) {
				btn.width = (main_width / btns.length) - ((btns.length - 1) * atomPixel);
				btn.borderLeft = `${atomPixel} #9da1a0`;
			}
			btns[0].borderLeftWidth = 0;
		}
	}
}

function close(self) {
	if ( self.defaultClose ) 
		self.close();
}

/**
 * @class Dialog
 */
export class Dialog extends Navigation {

	m_btns_count = 0;
	
	/**
	 * @event onClickButton
	 */
	event onClickButton;
	
	/**
	 * @defaultClose
	 */
	defaultClose = true;

	/**
	 * @get length btns
	 */
	get length() {
		return this.m_btns_count;
	}
	
	/**
	 * @overwrite
	 */
	render(vc) {
		return (
			<Indep 
				width="full" 
				height="full" backgroundColor="#0005" receive=1 visible=0 opacity=0>
				<LimitIndep id="main" class="x_dialog" alignX="center" alignY="center">
					<Hybrid id="title" class="title" />
					<Hybrid id="con" class="content">{vc}</Hybrid>
					<Clip id="btns" class="buttons" />
				</LimitIndep>
			</Indep>
		);
	}

	get title() { return this.IDs.title.innerText }
	get content() { return this.IDs.con.innerText }
	set title(value) {
		this.IDs.title.removeAllChild();
		this.IDs.title.appendText(value || '');
	}
	set content(value) {
		this.IDs.con.removeAllChild();
		this.IDs.con.appendText(value || '');
	}
	
	get buttons() {
		var btns = this.IDs.btns;
		var btn = btns.first;
		var rv = [];
		while (btn) {
			rv.push(btn);
			btn = btn.next;
		}
		return rv;
	}
	
	set buttons(btns) {
		if ( Array.isArray(btns) ) {
			this.IDs.btns.removeAllChild();
			this.m_btns_count = btns.length;

			for ( var i = 0; i < btns.length; i++ ) {
				var btn = render(
					<Button 
						index=i
						class="button" 
						width="full"
						onClick="triggerClickButton"
						defaultHighlighted=0>{btns[i]}</Button>,
					this.IDs.btns
				);
			}
			if ( this.visible ) {
				compute_buttons_width(this);
			}
		}
	}

	show() {
		if (!this.visible) {
			this.appendTo(langou.root);
			this.visible = 1;
			
			langou.nextFrame(()=>{
				compute_buttons_width(this);
				var main = this.IDs.main;
				main.originX = main.finalWidth / 2;
				main.originY = main.finalHeight / 2;
				main.scale = '0.3 0.3';
				main.transition({ scale : '1 1', time: 200 });
				this.dom.opacity = 0.3;
				this.transition({ opacity : 1, time: 200 });
			});
			this.registerNavigation(0);
		}
	}
	
	close() {
		if ( this.visible ) {
			var main = this.IDs.main;
			main.originX = main.finalWidth / 2;
			main.originY = main.finalHeight / 2;
			main.transition({ scale : '0.5 0.5', time: 200 });
			this.transition({ opacity : 0.15, time: 200 }, ()=>{ this.remove() });
			this.unregisterNavigation(0, null);
		} else {
			this.unregisterNavigation(0, null);
			this.remove();
		}
	}

	triggerClickButton(evt) {
		this.trigger('ClickButton', evt.sender.index);
		close(this);
	}

	/**
	 * @overwrite 
	 */
	navigationBack() {
		if ( this.length ) {
			this.trigger('ClickButton', 0);
		}
		close(this);
		return true;
	}

	/**
	 * @overwrite 
	 */
	navigationEnter(focus) {
		if ( !this.dom.hasChild(focus) ) {
			if ( this.length ) {
				this.trigger('ClickButton', this.length - 1);
			}
			close(this);
		}
	}
}

export const CONSTS = {
	OK: 'OK',
	Cancel: 'Cancel',
	placeholder: 'Please enter..',
};

export function alert(msg, cb = util.noop) {
	var dag = render(
		<Dialog buttons=[CONSTS.OK] onClickButton=(e=>cb(e.data))>{msg}</Dialog>
	);
	dag.show();
	return dag;
}

export function confirm(msg, cb = util.noop) {
	var dag = render(
		<Dialog buttons=[CONSTS.Cancel, CONSTS.OK] onClickButton=(e=>cb(e.data))>{msg}</Dialog>
	);
	dag.show();
	return dag;
}

function handle_prompt_enter(ev) {
	var dag = ev.sender.owner;
	dag.trigger('ClickButton', 1);
	close(dag);
}

export function prompt(msg, text = '', cb = util.noop) {
	if ( typeof text == 'function' ) {
		cb = text;
		text = '';
	}
	var dag = render(
		<Dialog buttons=[CONSTS.Cancel, CONSTS.OK] onClickButton=(e=>cb(e.data, e.data ? dag.IDs.input.value: ''))>
			<Span>
				{msg}
				<Input id="m_input" class="prompt"
					returnType="done" onKeyEnter=handle_prompt_enter
					value=text placeholder=CONSTS.placeholder />
			</Span>
		</Dialog>
	);
	dag.show();
	dag.onMounted.once(e=>dag.IDs.input.focus());
	return dag;
}

export function show(title, msg, buttons, cb = ()=>{ }) {
	var dag = render(
		<Dialog title=title buttons=buttons onClickButton=(e=>cb(e.data))>{msg}</Dialog>
	);
	dag.show();
	return dag;
}
