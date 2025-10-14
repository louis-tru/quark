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
import { List,ListIterator,ClickType } from './event';
import { KeyboardKeyCode } from './keyboard';
import {Window} from './window';
import {Jsx,link,ViewController,RenderResult,VDom,assertDom,VirtualDOM} from './ctr';
import {mainScreenScale} from './screen';
import {View,Label,Text,Box} from './view';
import * as types from './types';
import app from './app';
import {createCss} from './css';

const px = 1 / mainScreenScale();
const Transition_Time = 400;
const Navigation_Stack: WeakMap<Window, List<Navigation>> = new WeakMap();

/**
 * @enum NavStatus
*/
export enum NavStatus {
	Init = -1, //!<
	Foreground,//!<
	Background,//!<
}

/**
 * @class NavigationStatus
 * @extends ViewController
 * 
 * Navigation component base type
 */
class NavigationStatus<P={},S={}> extends ViewController<P,S> {

	/**
	 * 0=init or exit,1=background,2=foreground
	 */
	readonly navStatus: NavStatus = NavStatus.Init;

	/**
	 * When the component leaves
	*/
	intoLeave(animate: number) {
		(this as any).navStatus = NavStatus.Init;
	}

	/**
	 * When the component enters the background
	*/
	intoBackground(animate: number) {
		(this as any).navStatus = NavStatus.Background;
	}

	/**
	 * When the component enters the foreground
	*/
	intoForeground(animate: number) {
		(this as any).navStatus = NavStatus.Foreground;
	}
}

/**
 * @class Navigation
 * @extends NavigationStatus
 */
export class Navigation<P={},S={}> extends NavigationStatus<{
	onBackground?:(selnder: Navigation)=>void,
	onForeground?:(selnder: Navigation)=>void,
}&P,S> {
	private _iterator: ListIterator<Navigation> | null = null;
	private _focusResume: View | null = null;

	/**
	 * @get navStack:List<Navigation>
	 * 
	 * The navigation stack to which the navigation component belongs
	*/
	protected readonly navStack = (function(window: Window) {
		let stack = Navigation_Stack.get(window);
		if (stack)
			return stack;

		// Init global navigation stack
		stack = new List<Navigation>;
		Navigation_Stack.set(window, stack);

		const root = window.root;

		window.onClose.on(function() {
			Navigation_Stack.delete(window);
		});

		root.onBack.on(function(e) {
			let last = stack.end.prev;
			while (last.value) {
				if ( last.value.navigationBack() ) {
					e.cancelDefault(); // Cancel default action
					break;
				}
				last = last.prev;
			}
		});

		root.onClick.on(function(e) {
			if ( e.type == ClickType.Keyboard ) { // Keyboard triggered events
				let back = stack.back;
				if ( back ) {
					back.navigationEnter(e.sender);
				}
			}
		});

		root.onKeyDown.on(function(e) {
			let nav = stack.back;
			if ( nav ) {
				let nextFocus = e.nextFocus;
	
				switch(e.keycode) {
					case KeyboardKeyCode.LEFT: // left
						nextFocus = nav.navigationLeft(nextFocus); break;
					case KeyboardKeyCode.UP: // up
						nextFocus = nav.navigationTop(nextFocus); break;
					case KeyboardKeyCode.RIGHT: // right
						nextFocus = nav.navigationRight(nextFocus); break;
					case KeyboardKeyCode.DOWN: // down
						nextFocus = nav.navigationDown(nextFocus); break;
					case KeyboardKeyCode.MENU: // menu
						nav.navigationMenu();
					default: return;
				}
				e.nextFocus = nextFocus;
			}
		});

		return stack;
	})(this.window);

	/**
	 * When initializing navigation, return a focus view
	 */
	initFocus(): View | null {
		return null;
	}

	intoBackground(animate: number) {
		super.intoBackground(animate);
		this.triggerBackground();
	}

	intoForeground(animate: number) {
		super.intoForeground(animate);
		this.triggerForeground();
	}

	protected triggerBackground() {
		this.props.onBackground?.call(null, this);
	}

	protected triggerForeground() {
		this.props.onForeground?.call(null, this);
	}

	protected triggerUnload() {
		if ( this._iterator ) { // force delete global nav stack
			this.navStack.remove(this._iterator);
			this._iterator = null;
		}
	}

	/**
	 * Called when a navigation component is created
	 * @method registerNavigation(animate)
	 * @param animate?:Uint
	 */
	registerNavigation(animate: Uint = 0) {
		if ( !this._iterator ) { // No need to repeat it
			this._iterator = this.navStack.pushBack(this);
			let prev = this._iterator.prev;
			if ( prev !== this.navStack.end ) {
				let focus = this.window.focusView;
				prev.value._focusResume =
					focus && prev.value.metaView.isSelfChild(focus) ? focus : null;
				prev.value.intoBackground(animate);
			}
			let view = this.initFocus();
			if ( view ) {
				view.focus();
			}
			this.intoForeground(animate);
		}
	}

	/**
	 * Called when a navigation component is unmounted
	 * @method unregisterNavigation(animate)
	 * @param animate?:Uint
	 */
	unregisterNavigation(animate: Uint = 0) {
		if ( this._iterator ) {
			let last = this.navStack.back;
			this.navStack.remove(this._iterator);
			this._iterator = null;
			if (!last || last !== this)
				return;

			this.intoLeave(animate);
			last = this.navStack.back;
			if ( last ) {
				if (last._focusResume) {
					last._focusResume.focus();
				}
				last.intoForeground(animate);
			}
		}
	}

	/**
	 * * When a navigation event occurs, the system will first send the event to the focus view,
	 * 	If the event can be successfully transmitted to root,
	 * 	The event will ultimately be sent to the top of the current navigation list stack
	 * 
	 * * If false is returned here,
	 * 	it will continue to pass to the bottom of the navigation list stack,
	 * 	Until it returns true or reaches the bottom of the stack to exit the application
	*/
	navigationBack(): boolean {
		return true;
	}

	/**
	 * Called when the remote control presses OK
	*/
	navigationEnter(focus: View) {
		// Rewrite this function to implement your logic
	}

	/**
	 * * Called when the remote control presses Up
	 * 
	 * * The focus will not change in any way is returning null
	 */
	navigationTop(focus: View | null): View | null {
		return focus;
	}

	/**
	 * * Called when the remote control presses Down
	 * 
	 * * The focus will not change in any way is returning null
	 */
	navigationDown(focus: View | null): View | null {
		return focus;
	}

	/**
	 * * Called when the remote control presses Left
	 * 
	 * * The focus will not change in any way is returning null
	 */
	navigationLeft(focus: View | null): View | null {
		return focus;
	}

	/**
	 * * Called when the remote control presses Right
	 * 
	 * * The focus will not change in any way is returning null
	 */
	navigationRight(focus: View | null): View | null {
		return focus;
	}

	/**
	 * When pressing the menu button, it will be called up
	 */
	navigationMenu() {
		// Rewrite this function to implement your logic
	}
}

