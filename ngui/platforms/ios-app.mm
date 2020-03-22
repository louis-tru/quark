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

#import <UIKit/UIKit.h>
#import <OpenGLES/ES2/glext.h>
#import "nxkit/loop.h"
#import "ios-gl-1.h"
#import "ios-ime-helper-1.h"
#import "mac-app.h"
#import "ngui/app.h"
#import "ngui/display-port.h"
#import "ngui/app-1.h"
#import "ngui/event.h"
#import <MessageUI/MFMailComposeViewController.h>

using namespace ngui;

typedef DisplayPort::Orientation Orientation;
typedef DisplayPort::StatusBarStyle StatusBarStyle;

static ApplicationDelegate* app_delegate = nil;
static GLDrawProxy* gl_draw_context = nil;
static NSString* app_delegate_name = @"";

/**
 * @interface GLView
 */
@interface GLView: UIView;
@property (assign, nonatomic) AppInl* app;
@end

/**
 * @interface ApplicationDelegate
 */
@interface RootViewController: UIViewController;
@end

/**
 * @interface ApplicationDelegate
 */
@interface ApplicationDelegate()<MFMailComposeViewControllerDelegate> {
	UIWindow* _window;
	BOOL      _is_background;
	Callback<>  _render_exec;
}
@property (strong, nonatomic) GLView* glview;
@property (strong, nonatomic) IOSIMEHelprt* ime;
@property (strong, nonatomic) CADisplayLink* display_link;
@property (strong, nonatomic) UIApplication* host;
@property (strong, nonatomic) RootViewController* root_ctr;
@property (assign, nonatomic) Orientation setting_orientation;
@property (assign, nonatomic) Orientation current_orientation;
@property (assign, nonatomic) bool visible_status_bar;
@property (assign, nonatomic) UIStatusBarStyle status_bar_style;
@property (assign, atomic) NSInteger render_task_count;
@end

/**
 * @implementation RootViewController
 * ***********************************************************************************
 */
@implementation RootViewController

- (BOOL)shouldAutorotate {
	return YES;
}

- (UIInterfaceOrientationMask)supportedInterfaceOrientations {
	switch ( app_delegate.setting_orientation ) {
		case Orientation::ORIENTATION_PORTRAIT:
			return UIInterfaceOrientationMaskPortrait;
		case Orientation::ORIENTATION_LANDSCAPE:
			return UIInterfaceOrientationMaskLandscapeRight;
		case Orientation::ORIENTATION_REVERSE_PORTRAIT:
			return UIInterfaceOrientationMaskPortraitUpsideDown;
		case Orientation::ORIENTATION_REVERSE_LANDSCAPE:
			return UIInterfaceOrientationMaskLandscapeLeft;
		case Orientation::ORIENTATION_USER: default:
			return UIInterfaceOrientationMaskAll;
		case Orientation::ORIENTATION_USER_PORTRAIT:
			return UIInterfaceOrientationMaskPortrait | UIInterfaceOrientationMaskPortraitUpsideDown;
		case Orientation::ORIENTATION_USER_LANDSCAPE:
			return UIInterfaceOrientationMaskLandscape;
		case Orientation::ORIENTATION_USER_LOCKED:
			switch(app_delegate.current_orientation  ) {
				default:
				case Orientation::ORIENTATION_INVALID:
					return UIInterfaceOrientationMaskAll;
				case Orientation::ORIENTATION_PORTRAIT:
					return UIInterfaceOrientationMaskPortrait;
				case Orientation::ORIENTATION_LANDSCAPE:
					return UIInterfaceOrientationMaskLandscapeRight;
				case Orientation::ORIENTATION_REVERSE_PORTRAIT:
					return UIInterfaceOrientationMaskPortraitUpsideDown;
				case Orientation::ORIENTATION_REVERSE_LANDSCAPE:
					return UIInterfaceOrientationMaskLandscapeLeft;
			}
	}
	return UIInterfaceOrientationMaskAll;
}

