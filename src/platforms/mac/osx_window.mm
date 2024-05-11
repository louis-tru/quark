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

#import "../../ui/app.h"
#include "./mac_app.h"
#include "../../ui/window.h"
#include "../../render/render_mac.h"

using namespace qk;

// ***************** W i n d o w *****************

QkWindowDelegate* WindowImpl::delegate() {
	return ((__bridge QkWindowDelegate*)(this));
}

@interface QkWindowDelegate()
@property (assign, nonatomic) BOOL isClose;
@property (assign, nonatomic) BOOL isBackground;
@end

@implementation QkWindowDelegate

- (id) init:(Window::Options&)opts win:(Window*)win render:(Render*)render {
	if ( !(self = [super init]) ) return nil;

	NSWindowStyleMask style = NSWindowStyleMaskBorderless |
		NSWindowStyleMaskTitled | NSWindowStyleMaskClosable |
		NSWindowStyleMaskResizable | NSWindowStyleMaskMiniaturizable;
	UIScreen* screen = UIScreen.mainScreen;

	float w = opts.frame.size.x() > 0 ?
		opts.frame.size.x(): screen.frame.size.width / 2;
	float h = opts.frame.size.y() > 0 ?
		opts.frame.size.y(): screen.frame.size.height / 2;
	float x = opts.frame.origin.x() > 0 ?
		opts.frame.origin.x(): (screen.frame.size.width - w) / 2.0;
	float y = opts.frame.origin.y() > 0 ?
		opts.frame.origin.y(): (screen.frame.size.height - h) / 2.0;

	UIWindow *uiwin = [[UIWindow alloc] initWithContentRect:NSMakeRect(x, y, w, h)
																						styleMask:style
																							backing:NSBackingStoreBuffered
																								defer:NO
																								screen:screen];
	uiwin.title = [NSString stringWithUTF8String:opts.title.c_str()];
	uiwin.delegate = self;

	self.isBackground = NO;
	self.isClose = NO;
	self.win = win;
	self.uiwin = uiwin;
	self.root_ctr = [QkRootViewController new];
	self.root_ctr.win = win;
	self.uiwin.contentViewController = self.root_ctr;
	self.ime = qk_make_ime_helper(win);

	if (opts.frame.origin.x() < 0 && opts.frame.origin.y() < 0) {
		[uiwin center];
	}

	UIView *rootView = self.root_ctr.view;
	UIView *view = render->surface()->surfaceView();

	view.frame = rootView.bounds;
	view.translatesAutoresizingMaskIntoConstraints = NO;

	[rootView addSubview:view];
	[rootView addConstraint:[NSLayoutConstraint
													constraintWithItem:view
													attribute:NSLayoutAttributeWidth
													relatedBy:NSLayoutRelationEqual
													toItem:rootView
													attribute:NSLayoutAttributeWidth
													multiplier:1
													constant:0]];
	[rootView addConstraint:[NSLayoutConstraint
													constraintWithItem:view
													attribute:NSLayoutAttributeHeight
													relatedBy:NSLayoutRelationEqual
													toItem:rootView
													attribute:NSLayoutAttributeHeight
													multiplier:1
													constant:0]];
	[rootView addSubview:self.ime.view];
	return self;
}

- (BOOL)windowShouldClose:(NSWindow*)sender {
	if (!self.isClose) {
		self.isClose = YES;
		self.win->host()->loop()->post(Cb([self](auto&e){
			self.win->close(); // close destroy window
		}));
	}
	return YES;
}

- (NSSize)windowWillResize:(NSWindow*)sender toSize:(NSSize)size {
	return size;
}

- (void)windowDidMiniaturize:(NSNotification*)notification {
	if (self.isBackground) return;
	self.isBackground = YES;
	Inl_Application(self.win->host())->triggerBackground(self.win);
	Qk_DEBUG("windowDidMiniaturize, triggerBackground");
}

- (void)windowDidDeminiaturize:(NSNotification*)notification {
	if (!self.isBackground) return;
	self.isBackground = NO;
	Inl_Application(self.win->host())->triggerForeground(self.win);
	Qk_DEBUG("windowDidDeminiaturize,triggerForeground");
}

@end

