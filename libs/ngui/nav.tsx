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

import 'ngui/util';
import 'ngui/sys';
import { List, KEYCODE_MENU } from './event';
import { 
	ViewController, Div, Indep, 
	Limit, Button, Text, TextNode, Clip, render, ngui
} from './index';
// import {Color,parseColor} from 'ngui/value';

export const FOREGROUND_ACTION_INIT = 0;
export const FOREGROUND_ACTION_RESUME = 1;
const DEFAULT_TRANSITION_TIME = 400;

const G_navigationStack = new List();
var G_navigationInit_ok = false;

function get_valid_focus(nav, focus_move) {
	if (!nav) return null;
	var view = nav.__view__;
	return view.hasChild(focus_move) ? view.firstButton() : focus_move;
}

function G_navigationInit(nav) {
	if ( nav.m_stack !== G_navigationStack || G_navigationInit_ok || !ngui.root) {
		return;
	}

	// initialize
	G_navigationInit_ok = true;

	var root = ngui.root;

	root.onBack.on(function(ev) {
		var last = G_navigationStack.last;
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
		if ( ev.keyboard ) { // 需要键盘产生的事件
			var last = G_navigationStack.last;
			if ( last ) {
				last.value.navigationEnter(ev.sender);
			}
		}
	});

	root.onKeyDown.on(function(ev) {
		var last = G_navigationStack.last;
		if ( last ) {
			var focus_move = ev.focusMove;
			var nav = last.value;

			switch(ev.keycode) {
				case 37: // left
					focus_move = nav.navigationLeft(focus_move);
					break;
				case 38: // up
					focus_move = nav.navigationTop(focus_move);
					break;
				case 39: // right
					focus_move = nav.navigationRight(focus_move);
					break;
				case 40: // down
					focus_move = nav.navigationDown(focus_move);
					break;
				case KEYCODE_MENU:
					nav.navigationMenu();
				default: return;
			}
			ev.focusMove = focus_move;
		}
	});
}

/**
 * @class Status
 */
class Status extends ViewController {
	m_status = -1; // 1=background,0=foreground,-1=init or exit
	// @public
	get status() { return this.m_status }
	intoBackground(time) { this.m_status = 1 }
	intoForeground(time, action, data) { this.m_status = 0 }
	intoLeave(time) { this.m_status = -1 }
}

/**
 * @class Navigation
 */
export class Navigation extends Status {
	
	m_stack = G_navigationStack;
	m_iterator = null;
	m_focus_resume = null;
	
	/**
	 * @event onBackground
	 */
	event onBackground;
	
	/**
	 * @event onForeground
	 */
	event onForeground;
	
	intoBackground(time) { 
		super.intoBackground(time);
		this.triggerBackground();
	}
	
	intoForeground(time, action, data) {
		super.intoForeground(time, action, data);
		this.triggerForeground({ action: action, data: data });
	}
	
	/**
	 * @func defaultFocus() 导航初始化时,返回一个焦点视图,重写这个函数
	 */
	defaultFocus() {
		// var dom = this.dom;
		// if ( dom ) {
		//   return dom.firstButton();
		// }
		return null;
	}
	
	triggerRemove(e) {
		if ( this.m_iterator ) { // force delete global nav stack
			this.m_stack.del(this.m_iterator);
			this.m_iterator = null;
		}
		super.triggerRemove(e);
	}

	/**
	 * @func registerNavigation()
	 */
	registerNavigation(time) {
		if ( !this.m_iterator ) { // No need to repeat it
			G_navigationInit(this);
			this.m_iterator = this.m_stack.push(this);
			// console.log('push_navigation()-----', this.m_stack.length);
			ngui.lock(()=>{
				var prev = this.m_iterator.prev;
				if ( prev ) {
					var focus = ngui.app.focusView;
					prev.m_focus_resume = prev.value.__view__.hasChild(focus) ? focus : null;
					prev.value.intoBackground(time);
				}
				var view = this.defaultFocus();
				if ( view ) {
					view.focus();
				}
				this.intoForeground(time, FOREGROUND_ACTION_INIT, null);
			});
		}
	}

