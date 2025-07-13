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
import {View,Matrix,Input} from './view';
import {_CVD,link,ViewController,RenderData} from './ctr';
import {createCss} from './css';
import {mainScreenScale} from './screen';
import {Navigation} from './nav';
import {Window} from './window';
import {ClickEvent} from './event';

const px = 1 / mainScreenScale();

createCss({
	'.qk_dialog': {
	},
	'.qk_dialog.main': {
		minWidth: 160, // min width
		maxWidth: '40!',// max width
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
		weight: 1,
	},
	'.qk_dialog .content': {
		weight: 1,
		textSize: 14,
		width: 'match',
		margin: [2,10,20,10],
		textAlign: 'center',
		textColor: '#333',
		textWhiteSpace: "preLine",
		clip: true,
		//backgroundColor: '#f00',
	},
	'.qk_dialog .buttons': {
		width: 'match',
		weight: 1,
		borderTop: `${px} #9da1a0`,
	},
	'.qk_dialog.sheet .buttons': {
		marginTop: 10,
		borderWidthTop: 0,
	},
	'.qk_dialog .button': {
		height: 43,
		minWidth: 68,
		maxWidth: 'match',
		weight: 1,
		padding: [0,5],
		borderColor: `#9da1a0`,
		textSize: 18,
		textLineHeight: 1,
		textColor:"#0079ff",
		textAlign: "center",
		textOverflow: 'ellipsis',
		textWhiteSpace: 'noWrap',
	},
	'.qk_dialog.sheet .button': {
		height: 45,
		width: 'match',
		maxWidth: 'none',
	},
	'.qk_dialog .button.gray': {
		textColor: '#555',
	},
	'.qk_dialog .button:normal': {
		backgroundColor: '#fff', time: 180
	},
	'.qk_dialog .button:hover': {
		backgroundColor: '#f4f4f4', time: 50
	},
	'.qk_dialog .button:active': {
		backgroundColor: '#E9E9E9', time: 50
	},
	'.qk_dialog .prompt': {
		marginTop: 10,
		padding: [0,5],
		width: "match",
		height: 30,
		backgroundColor: "#eee",
		borderRadius: 8,
	},
});

/**
 * @const Consts:...
*/
export const Consts = {
	Ok: 'OK', //!< {'OK'}
	Cancel: 'Cancel', //!< {'Cancel'}
	Placeholder: 'Please enter..', //!< {'Please enter..'}
};

