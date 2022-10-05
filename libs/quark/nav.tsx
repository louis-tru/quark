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

import utils from './util';
import { List, KeyboardKeyName, ClickType } from './event';
import quark, { 
	ViewController, Div, Indep, View,
	Limit, Button, Text, TextNode, Clip, _CVD,
} from './index';
import * as value from './value';
import {prop} from './ctr';
import {event, EventNoticer, Event, ListItem} from './event';

const TRANSITION_TIME = 400;
const g_navigationStack = new List<Navigation>();
var   g_navigationInit_ok = false;

function get_valid_focus(nav: Navigation, focus_move: View | null) {
	if (!nav) return null;
	var view = nav.__meta__;
	return focus_move && view.hasChild(focus_move) ? view.firstButton() : focus_move;
}

export enum Status {
	INIT = -1,
	FOREGROUND = 0,
	BACKGROUND = 1,
}

/**
 * @class Status
 */
class NavigationStatus extends ViewController {
	private m_status: Status = Status.INIT; // 1=background,0=foreground,-1=init or exit
	get status() { return this.m_status }
	intoBackground(animate: number) { this.m_status = 1 }
	intoForeground(animate: number) { this.m_status = 0 }
	intoLeave(animate: number) { this.m_status = -1 }
}

/**
 * @class Navigation
 */
export class Navigation extends NavigationStatus {

	private m_stack = g_navigationStack;
	private m_iterator: ListItem<Navigation> | null = null;
	private m_focus_resume: View | null = null;

	@event readonly onBackground: EventNoticer<Event<void, Navigation>>;
	@event readonly onForeground: EventNoticer<Event<void, Navigation>>;

	private static _navigationInit(nav: Navigation) {
		if ( (nav as any).m_stack !== g_navigationStack || g_navigationInit_ok || !quark.root) {
			return;
		}

		// initialize
		g_navigationInit_ok = true;

		var root = quark.root;

		root.onBack.on(function(ev) {
			var last = g_navigationStack.last;
			while(last) {
				if ( last.value.navigationBack() ) {
					ev.cancelDefault(); // 取消默认动作
					break;
				}
				last = last.prev;
			}
		});

		root.onClick.on(function(ev) {
			// console.log('onClick--', ev.keyboard );
			if ( ev.type == ClickType.KEYBOARD ) { // 需要键盘产生的事件
				var last = g_navigationStack.last;
				if ( last ) {
					last.value.navigationEnter(ev.sender);
				}
			}
		});

		root.onKeyDown.on(function(ev) {
			var last = g_navigationStack.last;
			if ( last ) {
				var focus_move = ev.focusMove;
				var nav = last.value;
	
				switch(ev.keycode) {
					case KeyboardKeyName.LEFT: // left
						focus_move = nav.navigationLeft(focus_move);
						break;
					case KeyboardKeyName.UP: // up
						focus_move = nav.navigationTop(focus_move);
						break;
					case KeyboardKeyName.RIGHT: // right
						focus_move = nav.navigationRight(focus_move);
						break;
					case KeyboardKeyName.DOWN: // down
						focus_move = nav.navigationDown(focus_move);
						break;
					case KeyboardKeyName.MENU: // menu
						nav.navigationMenu();
					default: return;
				}
				ev.focusMove = focus_move;
			}
		});
	}

	protected triggerBackground() {
		this.trigger('Background');
	}

	protected triggerForeground() {
		this.trigger('Foreground');
	}

	intoBackground(animate: number) { 
		super.intoBackground(animate);
		this.triggerBackground();
	}

	intoForeground(animate: number) {
		super.intoForeground(animate);
		this.triggerForeground();
	}

	/**
	 * @func defaultFocus() 导航初始化时,返回一个焦点视图,重写这个函数
	 */
	defaultFocus(): View | null {
		return null;
	}

	protected triggerRemove() {
		if ( this.m_iterator ) { // force delete global nav stack
			this.m_stack.del(this.m_iterator);
			this.m_iterator = null;
		}
		return super.triggerRemove();
	}

	/**
	 * @func registerNavigation()
	 */
	registerNavigation(animate: number = 0) {
		if ( !this.m_iterator ) { // No need to repeat it
			Navigation._navigationInit(this);
			this.m_iterator = this.m_stack.push(this);
			// console.log('push_navigation()-----', this.m_stack.length);
			quark.lock(()=>{
				var prev = (this.m_iterator as ListItem<Navigation>).prev;
				if ( prev ) {
					var focus = quark.app.focusView;
					prev.value.m_focus_resume = focus && prev.value.__meta__.hasChild(focus) ? focus : null;
					prev.value.intoBackground(animate);
				}
				var view = this.defaultFocus();
				if ( view ) {
					view.focus();
				}
				this.intoForeground(animate);
			});
		}
	}