/**
 * @class NavPageCollection
 * @extends Navigation
 */
export class NavPageCollection<P={},S={}> extends Navigation<{
	enableAnimate?: boolean,
	padding?: number;
	navbarHeight?: number;
	navbarHidden?: boolean;
	clip?: boolean; // clip box render
	onPush?: (page: NavPage, sender: NavPageCollection)=>void,
	onPop?: (page: NavPage, sender: NavPageCollection)=>void,
}&P,S> {
	private _substack = new List<NavPage>();
	private _busy = false;

	/**
	 * @getset enableAnimate
	 */
	@link     enableAnimate: boolean = true;
	@link.acc padding = app.current.screen.statusBarHeight; // ios/android, 20;
	@link.acc navbarHeight = 44;
	@link.acc navbarHidden = false;

	get length() { return this._substack.length }
	get pages() { return this._substack.toArray() }
	get current() {
		util.assert(this.length, 'Empty NavPageCollection');
		return this._substack.back!;
	}

	/**
	*/
	isCurrent(page: NavPage): boolean {
		return this._substack.back === page;
	}

	protected triggerPush(page: NavPage) {
		this.props.onPush?.call(null, page, this);
	}

	protected triggerPop(page: NavPage) {
		this.props.onPop?.call(null, page, this);
	}

	protected render() {
		return (
			<free width="100%" height="100%" clip={!!this.props.clip}>
				<free width="100%" height="100%" ref="page" />
				<free
					ref="navbar"
					width="100%"
					height={this.navbarHeight}
					paddingTop={Math.max(this.padding, 0)}
					visible={!this.navbarHidden}
					receive={false} // no receive event
				/>
			</free>
		);
	}

	protected triggerMounted() {
		let page = this.children[0];
		if (page) {
			this.push(page as VirtualDOM<NavPage>);
		}
		util.nextTick(()=>
			this.registerNavigation(0)
		);
		return super.triggerMounted();
	}

	protected triggerUnload() {
		for (let e of this.pages)
			e.destroy();
		return super.triggerUnload();
	}

	/**
	 * Add a navigation page to the end and display
	*/
	push(arg: VDom<NavPage>, animate?: boolean) {
		if ( this._busy )
			return;
		assertDom(arg, NavPage, 'The argument navpage is not of the correct type, only for NavPage type');

		let time = this.enableAnimate && animate && this.length ? Transition_Time : 0;
		let prev = this._substack.back;
		let page = arg.newDom(this);

		page.appendTo(this.refs.page as View);

		// set page
		(page as any).navStack = this._substack; // private props visit
		(page as any)._prevPage = prev; // private props visit

		if (prev) { // set next page
			(prev as any)._nextPage = page; // private props visit
		}

		this._busy = time ? true: false;
		if ( time ) {
			setTimeout(()=>{ this._busy = false }, time);
		}
		getNavbarDom(page).setBackText(prev ? prev.title : '');

		page.registerNavigation(time);

		// switch and animate
		this.triggerPush(page);
	}

	/**
	 * @method pop(animate?:boolean,count?:Uint)
	 * 
	 * Remove pages from the end of the navigation
	*/
	pop(animate: boolean = false, count = 1) {
		if ( this._busy )
			return;
		count = Number(count) || 0;
		count = Math.min(this.length - 1, count);

		if ( count < 1 ) {
			return;
		}
		let time = this.enableAnimate && animate ? Transition_Time : 0;
		let arr  = this.pages.splice(this.length - count);
		let next = arr.pop();
		if (!next)
			return;

		arr.forEach(page=>page.intoLeave(0)); // private props visit

		this._busy = time ? true: false;
		if ( time ) {
			setTimeout(()=>{ this._busy = false }, time);
		}

		next.unregisterNavigation(time);

		// switch and animate
		this.triggerPop(next);
	}

	navigationBack(): boolean {
		if (this._substack.length)
			return this._substack.back!.navigationBack();
		return false;
	}

	navigationEnter(focus: View) {
		if (this._substack.length)
			this._substack.back!.navigationEnter(focus);
	}

	navigationTop(focus: View | null) {
		if (this._substack.length)
			return this._substack.back!.navigationTop(focus);
		return focus;
	}

	navigationDown(focus: View | null) {
		if (this._substack.length)
			return this._substack.back!.navigationDown(focus);
		return focus;
	}

	navigationLeft(focus: View | null) {
		if (this._substack.length)
			return this._substack.back!.navigationLeft(focus);
		return focus;
	}

	navigationRight(focus: View | null) {
		if (this._substack.length)
			return this._substack.back!.navigationRight(focus);
		return focus;
	}

	navigationMenu() {
		if (this._substack.length)
			this._substack.back!.navigationMenu(); // private props visit
	}
}

