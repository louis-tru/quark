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

import util from './util';
import {
	_CVD,mainScreenScale,createCss,link,View,Matrix,Input,
	ViewController, RenderData
} from './index';
import {Navigation} from './nav';
import {Window} from './window';
import * as types from './types';
import {ClickEvent} from './event';

const px = 1 / mainScreenScale();

createCss({
	'.qk_dialog': {
	},
	'.qk_dialog.main': {
		minWidth: 160, // min width
		maxWidth: '40!',// max width
		//maxHeight: '40!',
		align: 'centerMiddle',
		backgroundColor: '#fff',
		borderRadius: 12,
	},
	'.qk_dialog.sheet': {
		width: 'match',
		maxWidth: 'none',
		margin: 10,
		align: 'centerBottom',
	},
	'.qk_dialog .title': {
		width: 'match',
		margin: [18,10,0,10],
		textAlign: 'center',
		textWeight: 'bold',
		textSize: 18,
		textOverflow: 'ellipsis',
		textWhiteSpace: 'noWrap',
		backgroundColor: '#f00',
		align: "centerNew"
	},
	'.qk_dialog .content': {
		width: 'match',
		margin: [2,10,20,10],
		textAlign: 'center',
		textSize: 14,
		textColor: '#333',
		backgroundColor: '#ff0',
		align: "centerNew"
	},
	'.qk_dialog .buttons': {
		width: 'match',
		borderRadiusLeftBottom: 12,
		borderRadiusRightBottom: 12,
	},
	'.qk_dialog.sheet .buttons': {
		borderRadius: 12,
		backgroundColor: '#fff',
		marginTop: 10,
	},
	'.qk_dialog .button': {
		height: 43,
		// borderTop: `${px} #9da1a0`,
		borderColorTop: `#9da1a0`,
		textSize: 18,
		textLineHeight: 43,
		textColor:"#0079ff",
	},
	'.qk_dialog.sheet .button': {
		height: 45,
		textLineHeight: 45,
	},
	'.qk_dialog .button.gray': {
		textColor:"#000",
	},
	'.qk_dialog .button:normal': {
		backgroundColor: '#fff', time: 180
	},
	'.qk_dialog .button:hover': {
		backgroundColor: '#E1E4E455', time: 50
	},
	'.qk_dialog .button:down': {
		backgroundColor: '#E1E4E4', time: 50
	},
	'.qk_dialog .prompt': {
		marginTop: 10,
		width: "match",
		height: 30,
		backgroundColor: "#eee",
		borderRadius: 8,
	},
});

export const Consts = {
	Ok: 'OK',
	Cancel: 'Cancel',
	Placeholder: 'Please enter..',
};

/**
 * @class Dialog
 */
export class Dialog<P={},S={}> extends Navigation<{
	onAction?:(e:number, sender: Dialog<P,S>)=>void;
	title?: string;
	content?: string;
	autoClose?: boolean;
	buttons?: RenderData[];
}&P,S> {
	private _buttons = [] as RenderData[];

	private _autoClose() {
		if (this.autoClose)
			this.close();
	}

	get length() {
		return this._buttons.length;
	}

	@link title = '';
	@link content = '';
	@link autoClose = true;

	@link
	get buttons() {
		return this._buttons;
	}

	set buttons(value) {
		if ( Array.isArray(value) ) {
			this._buttons = value;
			this.update();
		}
	}

	protected triggerAction(index: number) {
		this.props.onAction?.call(null, index, this);
		this._autoClose();
	}

	private handleClick = (e: ClickEvent)=>{
		let idx = this.asRef<View>('btns').childDoms.indexOf(e.origin);
		this.triggerAction(idx);
	};

	protected render() {
		return (
			<free width="100%" height="100%" backgroundColor="#0004" receive={true} visible={false} opacity={0}>
				<matrix ref="main" class="qk_dialog main">
					<text ref="title" class="title" value={this.title} />
					<text ref="con" class="content">{this.content||this.children}</text>
					{/* <free ref="btns" class="buttons">
					{
						this._buttons.map((e,i)=>(
							<button
								key={i}
								class="button"
								borderWidthTop={px}
								onClick={this.handleClick}
							>{e}</button>
						))
					}
					</free> */}
				</matrix>
			</free>
		);
	}

	show() {
		if (!this.asDom().visible) {
			super.appendTo(this.window.root);
			this.asDom().visible = true;
			this.window.nextFrame(()=>{
				let main = this.refs.main as Matrix;
				let size = main.clientSize;
				main.scale = new types.Vec2({x:0.2, y:0.2});
				main.transition({ scale : '1 1', time: 30000 });
				this.asDom().opacity = 0.2;
				this.asDom().transition({ opacity : 1, time: 30000 });
			});
			this.registerNavigation(0);
		}
	}

	close() {
		if ( this.asDom().visible ) {
			let main = this.refs.main as Matrix;
			let size = main.clientSize;
			main.transition({ scale : '0.2 0.2', time: 300 });
			this.asDom().transition({ opacity : 0.05, time: 300 }, ()=>{ this.destroy() });
			this.unregisterNavigation(0);
		} else {
			this.unregisterNavigation(0);
			this.destroy();
		}
	}

	navigationBack() {
		if ( this.length ) {
			this.triggerAction(0);
		} else {
			this._autoClose();
		}
		return true;
	}

	navigationEnter(focus: View) {
		if ( !this.asDom().isSelfChild(focus) ) {
			if ( this.length ) {
				this.triggerAction(this.length - 1);
			} else {
				this._autoClose();
			}
		}
	}

	appendTo(): View { throw Error.new('Access forbidden.') }
	afterTo(): View { throw Error.new('Access forbidden.') }
}