	/**
	 * @func unregisterNavigation(time, data)
	 */
	unregisterNavigation(time, data) {
		if ( this.m_iterator ) {
			// util.assert(this.m_iterator, 'Bad iterator!');
			var last = this.m_stack.last;
			this.m_stack.del(this.m_iterator);
			this.m_iterator = null;
			if (!last || last.value !== this) return;
			
			ngui.lock(()=>{
				this.intoLeave(time);
				var last = this.m_stack.last;
				if ( last ) {
					if (last.value.m_focus_resume) {
						last.value.m_focus_resume.focus();
					}
					last.value.intoForeground(time, FOREGROUND_ACTION_RESUME, data);
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

	navigationEnter(focus) {
		// Rewrite this function to implement your logic
	}

	/**
	 * navigationTop()
	 * navigationDown()
	 * navigationLeft()
	 * navigationRight()
	 * 返回null时焦点不会发生任何改变
	 */
	navigationTop(focus_move) {
		return get_valid_focus(this, focus_move);
	}

	navigationDown(focus_move) {
		return get_valid_focus(this, focus_move);
	}

	navigationLeft(focus_move) {
		return get_valid_focus(this, focus_move);
	}

	navigationRight(focus_move) {
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
function refresh_bar_style(self, time) {
	if ( self.IDs.navbar && self.current ) {
		time = self.enableAnimate && time;
		var navbar = self.navbar || { 
			height: 0, border: 0, backgroundColor: '#0000', borderColor: '#0000' 
		};
		var toolbar = self.toolbar || { 
			height: 0, border: 0, backgroundColor: '#0000', borderColor: '#0000' 
		};
		var navbarHidden = self.$navbarHidden || self.navbar.$hidden;
		var toolbarHidden = self.$toolbarHidden || self.toolbar.$hidden
		var navbar_height = navbarHidden ? 0 : navbar.height + self.m_padding + navbar.border;
		var toolbar_height = toolbarHidden ? 0 : toolbar.height + toolbar.border;
		
		if ( time ) {
			if ( !navbarHidden ) self.IDs.navbar.show();
			if ( !toolbarHidden ) self.IDs.toolbar.show();
			ngui.lock(()=>{
				self.IDs.navbar.transition({
					height: Math.max(0, navbar_height - navbar.border), 
					borderBottom: `${navbar.border} ${navbar.borderColor}`, 
					backgroundColor: navbar.backgroundColor,
					time: time,
				});
				//console.log(navbar.backgroundColor, 'OKOK1', time);
				self.IDs.toolbar.transition({ 
					height: Math.max(0, toolbar_height - toolbar.border),
					borderTop: `${toolbar.border} ${toolbar.borderColor}`, 
					backgroundColor: toolbar.backgroundColor,
					time: time,
				});
				self.IDs.page.transition({ height: navbar_height + toolbar_height + '!', time: time }, ()=>{
					if ( navbarHidden ) self.IDs.navbar.hide();
					if ( toolbarHidden ) self.IDs.toolbar.hide();
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
	m_padding = ngui.statusBarHeight; // ios/android, 20
	m_pages = null;
	m_substack = null;
	m_default_toolbar = null;
	m_animating = false;
	$navbarHidden = false;
	$toolbarHidden = false;
	
	/**
	 * @field enableAnimate
	 */
	enableAnimate = true;
	
	event onPush;
	event onPop;
	
	get padding() { return this.m_padding }
	get navbarHidden() { return this.$navbarHidden }
	get toolbarHidden() { return this.$toolbarHidden }
	set navbarHidden(value) { this.setNavbarHidden(value, false) }
	set toolbarHidden(value) { this.setToolbarHidden(value, false) }
		
	get length() { return this.m_pages.length }
	get pages() { return this.m_pages.slice() }
	get current() { return this.m_pages.last(0) || null }
	get navbar() { return this.length ? this.current.navbar : null }
	get toolbar() { return this.length ? this.current.toolbar : null }
	get defaultToolbar() { return this.m_default_toolbar }
	
	set padding(value) {
		util.assert(typeof value == 'number');
		this.m_padding = Math.max(value, 0);
		refresh_bar_style(this, 0);
	}
	
	/**
	 * @func setNavbarHidden
	 */
	setNavbarHidden(value, time) {
		this.$navbarHidden = !!value;
		refresh_bar_style(this, time ? DEFAULT_TRANSITION_TIME : 0);
	}
	
	/**
	 * @func setToolbarHidden
	 */
	setToolbarHidden(value, time) {
		this.$toolbarHidden = !!value;
		refresh_bar_style(this, time ? DEFAULT_TRANSITION_TIME : 0);
	}
	
	/**
	 * @set defaultToolbar {Toolbar} # Set default toolbar
	 */
	set defaultToolbar(value) {
		if (value) {
			value = render(value);
			util.assert(value instanceof Toolbar, 'Type not correct');
			util.assert(!value.m_collection || value.m_collection !== this);
			if ( value !== this.m_default_toolbar ) {
				if ( this.m_default_toolbar ) {
					this.m_default_toolbar.remove();
				}
				this.m_default_toolbar = value;
				this.m_default_toolbar.m_collection = this;
			}
		} else { // cancel
			if ( this.m_default_toolbar ) {
				this.m_default_toolbar.remove();
				this.m_default_toolbar = null;
			}
		}
	}
	
	constructor() {
		super();
		this.m_pages = [];
		this.m_substack = new List();
	}

	render(...vdoms) {
		this.m_default_page = vdoms.find(e=>e);
		return (
			<Clip width="100%" height="100%">
				<Div id="navbar" width="100%" />
				<Div id="page" width="100%" />
				<Div id="toolbar" width="100%" />
			</Clip>
		);
	}

	triggerMounted(e) {
		if (this.m_default_page) {
			/* delay 因为是第一次加载,布局系统还未初始化
			 * 无法正确的获取数值来进行title bar的排版计算
			 * 所以这里延时一帧画面
			 */
			ngui.nextFrame(e=>this.push(this.m_default_page));
		}
		super.triggerMounted(e);
		ngui.nextFrame(e=>this.registerNavigation(0));
	}

	triggerRemove(e) {
		this.m_pages.forEach(e=>e.remove());
		this.m_pages = [];
		super.triggerRemove(e);
	}

	push(page, animate) {
		
		if ( this.m_animating ) {
			return;
		}
		var time = this.enableAnimate && animate && this.length ? DEFAULT_TRANSITION_TIME : 0;
		var prev = this.current;
		
		if ( page ) {
			if ( page instanceof NavPage ) { // dom
				util.assert(!page.collection, 'NavPage can only be a new entity');
				render(page, this.IDs.page);
			} else {
				if (ViewController.typeOf(page, NavPage)) {
					page = render(page, this.IDs.page);
				} else {
					page = render(<NavPage>{page}</NavPage>, this.IDs.page);
				}
			}
		}

		util.assert(page instanceof NavPage, 
			'The argument navpage is not of the correct type, '+
			'Only for NavPage entities or NavPage VX data.');
		
		// set page
		page.m_stack = this.m_substack;
		page.m_collection = this;
		page.m_prevPage = prev;
		
		if (prev) { // set next page
			prev.m_nextPage = page;
		}
		
		if (!page.m_navbar) { // Create default navbar
			page.navbar = <Navbar />;
		}
		
		if (!page.m_toolbar) { // use default toolbar
			if (this.defaultToolbar) {
				page.toolbar = this.defaultToolbar;
			} else {
				page.toolbar = <Toolbar />;
			}
		}
		
		this.m_pages.push(page);
		
		page.navbar.m_collection = this;
		page.toolbar.m_collection = this;
		page.navbar.appendTo(this.IDs.navbar);
		page.toolbar.appendTo(this.IDs.toolbar);
		
		this.m_animating = time;
		if ( time ) {
			setTimeout(()=>{ this.m_animating = false }, time);
		}
		
		page.navbar.$setBackText(prev ? prev.title : '');
		
		refresh_bar_style(this, time);
		
		// switch and animate
		this.triggerPush(page);

		page.registerNavigation(time);
	}
	
	pop(animate) {
		this.pops(1, animate);
	}
	
	pops(count, animate) {
		count = Number(count) || 0;
		count = Math.min(this.length - 1, count);
		
		if ( count < 1 ) {
			return;
		}
		if ( this.m_animating ) {
			return;
		}

		var time = this.enableAnimate && animate ? DEFAULT_TRANSITION_TIME : 0;
		// var page = this.m_pages[this.length - 1 - count];
		var arr  = this.m_pages.splice(this.length - count);
		var next = arr.pop();

		if (next) {
			arr.forEach(page=>page.intoLeave(false));

			this.m_animating = time;
			if ( time ) {
				setTimeout(()=>{ this.m_animating = false }, time);
			}
			refresh_bar_style(this, time);

			// switch and animate
			this.triggerPop(next);

			next.unregisterNavigation(time, null);
		}
	}

	// @overwrite
	navigationBack() {
		if (this.m_pages.length)
			return this.m_pages.last(0).navigationBack();
		return false;
	}
	// @overwrite
	navigationEnter(focus) {
		if (this.m_pages.length) 
			this.m_pages.last(0).navigationEnter(focus);
	}
	// @overwrite
	navigationTop(focus_move) {
		return get_valid_focus(this.m_pages.last(0), focus_move);
	}
	// @overwrite
	navigationDown(focus_move) {
		return get_valid_focus(this.m_pages.last(0), focus_move);
	}
	// @overwrite
	navigationLeft(focus_move) {
		return get_valid_focus(this.m_pages.last(0), focus_move);
	}
	// @overwrite
	navigationRight(focus_move) {
		return get_valid_focus(this.m_pages.last(0), focus_move);
	}
	// @overwrite
	navigationMenu() {
		if (this.m_pages.length) 
			this.m_pages.last(0).navigationMenu();
	}

}

/**
 * @class Bar
 */
class Bar extends Status {
	$height = 44;
	$hidden = false;
	$border = ngui.atomPixel;
	$borderColor = '#b3b3b3';
	$backgroundColor = '#f9f9f9';
	m_page = null;
	m_collection = null;
	
	get height() { return this.$height }
	get hidden() { return this.$hidden }
	get border() { return this.$border }
	get borderColor() { return this.$borderColor }
	get backgroundColor() { return this.$backgroundColor }
	
	get collection() { return this.m_collection }
	get page() { return this.m_page }
	get isCurrent() { return this.m_page && this.m_page.isCurrent }
	
	set height(value) {
		util.assert(typeof value == 'number');
		this.$height = value;
		this.refreshStyle(0);
	}
	set hidden(value) {
		this.$hidden = !!value;
		this.refreshStyle(0);
	}
	set border(value) {
		util.assert(typeof value == 'number');
		this.$border = value; 
		this.refreshStyle(0); 
	}
	set borderColor(value) {
		this.$border = value; 
		this.refreshStyle(0); 
	}
	set backgroundColor(value) {
		this.$backgroundColor = value; 
		this.refreshStyle(0); 
	}
	
	setHidden(value, time) {
		this.$hidden = !!value;
		this.refreshStyle(time ? DEFAULT_TRANSITION_TIME : 0);
	}
	
	/**
	 * @fun refreshStyle
	 */
	refreshStyle(time) {
		if (this.isCurrent) {
			refresh_bar_style(this.m_page.collection, time);
		}
	}
	
	get visible() {
		return this.dom.visible;
	}
	
	set visible(value) {
		if ( value ) {
			if (this.isCurrent) {
				this.dom.visible = 1;
			}
		} else {
			if (!this.isCurrent) {
				this.dom.visible = 0;
			}
		}
	}

}

/**
 * @func navbar_compute_title_layout
 */
function navbar_compute_title_layout(self) {
	if ( self.$defaultStyle ) {
		
		var back_text = self.IDs.back_text1.value;
		var title_text = self.IDs.title_text_panel.value;
		var backIconVisible = self.$backIconVisible;

		if ( self.page && self.page.m_prevPage ) {
			self.IDs.back_text_btn.visible = true;
		} else {
			self.IDs.back_text_btn.visible = false;
			back_text = '';
			backIconVisible = false;
		}
		
		var nav_width = self.collection ? self.collection.dom.finalWidth : 0;
		
		// console.log('----------------------nav_width', nav_width);

		var back_width = self.IDs.back_text1.simpleLayoutWidth(back_text) + 3; // 3间隔
		var title_width = self.IDs.title_text_panel.simpleLayoutWidth(title_text);
		var menu_width = Math.min(nav_width / 3, Math.max(self.$titleMenuWidth, 0));
		var marginLeft = 0;
		var min_back_width = 6;
		
		if ( backIconVisible ) {
			min_back_width += self.IDs.back_text0.simpleLayoutWidth('\uedc5');
			back_width += min_back_width;
		}
		
		self.IDs.title_panel.marginLeft = marginLeft;
		self.IDs.title_panel.marginRight = menu_width;
		self.IDs.title_panel.show();
		self.IDs.back_text0.visible = backIconVisible;
		
		if ( nav_width ) {
			var title_x = nav_width / 2 - title_width / 2 - marginLeft;
			if ( back_width <= title_x ) {
				back_width = title_x;
			} else { // back 的宽度超过title-x位置
				//console.log(back_width, (nav_width - menu_width - marginLeft) - title_width);
				back_width = Math.min(back_width, (nav_width - menu_width - marginLeft) - title_width);
				back_width = Math.max(min_back_width, back_width);
			}
			title_width = nav_width - back_width - menu_width - marginLeft;
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
		self.IDs.back_text_panel.width = Math.floor(back_text_num * 100000) / 1000 + '%';
		self.IDs.title_text_panel.width = Math.floor(titl_text_num * 100000) / 1000 + '%';

	} else {
		self.IDs.title_panel.hide(); // hide title text and back text
	}
}

/**
 * @class Navbar
 */
export class Navbar extends Bar {
	m_back_panel_width = 0;
	m_title_panel_width = 0;
	$defaultStyle = true;
	$backIconVisible = true;
	$titleMenuWidth = 40; // display right menu button width
	$backgroundColor = '#2c86e5'; // 3c89fb
	
	// @public
	get backIconVisible() { return this.$backIconVisible }
	get defaultStyle() { return this.$defaultStyle }
	get titleMenuWidth() { return this.$titleMenuWidth }
	
	set backIconVisible(value) {
		this.$backIconVisible = !!value;
		navbar_compute_title_layout(this);
	}
	
	set defaultStyle(value) {
		this.$defaultStyle = !!value;
		navbar_compute_title_layout(this);
	}
	
	set titleMenuWidth(value) {
		util.assert(typeof value == 'number');
		this.$titleMenuWidth = value;
		navbar_compute_title_layout(this);
	}

	refreshStyle(time) {
		if (this.isCurrent) {
			this.dom.alignY = 'bottom';
			this.dom.height = this.height;
			this.IDs.title_text_panel.textLineHeight = this.height;
			this.IDs.back_text_btn.textLineHeight = this.height;
			super.refreshStyle(time);
		}
	}
	
	/**
	 * @overwrite
	 */
	render(...vdoms) {
		var height = this.height;
		var textSize = 16;
		return (
			<Indep width="100%" height=height visible=0 alignY="bottom">
				{vdoms}
				<Indep id="title_panel" width="full" height="100%" visible=0>
					<Div id="back_text_panel" height="full">
						<Limit maxWidth="100%">
							<!--textColor="#0079ff"-->
							<Button id="back_text_btn" 
								onClick=(e=>this.collection.pop(true))
								textColor=this.backTextColor
								width="full" 
								textLineHeight=height 
								textSize=textSize
								textWhiteSpace="no_wrap" textOverflow="ellipsis">
								<Div width=6 />
								<TextNode id="back_text0" 
									textLineHeight="auto" 
									textSize=20
									height=26 y=2
									textColor="inherit" 
									textFamily="icon" value='\uedc5' />
								<TextNode id="back_text1" />
							</Button>
						</Limit>
					</Div>
					
					<Text id="title_text_panel" 
						height="full"
						textColor=this.titleTextColor
						textLineHeight=height 
						textSize=textSize
						textWhiteSpace="no_wrap" 
						textStyle="bold" textOverflow="ellipsis" />
						
				</Indep>
			</Indep>
		);
	}

	/**
	 * @fun setBackText # set navbar back text
	 */
	$setBackText(value) {
		this.IDs.back_text1.value = value;
		navbar_compute_title_layout(this);
	}
	
	/**
	 * @fun $setTitleText # set navbar title text
	 */
	$setTitleText(value) {
		this.IDs.title_text_panel.value = value;
		navbar_compute_title_layout(this);
	}
	
	intoBackground(time) {
		if ( time ) { 
			if ( this.$defaultStyle ) {
				var back_icon_width = this.IDs.back_text0.visible ? this.IDs.back_text0.clientWidth : 0;
				this.IDs.back_text1.transition({ 
					x: -this.IDs.back_text1.clientWidth, time: time,
				});
				this.IDs.title_text_panel.transition({ 
					x: -this.m_back_panel_width + back_icon_width, time: time,
				});
			}
			this.dom.transition({ opacity: 0, time: time }, ()=>{ this.dom.hide() });
		} else {
			this.dom.opacity = 0;
			this.dom.hide();
		}
		super.intoBackground(time);
	}
	
	intoForeground(time, action, data) { 
		this.dom.show(); // show
		if ( time ) {
			if ( this.$defaultStyle ) {
				var back_icon_width = 0; // this.IDs.back_text0.visible ? 20 : 0;
				if ( this.status == -1 ) {
					this.IDs.back_text1.x = this.m_back_panel_width - back_icon_width;
					this.IDs.title_text_panel.x = this.m_title_panel_width + this.$titleMenuWidth;
				}
				this.IDs.back_text1.transition({ x: 0, time: time });
				this.IDs.title_text_panel.transition({ x: 0, time: time });
			} else {
				this.IDs.back_text1.x = 0;
				this.IDs.title_text_panel.x = 0;
			}
			this.dom.opacity = 0;
			this.dom.transition({ opacity: 1, time: time });
		} else {
			this.dom.opacity = 1;
			this.IDs.back_text1.x = 0;
			this.IDs.title_text_panel.x = 0;
		}
		super.intoForeground(time, action, data);
	}
	
	intoLeave(time) { 
		if ( this.status == 0 && time ) {
			if ( this.$defaultStyle ) {
				var back_icon_width = this.IDs.back_text0.visible ? this.IDs.back_text0.clientWidth : 0;
				this.IDs.back_text1.transition({ x: this.m_back_panel_width - back_icon_width, time: time });
				this.IDs.title_text_panel.transition({ 
					x: this.m_title_panel_width + this.$titleMenuWidth, time: time,
				});
			}
			this.dom.transition({ opacity: 0, time: time }, ()=>{ this.remove() });
		} else {
			this.remove();
		}
		super.intoLeave(time);
	}
}

Navbar.defineProps({ backTextColor: '#fff', titleTextColor: '#fff' }, Navbar);

/**
 * @class Toolbar
 */
export class Toolbar extends Bar {

	$height = 49;

	/**
	 * @overwrite
	 */
	render(...vdoms) {
		return (
			<Indep width="100%" height="full" visible=0>{vdoms}</Indep>
		);
	}
	
	intoForeground(time, action, data) {
		if ( this.isDefault ) {
			this.m_page = this.collection.current;
		}
		if ( time ) {
			var page = (this.page.nextPage || this.page.prevPage);
			if (!page || page.toolbar !== this) {
				this.dom.show();
				this.dom.opacity = 0;
				this.dom.transition({ opacity: 1, time: time });
			}
		} else {
			this.dom.show();
			this.dom.opacity = 1;
		}
		super.intoForeground(time, action, data);
	}
	
	intoBackground(time) {
		if ( this.collection.current.toolbar !== this ) {
			if ( time ) {
				this.dom.transition({ opacity: 0, time: time }, ()=>{ this.dom.hide() });
			} else {
				this.dom.opacity = 0;
				this.dom.hide();
			}
		}
		super.intoBackground(time);
	}

	intoLeave(time) {
		if ( this.collection.current.toolbar !== this ) {
			if ( this.status == 0 && time ) {
				this.dom.transition({ opacity: 0, time: time }, ()=>{
					if ( this.collection.defaultToolbar !== this ) {
						this.remove();
					} else {
						this.dom.hide();
					}
				});
			
			} else {
				if ( this.collection.defaultToolbar !== this ) {
					this.remove();
				} else {
					this.dom.hide();
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
function backgroundColorReverse(self) {
	var color = self.dom.backgroundColor.reverse();
	color.a = 255 * 0.6;
	return color;
}

// Basic
/**
 * @class NavPage
 */
export class NavPage extends Navigation {
	m_title = '';
	m_navbar = null;
	m_toolbar = null;
	m_collection = null;
	m_prevPage = null;
	m_nextPage = null;

	// @public
	get title() { return this.m_title }
	get collection() { return this.m_collection }
	get navbar() { 
		if ( this.m_navbar ) {
			return this.m_navbar;
		} else {
			this.navbar = <Navbar />;
			return this.m_navbar;
		}
	}
	get toolbar() { 
		if ( this.m_toolbar ) {
			return this.m_toolbar;
		} else {
			this.toolbar = <Toolbar />;
			return this.m_toolbar;
		}
	}
	get prevPage() { return this.m_prevPage }
	get nextPage() { return this.m_nextPage }
	get isCurrent() { return this.m_collection && this.m_collection.current === this }

	set title(value) {
		this.m_title = String(value);
		if (this.m_navbar) {
			this.m_navbar.$setTitleText(this.m_title);
		}
		if (this.m_nextPage && this.m_nextPage.navbar) {
			this.m_nextPage.navbar.$setBackText(value);
		}
	}
	
	set navbar(value) {
		if (value) {
			value = render(value);
			util.assert(value instanceof Navbar, 'Type not correct');
			if (value !== this.m_navbar) {
				util.assert(!value.m_page);
				if (this.m_navbar) {
					this.navbar.remove();
				}
				this.m_navbar = value;
				this.m_navbar.m_page = this;
				this.m_navbar.$setTitleText(this.m_title);
				this.m_navbar.$setBackText(this.m_prevPage ? this.m_prevPage.m_title : '');
				this.m_navbar.refreshStyle(false);
			}
		}
	}
	
	set toolbar(value) {
		if (value) {
			value = render(value);
			util.assert(value instanceof Toolbar, 'Type not correct');
			if (value !== this.m_toolbar) {
				util.assert(!value.m_page || value.isDefault);
				if (this.m_toolbar) {
					if ( !this.m_toolbar.isDefault ) {
						this.m_toolbar.remove();
					}
				}
				this.m_toolbar = value;
				this.m_toolbar.m_page = this;
				this.m_toolbar.refreshStyle(false);
			} else {
				this.m_toolbar.m_page = this;
			}
		}
	}
	
	// @overwrite
	render(...vdoms) {
		return (
			<Indep width="100%" height="full" backgroundColor=this.backgroundColor visible=0>{vdoms}</Indep>
		);
	}
	
	// @overwrite
	intoBackground(time) {
		//console.log( this.nextPage == null ? 'null' : 'no null' )
		if ( this.nextPage == null ) return;
		//console.log( 'natpage intoBackground' )
		this.navbar.intoBackground(time);
		this.toolbar.intoBackground(time);
		if ( this.status != 1 ) {
			if ( time && this.dom.parent.finalVisible ) {
				this.dom.transition({ x: this.dom.parent.finalWidth / -3, visible: false, time: time });
			} else {
				this.dom.style = { x: (this.dom.parent.finalWidth || 100) / -3, visible: false };
			}
		}
		super.intoBackground(time);
	}
	
	// @overwrite
	intoForeground(time, action, data) {
		if ( this.status == 0 ) return;
		this.navbar.intoForeground(time, action, data);
		this.toolbar.intoForeground(time, action, data);
		this.m_nextPage = null;
		if ( this.status == -1 ) {
			if ( time && this.dom.parent.finalVisible ) {
				this.dom.style = { 
					borderLeftColor: backgroundColorReverse(this), 
					borderLeftWidth: ngui.atomPixel, 
					x: this.dom.parent.finalWidth, 
					visible: 1,
				};
				this.dom.transition({ x: 0, time: time }, ()=>{ 
					this.dom.borderLeftWidth = 0;
				});
			} else {
				this.dom.style = { x: 0, borderLeftWidth: 0, visible: 1 };
			}
			this.m_toolbar.m_page = this;
		} 
		else if ( this.status == 1 ) {
			if ( time && this.dom.parent.finalVisible ) {
				this.dom.visible = 1;
				this.dom.transition({ x: 0, time: time });
			} else {
				this.dom.style = { x: 0, visible: 1 };
			}
			this.m_toolbar.m_page = this;
		}
		super.intoForeground(time, action, data);
	}
	
	// @overwrite
	intoLeave(time) { 
		this.navbar.intoLeave(time);
		this.toolbar.intoLeave(time);
		if ( this.status == 0 ) {
			if ( time && this.dom.parent.finalVisible ) {
				this.dom.style = { 
					borderLeftColor: backgroundColorReverse(this), 
					borderLeftWidth: ngui.atomPixel, 
				};
				this.dom.transition({ x: this.dom.parent.finalWidth, visible: 0, time: time }, ()=>{
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
	triggerRemove(e) {
		if (this.m_navbar) {
			this.m_navbar.remove();
		}
		if (this.m_toolbar && !this.m_toolbar.isDefault) {
			this.m_toolbar.remove();
		}
		super.triggerRemove(e);
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

NavPage.defineProps({ backgroundColor: '#fff' });