createCss({
	'.qk_navbar_back:normal': {
		opacity: 1,
		// textShadow: "0 0 0 #000"
	},
	'.qk_navbar_back:hover': {
		opacity: 0.8,
		// textShadow: "0 0 4 #000"
	},
	'.qk_navbar_back:active': {
		opacity: 0.6,
		// textShadow: "0 0 6 #000"
	},
});

/**
 * @class Navbar
 * @extends NavigationStatus
 */
export class Navbar<P={},S={}> extends NavigationStatus<{
	hidden?: boolean;
	visibleBackIcon?: boolean;
	visibleBackText?: boolean;
	backTextColor?: types.ColorStrIn;
	titleTextColor?: types.ColorStrIn;
	menu?: VDom,
}&P,S> {
	private _backText: string = '';
	private _titleText: string = '';

	@link hidden: boolean = false; //!<
	@link visibleBackIcon: boolean = true; //!<
	@link visibleBackText: boolean = true; //!<
	@link backTextColor: types.ColorStrIn = '#000'; //!<
	@link titleTextColor: types.ColorStrIn = '#147EFF'; //!<

	/**
	 * @get page:NavPage
	*/
	get page() { return this.owner as NavPage }

	/**
	 * Set navbar back text
	 */
	setBackText(value: string) {
		if (!this.hidden)
			this.asRef<Label>('back_text').value = value;
		this._backText = value;
	}

	/**
	 * Set navbar title text
	 */
	setTitleText(value: string) {
		if (!this.hidden)
			this.asRef<Text>('title').value = value;
		this._titleText = value;
	}

	private _handleBack = ()=>{
		this.page.collection.pop(true);
	}

	protected render() {
		return (
			this.hidden ? null:
			<free width="100%" height="100%" align="centerBottom">
				{this.renderBody()}
				{this.children}
			</free>
		);
	}

	protected renderBody() {
		let showBack = !this.page.isFirstPage;
		return (
			<flex width="100%" height="100%" itemsAlign="centerCenter">
				<button
					class="qk_navbar_back"
					maxWidth="40%"
					height="100%"
					paddingLeft={5}
					paddingRight={5}
					textColor={this.backTextColor}
					textLineHeight={1} // 100%
					textSize={16}
					onClick={this._handleBack}
					textFamily="iconfont"
					value={showBack && this.visibleBackIcon ? "\uedc5": ""}
					textWhiteSpace="noWrap"
				>
					<label
						ref="back_text"
						textFamily="default"
						textSize={16}
						textOverflow="ellipsis"
						visible={showBack && this.visibleBackText}
						value={this._backText}
					/>
				</button>
				<text
					ref="title"
					weight={[0,1]}
					height="100%"
					textColor={this.titleTextColor}
					textLineHeight={1}
					textSize={16}
					textWhiteSpace="noWrap"
					textWeight="bold"
					textOverflow="ellipsisCenter"
					textAlign="center"
					value={this._titleText}
				/>
				<box
					maxWidth="40%"
					height="100%"
					paddingLeft={5}
				>{this.renderMenu()}</box>
			</flex>
		);
	}

	protected renderMenu() {
		return this.props.menu;
	}

	intoLeave(time: number) {
		if (this.navStatus == NavStatus.Foreground && time &&
				!this.hidden &&
				this.asDom().parent!.level
		) {
			this.asDom().transition({ opacity: 0, time }).then(()=>this.destroy());
		} else {
			this.destroy();
		}
		super.intoLeave(time);
	}

	intoBackground(time: number) {
		if ( time && !this.hidden && this.asDom().parent!.level ) {
			this.asDom().transition({ opacity: 0, visible: false, time });
		} else {
			this.asDom().opacity = 0;
			this.asDom().visible = false;
		}
		super.intoBackground(time);
	}

	intoForeground(time: number) {
		this.asDom().visible = true;
		if ( time && !this.hidden && this.asDom().parent!.level ) {
			if (this.navStatus == NavStatus.Init) {
				this.asDom().transition({ opacity: 1, time }, {opacity:0});
			}
			else { // NavStatus.Background
				this.asDom().transition({ opacity: 1, time });
			}
		} else {
			this.asDom().opacity = 1;
		}
		super.intoForeground(time);
	}
}