/**
 * @class Sheet
 */
export class Sheet<P={},S={}> extends Dialog<P,S> {
	protected triggerUpdate() {
		return (Navigation as any).prototype.triggerUpdate.call(this);
	}

	render() {
		let length = this.length;
		let content = this.content ? this.content :
			this.children.length ? this.children: null;
		return (
			<free
				width="100%" height="100%"
				backgroundColor="#0008"
				onClick={()=>this.navigationBack()} visible={false} opacity={0}
			>
			{content?
				<free ref="main" class="qk_dialog sheet">{content}</free>:
				<free ref="main" class="qk_dialog sheet">
					<free class="buttons" clip={true}>
					{
						length?
						this.buttons.slice().map((e,i)=>(
							<button
								key={i}
								class="button"
								width="100%"
								onClick={e=>this.triggerAction(length-i)}
								borderWidthTop={i?px:0}
							>{e}</button>
						)):
						<button
							class="button"
							width="100%"
							onClick={e=>this.triggerAction(1)}
							value={Consts.Ok}
						/>
					}
					</free>
					<free class="buttons" clip={true}>
						<button
							class="button gray"
							width="100%"
							onClick={()=>this.triggerAction(0)}
							value={Consts.Cancel}
						/>
					</free>
				</free>
			}
			</free>
		);
	}

	show() {
		if (!this.asDom().visible) {
			ViewController.prototype.appendTo.call(this, this.window.root);
			this.asDom().visible = true;
			this.window.nextFrame(()=>{
				let main = this.refs.main as Matrix;
				main.y = main.clientSize.y;
				main.transition({ y: 0, time: 250 });
				this.asDom().opacity = 0.3;
				this.asDom().transition({ opacity : 1, time: 250 });
			});
			this.registerNavigation(0);
		}
	}

	close() {
		if ( this.asDom().visible ) {
			let main = this.refs.main as Matrix;
			main.transition({ y: main.clientSize.y, time: 250 });
			this.asDom().transition({ opacity : 0.15, time: 250 }, ()=>{ this.destroy() });
			this.unregisterNavigation(0);
		} else {
			this.unregisterNavigation(0);
			this.destroy();
		}
	}
}

export function alert(window: Window, msg: string | {msg?:string, title?: string}, cb = util.noop) {
	let message: any;
	if (typeof msg == 'string')
		message = {msg};
	let { msg: _msg = '', title = 'AAAAABBCCCCDDDD' } = message;
	let dag = (
		<Dialog buttons={[Consts.Ok]} onAction={cb} title={title}>{_msg}</Dialog>
	).newDom(window.rootCtr) as Dialog;
	dag.show();
	return dag;
}

export function confirm(window: Window, msg: string, cb: (ok: boolean)=>void = util.noop) {
	let dag = (
		<Dialog buttons={[Consts.Cancel, Consts.Ok]} onAction={e=>cb(!!e)}>{msg}</Dialog>
	).newDom(window.rootCtr) as Dialog;
	dag.show();
	return dag;
}

export function prompt(window: Window, msg: string | {
		msg?: string, text?: string, placeholder?: string, security?: boolean 
	},
	cb: (ok: boolean, str: string)=>void = util.noop
) {
	let message: any;
	if (typeof msg == 'string')
		message = {msg};
	let { msg: _msg = '', text = '', placeholder = Consts.Placeholder, security = false } = message;
	let dag = (
		<Dialog
			action_time={100}
			buttons={[Consts.Cancel, Consts.Ok]} 
			onAction={e=>cb(!!e, e ? (dag.refs.input as Input).value: '')}
		>
			{_msg}
			<input
				security={security}
				ref="input"
				class="prompt"
				returnType="done"
				value={text}
				placeholder={placeholder}
				onKeyEnter={()=>(dag as any).triggerAction(1)}
			/>
		</Dialog>
	).newDom(window.rootCtr) as Dialog;
	dag.show();
	(dag.refs.input as Input).focus();
	return dag;
}

export function show(window: Window, title: string, msg: string, buttons: RenderData[] = [Consts.Ok], cb: (index: number)=>void = util.noop) {
	let dag = (
		<Dialog title={title} buttons={buttons} onAction={cb}>{msg}</Dialog>
	).newDom(window.rootCtr) as Dialog;
	dag.show();
	return dag;
}

export function sheet(window: Window, content: string) {
	let dag = (<Sheet content={content} />).newDom(window.rootCtr) as Sheet;
	dag.show();
	return dag;
}

export function sheetConfirm(window: Window, buttons: RenderData[] = [Consts.Ok], cb: (index: number)=>void = util.noop) {
	let dag = (
		<Sheet buttons={buttons} onAction={cb} />
	).newDom(window.rootCtr) as Sheet;
	dag.show();
	return dag;
}

export class DialogController<P={},S={}> extends ViewController<P,S> {
	alertDialog(msg: string | {msg?:string, title?: string}, cb = util.noop) {
		return alert(this.window, msg, cb);
	}
	promptDialog(msg: string | { msg?: string, text?: string, placeholder?: string, security?: boolean },
		cb: (ok: boolean, str: string)=>void = util.noop
	) {
		return prompt(this.window, msg, cb);
	}
	confirmDialog(msg: string, cb: (ok: boolean)=>void = util.noop) {
		return confirm(this.window, msg, cb);
	}
	showDialog(title: string, msg: string, buttons: RenderData[] = [Consts.Ok], cb: (index: number)=>void = util.noop) {
		return show(this.window, title, msg, buttons, cb);
	}
	sheetDialog(content: string) {
		return sheet(this.window, content);
	}
}
