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
#import <MessageUI/MFMailComposeViewController.h>

typedef UIEvent AppleUIEvent;

#import "../../util/loop.h"
#import "../../app.inl"
#import "../../display.h"
#import "../../event.h"
#import "../apple/apple-render.h"
#import "./ios-ime-helper.h"

using namespace flare;

typedef Display::Orientation Orientation;
typedef Display::StatusBarStyle StatusBarStyle;

static ApplicationDelegate* G_AppDelegate = nil;
static RenderApple* G_render = nil;
static NSString* G_AppDelegate_name = @"";

/**
 * @interface MainView
 */
@interface MainView: UIView;
	@property (assign, nonatomic) AppInl* app;
@end

/**
 * @interface RootViewController
 */
@interface RootViewController: UIViewController;
@end

/**
 * @interface ApplicationDelegate
 */
@interface ApplicationDelegate()<MFMailComposeViewControllerDelegate>
	{
		UIWindow* _window;
		BOOL _is_background;
		Cb  _render_exec;
	}
	@property (strong, nonatomic) MainView* view;
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

@implementation MainView

	+ (Class)layerClass {
		return G_render->layerClass();
	}

	- (BOOL)isMultipleTouchEnabled {
		return YES;
	}

	- (BOOL)isUserInteractionEnabled {
		return YES;
	}

	- (List<TouchPoint>)toUITouchs:(NSSet<UITouch*>*)touches {
		NSEnumerator* enumerator = [touches objectEnumerator];
		List<TouchPoint> rv; // (uint(touches.count));
		
		Vec2 size = _app->display()->size();
		
		float scale_x = size.width() / G_AppDelegate.view.frame.size.width;
		float scale_y = size.height() / G_AppDelegate.view.frame.size.height;
		
		for (UITouch* touch in enumerator) {
			CGPoint point = [touch locationInView:touch.view];
			CGFloat force = touch.force;
			// CGFloat angle = touch.altitudeAngle;
			// CGFloat max_force = touch.maximumPossibleForce;
			rv.push_back({
				uint32_t((size_t)touch % Uint32::limit_max),
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

	- (void)touchesBegan:(NSSet<UITouch *> *)touches withEvent:(nullable AppleUIEvent *)event {
		_app->dispatch()->onTouchstart( [self toUITouchs:touches] );
	}

	- (void)touchesMoved:(NSSet<UITouch *> *)touches withEvent:(nullable AppleUIEvent *)event {
		// F_DEBUG(APP, "touchesMoved, count: %d", touches.count);
		_app->dispatch()->onTouchmove( [self toUITouchs:touches] );
	}

	- (void)touchesEnded:(NSSet<UITouch *> *)touches withEvent:(nullable AppleUIEvent *)event{
		_app->dispatch()->onTouchend( [self toUITouchs:touches] );
	}

	- (void)touchesCancelled:(NSSet<UITouch *> *)touches withEvent:(nullable AppleUIEvent *)event {
		_app->dispatch()->onTouchcancel( [self toUITouchs:touches] );
	}

@end

@implementation RootViewController

	- (BOOL)shouldAutorotate {
		return YES;
	}

	- (UIInterfaceOrientationMask)supportedInterfaceOrientations {
		switch ( G_AppDelegate.setting_orientation ) {
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
				switch(G_AppDelegate.current_orientation  ) {
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
			Orientation ori = G_AppDelegate.app->display()->orientation();
			::CGRect rect = G_AppDelegate.view.frame;
			G_AppDelegate.app->render_loop()->post(Cb([ori, rect](CbData& d) {
				G_render->resize(rect);
				if (ori != G_AppDelegate.current_orientation) {
					G_AppDelegate.current_orientation = ori;
					G_AppDelegate.app->main_loop()->post(Cb([](CbData& e) {
						G_AppDelegate.app->display()->F_Trigger(Orientation);
					}));
				}
			}));
		} completion:nil];
		[super viewWillTransitionToSize:size withTransitionCoordinator:coordinator];
	}

	- (UIStatusBarStyle)preferredStatusBarStyle {
		return G_AppDelegate.status_bar_style;
	}

	- (UIStatusBarAnimation)preferredStatusBarUpdateAnimation {
		// UIApplicationWillChangeStatusBarFrameNotification
		return UIStatusBarAnimationSlide;
	}

	- (BOOL)prefersStatusBarHidden {
		return !G_AppDelegate.visible_status_bar;
	}

@end

@implementation ApplicationDelegate

	static void render_exec_func(CbData& evt, Object* ctx) {
		G_AppDelegate.render_task_count--;
		_inl_app(G_AppDelegate.app)->triggerRender();
	}

	- (void)display_link_callback:(CADisplayLink*)displayLink {
		if (self.render_task_count == 0) {
			self.render_task_count++;
			_app->render_loop()->post(_render_exec);
		} else {
			F_DEBUG(APP, "miss frame");
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

	- (void)resize {
		::CGRect rect = G_AppDelegate.view.frame;
		_app->render_loop()->post(Cb([self, rect](CbData& d) {
			G_render->resize(rect);
		}));
	}

	+ (void)set_application_delegate:(NSString*)name {
		G_AppDelegate_name = name;
	}

	- (BOOL)application:(UIApplication*)app didFinishLaunchingWithOptions:(NSDictionary*)options {
		F_ASSERT(!G_AppDelegate); 
		G_AppDelegate = self;
		
		//[app setStatusBarStyle:UIStatusBarStyleLightContent];
		//[app setStatusBarHidden:NO];
		
		_app = Inl_Application(Application::shared()); 
		F_ASSERT(self.app);
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
		
		UIView* root_view = self.window.rootViewController.view;
		self.view = [[MainView alloc] initWithFrame:[root_view bounds]];
		self.view.contentScaleFactor = UIScreen.mainScreen.scale;
		self.view.translatesAutoresizingMaskIntoConstraints = NO;
		self.view.app = _inl_app(self.app);
		self.ime = [[IOSIMEHelprt alloc] initWithApplication:self.app];
		
		[root_view addSubview:self.view];
		[root_view addSubview:self.ime];
		[root_view addConstraint:[NSLayoutConstraint
												constraintWithItem:self.view
												attribute:NSLayoutAttributeWidth
												relatedBy:NSLayoutRelationEqual
												toItem:root_view
												attribute:NSLayoutAttributeWidth
												multiplier:1
												constant:0]];
		[root_view addConstraint:[NSLayoutConstraint
												constraintWithItem:self.view
												attribute:NSLayoutAttributeHeight
												relatedBy:NSLayoutRelationEqual
												toItem:root_view
												attribute:NSLayoutAttributeHeight
												multiplier:1
												constant:0]];
		
		[self add_system_notification];
		
		::CGRect rect = self.view.frame;
		
		_app->render_loop()->post(Cb([self, rect](CbData& d) {
			G_render->setView(self.view);
			G_render->resize(rect);
			_inl_app(self.app)->triggerLoad();
			[self.display_link addToRunLoop:[NSRunLoop mainRunLoop]
															forMode:NSDefaultRunLoopMode];
		}));

		return YES;
	}

	- (void)application:(UIApplication*)app didChangeStatusBarFrame:(::CGRect)frame {
		if ( G_AppDelegate && !_is_background ) {
			[self resize];
		}
	}

	- (void)applicationWillResignActive:(UIApplication*) application {
		_inl_app(_app)->triggerPause();
	}

	- (void)applicationDidBecomeActive:(UIApplication*) application {
		_inl_app(_app)->triggerResume();
		[self resize];
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

// ******************************* Application *******************************

Render* Render::create(Application* host, cJSON& options) {
	G_render = RenderApple::create(host, options);
	return G_render->render();
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
	NSURL* url2 = [NSURL URLWithString:[NSString stringWithUTF8String:*url]];
	dispatch_async(dispatch_get_main_queue(), ^{
		[G_AppDelegate.host openURL:url2 options:@{} completionHandler:nil];
	});
}

static NSArray<NSString*>* split_ns_array(cString& str) {
	NSMutableArray<NSString*>* arr = [NSMutableArray<NSString*> new];
	for (auto& i : str.split(',')) {
		[arr addObject: [NSString stringWithUTF8String:i.c_str()]];
	}
	return arr;
}

/**
 * @func send_email()
 */
void Application::send_email(cString& recipient,
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
		mail.mailComposeDelegate = G_AppDelegate;
			[G_AppDelegate.root_ctr presentViewController:mail animated:YES completion:nil];
	});
}

/**
 * @func ime_keyboard_open
 */
void AppInl::ime_keyboard_open(KeyboardOptions options) {
	dispatch_async(dispatch_get_main_queue(), ^{
		[G_AppDelegate.ime set_keyboard_type:options.type];
		[G_AppDelegate.ime set_keyboard_return_type:options.return_type];
		if ( options.is_clear ) {
			[G_AppDelegate.ime clear];
		}
		[G_AppDelegate.ime open];
	});
}

/**
 * @func ime_keyboard_can_backspace
 */
void AppInl::ime_keyboard_can_backspace(bool can_backspace, bool can_delete) {
	dispatch_async(dispatch_get_main_queue(), ^{
		[G_AppDelegate.ime set_keyboard_can_backspace:can_backspace can_delete:can_delete];
	});
}

/**
 * @func ime_keyboard_close
 */
void AppInl::ime_keyboard_close() {
	dispatch_async(dispatch_get_main_queue(), ^{
		[G_AppDelegate.ime close];
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

// ******************************* Display *******************************

/**
 * @func default_atom_pixel
 */
float Display::default_atom_pixel() {
	return 1.0 / UIScreen.mainScreen.scale;
}

/**
 * @func keep_screen(keep)
 */
void Display::keep_screen(bool keep) {
	dispatch_async(dispatch_get_main_queue(), ^{
		if ( keep ) {
			G_AppDelegate.host.idleTimerDisabled = YES;
		} else {
			G_AppDelegate.host.idleTimerDisabled = NO;
		}
	});
}

/**
 * @func status_bar_height()
 */
float Display::status_bar_height() {
	::CGRect rect = G_AppDelegate.host.statusBarFrame;
	return F_MIN(rect.size.height, 20) * UIScreen.mainScreen.scale / _scale[1];
}

/**
 * @func default_status_bar_height
 */
float Display::default_status_bar_height() {
	if (G_AppDelegate && G_AppDelegate.app) {
		return G_AppDelegate.app->display()->status_bar_height();
	} else {
		return 20;
	}
}

/**
 * @func set_visible_status_bar(visible)
 */
void Display::set_visible_status_bar(bool visible) {
	
	if ( visible != G_AppDelegate.visible_status_bar ) {
		G_AppDelegate.visible_status_bar = visible;
		dispatch_async(dispatch_get_main_queue(), ^{
			//if ( visible ) {
			//  [G_AppDelegate.host setStatusBarHidden:NO withAnimation:UIStatusBarAnimationSlide];
			//} else {
			//  [G_AppDelegate.host setStatusBarHidden:YES withAnimation:UIStatusBarAnimationSlide];
			//}
			[G_AppDelegate refresh_status];
			
			::CGRect rect = G_AppDelegate.view.frame;
			_host->render_loop()->post(Cb([this, rect](CbData& ev) {

				if ( !G_render->resize(rect) ) {
					// 绘图表面尺寸没有改变，表示只是单纯状态栏改变，这个改变也当成change通知给用户
					_host->main_loop()->post(Cb([this](CbData& e) {
						F_Trigger(Change);
					}));
				}
			}), 16000); /* 延时16ms(一帧画面时间),给足够的时间让RootViewController重新刷新状态 */
		});
	}
}

/**
 * @func set_status_bar_text_color(color)
 */
void Display::set_status_bar_style(StatusBarStyle style) {
	UIStatusBarStyle style_2;
	if ( style == STATUS_BAR_STYLE_WHITE ) {
		style_2 = UIStatusBarStyleLightContent;
	} else {
		style_2 = UIStatusBarStyleDefault;
	}
	if ( G_AppDelegate.status_bar_style != style_2 ) {
		G_AppDelegate.status_bar_style = style_2;
		dispatch_async(dispatch_get_main_queue(), ^{
			//[G_AppDelegate.host setStatusBarStyle:G_AppDelegate.status_bar_style];
			[G_AppDelegate refresh_status];
		});
	}
}

/**
 * @func request_fullscreen(fullscreen)
 */
void Display::request_fullscreen(bool fullscreen) {
	set_visible_status_bar(!fullscreen);
}

/**
 * @func orientation()
 */
Orientation Display::orientation() {
	Orientation r = ORIENTATION_INVALID;
	switch ( G_AppDelegate.host.statusBarOrientation ) {
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
void Display::set_orientation(Orientation orientation) {
	if ( G_AppDelegate.setting_orientation != orientation ) {
		G_AppDelegate.setting_orientation = orientation;
		dispatch_async(dispatch_get_main_queue(), ^{
			[G_AppDelegate refresh_status];
		});
	}
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
			if ( [G_AppDelegate_name isEqual:@""] ) {
				UIApplicationMain(argc, argv, nil, NSStringFromClass(ApplicationDelegate.class));
			} else {
				UIApplicationMain(argc, argv, nil, G_AppDelegate_name);
			}
		}
	}
	return 0;
}