	/**
	 * @func unregisterNavigation(time, data)
	 */
	unregisterNavigation(animate: number = 0) {
		if ( this.m_iterator ) {
			// utils.assert(this.m_iterator, 'Bad iterator!');
			var last = this.m_stack.last;
			this.m_stack.del(this.m_iterator);
			this.m_iterator = null;
			if (!last || last.value !== this) return;

			quark.lock(()=>{
				this.intoLeave(animate);
				var last = this.m_stack.last;
				if ( last ) {
					if (last.value.m_focus_resume) {
						last.value.m_focus_resume.focus();
					}
					last.value.intoForeground(animate);
				}
			});
		}
	}

	/* 导航事件产生时系统会首先将事件发送给焦点视图，事件如果能成功传递到root,
	 * 那么事件最终将发送到达当前导航列表栈最顶端
	 */

	navigationBack() {
		/* 这里如果返回false会继续往导航列表栈底端传递，直到返回true或到达栈底退出应用程序 */
		return true;
	}

	navigationEnter(focus: View) {
		// Rewrite this function to implement your logic
	}

	/**
	 * navigationTop()
	 * navigationDown()
	 * navigationLeft()
	 * navigationRight()
	 * 返回null时焦点不会发生任何改变
	 */
	navigationTop(focus_move: View | null) {
		return get_valid_focus(this, focus_move);
	}

	navigationDown(focus_move: View | null) {
		return get_valid_focus(this, focus_move);
	}

	navigationLeft(focus_move: View | null) {
		return get_valid_focus(this, focus_move);
	}

	navigationRight(focus_move: View | null) {
		return get_valid_focus(this, focus_move);
	}

	/* 按下menu按键时会调用 */
	navigationMenu() {
		// Rewrite this function to implement your logic
	}

}

/**
 * @func refresh_bar_style
 */
function refresh_bar_style(self: NavPageCollection, time: number) {
	if ( self.IDs.navbar && self.length ) {
		time = self.enableAnimate ? time : 0;
		var navbar = self.navbar || { 
			height: 0, border: 0, backgroundColor: '#0000', borderColor: '#0000' 
		};
		var toolbar = self.toolbar || { 
			height: 0, border: 0, backgroundColor: '#0000', borderColor: '#0000' 
		};
		var navbarHidden = (self as any).$navbarHidden || (self as any).navbar.$hidden; // private props visit
		var toolbarHidden = (self as any).$toolbarHidden || (self as any).toolbar.$hidden; // private props visit
		var navbar_height = navbarHidden ? 0 : navbar.height + (self as any).m_padding as number + navbar.border; // private props visit
		var toolbar_height = toolbarHidden ? 0 : toolbar.height + toolbar.border;

		if ( time ) {
			if ( !navbarHidden ) self.find('navbar').show();
			if ( !toolbarHidden ) self.find('toolbar').show();
			quark.lock(()=>{
				self.find('navbar').transition({
					height: Math.max(0, navbar_height - navbar.border), 
					borderBottom: `${navbar.border} ${navbar.borderColor}`, 
					backgroundColor: navbar.backgroundColor,
					time: time,
				});
				//console.log(navbar.backgroundColor, 'OKOK1', time);
				self.find('toolbar').transition({ 
					height: Math.max(0, toolbar_height - toolbar.border),
					borderTop: `${toolbar.border} ${toolbar.borderColor}`, 
					backgroundColor: toolbar.backgroundColor,
					time: time,
				});
				self.find('page').transition({ height: navbar_height + toolbar_height + '!', time: time }, ()=>{
					if ( navbarHidden ) self.find('navbar').hide();
					if ( toolbarHidden ) self.find('toolbar').hide();
				});
			});
		} else {
			var style = { 
				height: Math.max(0, navbar_height - navbar.border), 
				borderBottom: `${navbar.border} ${navbar.borderColor}`,
				backgroundColor: navbar.backgroundColor, 
				visible: !navbarHidden, 
			};
			self.IDs.navbar.style = style;
			//console.log(navbar.backgroundColor, 'OKOK2', time);
			self.IDs.toolbar.style = { 
				height: Math.max(0, toolbar_height - toolbar.border), 
				borderTop: `${toolbar.border} ${toolbar.borderColor}`, 
				backgroundColor: toolbar.backgroundColor, 
				visible: !toolbarHidden,
			};
			self.IDs.page.style = { height: navbar_height + toolbar_height + '!' };
		}
	}
}

