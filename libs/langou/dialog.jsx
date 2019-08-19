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
	LimitIndep, Button, atomPixel as px, langou, render
} from 'langou/langou';
import { Navigation } from 'langou/nav';

CSS({
	
	'.x_dialog': {
	},

	'.x_dialog.main': {
		minWidth: 380,
		maxWidth: '40!',
		maxHeight: '40!',
		alignX: 'center',
		alignY: 'center',
		backgroundColor: '#fff',
		borderRadius: 12,
	},
	
	'.x_dialog.sheet': {
		width: 'full',
		margin: 10,
		alignY: 'bottom',
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

	'.x_dialog.sheet .buttons': {
		borderRadius: 12,
		backgroundColor: '#fff',
		marginTop: 10,
	},
	
	'.x_dialog .button': {
		height: 43,
		// borderTop: `${px} #9da1a0`,
		borderTopColor: `#9da1a0`,
		textSize: 18,
		textLineHeight: 43,
		textColor:"#0079ff",
	},

	'.x_dialog .button.gray': {
		textColor:"#000",
	},

	'.x_dialog .button:normal': {
		backgroundColor: '#fff', time: 180
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
	var len = self.length;
	if (!len || !self.dom.visible) return;
	
	if ( len == 1 ) {
		self.IDs.btns.first.width = 'full';
	} else  {
		var main_width = self.IDs.main.finalWidth;
		if ( main_width ) {
			var btn = self.IDs.btns.first;
			while (btn) {
				btn.width = (main_width / len) - ((len - 1) * px);
				btn.borderLeft = `${px} #9da1a0`;
				btn.borderTopWidth = px;
				btn = btn.next;
			}
			self.IDs.btns.first.borderLeftWidth = 0;
		}
	}
}

function actionClose(self) {
	if (self.actionClose) 
		self.close();
}

/**
 * @class Dialog
 */
export class Dialog extends Navigation {
	m_buttons = null;
	
	/**
	 * @event onAction
	 */
	event onAction;
	
	/**
	 * @actionClose
	 */
	actionClose = true;

	/**
	 * @get length btns
	 */
	get length() {
		return this.m_buttons.length;
	}

	constructor() {
		super();
		this.m_buttons = [];
	}

	/**
	 * @overwrite
	 */
	render(...vdoms) {
		return (
			<Indep width="100%" height="100%" backgroundColor="#0005" receive=1 visible=0 opacity=0>
				<LimitIndep id="main" class="x_dialog main">
					<Hybrid id="title" class="title">{this.title}</Hybrid>
					<Hybrid id="con" class="content">{this.content||vdoms}</Hybrid>
					<Clip id="btns" class="buttons">
					{
						this.m_buttons.map((e, i)=>(
							<Button 
								index=i
								class="button"
								borderTopWidth=px
								onClick="triggerAction"
								defaultHighlighted=0>{e}</Button>
						))
					}
					</Clip>
				</LimitIndep>
			</Indep>
		);
	}
	
	get buttons() {
		return this.m_buttons;
	}

	set buttons(value) {
		if ( Array.isArray(value) ) {
			this.m_buttons = value;
			this.markRerender();
		}
	}

	triggerUpdate(e) {
		compute_buttons_width(this);
		super.triggerUpdate(e);
	}

	show() {
		if (!this.dom.visible) {
			this.appendTo(langou.root);
			this.dom.visible = 1;
			langou.nextFrame(()=>{
				compute_buttons_width(this);
				var main = this.IDs.main;
				main.originX = main.finalWidth / 2;
				main.originY = main.finalHeight / 2;
				main.scale = '0.3 0.3';
				main.transition({ scale : '1 1', time: 200 });
				this.dom.opacity = 0.3;
				this.dom.transition({ opacity : 1, time: 200 });
			});
			this.registerNavigation(0);
		}
	}
	
	close() {
		if ( this.dom.visible ) {
			var main = this.IDs.main;
			main.originX = main.finalWidth / 2;
			main.originY = main.finalHeight / 2;
			main.transition({ scale : '0.5 0.5', time: 200 });
			this.dom.transition({ opacity : 0.15, time: 200 }, ()=>{ this.remove() });
			this.unregisterNavigation(0, null);
		} else {
			this.unregisterNavigation(0, null);
			this.remove();
		}
	}

	triggerAction(evt) {
		this.trigger('Action', evt.sender.index);
		actionClose(this);
	}

	/**
	 * @overwrite 
	 */
	navigationBack() {
		if ( this.length ) {
			this.trigger('Action', 0);
		}
		actionClose(this);
		return true;
	}

	/**
	 * @overwrite 
	 */
	navigationEnter(focus) {
		if ( !this.dom.hasChild(focus) ) {
			if ( this.length ) {
				this.trigger('Action', this.length - 1);
			}
			actionClose(this);
		}
	}
}

Dialog.defineProps({title: '', content: ''});

/**
 * @class Sheet
 */
export class Sheet extends Dialog {

	triggerUpdate(e) {
		return Navigation.prototype.triggerUpdate.call(this, e);
	}

	render(...vdoms) {
		return (
			<Indep width="100%" height="100%" backgroundColor="#0005" onClick="navigationBack" visible=0 opacity=0>
				<Indep id="main" class="x_dialog sheet">
					<Clip class="buttons">
					{
						this.m_buttons.slice().reverse().map((e, i)=>(
							<Button 
								index=(this.length-i)
								class="button"
								width="100%"
								onClick="triggerAction"
								borderTopWidth=(!this.length?0:px)
								defaultHighlighted=0>{e}</Button>
						))
					}
					</Clip>
					<Clip class="buttons">
						<Button 
							index=0
							class="button gray"
							width="100%"
							onClick="triggerAction"
							defaultHighlighted=0>{CONSTS.CANCEL}</Button>
					</Clip>
				</Indep>
			</Indep>
		);
	}

	show() {
		if (!this.dom.visible) {
			this.appendTo(langou.root);
			this.dom.visible = 1;
			langou.nextFrame(()=>{
				var main = this.IDs.main;
				var height = main.finalHeight;
				main.y = height;
				main.transition({ y: 0, time: 200 });
				this.dom.opacity = 0.3;
				this.dom.transition({ opacity : 1, time: 200 });
			});
			this.registerNavigation(0);
		}
	}
	
	close() {
		if ( this.dom.visible ) {
			var main = this.IDs.main;
			var height = main.finalHeight;
			main.transition({ y: height, time: 200 });
			this.dom.transition({ opacity : 0.15, time: 200 }, ()=>{ this.remove() });
			this.unregisterNavigation(0, null);
		} else {
			this.unregisterNavigation(0, null);
			this.remove();
		}
	}

}

export const CONSTS = {
	OK: 'OK',
	CANCEL: 'Cancel',
	placeholder: 'Please enter..',
};

export function alert(msg, cb = util.noop) {
	var dag = render(
		<Dialog buttons=[CONSTS.OK] onAction=(e=>cb(e.data))>{msg}</Dialog>
	);
	dag.show();
	return dag;
}

export function confirm(msg, cb = util.noop) {
	var dag = render(
		<Dialog buttons=[CONSTS.CANCEL, CONSTS.OK] onAction=(e=>cb(e.data))>{msg}</Dialog>
	);
	dag.show();
	return dag;
}

function handle_prompt_enter(ev) {
	var dag = ev.sender.owner;
	dag.trigger('Action', 1);
	actionClose(dag);
}

export function prompt(msg, text = '', cb = util.noop) {
	if ( typeof text == 'function' ) {
		cb = text;
		text = '';
	}
	var dag = render(
		<Dialog buttons=[CONSTS.CANCEL, CONSTS.OK] onAction=(e=>cb(e.data, e.data ? dag.IDs.input.value: ''))>
			<Span>
				{msg}
				<Input id="input" class="prompt"
					returnType="done" onKeyEnter=handle_prompt_enter
					value=text placeholder=CONSTS.placeholder />
			</Span>
		</Dialog>
	);
	dag.show();
	dag.onMounted.once(e=>dag.IDs.input.focus());
	return dag;
}

export function show(title, msg, buttons, cb = util.noop) {
	var dag = render(
		<Dialog title=title buttons=buttons onAction=(e=>cb(e.data))>{msg}</Dialog>
	);
	dag.show();
	return dag;
}

export function sheet(buttons, cb = util.noop) {
	var dag = render(
		<Sheet buttons=buttons onAction=(e=>cb(e.data)) />
	);
	dag.show();
	return dag;
}