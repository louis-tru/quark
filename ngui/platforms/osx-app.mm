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

// ******************** U n r e a l i z e d ********************

#import <AppKit/AppKit.h>
#import "nutils/loop.h"
#import "mac-app.h"
#import "osx-gl-1.h"
#import "ngui/app.h"
#import "ngui/display-port.h"
#import "ngui/app-1.h"
#import "ngui/event.h"

using namespace ngui;

typedef DisplayPort::Orientation Orientation;
typedef DisplayPort::StatusBarStyle StatusBarStyle;

@class ApplicationOptions;
@class MacIMEHelprt;

static ApplicationOptions* app_options = nil;
static ApplicationDelegate* app_delegate = nil;
static GLDrawProxy* gl_draw_context = nil;
static NSString* app_delegate_name = @"";

#define UIApplication NSApplication
#define UIView NSView
#define UIColor NSColor
#define UIScreen NSScreen

/**
 * @interface GLView
 */
@interface GLView: NSOpenGLView {
	CVDisplayLinkRef _display_link;
}
@property (assign, nonatomic) AppInl* app;
@end

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

/**
 * @interface ApplicationDelegate
 */
@interface ApplicationDelegate()<NSWindowDelegate> {
	UIWindow* _window;
	BOOL      _is_background;
	BOOL      _is_pause;
	BOOL      _loaded;
	Callback  _render_exec;
}
@property (strong, nonatomic) GLView* glview;
@property (strong, nonatomic) MacIMEHelprt* ime;
@property (strong, nonatomic) UIApplication* host;
@property (assign, atomic) NSInteger render_task_count;
- (void)display_link_callback:(const CVTimeStamp*)outputTime;
- (void)initialize;
@end

//* ***********************************************************************************

/**
 * @implementation GLView
 */
@implementation GLView

static CVReturn display_link_callback(CVDisplayLinkRef displayLink,
																			const CVTimeStamp* now,
																			const CVTimeStamp* outputTime,
																			CVOptionFlags flagsIn,
																			CVOptionFlags* flagsOut,
																			void* displayLinkContext)
{
	[app_delegate display_link_callback:outputTime];
	return kCVReturnSuccess;
}

- (NSOpenGLPixelFormat*)createPixelFormat {
	NSOpenGLPixelFormatAttribute attribs[] =
	{
		NSOpenGLPFANoRecovery,
		NSOpenGLPFAAccelerated,
		NSOpenGLPFADoubleBuffer,
		NSOpenGLPFAColorSize, 32,
		NSOpenGLPFAAlphaSize, 8,
		NSOpenGLPFAScreenMask,
		CGDisplayIDToOpenGLDisplayMask(kCGDirectMainDisplay),
		0
	};
	return [[NSOpenGLPixelFormat alloc] initWithAttributes:attribs];
}

- (void)prepareOpenGL {
	
	// Synchronize buffer swaps with vertical refresh rate
	GLint swapInt = 1;
	[self.openGLContext setValues:&swapInt forParameter:NSOpenGLCPSwapInterval];

	// Create a display link capable of being used with all active displays
	CVDisplayLinkCreateWithActiveCGDisplays(&_display_link);
	
	// Set the renderer output callback function
	CVDisplayLinkSetOutputCallback(_display_link, &display_link_callback, (__bridge void*)self);

	// Set the display link for the current renderer
	CGLContextObj cglContext = [self.openGLContext CGLContextObj];
	CGLPixelFormatObj cglPixelFormat = (__bridge CGLPixelFormatObj)([self createPixelFormat]);
	CVDisplayLinkSetCurrentCGDisplayFromOpenGLContext(_display_link, cglContext, cglPixelFormat);
	
	// init
	[app_delegate initialize];
	
	// Activate the display link
	CVDisplayLinkStart(_display_link);
}

- (void)dealloc {
	// Release the display link
	CVDisplayLinkRelease(_display_link);
}

@end

/**
 * @implementation ApplicationOptions
 */

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
	
	if (o_w.is_uint()) _width = XX_MAX(1, o_w.to_uint());
	if (o_h.is_uint()) _height = XX_MAX(1, o_h.to_uint());
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
 * @implementation ApplicationDelegate
 */
@implementation ApplicationDelegate

static void render_exec_func(Cb& evt, Object* ctx) {
	app_delegate.render_task_count--;
	_inl_app(app_delegate.app)->onRender();
}

- (void)display_link_callback:(const CVTimeStamp*)outputTime {
	if (self.render_task_count == 0) {
		self.render_task_count++;
		_app->render_loop()->post(_render_exec);
	} else {
		 XX_DEBUG("miss frame");
	}
}