- (void)viewWillTransitionToSize:(CGSize)size
			 withTransitionCoordinator:(id<UIViewControllerTransitionCoordinator>)coordinator
{
	[coordinator animateAlongsideTransition:^(id context) {
		Orientation ori = display_port()->orientation();
		::CGRect rect = app_delegate.glview.frame;
		app_delegate.app->render_loop()->post(Cb([ori, rect](CbD& d) {
			gl_draw_context->refresh_surface_size(rect);
			if (ori != app_delegate.current_orientation) {
				app_delegate.current_orientation = ori;
				main_loop()->post(Cb([](CbD& e) {
					(app_delegate.app)->display_port()->NX_TRIGGER(orientation);
				}));
			}
		}));
	} completion:nil];
	[super viewWillTransitionToSize:size withTransitionCoordinator:coordinator];
}

- (UIStatusBarStyle)preferredStatusBarStyle {
	return app_delegate.status_bar_style;
}

- (UIStatusBarAnimation)preferredStatusBarUpdateAnimation {
	// UIApplicationWillChangeStatusBarFrameNotification
	return UIStatusBarAnimationSlide;
}

- (BOOL)prefersStatusBarHidden {
	return !app_delegate.visible_status_bar;
}

@end

/**
 * @implementation OGLView
 * ***********************************************************************************
 */
@implementation GLView

+ (Class)layerClass {
	return [CAEAGLLayer class];
}

- (BOOL)isMultipleTouchEnabled {
	return YES;
}

- (BOOL)isUserInteractionEnabled {
	return YES;
}

- (List<GUITouch>)toGUITouchs:(NSSet<UITouch*>*)touches {
	NSEnumerator* enumerator = [touches objectEnumerator];
	List<GUITouch> rv; // (uint(touches.count));
	
	Vec2 size = _app->display_port()->size();
	
	float scale_x = size.width() / app_delegate.glview.frame.size.width;
	float scale_y = size.height() / app_delegate.glview.frame.size.height;
	
	for (UITouch* touch in enumerator) {
		CGPoint point = [touch locationInView:touch.view];
		CGFloat force = touch.force;
		// CGFloat angle = touch.altitudeAngle;
		// CGFloat max_force = touch.maximumPossibleForce;
		rv.push({
			uint((size_t)touch % Uint::max),
			0, 0,
			float(point.x * scale_x),
			float(point.y * scale_y),
			float(force),
			false,
			nullptr,
		});
	}
	
	return rv;
}

- (void)touchesBegan:(NSSet<UITouch *> *)touches withEvent:(nullable UIEvent *)event {
	_app->dispatch()->dispatch_touchstart( [self toGUITouchs:touches] );
}

- (void)touchesMoved:(NSSet<UITouch *> *)touches withEvent:(nullable UIEvent *)event {
	// NX_DEBUG("touchesMoved, count: %d", touches.count);
	_app->dispatch()->dispatch_touchmove( [self toGUITouchs:touches] );
}

- (void)touchesEnded:(NSSet<UITouch *> *)touches withEvent:(nullable UIEvent *)event{
	_app->dispatch()->dispatch_touchend( [self toGUITouchs:touches] );
}

- (void)touchesCancelled:(NSSet<UITouch *> *)touches withEvent:(nullable UIEvent *)event {
	_app->dispatch()->dispatch_touchcancel( [self toGUITouchs:touches] );
}

@end

/**
 * @implementation ApplicationDelegate
 * ***********************************************************************************
 */
@implementation ApplicationDelegate

static void render_exec_func(CbD& evt, Object* ctx) {
	app_delegate.render_task_count--;
	_inl_app(app_delegate.app)->triggerRender();
}

- (void)display_link_callback:(CADisplayLink*)displayLink {
	if (self.render_task_count == 0) {
		self.render_task_count++;
		_app->render_loop()->post(_render_exec);
	} else {
		NX_DEBUG("miss frame");
	}
}