/**
 * @class NavPageCollection
 */
export class NavPageCollection extends Navigation {
	private m_padding = quark.statusBarHeight; // ios/android, 20
	private m_pages: NavPage[] = [];
	private m_substack = new List<Navigation>();
	private m_default_toolbar: Toolbar | null = null;
	private m_animating = false;
	private m_default_page: any;
	protected $navbarHidden = false;
	protected $toolbarHidden = false;

	/**
	 * @field enableAnimate
	 */
	enableAnimate = true;

	@event readonly onPush: EventNoticer<Event<NavPage, NavPageCollection>>;
	@event readonly onPop: EventNoticer<Event<NavPage, NavPageCollection>>;

	get padding() { return this.m_padding }
	get navbarHidden() { return this.$navbarHidden }
	get toolbarHidden() { return this.$toolbarHidden }
	set navbarHidden(value) { this.setNavbarHidden(value, false) }
	set toolbarHidden(value) { this.setToolbarHidden(value, false) }

	get length() { return this.m_pages.length }
	get pages() { return this.m_pages.slice() }
	get current() {
		utils.assert(this.length, 'current empty');
		return this.m_pages.indexReverse(0);
	}
	get navbar() { return (this.current as NavPage).navbar }
	get toolbar() { return (this.current as NavPage).toolbar }
	get defaultToolbar(): Toolbar | null { return this.m_default_toolbar }

	isCurrent(page: NavPage) {
		return page && this.m_pages.indexReverse(0) === page;
	}

	set padding(value) {
		utils.assert(typeof value == 'number');
		this.m_padding = Math.max(value, 0);
		refresh_bar_style(this, 0);
	}

	protected triggerPush(page: NavPage) {
		this.trigger('Push', page);
	}

	protected triggerPop(page: NavPage) {
		this.trigger('Pop', page);
	}

	/**
	 * @func setNavbarHidden
	 */
	setNavbarHidden(value: boolean, animate?: boolean) {
		this.$navbarHidden = !!value;
		refresh_bar_style(this, animate ? TRANSITION_TIME : 0);
	}

	/**
	 * @func setToolbarHidden
	 */
	setToolbarHidden(value: boolean, animate?: boolean) {
		this.$toolbarHidden = !!value;
		refresh_bar_style(this, animate ? TRANSITION_TIME : 0);
	}

	/**
	 * @set defaultToolbar {Toolbar} # Set default toolbar
	 */
	set defaultToolbar(value: Toolbar | null) {
		if (value) {
			var bar = quark.render(value) as Toolbar;
			utils.assert(bar instanceof Toolbar, 'Type not correct');
			utils.assert(!bar.collection || bar.collection !== this);
			if ( bar !== this.m_default_toolbar ) {
				if ( this.m_default_toolbar ) {
					this.m_default_toolbar.remove();
				}
				this.m_default_toolbar = bar;
				(this.m_default_toolbar as any).m_collection = this; // private props visit
			}
		} else { // cancel
			if ( this.m_default_toolbar ) {
				this.m_default_toolbar.remove();
				this.m_default_toolbar = null;
			}
		}
	}

	protected render(...vdoms: any[]) {
		this.m_default_page = vdoms.find(e=>e);
		return (
			<Clip width="100%" height="100%">
				<Div id="navbar" width="100%" />
				<Div id="page" width="100%" />
				<Div id="toolbar" width="100%" />
			</Clip>
		);
	}

	protected triggerMounted() {
		if (this.m_default_page) {
			/* delay 因为是第一次加载,布局系统还未初始化
			 * 无法正确的获取数值来进行title bar的排版计算
			 * 所以这里延时一帧画面
			 */
			quark.nextFrame(()=>this.push(this.m_default_page));
		}
		quark.nextFrame(()=>this.registerNavigation(0));
		return super.triggerMounted();
	}

	protected triggerRemove() {
		this.m_pages.forEach(e=>e.remove());
		this.m_pages = [];
		return super.triggerRemove();
	}