- (UIWindow*)window {
	return _window;
}

- (void)refresh_surface_size_with_rect:(::CGRect)rect {
	if (!_loaded) return;
	_app->render_loop()->post(Cb([self, rect](Cb& d) {
		gl_draw_context->refresh_surface_size(rect);
	}));
	LOG("refresh_surface_size, %f, %f", rect.size.width, rect.size.height);
}

- (void)refresh_surface_size {
	[self refresh_surface_size_with_rect:app_delegate.glview.frame];
}

+ (void)set_application_delegate:(NSString*)name {
	app_delegate_name = name;
}

- (void)background {
	if (_loaded && !_is_background) {
		[self pause];
		_is_background = YES;
		XX_DEBUG("onBackground");
		_inl_app(_app)->onBackground();
	}
}

- (void)foreground {
	if (_loaded && _is_background) {
		_is_background = NO;
		XX_DEBUG("onForeground");
		_inl_app(_app)->onForeground();
		[self resume];
	}
}

- (void)pause {
	if (_loaded && !_is_pause) {
		XX_DEBUG("onPause");
		_is_pause = YES;
		_inl_app(_app)->onPause();
	}
}

- (void)resume {
	if (_loaded && _is_pause) {
		XX_DEBUG("onResume");
		_is_pause = NO;
		_inl_app(_app)->onResume();
		[self refresh_surface_size];
	}
}

- (void)add_system_notification {
	// TODO ..
}

- (void)initialize {
	NSOpenGLContext* context = self.glview.openGLContext;
	::CGRect rect = self.glview.frame;
	_app->render_loop()->post_sync(Cb([self, rect, context](Cb& d) {
		gl_draw_context->initialize(self.glview, context);
		gl_draw_context->refresh_surface_size(rect);
		_inl_app(_app)->onLoad();
		_loaded = YES;
	}));
	[self foreground];
}

- (void)applicationDidFinishLaunching:(NSNotification*) notification {
	UIApplication* app = UIApplication.sharedApplication;
	XX_ASSERT(!app_delegate);
	app_delegate = self;
	_app = Inl_GUIApplication(GUIApplication::shared());
	XX_ASSERT(_app);
	
	UIScreen* screen = UIScreen.mainScreen;
	NSWindowStyleMask style = NSWindowStyleMaskBorderless |
		NSWindowStyleMaskTitled | NSWindowStyleMaskClosable |
		NSWindowStyleMaskResizable | NSWindowStyleMaskMiniaturizable;
	::CGRect sframe = screen.frame;
	
	float scale = screen.backingScaleFactor;
	float width = app_options.width > 0 ? app_options.width: sframe.size.width;
	float height = app_options.height > 0 ? app_options.height: sframe.size.height;
	float x = app_options.x > 0 ? app_options.x: (sframe.size.width - width) / 2.0;
	float y = app_options.y > 0 ? app_options.y: (sframe.size.height - height) / 2.0;
	
	_window = [[UIWindow alloc] initWithContentRect:NSMakeRect(x, y, width, height)
																				styleMask:style
																					backing:NSBackingStoreBuffered
																						defer:NO
																					 screen:nil];
	_is_background = YES;
	_is_pause = YES;
	_loaded = NO;
	_render_exec = Cb(render_exec_func);
	
	self.host = app;
	self.render_task_count = 0;
	
	self.window.delegate = self;
	self.window.backgroundColor = app_options.background_color;
	self.window.title = [NSString stringWithFormat:@"%s", *app_options.title];
	[self.window makeKeyAndOrderFront:nil];
	
	if (app_options.x < 0 && app_options.y < 0) {
		[self.window center];
	}
	
	UIView* view = self.window.contentView;
	self.glview = [[GLView alloc] initWithFrame:[view bounds]];
	//[self.glview scaleUnitSquareToSize:NSMakeSize(scale, scale)];
	self.glview.layer.contentsScale = scale;
	self.glview.translatesAutoresizingMaskIntoConstraints = NO;
	self.glview.wantsBestResolutionOpenGLSurface = YES;
	self.glview.app = _inl_app(self.app);
	//self.ime = [[IOSIMEHelprt alloc] initWithApplication:self.app];
	
	[view addSubview:self.glview];
	// [view addSubview:self.ime];
	[view addConstraint:[NSLayoutConstraint
											 constraintWithItem:self.glview
											 attribute:NSLayoutAttributeWidth
											 relatedBy:NSLayoutRelationEqual
											 toItem:view
											 attribute:NSLayoutAttributeWidth
											 multiplier:1
											 constant:0]];
	[view addConstraint:[NSLayoutConstraint
											 constraintWithItem:self.glview
											 attribute:NSLayoutAttributeHeight
											 relatedBy:NSLayoutRelationEqual
											 toItem:view
											 attribute:NSLayoutAttributeHeight
											 multiplier:1
											 constant:0]];
	
	[self add_system_notification];
}

