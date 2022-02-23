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

#import <AppKit/AppKit.h>

typedef UIEvent AppleUIEvent;

#import "../../util/loop.h"
#import "../../app.inl"
#import "../../display.h"
#import "../../event.h"
#import "../apple/apple_render.h"
#import "./ios_ime_helper.h"

using namespace flare;

typedef Display::Orientation Orientation;
typedef Display::StatusBarStyle StatusBarStyle;

@class ApplicationOptions;
@class OsxIMEHelprt;

static ApplicationDelegate* appDelegate = nil;
static RenderApple* renderApple = nil;
static NSString* appDelegateName = @"";
static ApplicationOptions* appOptions = nil;

#define UIApplication NSApplication
#define UIView NSView
#define UIColor NSColor
#define UIScreen NSScreen

typedef CGRect Rect;

/**
 * @interface ApplicationOptions
 */
@interface ApplicationOptions: NSObject;
	@property (assign, nonatomic) int x;
	@property (assign, nonatomic) int y;
	@property (assign, nonatomic) int width;
	@property (assign, nonatomic) int height;
	@property (strong, nonatomic) UIColor* background_color;
	@property (assign, nonatomic) String title;
@end

@implementation ApplicationOptions

	- (id)init:(cJSON&) options {
		self = [super init];
		if (!self) return self;
		
		self.x = -1;
		self.y = -1;
		self.width = -1;
		self.height = -1;
		self.background_color = [UIColor blackColor];
		self.title = String();
		
		cJSON& o_x = options["x"];
		cJSON& o_y = options["y"];
		cJSON& o_w = options["width"];
		cJSON& o_h = options["height"];
		cJSON& o_b = options["background"];
		cJSON& o_t = options["title"];
		
		if (o_w.is_uint()) _width = F_MAX(1, o_w.to_uint());
		if (o_h.is_uint()) _height = F_MAX(1, o_h.to_uint());
		if (o_x.is_uint()) _x = o_x.to_uint();
		if (o_y.is_uint()) _y = o_y.to_uint();
		if (o_t.is_string()) _title = o_t.to_string();
		if (o_b.is_uint()) {
			FloatColor color = Color(o_b.to_uint() << 8).to_float_color();
			_background_color = [UIColor colorWithSRGBRed:color.r()
																							green:color.g()
																							blue:color.b()
																							alpha:1];
		}
		return self;
	}

@end

/**
 * @interface ApplicationDelegate
 */
@interface ApplicationDelegate()<NSWindowDelegate>
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