- (void)refresh_status {
	if ( self.window.rootViewController == self.root_ctr ) {
		self.window.rootViewController = nil;
		self.window.rootViewController = self.root_ctr;
	}
}

- (UIWindow*)window {
	return _window;
}

- (void)add_system_notification {
	// TODO ..
}

- (void)refresh_surface_size {
	::CGRect rect = app_delegate.glview.frame;
	_app->render_loop()->post(Cb([self, rect](CbD& d) {
		gl_draw_context->refresh_surface_size(rect);
	}));
}

+ (void)set_application_delegate:(NSString*)name {
	app_delegate_name = name;
}

- (BOOL)application:(UIApplication*)app didFinishLaunchingWithOptions:(NSDictionary*)options {
	ASSERT(!app_delegate); 
	app_delegate = self;
	
	//[app setStatusBarStyle:UIStatusBarStyleLightContent];
	//[app setStatusBarHidden:NO];
	
	_app = Inl_GUIApplication(GUIApplication::shared()); 
	ASSERT(self.app);
	_window = [[UIWindow alloc] initWithFrame:[[UIScreen mainScreen] bounds]];
	_is_background = NO;
	_render_exec = Cb(render_exec_func);
	
	self.host = app;
	self.setting_orientation = Orientation::ORIENTATION_USER;
	self.current_orientation = Orientation::ORIENTATION_INVALID;
	self.visible_status_bar = YES;
	self.status_bar_style = UIStatusBarStyleLightContent;
	self.render_task_count = 0;
	self.root_ctr = [[RootViewController alloc] init];
	self.display_link = [CADisplayLink displayLinkWithTarget:self
																									selector:@selector(display_link_callback:)];
	self.window.backgroundColor = [UIColor blackColor];
	self.window.rootViewController = self.root_ctr;
	
	[self.window makeKeyAndVisible];
	
	UIView* view = self.window.rootViewController.view;
	self.glview = [[GLView alloc] initWithFrame:[view bounds]];
	self.glview.contentScaleFactor = UIScreen.mainScreen.scale;
	self.glview.translatesAutoresizingMaskIntoConstraints = NO;
	self.glview.app = _inl_app(self.app);
	self.ime = [[IOSIMEHelprt alloc] initWithApplication:self.app];
	
	[view addSubview:self.glview];
	[view addSubview:self.ime];
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
	
	CAEAGLLayer* layer = (CAEAGLLayer*)self.glview.layer;
	::CGRect rect = self.glview.frame;
	
	layer.drawableProperties = [NSDictionary dictionaryWithObjectsAndKeys:
															[NSNumber numberWithBool: NO],
															kEAGLDrawablePropertyRetainedBacking,
															kEAGLColorFormatRGBA8,
															kEAGLDrawablePropertyColorFormat, nil];
	
	_app->render_loop()->post(Cb([self, layer, rect](CbD& d) {
		gl_draw_context->set_surface_view(self.glview, layer);
		gl_draw_context->refresh_surface_size(rect);
		_inl_app(self.app)->triggerLoad();
		[self.display_link addToRunLoop:[NSRunLoop mainRunLoop]
														forMode:NSDefaultRunLoopMode];
	}));
	
	return YES;
}

- (void)application:(UIApplication*)app didChangeStatusBarFrame:(::CGRect)frame {
	if ( app_delegate && !_is_background ) {
		[self refresh_surface_size];
	}
}

- (void)applicationWillResignActive:(UIApplication*) application {
	_inl_app(_app)->triggerPause();
}

- (void)applicationDidBecomeActive:(UIApplication*) application {
	_inl_app(_app)->triggerResume();
	[self refresh_surface_size];
}

- (void)applicationDidEnterBackground:(UIApplication*) application {
	_is_background = YES;
	_inl_app(_app)->triggerBackground();
}

- (void)applicationWillEnterForeground:(UIApplication*) application {
	_is_background = NO;
	_inl_app(_app)->triggerForeground();
}

- (void)applicationDidReceiveMemoryWarning:(UIApplication*) application {
	_inl_app(_app)->triggerMemorywarning();
}