void Window::openImpl(Options &opts) {
	qk_post_messate_main(Cb([&opts,this](auto&e) {
		auto del = [[QkWindowDelegate alloc] init:opts win:this render:_render];
		CFBridgingRetain(del);
		_impl = (__bridge WindowImpl*)del;
		set_backgroundColor(opts.backgroundColor);
		activate();
	}), true);
}

void Window::closeImpl() {
	auto win = _impl->delegate();
	if (!win.isClose) {
		win.isClose = YES;
		qk_post_messate_main(Cb([win](auto&e) {
			[win.uiwin close]; // close platform window
		}), false);
	}
	//CFBridgingRelease(_impl);
	_impl = nullptr;
}

void Window::set_backgroundColor(Color val) {
	qk_post_messate_main(Cb([this,val](auto&e) {
		Color4f color = val.to_color4f();
		_impl->delegate().uiwin.backgroundColor =
			[UIColor colorWithSRGBRed:color.r() green:color.g() blue:color.b() alpha:color.a()];
	}), false);
	_backgroundColor = val;
}

void Window::activate() {
	qk_post_messate_main(Cb([this](auto&e) {
		[_impl->delegate().uiwin makeKeyAndOrderFront:nil];
	}), false);
	Inl_Application(_host)->setActiveWindow(this);
}

void Window::pending() {
}

void Window::setFullscreen(bool fullscreen) {
	qk_post_messate_main(Cb([this,fullscreen](auto&e) {
		auto uiwin = _impl->delegate().uiwin;
		auto screenSize = uiwin.screen.frame.size;
		auto size = uiwin.frame.size;
		if (screenSize.width == size.width && screenSize.height == size.height) {
			if (!fullscreen)
				[uiwin toggleFullScreen:nil];
		} else {
			if (fullscreen)
				[uiwin toggleFullScreen:nil];
		}
	}), false);
}

void Window::setCursorStyle(CursorStyle cursor, bool low) {
	static CursorStyle current_cursor_low = CursorStyle::Arrow;
	static CursorStyle current_cursor_high = CursorStyle::Normal;

	if (low) {
		current_cursor_low = cursor;
	} else {
		current_cursor_high = cursor;
	}
	cursor = current_cursor_high == CursorStyle::Normal ? current_cursor_low: current_cursor_high;

	if (cursor == CursorStyle::None) {
		[NSCursor hide];
	} else if (cursor == CursorStyle::NoneUntilMouseMoves) {
		[NSCursor setHiddenUntilMouseMoves:YES];
	} else {
		switch (cursor) {
			case CursorStyle::Normal:
			case CursorStyle::Arrow: [NSCursor.arrowCursor set]; break;
			case CursorStyle::Ibeam: [NSCursor.IBeamCursor set]; break;
			case CursorStyle::PointingHand: [NSCursor.pointingHandCursor set]; break;
			case CursorStyle::ClosedHand: [NSCursor.closedHandCursor set]; break;
			case CursorStyle::OpenHand: [NSCursor.openHandCursor set]; break;
			case CursorStyle::ResizeLeft: [NSCursor.resizeLeftCursor set]; break;
			case CursorStyle::ResizeRight: [NSCursor.resizeRightCursor set]; break;
			case CursorStyle::ResizeLeftRight: [NSCursor.resizeLeftRightCursor set]; break;
			case CursorStyle::ResizeUp: [NSCursor.resizeUpCursor set]; break;
			case CursorStyle::ResizeDown: [NSCursor.resizeDownCursor set]; break;
			case CursorStyle::ResizeUpDown: [NSCursor.resizeUpDownCursor set]; break;
			case CursorStyle::Crosshair: [NSCursor.crosshairCursor set]; break;
			case CursorStyle::DisappearingItem: [NSCursor.disappearingItemCursor set]; break;
			case CursorStyle::OperationNotAllowed: [NSCursor.operationNotAllowedCursor set]; break;
			case CursorStyle::DragLink: [NSCursor.dragLinkCursor set]; break;
			case CursorStyle::DragCopy: [NSCursor.dragCopyCursor set]; break;
			case CursorStyle::ContextualMenu: [NSCursor.contextualMenuCursor set]; break;
			case CursorStyle::IbeamForVertical: [NSCursor.IBeamCursorForVerticalLayout set]; break;
			default: break;
		}
		[NSCursor unhide];
	}
}