	push(arg: any, animate?: boolean) {
		if ( this.m_animating ) {
			return;
		}
		var time = this.enableAnimate && animate && this.length ? TRANSITION_TIME : 0;
		var prev = this.m_pages.indexReverse(0);
		var page: NavPage = arg;

		// console.log('push---', time, animate, this.length, this.enableAnimate);

		if ( arg ) {
			if ( arg instanceof NavPage ) { // dom
				utils.assert(!arg.collection, 'NavPage can only be a new entity');
				page = quark.render(arg, this.IDs.page as View) as NavPage;
			} else {
				if (ViewController.typeOf(arg, NavPage)) {
					page = quark.render(arg, this.IDs.page as View) as NavPage;
				} else {
					page = quark.render(<NavPage>{arg}</NavPage>, this.IDs.page as View) as NavPage;
				}
			}
		}

		utils.assert(page instanceof NavPage, 'The argument navpage is not of the correct type, '+
			'Only for NavPage entities or NavPage VX data.');

		// set page
		(page as any).m_stack = this.m_substack; // private props visit
		(page as any).m_collection = this; // private props visit
		(page as any).m_prevPage = prev; // private props visit

		if (prev) { // set next page
			(prev as any).m_nextPage = page; // private props visit
		}

		if (!(page as any).m_navbar) { // Create default navbar
			page.navbar = <Navbar />;
		}

		if (!(page as any).m_toolbar) { // use default toolbar
			if (this.defaultToolbar) {
				page.toolbar = this.defaultToolbar;
			} else {
				page.toolbar = <Toolbar />;
			}
		}

		this.m_pages.push(page);

		(page.navbar as any).m_collection = this; // private props visit
		(page.toolbar as any).m_collection = this; // private props visit
		page.navbar.appendTo(this.IDs.navbar as View);
		page.toolbar.appendTo(this.IDs.toolbar as View);

		this.m_animating = time ? true: false;
		if ( time ) {
			setTimeout(()=>{ this.m_animating = false }, time);
		}

		page.navbar.setBackText(prev ? prev.title : '');

		refresh_bar_style(this, time);

		// switch and animate
		this.triggerPush(page);

		page.registerNavigation(time);
	}

	pop(animate?: boolean) {
		this.pops(1, animate);
	}

	pops(count: number, animate?: boolean) {
		count = Number(count) || 0;
		count = Math.min(this.length - 1, count);
		
		if ( count < 1 ) {
			return;
		}
		if ( this.m_animating ) {
			return;
		}

		var time = this.enableAnimate && animate ? TRANSITION_TIME : 0;
		// var page = this.m_pages[this.length - 1 - count];
		var arr  = this.m_pages.splice(this.length - count);
		var next = arr.pop();

		if (next) {
			arr.forEach(page=>page.intoLeave(0)); // private props visit

			this.m_animating = time ? true: false;
			if ( time ) {
				setTimeout(()=>{ this.m_animating = false }, time);
			}
			refresh_bar_style(this, time);

			// switch and animate
			this.triggerPop(next);

			next.unregisterNavigation(time);
		}
	}

	// @overwrite
	navigationBack(): boolean {
		if (this.m_pages.length)
			return this.m_pages.indexReverse(0).navigationBack(); // private props visit
		return false;
	}
	// @overwrite
	navigationEnter(focus: View) {
		if (this.m_pages.length) 
			this.m_pages.indexReverse(0).navigationEnter(focus); // private props visit
	}
	// @overwrite
	navigationTop(focus_move: View | null) {
		return get_valid_focus(this.m_pages.indexReverse(0), focus_move);
	}
	// @overwrite
	navigationDown(focus_move: View | null) {
		return get_valid_focus(this.m_pages.indexReverse(0), focus_move);
	}
	// @overwrite
	navigationLeft(focus_move: View | null) {
		return get_valid_focus(this.m_pages.indexReverse(0), focus_move);
	}
	// @overwrite
	navigationRight(focus_move: View | null) {
		return get_valid_focus(this.m_pages.indexReverse(0), focus_move);
	}
	// @overwrite
	navigationMenu() {
		if (this.m_pages.length) 
			this.m_pages.indexReverse(0).navigationMenu(); // private props visit
	}

}

/**
 * @class Bar
 */
class Bar extends NavigationStatus {
	protected $height = 44;
	protected $hidden = false;
	protected $border = quark.atomPixel;
	protected $borderColor = '#b3b3b3';
	protected $backgroundColor = '#f9f9f9';
	protected m_page: NavPage;
	protected m_collection: NavPageCollection;

	get height() { return this.$height }
	get hidden() { return this.$hidden }
	get border() { return this.$border }
	get borderColor() { return this.$borderColor }
	get backgroundColor() { return this.$backgroundColor }
	
	get collection() { return this.m_collection }
	get page() { return this.m_page }
	get isCurrent() { return this.m_page && this.m_page.isCurrent }
	