@implementation ApplicationDelegate

	- (void)display_link_callback:(const CVTimeStamp*)outputTime {
		if (self.app->is_loaded()) {
			self.app->display()->render();
		}
	}

	- (UIWindow*)window {
		return _window;
	}

	- (void)resize_with:(CGRect)rect {
		if (_loaded) {
			renderApple->resize(appDelegate.view.frame);
			F_DEBUG("refresh_surface_size, %f, %f", rect.size.width, rect.size.height);
		}
	}

	- (void)resize {
		[self resize_with: appDelegate.view.frame];
	}

	+ (void)set_application_delegate:(NSString*)name {
		appDelegateName = name;
	}

	- (void)background {
		if (_loaded && !_is_background) {
			[self pause];
			_is_background = YES;
			F_DEBUG("onBackground");
			_inl_app(_app)->triggerBackground();
		}
	}

	- (void)foreground {
		if (_loaded && _is_background) {
			_is_background = NO;
			F_DEBUG("onForeground");
			_inl_app(_app)->triggerForeground();
			[self resume];
		}
	}

	- (void)pause {
		if (_loaded && !_is_pause) {
			F_DEBUG("onPause");
			_is_pause = YES;
			_inl_app(_app)->triggerPause();
		}
	}

	- (void)resume {
		if (_loaded && _is_pause) {
			F_DEBUG("onResume");
			_is_pause = NO;
			_inl_app(_app)->triggerResume();
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
		// 	_inl_app(_app)->triggerLoad();
		// 	_loaded = YES;
		// 	d.data->complete();
		// }));
		[self foreground];
	}

	- (void)applicationDidFinishLaunching:(NSNotification*) notification {
		F_ASSERT(!appDelegate);
		appDelegate = self;
		F_ASSERT(Application::shared());
		self.app = Application::shared(); 

		// UIApplication* host = UIApplication.sharedApplication;

		UIScreen* screen = UIScreen.mainScreen;
		NSWindowStyleMask style = NSWindowStyleMaskBorderless |
			NSWindowStyleMaskTitled | NSWindowStyleMaskClosable |
			NSWindowStyleMaskResizable | NSWindowStyleMaskMiniaturizable;
		CGRect frame = screen.frame;
		
		float scale = screen.backingScaleFactor;
		float width = appOptions.width > 0 ? appOptions.width: frame.size.width;
		float height = appOptions.height > 0 ? appOptions.height: frame.size.height;
		float x = appOptions.x > 0 ? appOptions.x: (frame.size.width - width) / 2.0;
		float y = appOptions.y > 0 ? appOptions.y: (frame.size.height - height) / 2.0;
		
		_window = [[UIWindow alloc] initWithContentRect:NSMakeRect(x, y, width, height)
																					styleMask:style
																						backing:NSBackingStoreBuffered
																							defer:NO
																						screen:nil];
		_is_background = YES;
		_is_pause = YES;
		_loaded = NO;
		
		self.window.delegate = self;
		self.window.backgroundColor = appOptions.background_color;
		self.window.title = [NSString stringWithFormat:@"%s", *appOptions.title];
		[self.window makeKeyAndOrderFront:nil];
		
		if (appOptions.x < 0 && appOptions.y < 0) {
			[self.window center];
		}
		
		UIView* rootView = self.window.contentView;

		self.view = renderApple->init(rootView.bounds);
		//[self.view scaleUnitSquareToSize:NSMakeSize(scale, scale)];
		// self.view.layer.contentsScale = scale;
		self.view.contentScaleFactor = scale;
		//self.view.translatesAutoresizingMaskIntoConstraints = NO;
		self.view.wantsBestResolutionOpenGLSurface = YES;

		//self.ime = [[OsxIMEHelprt alloc] initWithApplication:self.app];

		[rootView addSubview:self.view];
		// [view addSubview:self.ime];
		[rootView addConstraint:[NSLayoutConstraint
														constraintWithItem:self.view
														attribute:NSLayoutAttributeWidth
														relatedBy:NSLayoutRelationEqual
														toItem:view
														attribute:NSLayoutAttributeWidth
														multiplier:1
														constant:0]];
		[rootView addConstraint:[NSLayoutConstraint
														constraintWithItem:self.view
														attribute:NSLayoutAttributeHeight
														relatedBy:NSLayoutRelationEqual
														toItem:view
														attribute:NSLayoutAttributeHeight
														multiplier:1
														constant:0]];
		
		_app->display()->set_default_scale(UIScreen.mainScreen.scale);

		renderApple->resize(self.view.frame);

		_inl_app(_app)->triggerLoad();

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
		F_DEBUG("applicationDidHide, onBackground");
	}

	- (void)applicationWillUnhide:(NSNotification*)notification {
		F_DEBUG("applicationWillUnhide, onForeground");
	}

	- (void)applicationWillTerminate:(NSNotification*)notification {
		F_DEBUG("applicationWillTerminate");
		_inl_app(_app)->triggerUnload();
	}

	- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication*)sender {
		F_DEBUG("exit application");
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

Render* Render::Make(Application* host, const Options& opts) {
	appOptions = [[ApplicationOptions alloc] init:options];
	renderApple = RenderApple::Make(host, opts);
	return renderApple->render();
}

/**
 * @func pending() 挂起应用进程
 */
void Application::pending() {
	// exit(0);
}

/**
 * @func open_url()
 */
void Application::open_url(cString& url) {
	// TODO
}

/**
 * @func send_email
 */
void Application::send_email(cString& recipient,
																cString& subject,
																cString& cc, cString& bcc, cString& body)
{
	// TODO 
}

/**
 * @func ime_keyboard_open
 */
void AppInl::ime_keyboard_open(KeyboardOptions options) {
	// TODO
}

/**
 * @func ime_keyboard_can_backspace
 */
void AppInl::ime_keyboard_can_backspace(bool can_backspace, bool can_delete) {
	// TODO
}

/**
 * @func ime_keyboard_close
 */
void AppInl::ime_keyboard_close() {
	// TODO
}

/**
 * @func ime_keyboard_spot_location
 */
void AppInl::ime_keyboard_spot_location(Vec2 location) {
	// TODO...
}

/**
 * @func set_volume_up()
 */
void AppInl::set_volume_up() {
	// TODO ..
}

/**
 * @func set_volume_down()
 */
void AppInl::set_volume_down() {
	// TODO ..
}

// ***************** D i s p l a y *****************

/**
 * @func default_atom_pixel
 */
float Display::default_atom_pixel() {
	return 1.0 / UIScreen.mainScreen.backingScaleFactor;
}

/**
 * @func keep_screen(keep)
 */
void Display::keep_screen(bool keep) {
	// TODO
}

/**
 * @func status_bar_height()
 */
float Display::status_bar_height() {
	return 0;
}

/**
 * @func default_status_bar_height
 */
float Display::default_status_bar_height() {
	return 0;
}

/**
 * @func set_visible_status_bar(visible)
 */
void Display::set_visible_status_bar(bool visible) {
	// TODO
}

/**
 * @func set_status_bar_text_color(color)
 */
void Display::set_status_bar_style(StatusBarStyle style) {
	// TODO
}

/**
 * @func request_fullscreen(fullscreen)
 */
void Display::request_fullscreen(bool fullscreen) {
	// TODO
}

/**
 * @func orientation()
 */
Orientation Display::orientation() {
	return ORIENTATION_INVALID;
}

/**
 * @func set_orientation(orientation)
 */
void Display::set_orientation(Orientation orientation) {
	// noop
}

extern "C" F_EXPORT int main(int argc, Char* argv[]) {
	/**************************************************/
	/**************************************************/
	/*************** Start UI Application ************/
	/**************************************************/
	/**************************************************/
	AppInl::runMain(argc, argv);

	if ( app() ) {
		@autoreleasepool {
			[UIApplication sharedApplication];
			[NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
			if ( [appDelegateName isEqual:@""] ) {
				[UIApplication.sharedApplication setDelegate:[[ApplicationDelegate alloc] init]];
			} else {
				Class cls = NSClassFromString(appDelegateName);
				[UIApplication.sharedApplication setDelegate:[[cls alloc] init]];
			}
			[UIApplication.sharedApplication run];
		}
	}
	return 0;
}
