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
import index, {
	_CVD,link,ViewController,View,Label,Text, RenderResult,
	mainScreenScale,Window,VDom,assertDom,Box,VirtualDOM, createCss } from './index';
import * as types from './types';

const px = 1 / mainScreenScale();
const Transition_Time = 400;
const Navigation_Stack: WeakMap<Window, List<Navigation>> = new WeakMap();

export enum NavStatus {
	Init = -1,
	Foreground,
	Background,
}

/**
 * @class NavigationStatus
 */
class NavigationStatus<P={},S={}> extends ViewController<P,S> {
	// 0=init or exit,1=background,2=foreground
	readonly navStatus: NavStatus = NavStatus.Init;
	intoLeave(animate: number) {
		(this as any).navStatus = NavStatus.Init;
	}
	intoBackground(animate: number) {
		(this as any).navStatus = NavStatus.Background;
	}
	intoForeground(animate: number) {
		(this as any).navStatus = NavStatus.Foreground;
	}
}

/**
 * @class Navigation
 */
export class Navigation<P={},S={}> extends NavigationStatus<{
	onBackground?:(selnder: Navigation)=>void,
	onForeground?:(selnder: Navigation)=>void,
}&P,S> {
	private _iterator: ListIterator<Navigation> | null = null;
	private _focusResume: View | null = null;

	protected navStack = (function(window: Window) {
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
	 * @method initFocus()
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

	protected triggerDestroy() {
		if ( this._iterator ) { // force delete global nav stack
			this.navStack.remove(this._iterator);
			this._iterator = null;
		}
	}

	/**
	 * @method registerNavigation()
	 */
	registerNavigation(animate: number = 0) {
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
	 * @method unregisterNavigation(time)
	 */
	unregisterNavigation(animate: number = 0) {
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
	 When a navigation event occurs, the system will first send the event to the focus view,
	If the event can be successfully transmitted to root,
	The event will ultimately be sent to the top of the current navigation list stack
	*/
	navigationBack() {
		/*
		If false is returned here,
		it will continue to pass to the bottom of the navigation list stack,
		Until it returns true or reaches the bottom of the stack to exit the application
		*/
		return true;
	}

	navigationEnter(focus: View) {
		// Rewrite this function to implement your logic
	}

	/**
	 * When returning null, the focus will not change in any way
	 */
	navigationTop(focus: View | null): View | null {
		return focus;
	}

	navigationDown(focus: View | null): View | null {
		return focus;
	}

	navigationLeft(focus: View | null): View | null {
		return focus;
	}

	navigationRight(focus: View | null): View | null {
		return focus;
	}

	/* When pressing the menu button, it will be called up */
	navigationMenu() {
		// Rewrite this function to implement your logic
	}
}

/**
 * @class NavPageCollection
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
	 * @prop enableAnimate
	 */
	@link     enableAnimate: boolean = true;
	@link.acc padding = index.app.screen.statusBarHeight; // ios/android, 20;
	@link.acc navbarHeight = 44;
	@link.acc navbarHidden = false;

	get length() { return this._substack.length }
	get pages() { return this._substack.toArray() }
	get current() {
		util.assert(this.length, 'Empty NavPageCollection');
		return this._substack.back!;
	}

	/**
	 * @method isCurrent
	*/
	isCurrent(page: NavPage) {
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

	protected triggerDestroy() {
		for (let e of this.pages)
			e.destroy();
		return super.triggerDestroy();
	}

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
		if (!next) return;

		arr.forEach(page=>page.intoLeave(0)); // private props visit

		this._busy = time ? true: false;
		if ( time ) {
			setTimeout(()=>{ this._busy = false }, time);
		}

		next.unregisterNavigation(time);

		// switch and animate
		this.triggerPop(next);
	}

	// @overwrite
	navigationBack(): boolean {
		if (this._substack.length)
			return this._substack.back!.navigationBack();
		return false;
	}
	// @overwrite
	navigationEnter(focus: View) {
		if (this._substack.length)
			this._substack.back!.navigationEnter(focus);
	}
	// @overwrite
	navigationTop(focus: View | null) {
		if (this._substack.length)
			return this._substack.back!.navigationTop(focus);
		return focus;
	}
	// @overwrite
	navigationDown(focus: View | null) {
		if (this._substack.length)
			return this._substack.back!.navigationDown(focus);
		return focus;
	}
	// @overwrite
	navigationLeft(focus: View | null) {
		if (this._substack.length)
			return this._substack.back!.navigationLeft(focus);
		return focus;
	}
	// @overwrite
	navigationRight(focus: View | null) {
		if (this._substack.length)
			return this._substack.back!.navigationRight(focus);
		return focus;
	}
	// @overwrite
	navigationMenu() {
		if (this._substack.length)
			this._substack.back!.navigationMenu(); // private props visit
	}
}

createCss({
	'.x_navbar_back:normal': {
		opacity: 1,
		// textShadow: "0 0 0 #000"
	},
	'.x_navbar_back:hover': {
		opacity: 0.8,
		// textShadow: "0 0 4 #000"
	},
	'.x_navbar_back:active': {
		opacity: 0.6,
		// textShadow: "0 0 6 #000"
	},
});

/**
 * @class Navbar
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

	@link hidden = false;
	@link visibleBackIcon = true;
	@link visibleBackText = true;
	@link backTextColor: types.ColorStrIn = '#000';
	@link titleTextColor: types.ColorStrIn = '#147EFF';

	get page() { return this.owner as NavPage }

	/**
	 * @method setBackText() set navbar back text
	 */
	setBackText(value: string) {
		if (!this.hidden)
			this.refAs<Label>('back_text').value = value;
		this._backText = value;
	}

	/**
	 * @method setTitleText() set navbar title text
	 */
	setTitleText(value: string) {
		if (!this.hidden)
			this.refAs<Text>('title').value = value;
		this._titleText = value;
	}

	private _handleBack = ()=>{
		this.page.collection.pop(true);
	}

	protected render() {
		return (
			this.hidden ? null:
			<free width="100%" height="100%" align="centerBottom" visible={false}>
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
					class="x_navbar_back"
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
				this.domAs().parent!.level
		) {
			this.domAs().transition({ opacity: 0, time }, ()=>this.destroy());
		} else {
			this.destroy();
		}
		super.intoLeave(time);
	}

	intoBackground(time: number) {
		if ( time && !this.hidden && this.domAs().parent!.level ) {
			this.domAs().transition({ opacity: 0, visible: false, time });
		} else {
			this.domAs().opacity = 0;
			this.domAs().visible = false;
		}
		super.intoBackground(time);
	}

	intoForeground(time: number) {
		this.domAs().visible = true;
		if ( time && !this.hidden && this.domAs().parent!.level ) {
			if (this.navStatus == NavStatus.Init) {
				this.domAs().transition({ opacity: 1, time }, {opacity:0});
			}
			else { // NavStatus.Background
				this.domAs().transition({ opacity: 1, time });
			}
		} else {
			this.domAs().opacity = 1;
		}
		super.intoForeground(time);
	}
}

function getNavbarDom(page: NavPage): Navbar {
	return (page as any)._navbarDom;
}

function backgroundColorReverse(self: NavPage) {
	let color = self.domAs<Box>().backgroundColor.reverse();
	color.a = 255 * 0.6;
	return color;
}

/**
 * @class NavPage
 */
export class NavPage<P={},S={}> extends Navigation<{
	title?: string;
	navbar?: VDom<Navbar>;
	backgroundColor?: types.ColorStrIn;
}&P,{}&S> {
	private _title = '';
	private _navbar: VDom<Navbar> = <Navbar /> as VDom<Navbar>;
	private _prevPage: NavPage | null = null;
	private _nextPage: NavPage | null = null;
	private _navbarDom: Navbar;

	get prevPage() { return this._prevPage }
	get nextPage() { return this._nextPage }
	get collection() { return this.owner as NavPageCollection }
	get isCurrent() { return this.collection.isCurrent(this) }
	get isFirstPage() { return this.navStack.length == 0 || this.navStack.front === this }

	@link backgroundColor: types.ColorStrIn = '#fff';
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

	@link
	get navbar() { return this._navbar }
	set navbar(value) {
		assertDom(value, Navbar);
		if (this.isMounted) {
			this.renderNavbar(value);
		}
		this._navbar = value;
	}
	get navbarHidden() { return this.isMounted ? this._navbarDom.hidden: false }
	set navbarHidden(val) { this.setNavbarHidden(val) }

	setNavbarHidden(hidden: boolean) {
		if (this.isMounted) {
			this._navbarDom.hidden = hidden;
			this._navbarDom.update();
			this.domAs().style = { padding: this.paddingNavbar(hidden) };
		}
	}

	private paddingNavbar(isHidden: boolean) {
		return isHidden ? 0: this.collection.padding + this.collection.navbarHeight;
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
		const hidden = this.collection.navbarHidden || this._navbar.props.hidden;
		return (
			<matrix
				width="100%"
				height="match"
				visible={false}
				backgroundColor={this.backgroundColor}
				paddingTop={this.paddingNavbar(hidden)}
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

	protected triggerDestroy() {
		this._navbarDom?.destroy();
		return super.triggerDestroy();
	}

	intoLeave(time: number) {
		this._navbarDom.intoLeave(time);
		if ( this.navStatus == NavStatus.Foreground ) {
			let dom = this.domAs();
			if ( time && dom.parent!.level ) {
				let x = (dom.parent! as Box).clientSize.x;
				dom.style = {
					borderColorLeft: backgroundColorReverse(this),
					borderWidthLeft: px,
				};
				dom.transition({ x: x, visible: false, time }, ()=>{
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
			let dom = this.domAs();
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
		let dom = this.domAs();

		if ( this.navStatus == NavStatus.Init ) {
			if ( time && dom.parent!.level ) {
				let x = (dom.parent! as Box).clientSize.x;
				dom.style = {
					visible: true,
					borderColorLeft: backgroundColorReverse(this),
					borderWidthLeft: px,
				};
				dom.transition({ x: 0, time }, {x}, ()=>{
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