	set height(value) {
		utils.assert(typeof value == 'number');
		this.$height = value;
		this.refreshStyle(0);
	}
	set hidden(value) {
		this.$hidden = !!value;
		this.refreshStyle(0);
	}
	set border(value: number) {
		utils.assert(typeof value == 'number');
		this.$border = value; 
		this.refreshStyle(0); 
	}
	set borderColor(value: string) {
		this.$borderColor = value;
		this.refreshStyle(0);
	}
	set backgroundColor(value) {
		this.$backgroundColor = value; 
		this.refreshStyle(0); 
	}
	
	setHidden(value: boolean, animate?: boolean) {
		this.$hidden = !!value;
		this.refreshStyle(animate ? TRANSITION_TIME : 0);
	}
	
	/**
	 * @fun refreshStyle
	 */
	refreshStyle(time: number) {
		if (this.isCurrent) {
			refresh_bar_style(this.m_page.collection, time);
		}
	}
	
	get visible() {
		return this.domAs().visible;
	}
	
	set visible(value) {
		if ( value ) {
			if (this.isCurrent) {
				this.domAs().visible = true;
			}
		} else {
			if (!this.isCurrent) {
				this.domAs().visible = false;
			}
		}
	}

}

/**
 * @class Navbar
 */
export class Navbar extends Bar {
	private m_back_panel_width = 0;
	private m_title_panel_width = 0;
	protected $defaultStyle = true;
	protected $backIconVisible = true;
	protected $titleMenuWidth = 40; // display right menu button width
	protected $backgroundColor = '#2c86e5'; // 3c89fb

	@prop backTextColor = '#fff';
	@prop titleTextColor = '#fff';

	/**
	 * @func _navbar_compute_title_layout
	 */
	private _navbar_compute_title_layout() {
		var self: Navbar = this;
		if ( self.$defaultStyle ) {
			
			var back_text = (self.IDs.back_text1 as TextNode).value;
			var title_text = (self.IDs.title_text_panel as Text).value;
			var backIconVisible = self.$backIconVisible;

			if ( self.page && self.page.prevPage ) {
				(self.IDs.back_text_btn as View).visible = true;
			} else {
				(self.IDs.back_text_btn as View).visible = false;
				back_text = '';
				backIconVisible = false;
			}
			
			var nav_width = self.collection ? self.collection.domAs<Div>().finalWidth : 0;
			
			// console.log('----------------------nav_width', nav_width);

			var back_width = (self.IDs.back_text1 as TextNode).simpleLayoutWidth(back_text) + 3; // 3间隔
			var title_width = (self.IDs.title_text_panel as TextNode).simpleLayoutWidth(title_text);
			// console.log('back_width', 'title_width', back_width, title_width);
			var marginRight = Math.min(nav_width / 3, Math.max(self.$titleMenuWidth, 0));
			var marginLeft = 0;
			var min_back_width = 6;
			
			if ( backIconVisible ) {
				min_back_width += (self.IDs.back_text0 as TextNode).simpleLayoutWidth('\uedc5');
				back_width += min_back_width;
			}
			
			(self.IDs.title_panel as Indep).marginLeft = new value.Value(value.ValueType.PIXEL, marginLeft);
			(self.IDs.title_panel as Indep).marginRight = new value.Value(value.ValueType.PIXEL, marginRight);
			(self.IDs.title_panel as Indep).show();
			(self.IDs.back_text0 as TextNode).visible = backIconVisible;
			
			if ( nav_width ) {
				var title_x = nav_width / 2 - title_width / 2 - marginLeft;
				if ( back_width <= title_x ) {
					back_width = title_x;
				} else { // back 的宽度超过title-x位置
					//console.log(back_width, (nav_width - marginLeft - marginRight) - title_width);
					back_width = Math.min(back_width, (nav_width - marginLeft - marginRight) - title_width);
					back_width = Math.max(min_back_width, back_width);
				}
				title_width = nav_width - back_width -  marginLeft - marginRight;
				self.m_back_panel_width = back_width;// - min_back_width;
				self.m_title_panel_width = title_width;
			} else {
				self.m_back_panel_width = 0;
				self.m_title_panel_width = 0;
				back_width = 30;
				title_width = 70;
			}

			var back_text_num = back_width / (back_width + title_width);
			var titl_text_num = title_width / (back_width + title_width);

			// 为保证浮点数在转换后之和不超过100,向下保留三位小数
			(self.IDs.back_text_panel as Div).width = value.parseValue(Math.floor(back_text_num * 100000) / 1000 + '%');
			(self.IDs.title_text_panel as Div).width = value.parseValue(Math.floor(titl_text_num * 100000) / 1000 + '%');

		} else {
			(self.IDs.title_panel as View).hide(); // hide title text and back text
		}
	}

