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
// typedef UIEvent AppleUIEvent;
#import "../../util/loop.h"
#import "../../app.h"
#import "../../event.h"
#import "../../display.h"
#import "./osx_app.h"

using namespace qk;

// ***************** Q k . A p p l i c a t i o n . D e l e g a t e *****************

typedef Display::Orientation Orientation;

QkApplicationDelegate* __appDelegate = nil;

@class OsxIMEHelprt;


@interface QkApplicationDelegate()<NSWindowDelegate>
	{
		UIWindow*  _window;
		BOOL       _is_background;
		BOOL       _is_pause;
		BOOL       _loaded;
	}
	@property (strong, nonatomic) UIView* view;
	@property (strong, nonatomic) OsxIMEHelprt* ime;
	@property (strong, nonatomic) UIApplication* host;
	// @property (strong, nonatomic) CADisplayLink* display_link;
	// @property (strong, nonatomic) RootViewController* root_ctr;
	- (void)display_link_callback:(const CVTimeStamp*)outputTime;
@end

@implementation QkApplicationDelegate

	- (void)display_link_callback:(const CVTimeStamp*)outputTime {
		if (self.host->is_loaded()) {
			self.host->display()->render();
		}
	}

	- (UIWindow*)window {
		return _window;
	}

	- (void)resize_with:(CGRect)rect {
		if (_loaded) {
			renderApple->resize(__appDelegate.view.frame);
			Qk_DEBUG("refresh_surface_size, %f, %f", rect.size.width, rect.size.height);
		}
	}

	- (void)resize {
		[self resize_with: __appDelegate.view.frame];
	}

	- (void)background {
		if (_loaded && !_is_background) {
			[self pause];
			_is_background = YES;
			Qk_DEBUG("onBackground");
			Inl_Application(_app)->triggerBackground();
		}
	}

	- (void)foreground {
		if (_loaded && _is_background) {
			_is_background = NO;
			Qk_DEBUG("onForeground");
			Inl_Application(_app)->triggerForeground();
			[self resume];
		}
	}

	- (void)pause {
		if (_loaded && !_is_pause) {
			Qk_DEBUG("onPause");
			_is_pause = YES;
			Inl_Application(_app)->triggerPause();
		}
	}

	- (void)resume {
		if (_loaded && _is_pause) {
			Qk_DEBUG("onResume");
			_is_pause = NO;
			Inl_Application(_app)->triggerResume();
			[self resize];
		}
	}

	- (void)initialize {
		// NSOpenGLContext* context = self.glview.openGLContext;
		// CGRect rect = self.glview.frame;
		// typedef Callback<RunLoop::PostSyncData> Cb;
		// _app->render_loop()->post_sync(Cb([self, rect, context](Cb::Data& d) {
		// 	gl_draw_context->initialize(self.glview, context);
		// 	gl_draw_context->refresh_surface_size(rect);
		// 	Inl_Application(_app)->triggerLoad();
		// 	_loaded = YES;
		// 	d.data->complete();
		// }));
		[self foreground];
	}

	- (void)applicationDidFinishLaunching:(NSNotification*) notification {
		Qk_ASSERT(!__appDelegate);
		__appDelegate = self;
		Qk_ASSERT(Application::shared());
		_host = Application::shared();

		// UIApplication* app = UIApplication.sharedApplication;

		_render = dynamic_cast<QkAppleRender*>(_host->render());

		auto &opts = _host->options();

		NSWindowStyleMask style = NSWindowStyleMaskBorderless |
			NSWindowStyleMaskTitled | NSWindowStyleMaskClosable |
			NSWindowStyleMaskResizable | NSWindowStyleMaskMiniaturizable;
		UIScreen* screen = UIScreen.mainScreen;
		float scale = screen.backingScaleFactor;

		float w = opts.size.x() > 0 ? opts.size.x(): screen.frame.size.width / 2;
		float h = opts.size.y() > 0 ? opts.size.y(): screen.frame.size.height / 2;
		float x = opts.origin.x() > 0 ? opts.origin.x(): (screen.frame.size.width - w) / 2.0;
		float y = opts.origin.y() > 0 ? opts.origin.y(): (screen.frame.size.height - h) / 2.0;
		
		_window = [[UIWindow alloc] initWithContentRect:NSMakeRect(x, y, w, h)
																					styleMask:style
																						backing:NSBackingStoreBuffered
																							defer:NO
																						screen:nil];
		_is_background = YES;
		_is_pause = YES;
		_loaded = NO;

		Color4f color = opts.backgroundColor.to_color4f();

		self.window.delegate = self;
		self.window.backgroundColor = [UIColor colorWithSRGBRed:color.r()
																											green:color.g()
																											blue:color.b()
																											alpha:1];
		self.window.title = [NSString stringWithFormat:@"%s", opts.windowTitle.c_str()];
		[self.window makeKeyAndOrderFront:nil];
		
		if (opts.x < 0 && opts.y < 0) {
			[self.window center];
		}

		UIView *rootView = self.window.contentView;

		self.view = _render->init_view(rootView.bounds);
		//[self.view scaleUnitSquareToSize:NSMakeSize(scale, scale)];
		self.view.layer.contentsScale = scale;
		//self.view.contentScaleFactor = scale;
		//self.view.translatesAutoresizingMaskIntoConstraints = NO;
		//self.view.wantsBestResolutionOpenGLSurface = YES;

		//self.ime = [[OsxIMEHelprt alloc] initWithApplication:self.host];

		[rootView addSubview:self.view];
		// [view addSubview:self.ime];
		[rootView addConstraint:[NSLayoutConstraint
														constraintWithItem:self.view
														attribute:NSLayoutAttributeWidth
														relatedBy:NSLayoutRelationEqual
														toItem:self.view
														attribute:NSLayoutAttributeWidth
														multiplier:1
														constant:0]];
		[rootView addConstraint:[NSLayoutConstraint
														constraintWithItem:self.view
														attribute:NSLayoutAttributeHeight
														relatedBy:NSLayoutRelationEqual
														toItem:self.view
														attribute:NSLayoutAttributeHeight
														multiplier:1
														constant:0]];

		_host->display()->set_default_scale(scale);

		// [self refresh_surface_region]; // set size

		Inl_Application(_host)->triggerLoad();

		// [self.display_link addToRunLoop:[NSRunLoop mainRunLoop]
		// 												forMode:NSDefaultRunLoopMode];

	}

	- (void)applicationWillResignActive:(NSNotification*)notification {
		[self pause];
	}

	- (void)applicationDidBecomeActive:(NSNotification*)notification {
		[self resume];
	}

	- (void)applicationDidHide:(NSNotification*)notification {
		Qk_DEBUG("applicationDidHide, onBackground");
	}

	- (void)applicationWillUnhide:(NSNotification*)notification {
		Qk_DEBUG("applicationWillUnhide, onForeground");
	}

	- (void)applicationWillTerminate:(NSNotification*)notification {
		Qk_DEBUG("applicationWillTerminate");
		Inl_Application(_app)->triggerUnload();
	}

	- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication*)sender {
		Qk_DEBUG("exit application");
		return YES;
	}

	- (BOOL)applicationShouldHandleReopen:(NSApplication*)sender hasVisibleWindows:(BOOL)flag {
		return YES;
	}

	// ******* NSWindowDelegate *******

	- (BOOL)windowShouldClose:(NSWindow*)sender {
		return YES;
	}

	- (NSSize)windowWillResize:(NSWindow*)sender toSize:(NSSize)size {
		[self resize_with: NSMakeRect(0, 0, size.width, size.height)];
		return size;
	}

	- (void)windowDidMiniaturize:(NSNotification*)notification {
		[self background];
	}

	- (void)windowDidDeminiaturize:(NSNotification*)notification {
		[self foreground];
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