- (void)applicationWillTerminate:(UIApplication*)application {
	[self.display_link removeFromRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
	_inl_app(_app)->triggerUnload();
}

- (void) mailComposeController:(MFMailComposeViewController*)controller
					 didFinishWithResult:(MFMailComposeResult)result
												 error:(NSError*)error {
	[controller dismissViewControllerAnimated:YES completion:nil];
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
	NSURL* url2 = [NSURL URLWithString:[NSString stringWithUTF8String:*url]];
	dispatch_async(dispatch_get_main_queue(), ^{
		[app_delegate.host openURL:url2 options:@{ } completionHandler:nil];
	});
}

/**
 * @func split_ns_array(str)
 */
static NSArray<NSString*>* split_ns_array(cString& str) {
	NSMutableArray<NSString*>* arr = [NSMutableArray<NSString*> new];
	for (auto& i : str.split(',')) {
		[arr addObject: [NSString stringWithUTF8String:*i.value()]];
	}
	return arr;
}

/**
 * @func send_email
 */
void GUIApplication::send_email(cString& recipient,
																cString& subject,
																cString& cc, cString& bcc, cString& body) {
	id recipient_ = split_ns_array(recipient);
	id subject_ = [NSString stringWithUTF8String:*subject];
	id cc_ = split_ns_array(cc);
	id bcc_ = split_ns_array(bcc);
	id body_ = [NSString stringWithUTF8String:*body];
	dispatch_async(dispatch_get_main_queue(), ^{
		MFMailComposeViewController* mail = [MFMailComposeViewController new];
		[mail setToRecipients:recipient_];
		[mail setSubject:subject_];
		[mail setCcRecipients:cc_];
		[mail setBccRecipients:bcc_];
		[mail setMessageBody:body_ isHTML:NO];
		mail.mailComposeDelegate = app_delegate;
			[app_delegate.root_ctr presentViewController:mail animated:YES completion:nil];
	});
}

/**
 * @func initialize(options)
 */
void AppInl::initialize(cJSON& options) {
	ASSERT(!gl_draw_context);
	gl_draw_context = GLDrawProxy::create(this, options);
	m_draw_ctx = gl_draw_context->host();
	ASSERT(m_draw_ctx);
}

/**
 * @func ime_keyboard_open
 */
void AppInl::ime_keyboard_open(KeyboardOptions options) {
	dispatch_async(dispatch_get_main_queue(), ^{
		[app_delegate.ime set_keyboard_type:options.type];
		[app_delegate.ime set_keyboard_return_type:options.return_type];
		if ( options.is_clear ) {
			[app_delegate.ime clear];
		}
		[app_delegate.ime open];
	});
}

/**
 * @func ime_keyboard_can_backspace
 */
void AppInl::ime_keyboard_can_backspace(bool can_backspace, bool can_delete) {
	dispatch_async(dispatch_get_main_queue(), ^{
		[app_delegate.ime set_keyboard_can_backspace:can_backspace can_delete:can_delete];
	});
}

/**
 * @func ime_keyboard_close
 */
void AppInl::ime_keyboard_close() {
	dispatch_async(dispatch_get_main_queue(), ^{
		[app_delegate.ime close];
	});
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
	return 1.0 / UIScreen.mainScreen.scale;
}

/**
 * @func keep_screen(keep)
 */
void DisplayPort::keep_screen(bool keep) {
	dispatch_async(dispatch_get_main_queue(), ^{
		if ( keep ) {
			app_delegate.host.idleTimerDisabled = YES;
		} else {
			app_delegate.host.idleTimerDisabled = NO;
		}
	});
}

/**
 * @func status_bar_height()
 */
float DisplayPort::status_bar_height() {
	::CGRect rect = app_delegate.host.statusBarFrame;
	return NX_MIN(rect.size.height, 20) * UIScreen.mainScreen.scale / m_scale_value[1];
}

/**
 * @func default_status_bar_height
 */
float DisplayPort::default_status_bar_height() {
	if (app_delegate && app_delegate.app) {
		return app_delegate.app->display_port()->status_bar_height();
	} else {
		return 20;
	}
}

/**
 * @func set_visible_status_bar(visible)
 */
void DisplayPort::set_visible_status_bar(bool visible) {
	
	if ( visible != app_delegate.visible_status_bar ) {
		app_delegate.visible_status_bar = visible;
		dispatch_async(dispatch_get_main_queue(), ^{
			//if ( visible ) {
			//  [app_delegate.host setStatusBarHidden:NO withAnimation:UIStatusBarAnimationSlide];
			//} else {
			//  [app_delegate.host setStatusBarHidden:YES withAnimation:UIStatusBarAnimationSlide];
			//}
			[app_delegate refresh_status];
			
			::CGRect rect = app_delegate.glview.frame;
			m_host->render_loop()->post(Cb([this, rect](CbD& ev) {
				if ( !gl_draw_context->refresh_surface_size(rect) ) {
					// 绘图表面尺寸没有改变，表示只是单纯状态栏改变，这个改变也当成change通知给用户
					main_loop()->post(Cb([this](CbD& e){
						NX_TRIGGER(change);
					}));
				}
			}), 16000); /* 延时16ms(一帧画面时间),给足够的时间让RootViewController重新刷新状态 */
		});
	}
}

/**
 * @func set_status_bar_text_color(color)
 */
void DisplayPort::set_status_bar_style(StatusBarStyle style) {
	UIStatusBarStyle style_2;
	if ( style == STATUS_BAR_STYLE_WHITE ) {
		style_2 = UIStatusBarStyleLightContent;
	} else {
		style_2 = UIStatusBarStyleDefault;
	}
	if ( app_delegate.status_bar_style != style_2 ) {
		app_delegate.status_bar_style = style_2;
		dispatch_async(dispatch_get_main_queue(), ^{
			//[app_delegate.host setStatusBarStyle:app_delegate.status_bar_style];
			[app_delegate refresh_status];
		});
	}
}

/**
 * @func request_fullscreen(fullscreen)
 */
void DisplayPort::request_fullscreen(bool fullscreen) {
	set_visible_status_bar(!fullscreen);
}

/**
 * @func orientation()
 */
Orientation DisplayPort::orientation() {
	Orientation r = ORIENTATION_INVALID;
	switch ( app_delegate.host.statusBarOrientation ) {
		case UIInterfaceOrientationPortrait:
			r = ORIENTATION_PORTRAIT;
			break;
		case UIInterfaceOrientationPortraitUpsideDown:
			r = ORIENTATION_REVERSE_PORTRAIT;
			break;
		case UIInterfaceOrientationLandscapeLeft:
			r = ORIENTATION_REVERSE_LANDSCAPE;
			break;
		case UIInterfaceOrientationLandscapeRight:
			r = ORIENTATION_LANDSCAPE;
			break;
		default:
			r = ORIENTATION_INVALID;
			break;
	}
	return r;
}

/**
 * @func set_orientation(orientation)
 */
void DisplayPort::set_orientation(Orientation orientation) {
	if ( app_delegate.setting_orientation != orientation ) {
		app_delegate.setting_orientation = orientation;
		dispatch_async(dispatch_get_main_queue(), ^{
			[app_delegate refresh_status];
		});
	}
}

extern "C" NX_EXPORT int main(int argc, char* argv[]) {
	/**************************************************/
	/**************************************************/
	/*************** Start GUI Application ************/
	/**************************************************/
	/**************************************************/
	AppInl::runMain(argc, argv);
	
	if ( app() ) {
		@autoreleasepool {
			if ( [app_delegate_name isEqual:@""] ) {
				UIApplicationMain(argc, argv, nil, NSStringFromClass(ApplicationDelegate.class));
			} else {
				UIApplicationMain(argc, argv, nil, app_delegate_name);
			}
		}
	}
	return 0;
}