	get backIconVisible() { return this.$backIconVisible }
	get defaultStyle() { return this.$defaultStyle }
	get titleMenuWidth() { return this.$titleMenuWidth }
	
	set backIconVisible(value) {
		this.$backIconVisible = !!value;
		this._navbar_compute_title_layout();
	}

	set defaultStyle(value) {
		this.$defaultStyle = !!value;
		this._navbar_compute_title_layout();
	}

	set titleMenuWidth(value) {
		utils.assert(typeof value == 'number');
		this.$titleMenuWidth = value;
		this._navbar_compute_title_layout();
	}

	refreshStyle(time: number) {
		if (this.isCurrent) {
			(this.domAs() as Indep).alignY = value.parseAlign('bottom');
			(this.domAs() as Indep).height = new value.Value(value.ValueType.PIXEL, this.height);
			(this.IDs.title_text_panel as Text).textLineHeight = value.parseTextLineHeight(this.height);
			(this.IDs.back_text_btn as Button).textLineHeight = value.parseTextLineHeight(this.height);
			super.refreshStyle(time);
		}
	}

	/**
	 * @overwrite
	 */
	protected render(...vdoms: any[]) {
		var height = this.height;
		var textSize = 16;
		return (
			<Indep width="100%" height={height} visible={0} alignY="bottom">
				{vdoms}
				<Indep id="title_panel" width="full" height="100%" visible={0}>
					<Div id="back_text_panel" height="full">
						<Limit maxWidth="100%">
							{/* textColor="#0079ff"*/}
							<Button id="back_text_btn" 
								onClick={()=>this.collection.pop(true)}
								textColor={this.backTextColor}
								width="full" 
								textLineHeight={height} 
								textSize={textSize}
								textWhiteSpace="no_wrap" textOverflow="ellipsis">
								<Div width={6} />
								<TextNode id="back_text0" 
									textLineHeight="auto" 
									textSize={20}
									height={26} y={2}
									textColor="inherit" 
									textFamily="icon" value={'\uedc5'} />
								<TextNode id="back_text1" />
							</Button>
						</Limit>
					</Div>
					
					<Text id="title_text_panel" 
						height="full"
						textColor={this.titleTextColor}
						textLineHeight={height} 
						textSize={textSize}
						textWhiteSpace="no_wrap" 
						textStyle="bold" textOverflow="ellipsis" />
						
				</Indep>
			</Indep>
		);
	}

	/**
	 * @fun setBackText # set navbar back text
	 */
	setBackText(value: string) {
		(this.IDs.back_text1 as TextNode).value = value;
		this._navbar_compute_title_layout();
	}
	
	/**
	 * @fun $setTitleText # set navbar title text
	 */
	setTitleText(value: string) {
		(this.IDs.title_text_panel as Text).value = value;
		this._navbar_compute_title_layout();
	}
	
	intoBackground(time: number) {
		if ( time ) { 
			if ( this.$defaultStyle ) {
				var back_icon_width = (this.IDs.back_text0 as View).visible ? (this.IDs.back_text0 as TextNode).clientWidth : 0;
				(this.IDs.back_text1 as View).transition({ 
					x: -(this.IDs.back_text1 as TextNode).clientWidth, time: time,
				});
				(this.IDs.title_text_panel as View).transition({ 
					x: -this.m_back_panel_width + back_icon_width, time: time,
				});
			}
			this.domAs().transition({ opacity: 0, time: time }, ()=>{ this.domAs().hide() });
		} else {
			this.domAs().opacity = 0;
			this.domAs().hide();
		}
		super.intoBackground(time);
	}
	
	intoForeground(time: number) { 
		this.domAs().show(); // show
		if ( time ) {
			if ( this.$defaultStyle ) {
				var back_icon_width = 0; // this.IDs.back_text0.visible ? 20 : 0;
				if ( this.status == -1 ) {
					(this.IDs.back_text1 as View).x = this.m_back_panel_width - back_icon_width;
					(this.IDs.title_text_panel as View).x = this.m_title_panel_width + this.$titleMenuWidth;
				}
				(this.IDs.back_text1 as View).transition({ x: 0, time: time });
				(this.IDs.title_text_panel as View).transition({ x: 0, time: time });
			} else {
				(this.IDs.back_text1 as View).x = 0;
				(this.IDs.title_text_panel as View).x = 0;
			}
			this.domAs().opacity = 0;
			this.domAs().transition({ opacity: 1, time: time });
		} else {
			this.domAs().opacity = 1;
			(this.IDs.back_text1 as View).x = 0;
			(this.IDs.title_text_panel as View).x = 0;
		}
		super.intoForeground(time);
	}