/**
 * @class Dialog
 * @extends Navigation
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

	/**
	 * buttons length
	*/
	get length(): Uint {
		return this._buttons.length;
	}

	@link title: string = ''; //!<
	@link content: string = ''; //!<
	@link autoClose: boolean = true; //!<

	/**
	 * @getset buttons:RenderData[]
	*/
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

	protected handleClick = (e: ClickEvent)=>{
		this.triggerAction((e.sender as any as {key:number}).key);
	};

	protected render() {
		return (
			<free width="100%" height="100%" backgroundColor="#0004" receive={true} visible={false} opacity={0}>
				<matrix ref="main" class="qk_dialog main">
					<flex width="match" height="match" direction="column" crossAlign="both">
						<text ref="title" class="title" value={this.title} visible={true} />
						<text ref="con" class="content" visible={true}>{this.content||this.children}</text>
						<flex ref="btns" class="buttons" visible={!!this._buttons.length}>
						{
							this._buttons.map((e,i,arr)=>(
								<button
									key={i}
									class="button"
									borderWidthLeft={i ? px: 0}
									borderRadiusLeftBottom={i == 0 ? 12: 0}
									borderRadiusRightBottom={i == arr.length-1 ? 12: 0}
									onClick={this.handleClick}
								>{e}</button>
							))
						}
						</flex>
					</flex>
				</matrix>
			</free>
		);
	}

	/**
	 * @method show()
	*/
	show() {
		if (!this.asDom().visible) {
			super.appendTo(this.window.root);
			this.asDom().visible = true;
			this.asRef<Matrix>('main').transition({ scale: 1, time: 300 }, {scale : 0.2});
			this.asDom().transition({ opacity : 1, time: 300 }, {opacity: 0.2});
			this.registerNavigation(0);
		}
	}

	/**
	 * @method close()
	*/
	close() {
		if ( this.asDom().visible ) {
			this.asRef<Matrix>('main').transition({ scale: 0.2, time: 300 });
			this.asDom().transition({ opacity : 0, time: 300 }, ()=>this.destroy());
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
 * @extends Dialog
 */
export class Sheet<P={},S={}> extends Dialog<P,S> {
	protected triggerUpdate() {
		return (Navigation as any).prototype.triggerUpdate.call(this);
	}

	render() {
		let content = this.content ? this.content :
			this.children.length ? this.children: null;
		return (
			<free width="100%" height="100%" backgroundColor="#0004" visible={false} opacity={0}
				onClick={()=>this.navigationBack()}
			>
			{content?
				<matrix ref="main" class="qk_dialog sheet">{content}</matrix>:
				<matrix ref="main" class="qk_dialog sheet">
					<box class="buttons">
					{ this.length?
						this.buttons.map((e,i,arr)=>(
							<button
								key={arr.length-i}
								class="button"
								width="100%"
								onClick={this.handleClick}
								borderWidthTop={i?px:0}
								borderRadius={[i?0:12,i==arr.length-1?12:0]}
							>{e}</button>
						)):
						<button
							key={1}
							class="button"
							width="100%"
							onClick={this.handleClick}
							value={Consts.Ok}
							borderRadius={12}
						/>
					}
					</box>
					<box class="buttons">
						<button
							key={0}
							class="button gray"
							width="100%"
							onClick={this.handleClick}
							value={Consts.Cancel}
							borderRadius={12}
						/>
					</box>
				</matrix>
			}
			</free>
		);
	}

	show() {
		if (!this.asDom().visible) {
			ViewController.prototype.appendTo.call(this, this.window.root);
			this.asDom().visible = true;
			this.window.nextTickFrame(()=>{
				let main = this.refs.main as Matrix;
				main.transition({ y: 0, time: 300 }, {y: main.clientSize.y});
				this.asDom().transition({ opacity: 1, time: 300 }, {opacity: 0.2});
			});
			this.registerNavigation(0);
		}
	}

	close() {
		if ( this.asDom().visible ) {
			let main = this.refs.main as Matrix;
			main.transition({ y: main.clientSize.y, time: 300 });
			this.asDom().transition({ opacity : 0, time: 300 }, ()=>{ this.destroy() });
			this.unregisterNavigation(0);
		} else {
			this.unregisterNavigation(0);
			this.destroy();
		}
	}
}
/** @end */

/**
 * @method alert(window,msg,cb?)
 * @param window:Window
 * @param msg:string|object
 * @param cb?:Function
 * @return {Dialog}
*/
export function alert(window: Window, msg: string | {msg?:string, title?: string}, cb = util.noop) {
	let message: any;
	if (typeof msg == 'string')
		message = {msg};
	let { msg: _msg = '', title = '' } = message;
	let dag = (
		<Dialog buttons={[Consts.Ok]} onAction={cb} title={title}>{_msg}</Dialog>
	).newDom(window.rootCtr) as Dialog;
	dag.show();
	return dag;
}

/**
 * @method confirm(window,msg,cb?)
 * @param window:Window
 * @param ms:string
 * @param cb:Function
 * @return {Dialog}
*/
export function confirm(window: Window, msg: string, cb: (ok: boolean)=>void = util.noop) {
	let dag = (
		<Dialog buttons={[Consts.Cancel, Consts.Ok]} onAction={e=>cb(!!e)}>{msg}</Dialog>
	).newDom(window.rootCtr) as Dialog;
	dag.show();
	return dag;
}

/**
 * @method prompt(window,msg,cb?)
 * @param window:Window
 * @param msg:string|object
 * @param cb?:Function
 * @return {Dialog}
*/
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
			{_msg}{"\n"}
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
	//(dag.refs.input as Input).focus();
	return dag;
}

/**
 * @method show(window,title,msg,buttons?,cb?)
 * @param window:Window
 * @param title:string
 * @param msg:string
 * @param buttons?:RenderData[]
 * @param cb?:Function
 * @return {Dialog}
*/
export function show(window: Window, title: string, msg: string, buttons: RenderData[] = [Consts.Ok], cb: (index: number)=>void = util.noop) {
	let dag = (
		<Dialog title={title} buttons={buttons} onAction={cb}>{msg}</Dialog>
	).newDom(window.rootCtr) as Dialog;
	dag.show();
	return dag;
}

/**
 * @return {Sheet}
*/
export function sheet(window: Window, content: string) {
	let dag = (<Sheet content={content} />).newDom(window.rootCtr) as Sheet;
	dag.show();
	return dag;
}

/**
 * @method sheetConfirm(window,buttons?,cb?)
 * @param window:Window
 * @param buttons?:RenderData[]
 * @param cb?:Function
 * @return {Sheet}
*/
export function sheetConfirm(window: Window, buttons: RenderData[] = [Consts.Ok], cb: (index: number)=>void = util.noop) {
	let dag = (
		<Sheet buttons={buttons} onAction={cb} />
	).newDom(window.rootCtr) as Sheet;
	dag.show();
	return dag;
}

/**
 * @class DialogController
 * @extends ViewController
*/
export class DialogController<P={},S={}> extends ViewController<P,S> {

	/**
	 * @method alertDialog(msg:string:object,cb?)Dialog
	 */
	alertDialog(msg: string | {msg?:string, title?: string}, cb = util.noop) {
		return alert(this.window, msg, cb);
	}

	/**
	 * @method promptDialog(msg:string|object,cb?)Dialog
	 */
	promptDialog(msg: string | { msg?: string, text?: string, placeholder?: string, security?: boolean },
		cb: (ok: boolean, str: string)=>void = util.noop
	) {
		return prompt(this.window, msg, cb);
	}

	/**
	 * @method confirmDialog(msg:string,cb?)Dialog
	 */
	confirmDialog(msg: string, cb: (ok: boolean)=>void = util.noop) {
		return confirm(this.window, msg, cb);
	}

	/**
	 * @method showDialog(title:string,msg:string,buttons?:RenderData[],cb?:Function)Dialog
	 */
	showDialog(title: string, msg: string, buttons: RenderData[] = [Consts.Ok], cb: (index: number)=>void = util.noop) {
		return show(this.window, title, msg, buttons, cb);
	}

	/**
	* @method sheetDialog(content:string)Sheet
	*/
	sheetDialog(content: string) {
		return sheet(this.window, content);
	}
}
