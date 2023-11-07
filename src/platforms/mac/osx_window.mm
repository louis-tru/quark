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

#import "../../app.h"
#include "./mac_app.h"
#include "../../window.h"
#include "../../render/render_mac.h"

using namespace qk;

// ***************** W i n d o w *****************

QkWindowDelegate* WindowImpl::delegate() {
	return ((__bridge QkWindowDelegate*)(this));
}

UIWindow* WindowImpl::window() {
	return ((__bridge QkWindowDelegate*)(this)).window;
}

@interface QkWindowDelegate()
@property (assign, nonatomic) BOOL isClose;
@property (assign, nonatomic) BOOL isBackground;
@end

@implementation QkWindowDelegate

	- (id) init:(Window::Options&)opts win0:(Window*)win0 render:(Render*)render {
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

		UIWindow *window = [[UIWindow alloc] initWithContentRect:NSMakeRect(x, y, w, h)
																							styleMask:style
																								backing:NSBackingStoreBuffered
																									defer:NO
																									screen:screen];
		window.title = [NSString stringWithUTF8String:opts.title.c_str()];
		window.delegate = self;

		self.isBackground = NO;
		self.isClose = NO;
		self.win0 = win0;
		self.window = window;
		self.root_ctr = nil;
		self.ime = qk_make_ime_helper(shared_app());

		if (opts.frame.origin.x() < 0 && opts.frame.origin.y() < 0) {
			[window center];
		}

		UIView *rootView = window.contentView;
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
		return self;
	}

	- (BOOL)windowShouldClose:(NSWindow*)sender {
		if (!self.isClose) {
			self.isClose = YES;
			self.win0->host()->loop()->post(Cb([self](auto&e){
				self.win0->close(); // close destroy window
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
		Inl_Application(self.win0->host())->triggerBackground(self.win0);
		Qk_DEBUG("windowDidMiniaturize, triggerBackground");
	}

	- (void)windowDidDeminiaturize:(NSNotification*)notification {
		if (!self.isBackground) return;
		self.isBackground = NO;
		Inl_Application(self.win0->host())->triggerForeground(self.win0);
		Qk_DEBUG("windowDidDeminiaturize,triggerForeground");
	}

@end

void Window::pending() {
	// noop
}

void Window::openImpl(Options &opts) {
	qk_post_messate_main(Cb([&opts,this](auto&e) {
		auto del = [[QkWindowDelegate alloc] init:opts win0:this render:_render];
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
			[win.window close]; // close platform window
		}), false);
	}
	//CFBridgingRelease(_impl);
	_impl = nullptr;
}

void Window::set_backgroundColor(Color val) {
	qk_post_messate_main(Cb([this,val](auto&e) {
		Color4f color = val.to_color4f();
		_impl->window().backgroundColor = 
			[UIColor colorWithSRGBRed:color.r() green:color.g() blue:color.b() alpha:color.a()];
	}), false);
	_backgroundColor = val;
}

void Window::activate() {
	qk_post_messate_main(Cb([this](auto&e) {
		[_impl->window() makeKeyAndOrderFront:nil];
	}), false);
	Inl_Application(_host)->setActiveWindow(this);
}