function getNavbarDom(page: NavPage): Navbar {
	return (page as any)._navbarDom;
}

function backgroundColorReverse(self: NavPage) {
	let color = self.asDom<Box>().backgroundColor.reverse() as types.RemoveReadonly<types.Color>;
	color.a = 255 * 0.6;
	return color;
}

/**
 * @class NavPage
 * @extends Navigation
 */
export class NavPage<P={},S={}> extends Navigation<{
	title?: string;
	navbar?: VDom<Navbar>;
	backgroundColor?: types.ColorStrIn;
	includeNavbarPadding?: boolean;
}&P,{}&S> {
	private _title = '';
	private _navbar: VDom<Navbar> = <Navbar /> as VDom<Navbar>;
	private _prevPage: NavPage | null = null;
	private _nextPage: NavPage | null = null;
	private _navbarDom: Navbar;
	private _includeNavbarPadding = true;

	/** @get prevPage:NavPage|null */
	get prevPage() { return this._prevPage }
	/** @get nextPage:NavPage|null */
	get nextPage() { return this._nextPage }
	/** @get collection:NavPageCollection */
	get collection() { return this.owner as NavPageCollection }
	/** @get isCurrent:boolean */
	get isCurrent() { return this.collection.isCurrent(this) }
	/** @get isFirstPage:boolean */
	get isFirstPage() { return this.navStack.length == 0 || this.navStack.front === this }

	@link backgroundColor: types.ColorStrIn = '#fff'; //!<
	/**
	 * @getset title:string
	*/
	@link
	get title() { return this._title }
	set title(value: string) {
		if (value !== this._title) {
			this._title = String(value);
			if (this._navbarDom) {
				this._navbarDom.setTitleText(this._title);
			}
			if (this._nextPage && this._nextPage._navbarDom) {
				this._nextPage._navbarDom.setBackText(value);
			}
		}
	}

	/**
	 * @getset navbar:VDom<Navbar>
	*/
	@link
	get navbar() { return this._navbar }
	set navbar(value) {
		assertDom(value, Navbar);
		if (this.isMounted) {
			this.renderNavbar(value);
		}
		this._navbar = value;
	}

	/**
	 * @getset navbarHidden:boolean
	*/
	get navbarHidden() { return this.isMounted ? this._navbarDom.hidden: false }
	set navbarHidden(hidden) {
		if (this.isMounted) {
			this._navbarDom.hidden = hidden;
			this._navbarDom.update();
			this.asDom().style.paddingTop = hidden || !this._includeNavbarPadding ? 0: this.navbarHeight;
		}
	}

	/**
	 * @getset includeNavbarPadding:boolean
	*/
	@link
	get includeNavbarPadding() {
		return this._includeNavbarPadding;
	}
	set includeNavbarPadding(val: boolean) {
		this._includeNavbarPadding = val;
		if (this.isMounted) {
			const hidden = !this._includeNavbarPadding || this._navbarDom.hidden;
			this.asDom().style.paddingTop = hidden ? 0: this.navbarHeight;
		}
	}

	/**
	 * @get navbarHeight:number
	*/
	get navbarHeight() {
		return this.collection.padding + this.collection.navbarHeight;
	}

	private renderNavbar(navbar: VDom<Navbar>) {
		this._navbarDom = navbar.render(this, {
			parent: this.collection.refs.navbar as View,
			replace: {
				vdom: this._navbar,
				dom: this._navbarDom,
			},
		});
		(this._navbarDom as any)._page = this;
		this._navbarDom.setTitleText(this._title);
		this._navbarDom.setBackText(this.prevPage ? this.prevPage.title : '');
		this.update();
	}

	protected render() {
		const hidden =
			!this._includeNavbarPadding ||
			this.collection.navbarHidden ||
			(this._navbarDom ? this._navbarDom.hidden: this._navbar.props.hidden);
		return (
			<matrix
				width="100%"
				height="match"
				visible={false}
				backgroundColor={this.backgroundColor}
				paddingTop={hidden ? 0: this.navbarHeight}
			>
				{this.renderBody()}
			</matrix>
		);
	}

	protected renderBody(): RenderResult {
		return this.children;
	}

	protected triggerMounted() {
		this.renderNavbar(this._navbar);
	}

	protected triggerUnload() {
		this._navbarDom?.destroy();
		return super.triggerUnload();
	}

	intoLeave(time: number) {
		this._navbarDom.intoLeave(time);
		if ( this.navStatus == NavStatus.Foreground ) {
			let dom = this.asDom();
			if ( time && dom.parent!.level ) {
				let x = (dom.parent! as Box).clientSize.x;
				dom.style = {
					borderColorLeft: backgroundColorReverse(this),
					borderWidthLeft: px,
				};
				dom.transition({ x: x, visible: false, time }).then(()=>{
					this.destroy();
				});
				super.intoLeave(time);
				return;
			}
		}
		super.intoLeave(time);
		this.destroy();
	}

	intoBackground(time: number) {
		if ( !this._nextPage )
			return;
		this._navbarDom.intoBackground(time);
		if ( this.navStatus != NavStatus.Background ) {
			let dom = this.asDom();
			let x = (dom.parent as Box).clientSize.x || 100;
			if ( time && dom.parent!.level ) {
				dom.transition({x: x * -0.33, visible: false, time });
			} else {
				dom.style = { x: x * -0.33, visible: false };
			}
		}
		super.intoBackground(time);
	}

	intoForeground(time: number) {
		if ( this.navStatus == NavStatus.Foreground )
			return;
		this._navbarDom.intoForeground(time);
		this._nextPage = null;
		let dom = this.asDom();

		if ( this.navStatus == NavStatus.Init ) {
			if ( time && dom.parent!.level ) {
				let x = (dom.parent! as Box).clientSize.x;
				dom.style = {
					visible: true,
					borderColorLeft: backgroundColorReverse(this),
					borderWidthLeft: px,
				};
				dom.transition({ x: 0, time }, {x}).then(()=>{
					dom.style.borderWidthLeft = 0;
				});
			} else {
				dom.style = { x: 0, borderWidthLeft: 0, visible: true };
			}
		}
		else if ( this.navStatus == NavStatus.Background ) {
			if ( time && dom.parent!.level ) {
				//let x = (dom.parent! as Box).clientSize.x;
				dom.visible = true;
				dom.transition({ x: 0, time }/*, { x: -x }*/);
			} else {
				dom.style = { x: 0, visible: true };
			}
		}
		super.intoForeground(time);
	}

	navigationBack() {
		if ( this._prevPage ) {
			this.collection.pop(true);
			return true;
		} else {
			return false;
		}
	}
}