	intoLeave(time: number) { 
		if ( this.status == 0 && time ) {
			if ( this.$defaultStyle ) {
				var back_icon_width = (this.IDs.back_text0 as View).visible ? (this.IDs.back_text0 as TextNode).clientWidth : 0;
				(this.IDs.back_text1 as View).transition({ x: this.m_back_panel_width - back_icon_width, time: time });
				(this.IDs.title_text_panel as View).transition({ 
					x: this.m_title_panel_width + this.$titleMenuWidth, time: time,
				});
			}
			this.domAs().transition({ opacity: 0, time: time }, ()=>{ this.remove() });
		} else {
			this.remove();
		}
		super.intoLeave(time);
	}
}

/**
 * @class Toolbar
 */
export class Toolbar extends Bar {

	protected $height = 49;

	/**
	 * @overwrite
	 */
	protected render(...vdoms: any[]) {
		return (
			<Indep width="100%" height="full" visible={0}>{vdoms}</Indep>
		);
	}
	
	intoForeground(time: number) {
		if ( this.isDefault ) {
			this.m_page = this.collection.current as NavPage;
		}
		if ( time ) {
			var page = (this.page.nextPage || this.page.prevPage);
			if (!page || page.toolbar !== this) {
				this.domAs().show();
				this.domAs().opacity = 0;
				this.domAs().transition({ opacity: 1, time: time });
			}
		} else {
			this.domAs().show();
			this.domAs().opacity = 1;
		}
		super.intoForeground(time);
	}
	
	intoBackground(time: number) {
		if ( this.collection.current.toolbar !== this ) {
			if ( time ) {
				this.domAs().transition({ opacity: 0, time: time }, ()=>{ this.domAs().hide() });
			} else {
				this.domAs().opacity = 0;
				this.domAs().hide();
			}
		}
		super.intoBackground(time);
	}

	intoLeave(time: number) {
		if ( this.collection.current.toolbar !== this ) {
			if ( this.status == 0 && time ) {
				this.domAs().transition({ opacity: 0, time: time }, ()=>{
					if ( this.collection.defaultToolbar !== this ) {
						this.remove();
					} else {
						this.domAs().hide();
					}
				});
			
			} else {
				if ( this.collection.defaultToolbar !== this ) {
					this.remove();
				} else {
					this.domAs().hide();
				}
			}
		}
		super.intoLeave(time);
	}
	
	get isDefault() {
		return this.collection && this.collection.defaultToolbar === this;
	}
}

/**
 * @func backgroundColorReverse
 */
function backgroundColorReverse(self: NavPage) {
	var color = self.domAs<Indep>().backgroundColor.reverse();
	color.a = 255 * 0.6;
	return color;
}

/**
 * @class NavPage
 */
export class NavPage extends Navigation {
	private m_title = '';
	private m_navbar: Navbar;
	private m_toolbar: Toolbar;
	private m_collection: NavPageCollection;
	private m_prevPage: NavPage | null = null;
	private m_nextPage: NavPage | null = null;

	@prop backgroundColor = '#fff';

	// @public
	get title() { return this.m_title }
	get collection() { return this.m_collection }
	get navbar(): Navbar { 
		if ( this.m_navbar ) {
			return this.m_navbar;
		} else {
			this.navbar = <Navbar />;
			return this.m_navbar;
		}
	}
	get toolbar(): Toolbar { 
		if ( this.m_toolbar ) {
			return this.m_toolbar;
		} else {
			this.toolbar = <Toolbar />;
			return this.m_toolbar;
		}
	}
	get prevPage() { return this.m_prevPage }
	get nextPage() { return this.m_nextPage }
	get isCurrent() { return this.m_collection && this.m_collection.isCurrent(this) }

	set title(value) {
		this.m_title = String(value);
		if (this.m_navbar) {
			this.m_navbar.setTitleText(this.m_title);
		}
		if (this.m_nextPage && this.m_nextPage.navbar) {
			this.m_nextPage.navbar.setBackText(value);
		}
	}

