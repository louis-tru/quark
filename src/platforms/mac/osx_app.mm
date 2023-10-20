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

#import <MacTypes.h>
#import <AppKit/AppKit.h>
// typedef UIEvent MacUIEvent;
#import "../../util/loop.h"
#import "../../app.h"
#import "../../event.h"
#import "../../display.h"
#import "./osx_app.h"

using namespace qk;

// ***************** Q k . A p p l i c a t i o n . D e l e g a t e *****************

QkApplicationDelegate* __appDelegate = nil;

@implementation QkApplicationDelegate

	- (void)applicationDidFinishLaunching:(NSNotification*) notification {
		Qk_ASSERT(!__appDelegate);
		Qk_ASSERT(Application::shared());
		__appDelegate = self;
		_host = Application::shared();
		_app = UIApplication.sharedApplication;
		_render = dynamic_cast<QkMacRender*>(_host->render());
		
		_is_background = NO;
		_is_pause = YES;

		auto &opts = _host->options();

		NSWindowStyleMask style = NSWindowStyleMaskBorderless |
			NSWindowStyleMaskTitled | NSWindowStyleMaskClosable |
			NSWindowStyleMaskResizable | NSWindowStyleMaskMiniaturizable;
		UIScreen* screen = UIScreen.mainScreen;

		float w = opts.windowFrame.size.x() > 0 ?
			opts.windowFrame.size.x(): screen.frame.size.width / 2;
		float h = opts.windowFrame.size.y() > 0 ?
			opts.windowFrame.size.y(): screen.frame.size.height / 2;
		float x = opts.windowFrame.origin.x() > 0 ?
			opts.windowFrame.origin.x(): (screen.frame.size.width - w) / 2.0;
		float y = opts.windowFrame.origin.y() > 0 ?
			opts.windowFrame.origin.y(): (screen.frame.size.height - h) / 2.0;

		Color4f color = opts.backgroundColor.to_color4f();

		self.window = [[UIWindow alloc] initWithContentRect:NSMakeRect(x, y, w, h)
																							styleMask:style
																								backing:NSBackingStoreBuffered
																									defer:NO
																								 screen:screen];
		self.root_ctr = nil; // nil
		self.window.backgroundColor = [UIColor colorWithSRGBRed:color.r()
																											green:color.g()
																											blue:color.b()
																											alpha:color.a()];
		self.window.title = [NSString stringWithFormat:@"%s", opts.windowTitle.c_str()];
		self.window.delegate = self;

		[self.window makeKeyAndOrderFront:nil];

		if (opts.windowFrame.origin.x() < 0 && opts.windowFrame.origin.y() < 0) {
			[self.window center];
		}

		UIView *rootView = self.window.contentView;

		self.ime = qk_ime_helper_new(_host);

		self.surface_view = self.render->make_surface_view(rootView.bounds);
		self.surface_view.translatesAutoresizingMaskIntoConstraints = NO;

		[rootView addSubview:self.surface_view];
		[rootView addConstraint:[NSLayoutConstraint
														constraintWithItem:self.surface_view
														attribute:NSLayoutAttributeWidth
														relatedBy:NSLayoutRelationEqual
														toItem:rootView
														attribute:NSLayoutAttributeWidth
														multiplier:1
														constant:0]];
		[rootView addConstraint:[NSLayoutConstraint
														constraintWithItem:self.surface_view
														attribute:NSLayoutAttributeHeight
														relatedBy:NSLayoutRelationEqual
														toItem:rootView
														attribute:NSLayoutAttributeHeight
														multiplier:1
														constant:0]];

		Inl_Application(_host)->triggerLoad();
	}

	- (void)applicationWillResignActive:(NSNotification*)notification {
		if (_is_pause) return;
		_is_pause = YES;
		Inl_Application(_host)->triggerPause();
		Qk_DEBUG("applicationWillResignActive, triggerPause");
	}

	- (void)applicationDidBecomeActive:(NSNotification*)notification {
		if (!_is_pause) return;
		_is_pause = NO;
		Inl_Application(_host)->triggerResume();
		_host->render()->reload();
		Qk_DEBUG("applicationDidBecomeActive, triggerResume");
	}

	- (void)applicationDidHide:(NSNotification*)notification {
		Qk_DEBUG("applicationDidHide, onBackground");
	}

	- (void)applicationWillUnhide:(NSNotification*)notification {
		Qk_DEBUG("applicationWillUnhide, onForeground");
	}

	- (void)applicationWillTerminate:(NSNotification*)notification {
		Inl_Application(_host)->triggerUnload();
		Qk_DEBUG("applicationWillTerminate, triggerUnload");
	}

	- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication*)sender {
		Qk_DEBUG("applicationShouldTerminateAfterLastWindowClosed, exit application");
		return YES;
	}

	- (BOOL)applicationShouldHandleReopen:(NSApplication*)sender hasVisibleWindows:(BOOL)flag {
		Qk_DEBUG("applicationShouldHandleReopen");
		return YES;
	}

	// ******* NSWindowDelegate *******

	- (BOOL)windowShouldClose:(NSWindow*)sender {
		return YES;
	}

	- (NSSize)windowWillResize:(NSWindow*)sender toSize:(NSSize)size {
		return size;
	}

	- (void)windowDidMiniaturize:(NSNotification*)notification {
		if (_is_background) return;
		_is_background = YES;
		[self applicationWillResignActive:notification];
		Inl_Application(_host)->triggerBackground();
		Qk_DEBUG("windowDidMiniaturize, triggerBackground");
	}

	- (void)windowDidDeminiaturize:(NSNotification*)notification {
		if (!_is_background) return;
		_is_background = NO;
		Inl_Application(_host)->triggerForeground();
		Qk_DEBUG("windowDidDeminiaturize,triggerForeground");
		[self applicationDidBecomeActive:notification];
	}

@end

// ***************** A p p l i c a t i o n *****************

void Application::pending() {
	// exit(0);
}

void Application::open_url(cString& url) {
	// TODO
}

void Application::send_email(cString& recipient,
														 cString& subject,
														 cString& cc, cString& bcc, cString& body)
{
	// TODO 
}

// ***************** E v e n t . D i s p a t c h *****************

void EventDispatch::set_volume_up() {
	// TODO ..
}

void EventDispatch::set_volume_down() {
	// TODO ..
}

void EventDispatch::set_ime_keyboard_open(KeyboardOptions options) {
	// TODO ..
}

void EventDispatch::set_ime_keyboard_can_backspace(bool can_backspace, bool can_delete) {
	// TODO ..
}

void EventDispatch::set_ime_keyboard_close() {
	// TODO ..
}

void EventDispatch::set_ime_keyboard_spot_location(Vec2 location) {
	// noop
}
