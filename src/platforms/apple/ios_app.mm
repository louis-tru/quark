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

#import <UIKit/UIKit.h>
#import <MessageUI/MFMailComposeViewController.h>
#import <MetalKit/MTKView.h>

typedef UIEvent AppleUIEvent;

#import "../../util/loop.h"
#import "../../app.inl"
#import "../../display.h"
#import "../../event.h"
#import "./apple_render.h"
#import "./ios_ime_helper.h"

using namespace quark;

typedef Display::Orientation Orientation;
typedef Display::StatusBarStyle StatusBarStyle;

static ApplicationDelegate* appDelegate = nil;
static RenderApple* renderApple = nil;

@interface ApplicationDelegate()<MFMailComposeViewControllerDelegate>
	{
		BOOL _is_background;
		int  _fps;
		Cb   _render_exec;
	}
	@property (strong, nonatomic) UIView* view;
	@property (strong, nonatomic) IOSIMEHelprt* ime;
	@property (strong, nonatomic) CADisplayLink* display_link;
	@property (strong, nonatomic) UIApplication* host;
	@property (strong, nonatomic) RootViewController* root_ctr;
	@property (strong, nonatomic) UIWindow *window;
	@property (assign, nonatomic) Orientation setting_orientation;
	@property (assign, nonatomic) Orientation current_orientation;
	@property (assign, nonatomic) bool visible_status_bar;
	@property (assign, nonatomic) UIStatusBarStyle status_bar_style;
	@property (assign, atomic) NSInteger render_task_count;
@end

@interface RootViewController()
	@property (weak, nonatomic) ApplicationDelegate* appSelf;
@end

