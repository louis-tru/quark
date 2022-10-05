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

import utils from './util';
import quark, {
	Indep, Hybrid, Clip, Input, Span, LimitIndep, Button, View, _CVD,
} from './index';
import { Navigation } from './nav';
import { event, EventNoticer, Event, ClickEvent } from './event';
import { prop } from './ctr';
import * as value from './value';

const {atomPixel: px, render} = quark;

quark.css({
	
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
		// textSize: 16,
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
		textColor: '#333',
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

	'.x_dialog.sheet .button': {
		height: 45,
		textLineHeight: 45,
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

export const CONSTS = {
	OK: 'OK',
	CANCEL: 'Cancel',
	placeholder: 'Please enter..',
};

/**
 * @class Dialog
 */
export class Dialog extends Navigation {
	private m_buttons = [];

	private _compute_buttons_width() {
		var self = this;
		var len = self.length;
		if (!len || !self.domAs().visible)
			return;
		
		if ( len == 1 ) {
			(self.find<Clip>('btns').first as Button).width = value.parseValue('full');
		} else  {
			var main_width = self.find<Indep>('main').finalWidth;
			if ( main_width ) {
				var btn = self.find<Clip>('btns').first as Button;
				while (btn) {
					btn.width = new value.Value(value.ValueType.PIXEL, (main_width / len) - ((len - 1) * px));
					btn.borderLeft = value.parseBorder(`${px} #9da1a0`);
					btn.borderTopWidth = px;
					btn = btn.next as Button;
				}
				(self.find<Clip>('btns').first as Button).borderLeftWidth = 0;
			}
		}
	}

	private _actionClose() {
		var self = this;
		if (self.actionClose) 
			self.close();
	}

	@prop title = '';
	@prop content = '';
	@event readonly onAction: EventNoticer<Event<number>>;

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

	render(...vdoms: any[]) {
		return (
			<Indep width="100%" height="100%" backgroundColor="#0008" receive={true} visible={false} opacity={0}>
				<LimitIndep id="main" class="x_dialog main">
					<Hybrid id="title" class="title">{this.title}</Hybrid>
					<Hybrid id="con" class="content">{this.content||vdoms}</Hybrid>
					<Clip id="btns" class="buttons">
					{
						this.m_buttons.map((e, i)=>(
							<Button 
								index={i}
								class="button"
								borderTopWidth={px}
								onClick={(e:any)=>this._handleClick(e)}
								defaultHighlighted={0}>{e}</Button>
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

	protected triggerUpdate() {
		this._compute_buttons_width();
		return super.triggerUpdate();
	}

	show() {
		if (!this.domAs().visible) {
			this.appendTo(quark.root);
			this.domAs().visible = true;
			quark.nextFrame(()=>{
				this._compute_buttons_width();
				var main = this.IDs.main as Indep;
				main.originX = main.finalWidth / 2;
				main.originY = main.finalHeight / 2;
				main.scale = new value.Vec2(0.2, 0.2);
				main.transition({ scale : '1 1', time: 250 });
				this.domAs().opacity = 0.2;
				this.domAs().transition({ opacity : 1, time: 250 });
			});
			this.registerNavigation(0);
		}
	}

	close() {
		if ( this.domAs().visible ) {
			var main = this.IDs.main as Indep;
			main.originX = main.finalWidth / 2;
			main.originY = main.finalHeight / 2;
			main.transition({ scale : '0.2 0.2', time: 300 });
			this.domAs().transition({ opacity : 0.05, time: 300 }, ()=>{ this.remove() });
			this.unregisterNavigation(0);
		} else {
			this.unregisterNavigation(0);
			this.remove();
		}
	}

	protected triggerAction(index: number) {
		this.trigger('Action', index);
	}

	protected _handleClick(evt: ClickEvent) {
		this.triggerAction((evt.sender as any).index);
		this._actionClose();
	}

	navigationBack() {
		if ( this.length ) {
			this.triggerAction(0);
		}
		this._actionClose();
		return true;
	}

	navigationEnter(focus: View) {
		if ( !this.domAs().hasChild(focus) ) {
			if ( this.length ) {
				this.triggerAction(this.length - 1);
			}
			this._actionClose();
		}
	}
}

/**
 * @class Sheet
 */
export class Sheet extends Dialog {

	protected triggerUpdate() {
		return (Navigation as any).prototype.triggerUpdate.call(this);
	}

	render(...vdoms: any[]) {
		var length = this.length;
		var content = this.content ? this.content : vdoms.length ? vdoms: null;
		return (
			<Indep width="100%" height="100%" backgroundColor="#0008" onClick={()=>this.navigationBack()} visible={0} opacity={0}>ABCD
			{content?
				<Indep id="main" class="x_dialog sheet">{content}</Indep>:
				<Indep id="main" class="x_dialog sheet">
					<Clip class="buttons">
					{
						length?
						this.buttons.slice().map((e,i)=>(
							<Button 
								index={length-i}
								class="button"
								width="100%"
								onClick={(e:any)=>this._handleClick(e)}
								borderTopWidth={i?px:0}
								defaultHighlighted={0}>{e}</Button>
						)):
						<Button 
							index={1}
							class="button"
							width="100%"
							onClick={(e:any)=>this._handleClick(e)}
							defaultHighlighted={0}>{CONSTS.OK}</Button>
					}
					</Clip>
					<Clip class="buttons">
						<Button 
							index={0}
							class="button gray"
							width="100%"
							onClick={(e:any)=>this._handleClick(e)}
							defaultHighlighted={0}>{CONSTS.CANCEL}</Button>
					</Clip>
				</Indep>
			}
			</Indep>
		);
	}

	show() {
		if (!this.domAs().visible) {
			this.appendTo(quark.root);
			this.domAs().visible = true;
			quark.nextFrame(()=>{
				var main = this.IDs.main as Indep;
				var height = main.finalHeight;
				main.y = height;
				main.transition({ y: 0, time: 250 });
				this.domAs().opacity = 0.3;
				this.domAs().transition({ opacity : 1, time: 250 });
			});
			this.registerNavigation(0);
		}
	}
	
	close() {
		if ( this.domAs().visible ) {
			var main = this.IDs.main as Indep;
			var height = main.finalHeight;
			main.transition({ y: height, time: 250 });
			this.domAs().transition({ opacity : 0.15, time: 250 }, ()=>{ this.remove() });
			this.unregisterNavigation(0);
		} else {
			this.unregisterNavigation(0);
			this.remove();
		}
	}

}

export function alert(msg: string | {msg?:string, title?:string}, cb = utils.noop) {
	var message: any;
	if (typeof msg == 'string')
		message = {msg};
	var { msg: _msg = '', title = '' } = message;
	var dag = render(
		<Dialog buttons={[CONSTS.OK]} onAction={()=>cb()} title={title}>{_msg}</Dialog>
	) as Dialog;
	dag.show();
	return dag;
}

export function confirm(msg: string, cb: (ok: boolean)=>void = utils.noop) {
	var dag = render(
		<Dialog buttons={[CONSTS.CANCEL, CONSTS.OK]} onAction={(e:any)=>cb(e.data)}>{msg}</Dialog>
	) as Dialog;
	dag.show();
	return dag;
}

export function prompt(msg: string | { msg?: string, text?: string, placeholder?: string, security?: boolean },
	cb: (ok: boolean, str: string)=>void = utils.noop) 
{
	var message: any;
	if (typeof msg == 'string')
		message = {msg};
	var { msg: _msg = '', text = '', placeholder = CONSTS.placeholder, security = false } = message;
	var dag = render(
		<Dialog 
			action_time={100} 
			buttons={[CONSTS.CANCEL, CONSTS.OK]} 
			onAction={(e:any)=>cb(e.data, e.data ? dag.find<Input>('input').value: '')}
		>
			<Span>
				{_msg}
				<Input security={security} id="input" class="prompt"
					returnType="done" onKeyEnter={()=>{
						(dag as any).triggerAction(1);
						(dag as any)._actionClose();
					}}
					value={text} placeholder={placeholder} />
			</Span>
		</Dialog>
	) as Dialog;
	dag.onMounted.once((e:any)=>dag.find<Input>('input').focus());
	dag.show();
	return dag;
}

export function show(title: string, msg: string, buttons: string[] = [CONSTS.OK], cb: (index: number)=>void = utils.noop) {
	var dag = render(
		<Dialog title={title} buttons={buttons} onAction={(e: Event<number>)=>cb(e.data)}>{msg}</Dialog>
	) as Dialog;
	dag.show();
	return dag;
}

export function sheet(content: any) {
	var dag = render(
		<Sheet content={content} />
	) as Sheet;
	dag.show();
	return dag;
}

export function sheetConfirm(buttons: string[] = [CONSTS.OK], cb: (index: number)=>void = utils.noop) {
	var dag = render(
		<Sheet buttons={buttons} onAction={(e: Event<number>)=>cb(e.data)} />
	) as Sheet;
	dag.show();
	return dag;
}