- (void)applicationWillResignActive:(NSNotification*)notification {
	[self pause];
}

- (void)applicationDidBecomeActive:(NSNotification*)notification {
	[self resume];
}

- (void)applicationDidHide:(NSNotification*)notification {
	LOG("applicationDidHide, onBackground");
}

- (void)applicationWillUnhide:(NSNotification*)notification {
	LOG("applicationWillUnhide, onForeground");
}

- (void)applicationWillTerminate:(NSNotification*)notification {
	XX_DEBUG("applicationWillTerminate");
	_inl_app(_app)->onUnload();
}

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication*)sender {
	XX_DEBUG("exit application");
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
	[self refresh_surface_size_with_rect: NSMakeRect(0, 0, size.width, size.height)];
	return size;
}

- (void)windowDidMiniaturize:(NSNotification*)notification {
	[self background];
}

- (void)windowDidDeminiaturize:(NSNotification*)notification {
	[self foreground];
}

@end

// ******************************* GUIApplication *******************************

/**
 * @func pending() 挂起应用进程
 */
void GUIApplication::pending() {
	// exit(0);
}

/**
 * @func open_url()
 */
void GUIApplication::open_url(cString& url) {
}

/**
 * @func send_email
 */
void GUIApplication::send_email(cString& recipient,
																cString& subject,
																cString& cc, cString& bcc, cString& body) {
}

/**
 * @func initialize(options)
 */
void AppInl::initialize(cJSON& options) {
	XX_ASSERT(!app_options);
	app_options = [[ApplicationOptions alloc] init:options];
	XX_ASSERT(!gl_draw_context);
	gl_draw_context = GLDrawProxy::create(this, options);
	m_draw_ctx = gl_draw_context->host();
}

/**
 * @func ime_keyboard_open
 */
void AppInl::ime_keyboard_open(KeyboardOptions options) {
}

/**
 * @func ime_keyboard_can_backspace
 */
void AppInl::ime_keyboard_can_backspace(bool can_backspace, bool can_delete) {
}

/**
 * @func ime_keyboard_close
 */
void AppInl::ime_keyboard_close() {
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

// ******************************* DisplayPort *******************************

/**
 * @func default_atom_pixel
 */
float DisplayPort::default_atom_pixel() {
	return 1.0 / UIScreen.mainScreen.backingScaleFactor;
}

/**
 * @func keep_screen(keep)
 */
void DisplayPort::keep_screen(bool keep) {
}

/**
 * @func status_bar_height()
 */
float DisplayPort::status_bar_height() {
	return 0;
}

/**
 * @func default_status_bar_height
 */
float DisplayPort::default_status_bar_height() {
	return 0;
}

/**
 * @func set_visible_status_bar(visible)
 */
void DisplayPort::set_visible_status_bar(bool visible) {
}

/**
 * @func set_status_bar_text_color(color)
 */
void DisplayPort::set_status_bar_style(StatusBarStyle style) {
}

/**
 * @func request_fullscreen(fullscreen)
 */
void DisplayPort::request_fullscreen(bool fullscreen) {
}

/**
 * @func orientation()
 */
Orientation DisplayPort::orientation() {
	return ORIENTATION_INVALID;
}

/**
 * @func set_orientation(orientation)
 */
void DisplayPort::set_orientation(Orientation orientation) {
}

extern "C" XX_EXPORT int main(int argc, char* argv[]) {
	/**************************************************/
	/**************************************************/
	/*************** Start GUI Application ************/
	/**************************************************/
	/**************************************************/
	AppInl::runMain(argc, argv);
	
	@autoreleasepool {
		[UIApplication sharedApplication];
		[NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
		if ( [app_delegate_name isEqual:@""] ) {
			[UIApplication.sharedApplication setDelegate:[[ApplicationDelegate alloc] init]];
		} else {
			Class cls = NSClassFromString(app_delegate_name);
			[UIApplication.sharedApplication setDelegate:[[cls alloc] init]];
		}
		[UIApplication.sharedApplication run];
	}
	return 0;
}