@implementation RootViewController

	-(BOOL)shouldAutorotate {
		return YES;
	}

	-(UIInterfaceOrientationMask)supportedInterfaceOrientations {
		switch ( self.appSelf.setting_orientation ) {
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
			case Orientation::ORIENTATION_USER_LOCKED: {
				switch (self.appSelf.current_orientation) {
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
		}
		return UIInterfaceOrientationMaskAll;
	}

	-(void)viewWillTransitionToSize:(CGSize)size
				withTransitionCoordinator:(id<UIViewControllerTransitionCoordinator>)coordinator
	{
		[coordinator animateAlongsideTransition:^(id context) {
			renderApple->resize(self.appSelf.view.frame);
			Orientation orient = self.appSelf.app->display()->orientation();
			if (orient != self.appSelf.current_orientation) {
				self.appSelf.current_orientation = orient;
				self.appSelf.app->loop()->post(Cb([](Cb::Data& e) {
					appDelegate.app->display()->Qk_Trigger(Orientation);
				}));
			}
		} completion:nil];
		[super viewWillTransitionToSize:size withTransitionCoordinator:coordinator];
	}

	-(UIStatusBarStyle)preferredStatusBarStyle {
		return self.appSelf.status_bar_style;
	}

	-(UIStatusBarAnimation)preferredStatusBarUpdateAnimation {
		// UIApplicationWillChangeStatusBarFrameNotification
		return UIStatusBarAnimationSlide;
	}

	-(BOOL)prefersStatusBarHidden {
		return !self.appSelf.visible_status_bar;
	}

	-(List<TouchPoint>)touchsList:(NSSet<UITouch*>*)touches {
		List<TouchPoint> rv;

		Vec2 size = self.appSelf.app->display()->size();

		float scale_x = size.x() /  self.appSelf.view.frame.size.width;
		float scale_y = size.y() /  self.appSelf.view.frame.size.height;

		for (UITouch* touch in [touches objectEnumerator]) {
			CGPoint point = [touch locationInView:touch.view];
			CGFloat force = touch.force;
			// CGFloat angle = touch.altitudeAngle;
			// CGFloat max_force = touch.maximumPossibleForce;
			rv.push_back({
				uint32_t((size_t)touch % Uint32::limit_max), 0, 0,
				float(point.x * scale_x), float(point.y * scale_y),
				float(force), false, nullptr,
			});
		}

		return rv;
	}

	-(void)touchesBegan:(NSSet<UITouch *> *)touches withEvent:(nullable AppleUIEvent *)event {
		_inl_app(self.appSelf.app)->dispatch()->onTouchstart( [self touchsList:touches] );
	}

	-(void)touchesMoved:(NSSet<UITouch *> *)touches withEvent:(nullable AppleUIEvent *)event {
		// Qk_DEBUG("touchesMoved, count: %d", touches.count);
		_inl_app(self.appSelf.app)->dispatch()->onTouchmove( [self touchsList:touches] );
	}

	-(void)touchesEnded:(NSSet<UITouch *> *)touches withEvent:(nullable AppleUIEvent *)event{
		_inl_app(self.appSelf.app)->dispatch()->onTouchend( [self touchsList:touches] );
	}

	-(void)touchesCancelled:(NSSet<UITouch *> *)touches withEvent:(nullable AppleUIEvent *)event {
		_inl_app(self.appSelf.app)->dispatch()->onTouchcancel( [self touchsList:touches] );
	}

@end

@implementation ApplicationDelegate

	static void render_exec_func(Cb::Data& evt, Object* ctx) {
		appDelegate.render_task_count--;
		appDelegate.app->display()->render();
	}

	- (void)display_link_callback:(CADisplayLink*)displayLink {
		auto _ = self.app;
#if Qk_USE_DEFAULT_THREAD_RENDER
		if (_->is_loaded()) {
			if (_fps == 0) { // 3 = 15, 1 = 30
				_->display()->render();
				_fps = 0;
			} else {
				_fps++;
			}
		}
#else
		if (self.render_task_count == 0) {
			self.render_task_count++;
			_->loop()->post(_render_exec);
		} else {
			Qk_DEBUG("miss frame");
		}
#endif
	}

	- (void)refresh_status {
		if ( self.window.rootViewController == self.root_ctr ) {
			self.window.rootViewController = nil;
			self.window.rootViewController = self.root_ctr;
		}
	}

	- (RootViewController*)root_ctr {
		 if (!_root_ctr) {
			 self.root_ctr = [[RootViewController alloc] init];
		 }
		 return _root_ctr;
	 }

	- (UIWindow*)window {
		 if (!_window) {
			 self.window = [[UIWindow alloc] initWithFrame:[[UIScreen mainScreen] bounds]];
		 }
		 return _window;
	 }

	- (void)resize {
		renderApple->resize(appDelegate.view.frame);
	}

	- (BOOL)application:(UIApplication*)app didFinishLaunchingWithOptions:(NSDictionary*)options {
		Qk_ASSERT(!appDelegate);
		appDelegate = self;
		Qk_ASSERT(Application::shared());
		_app = Application::shared(); 

		//[app setStatusBarStyle:UIStatusBarStyleLightContent];
		//[app setStatusBarHidden:NO];

		_is_background = NO;
		_render_exec = Cb(render_exec_func);
		
		self.host = app;
		self.render_task_count = 0;
		self.setting_orientation = Orientation::ORIENTATION_USER;
		self.current_orientation = Orientation::ORIENTATION_INVALID;
		self.visible_status_bar = YES;
		self.status_bar_style = UIStatusBarStyleDefault;
		self.root_ctr.appSelf = self;
		self.display_link = [CADisplayLink displayLinkWithTarget:self
																										selector:@selector(display_link_callback:)];
		self.window.backgroundColor = [UIColor blackColor];
		self.window.rootViewController = self.root_ctr;
		
		[self.window makeKeyAndVisible];
		
		UIView* rootView = self.window.rootViewController.view;
		
		self.view = renderApple->init(rootView.bounds);
		self.view.contentScaleFactor = UIScreen.mainScreen.scale;
		self.view.translatesAutoresizingMaskIntoConstraints = NO;
		self.view.multipleTouchEnabled = YES;
		self.view.userInteractionEnabled = YES;

		self.ime = [[IOSIMEHelprt alloc] initWithApplication:self.app];

		[rootView addSubview:self.view];
		[rootView addSubview:self.ime];
		[rootView addConstraint:[NSLayoutConstraint
												constraintWithItem:self.view
												attribute:NSLayoutAttributeWidth
												relatedBy:NSLayoutRelationEqual
												toItem:rootView
												attribute:NSLayoutAttributeWidth
												multiplier:1
												constant:0]];
		[rootView addConstraint:[NSLayoutConstraint
												constraintWithItem:self.view
												attribute:NSLayoutAttributeHeight
												relatedBy:NSLayoutRelationEqual
												toItem:rootView
												attribute:NSLayoutAttributeHeight
												multiplier:1
												constant:0]];
		
		_app->display()->set_default_scale(UIScreen.mainScreen.scale);

		renderApple->resize(self.view.frame);

		_inl_app(_app)->triggerLoad();

		[self.display_link addToRunLoop:[NSRunLoop mainRunLoop]
														forMode:NSDefaultRunLoopMode];
		return YES;
	}

	- (void)application:(UIApplication*)app didChangeStatusBarFrame:(CGRect)frame {
		if ( appDelegate && !_is_background ) {
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

// ***************** A p p l i c a t i o n *****************

Render* Render::Make(Application* host) {
	renderApple = RenderApple::Make(host);
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
	NSURL* url2 = [NSURL URLWithString:[NSString stringWithUTF8String:*url]];
	dispatch_async(dispatch_get_main_queue(), ^{
		[appDelegate.host openURL:url2 options:@{} completionHandler:nil];
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
		mail.mailComposeDelegate = appDelegate;
			[appDelegate.root_ctr presentViewController:mail animated:YES completion:nil];
	});
}

/**
 * @func ime_keyboard_open
 */
void AppInl::ime_keyboard_open(KeyboardOptions options) {
	dispatch_async(dispatch_get_main_queue(), ^{
		[appDelegate.ime set_keyboard_type:options.type];
		[appDelegate.ime set_keyboard_return_type:options.return_type];
		if ( options.is_clear ) {
			[appDelegate.ime clear];
		}
		[appDelegate.ime open];
	});
}

/**
 * @func ime_keyboard_can_backspace
 */
void AppInl::ime_keyboard_can_backspace(bool can_backspace, bool can_delete) {
	dispatch_async(dispatch_get_main_queue(), ^{
		[appDelegate.ime set_keyboard_can_backspace:can_backspace can_delete:can_delete];
	});
}

/**
 * @func ime_keyboard_close
 */
void AppInl::ime_keyboard_close() {
	dispatch_async(dispatch_get_main_queue(), ^{
		[appDelegate.ime close];
	});
}

/**
 * @func ime_keyboard_spot_location
 */
void AppInl::ime_keyboard_spot_location(Vec2 location) {
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
	return 1.0 / UIScreen.mainScreen.scale;
}

/**
 * @func keep_screen(keep)
 */
void Display::keep_screen(bool keep) {
	dispatch_async(dispatch_get_main_queue(), ^{
		if ( keep ) {
			appDelegate.host.idleTimerDisabled = YES;
		} else {
			appDelegate.host.idleTimerDisabled = NO;
		}
	});
}

/**
 * @func status_bar_height()
 */
float Display::status_bar_height() {
	CGRect rect = appDelegate.host.statusBarFrame;
	return Qk_MIN(rect.size.height, 20) * UIScreen.mainScreen.scale / _scale;
}

/**
 * @func default_status_bar_height
 */
float Display::default_status_bar_height() {
	if (appDelegate && appDelegate.app) {
		return appDelegate.app->display()->status_bar_height();
	} else {
		return 20;
	}
}

/**
 * @func set_visible_status_bar(visible)
 */
void Display::set_visible_status_bar(bool visible) {
	if ( visible == appDelegate.visible_status_bar ) return;
	appDelegate.visible_status_bar = visible;

	dispatch_async(dispatch_get_main_queue(), ^{
		//if ( visible ) {
		//  [appDelegate.host setStatusBarHidden:NO withAnimation:UIStatusBarAnimationSlide];
		//} else {
		//  [appDelegate.host setStatusBarHidden:YES withAnimation:UIStatusBarAnimationSlide];
		//}
		[appDelegate refresh_status];

		// TODO 延时16ms(一帧画面时间),给足够的时间让RootViewController重新刷新状态 ?
		renderApple->resize(appDelegate.view.frame);

		// TODO 绘图表面尺寸没有改变? 表示只是单纯状态栏改变? 这个改变也当成change通知给用户
		_host->loop()->post(Cb([this](Cb::Data& e) {
			Qk_Trigger(Change);
		}));
	});
}

/**
 * @func set_status_bar_text_color(color)
 */
void Display::set_status_bar_style(StatusBarStyle style) {
	UIStatusBarStyle style_2;
	if ( style == STATUS_BAR_STYLE_WHITE ) {
		style_2 = UIStatusBarStyleLightContent;
	} else {
		if (@available(iOS 13.0, *)) {
			style_2 = UIStatusBarStyleDarkContent;
		} else {
			style_2 = UIStatusBarStyleDefault;
		}
	}
	if ( appDelegate && appDelegate.status_bar_style != style_2 ) {
		appDelegate.status_bar_style = style_2;
		dispatch_async(dispatch_get_main_queue(), ^{
			[appDelegate refresh_status];
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
	switch ( appDelegate.host.statusBarOrientation ) {
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
	if ( appDelegate.setting_orientation != orientation ) {
		appDelegate.setting_orientation = orientation;
		dispatch_async(dispatch_get_main_queue(), ^{
			[appDelegate refresh_status];
		});
	}
}

extern "C" Qk_EXPORT int main(int argc, char* argv[]) {
	Application::runMain(argc, argv);
	if ( app() ) {
		@autoreleasepool {
			UIApplicationMain(argc, argv, nil, NSStringFromClass(ApplicationDelegate.class));
		}
	}
	return 0;
}