	set navbar(value: Navbar) {
		if (value) {
			value = quark.render(value) as Navbar;
			utils.assert(value instanceof Navbar, 'Type not correct');
			if (value !== this.m_navbar) {
				utils.assert(!value.page);
				if (this.m_navbar) {
					this.m_navbar.remove();
				}
				this.m_navbar = value;
				(this as any).m_navbar.m_page = this; // private props visit
				this.m_navbar.setTitleText(this.m_title);
				this.m_navbar.setBackText(this.prevPage ? this.prevPage.title : '');
				this.m_navbar.refreshStyle(0);
			}
		}
	}

	set toolbar(value: Toolbar) {
		if (value) {
			value = quark.render(value) as Toolbar;
			utils.assert(value instanceof Toolbar, 'Type not correct');
			if (value !== this.m_toolbar) {
				utils.assert(!value.page || value.isDefault);
				if (this.m_toolbar) {
					if ( !this.m_toolbar.isDefault ) {
						this.m_toolbar.remove();
					}
				}
				this.m_toolbar = value;
				(this as any).m_toolbar.m_page = this;
				this.m_toolbar.refreshStyle(0);
			} else {
				(this as any).m_toolbar.m_page = this;
			}
		}
	}

	// @overwrite
	protected render(...vdoms: any[]) {
		return (
			<Indep width="100%" height="full" backgroundColor={this.backgroundColor} visible={0}>{vdoms}</Indep>
		);
	}

	// @overwrite
	intoBackground(time: number) {
		// console.log('intoBackground', time);
		//console.log( this.nextPage == null ? 'null' : 'no null' )
		if ( this.nextPage == null ) return;
		//console.log( 'natpage intoBackground' )
		this.navbar.intoBackground(time);
		this.toolbar.intoBackground(time);
		if ( this.status != 1 ) {
			if ( time && (this.domAs().parent as Div).finalVisible ) {
				this.domAs().transition({ x: (this.domAs().parent as Div).finalWidth / -3, visible: false, time: time });
			} else {
				this.domAs().style = { x: ((this.domAs().parent as Div).finalWidth || 100) / -3, visible: false };
			}
		}
		super.intoBackground(time);
	}

	// @overwrite
	intoForeground(time: number) {
		// console.log('intoForeground', time);
		if ( this.status == 0 ) return;
		this.navbar.intoForeground(time);
		this.toolbar.intoForeground(time);
		this.m_nextPage = null;
		if ( this.status == -1 ) {
			if ( time && (this.domAs().parent as Div).finalVisible ) {
				this.domAs().style = { 
					borderLeftColor: backgroundColorReverse(this), 
					borderLeftWidth: quark.atomPixel, 
					x: (this.domAs().parent as Div).finalWidth, 
					visible: true,
				};
				this.domAs().transition({ x: 0, time: time }, ()=>{ 
					(this.domAs() as Indep).borderLeftWidth = 0;
				});
			} else {
				this.domAs().style = { x: 0, borderLeftWidth: 0, visible: true };
			}
			(this.m_toolbar as any).m_page = this;
		} 
		else if ( this.status == 1 ) {
			if ( time && (this.domAs().parent as Div).finalVisible ) {
				this.domAs().visible = true;
				this.domAs().transition({ x: 0, time: time });
			} else {
				this.domAs().style = { x: 0, visible: true };
			}
			(this.m_toolbar as any).m_page = this;
		}
		super.intoForeground(time);
	}

	// @overwrite
	intoLeave(time: number) {
		// console.log('intoLeave', time);
		this.navbar.intoLeave(time);
		this.toolbar.intoLeave(time);
		if ( this.status == 0 ) {
			if ( time && (this.domAs().parent as Div).finalVisible ) {
				this.domAs().style = { 
					borderLeftColor: backgroundColorReverse(this), 
					borderLeftWidth: quark.atomPixel, 
				};
				this.domAs().transition({ x: (this.domAs().parent as Div).finalWidth, visible: false, time: time }, ()=>{
					this.remove();
				});
				super.intoLeave(time);
				return;
			}
		}
		super.intoLeave(time);
		this.remove();
	}

	// @overwrite
	protected triggerRemove() {
		if (this.m_navbar) {
			this.m_navbar.remove();
		}
		if (this.m_toolbar && !this.m_toolbar.isDefault) {
			this.m_toolbar.remove();
		}
		return super.triggerRemove();
	}

	// @overwrite
	navigationBack() {
		if ( this.m_prevPage ) {
			this.m_collection.pop(true);
			return true;
		} else {
			return false;
		}
	}
}