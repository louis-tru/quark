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
#import "ngui/base/loop.h"
#import "ios-ogl-1.h"
#import "ios-ime-receiver-1.h"
#import "ios-app.h"
#import "../app.h"
#import "../display-port.h"
#import "../app-1.h"
#import "../event.h"
#import <MessageUI/MFMailComposeViewController.h>

using namespace ngui;
using namespace ngui;

typedef DisplayPort::Orientation Orientation;
typedef DisplayPort::StatusBarStyle StatusBarStyle;

static ApplicationDelegate* ios_app = nil;
static IOSGLDrawCore* ios_draw_core = nil;
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
  Mutex     _main_mutex;
  Condition _main_cond;
  Callback  _render_cb;
  UIWindow* _window;
  BOOL      _is_background;
}
@property (strong, nonatomic) GLView* glview;
@property (strong, nonatomic) IOSIMEReceiver* receiver;
@property (strong, nonatomic) CADisplayLink* display_link;
@property (strong, nonatomic) UIApplication* host;
@property (strong, nonatomic) RootViewController* root_ctr;
@property (assign, nonatomic) Orientation setting_orientation;
@property (assign, nonatomic) Orientation current_orientation;
@property (assign, nonatomic) bool visible_status_bar;
@property (assign, nonatomic) UIStatusBarStyle status_bar_style;
@end

/**
 * @implementation RootViewController
 */
@implementation RootViewController

- (BOOL)shouldAutorotate {
  return YES;
}

- (UIInterfaceOrientationMask)supportedInterfaceOrientations {
  switch ( ios_app.setting_orientation ) {
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
      switch(ios_app.current_orientation  ) {
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
    ::CGRect rect = ios_app.glview.frame;
    ios_app.app->render_loop()->post(Cb([ori, rect](Se& d) {
      ios_draw_core->refresh_surface_size(rect);
      if (ori != ios_app.current_orientation) {
        ios_app.current_orientation = ori;
        main_loop()->post(Cb([](Se& e) {
          (ios_app.app)->display_port()->XX_TRIGGER(orientation);
        }));
      }
    }));
  } completion:nil];
  [super viewWillTransitionToSize:size withTransitionCoordinator:coordinator];
}

- (UIStatusBarStyle)preferredStatusBarStyle {
  return ios_app.status_bar_style;
}

- (UIStatusBarAnimation)preferredStatusBarUpdateAnimation {
  // UIApplicationWillChangeStatusBarFrameNotification
  return UIStatusBarAnimationSlide;
}

- (BOOL)prefersStatusBarHidden {
  return !ios_app.visible_status_bar;
}

@end

/**
 * @implementation OGLView
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
  
  float scale_x = size.width() / ios_app.glview.frame.size.width;
  float scale_y = size.height() / ios_app.glview.frame.size.height;
  
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
  // XX_DEBUG("touchesMoved, count: %d", touches.count);
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
 */
@implementation ApplicationDelegate

static void render_loop_cb(Se& evt, Object* ctx) {
  { //
    ScopeLock scope(ios_app->_main_mutex);
    ios_app->_main_cond.notify_one();
  }
  _inl_app(ios_app.app)->onRender();
}

- (void)render_loop:(CADisplayLink*)displayLink {
  Lock lock(_main_mutex);
  _app->render_loop()->post(_render_cb);
  _main_cond.wait(lock);
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

- (BOOL)application:(UIApplication*)app didFinishLaunchingWithOptions:(NSDictionary*)options {
  XX_ASSERT(!ios_app); ios_app = self;
  
  //[app setStatusBarStyle:UIStatusBarStyleLightContent];
  //[app setStatusBarHidden:NO];
  
  _app = Inl_GUIApplication(GUIApplication::shared()); XX_ASSERT(self.app);
  _render_cb = Cb(render_loop_cb);
  _window = [[UIWindow alloc] initWithFrame:[[UIScreen mainScreen] bounds]];
  _is_background = NO;
  
  self.host = app;
  self.setting_orientation = Orientation::ORIENTATION_USER;
  self.current_orientation = Orientation::ORIENTATION_INVALID;
  self.visible_status_bar = YES;
  self.status_bar_style = UIStatusBarStyleLightContent;
  self.root_ctr = [[RootViewController alloc] init];
  self.display_link = [CADisplayLink displayLinkWithTarget:self selector:@selector(render_loop:)];
  self.window.backgroundColor = [UIColor blackColor];
  self.window.rootViewController = self.root_ctr;
  
  [self.window makeKeyAndVisible];
  
  UIView* view = self.window.rootViewController.view;
  self.glview = [[GLView alloc] initWithFrame:[view bounds]];
  self.glview.contentScaleFactor = UIScreen.mainScreen.scale;
  self.glview.translatesAutoresizingMaskIntoConstraints = NO;
  self.glview.app = _inl_app(self.app);
  self.receiver = [[IOSIMEReceiver alloc] initWithApplication:self.app];
  
  [view addSubview:self.glview];
  [view addSubview:self.receiver];
  [view addConstraint:[NSLayoutConstraint
                       constraintWithItem:self.self.glview
                       attribute:NSLayoutAttributeWidth
                       relatedBy:NSLayoutRelationEqual
                       toItem:view
                       attribute:NSLayoutAttributeWidth
                       multiplier:1
                       constant:0]];
  [view addConstraint:[NSLayoutConstraint
                       constraintWithItem:self.self.glview
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
  layer.opaque = YES;
  
  _app->render_loop()->post(Cb([self, layer, rect](Se& d) {
    ios_draw_core->set_surface_view(self.glview, layer);
    ios_draw_core->refresh_surface_size(rect);
    _inl_app(self.app)->onLoad();
    [self.display_link addToRunLoop:[NSRunLoop mainRunLoop] forMode:NSDefaultRunLoopMode];
  }));
  
  return YES;
}

- (void)add_system_notification {
  // TODO ..
}

- (void)refresh_surface_size {
  ::CGRect rect = ios_app.glview.frame;
  _app->render_loop()->post(Cb([self, rect](Se& d) {
    ios_draw_core->refresh_surface_size(rect);
  }));
}

- (void)application:(UIApplication*)app didChangeStatusBarFrame:(::CGRect)frame {
  if ( ios_app && !_is_background ) {
    [self refresh_surface_size];
  }
}

- (void)applicationWillResignActive:(UIApplication*) application {
  _inl_app(_app)->onPause();
}

- (void)applicationDidBecomeActive:(UIApplication*) application {
  _inl_app(_app)->onResume();
  [self refresh_surface_size];
}

- (void)applicationDidEnterBackground:(UIApplication*) application {
  _is_background = YES;
  _inl_app(_app)->onBackground();
}

- (void)applicationWillEnterForeground:(UIApplication*) application {
  _is_background = NO;
  _inl_app(_app)->onForeground();
}

- (void)applicationDidReceiveMemoryWarning:(UIApplication*) application {
  _inl_app(_app)->onMemorywarning();
}

- (void)applicationWillTerminate:(UIApplication*)application {
  [self.display_link removeFromRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
  _inl_app(_app)->onUnload();
}

+ (void)set_application_delegate:(NSString*)name {
  app_delegate_name = name;
}

- (void) mailComposeController:(MFMailComposeViewController*)controller
           didFinishWithResult:(MFMailComposeResult)result
                         error:(NSError*)error {
  [controller dismissViewControllerAnimated:YES completion:nil];
}

@end

/**
 * @func pending() 挂起应用进程
 */
void GUIApplication::pending() {
  exit(0);
}

/**
 * @func open_url()
 */
void GUIApplication::open_url(cString& url) {
  NSURL* url2 = [NSURL URLWithString:[NSString stringWithUTF8String:*url]];
  dispatch_async(dispatch_get_main_queue(), ^{
    [ios_app.host openURL:url2 options:@{ } completionHandler:nil];
  });
}

/**
 * @func split_NSArray(str)
 */
static NSArray<NSString*>* split_NSArray(cString& str) {
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
  MFMailComposeViewController* mail = [MFMailComposeViewController new];
  [mail setToRecipients:split_NSArray(recipient)];
  [mail setSubject:[NSString stringWithUTF8String:*subject]];
  [mail setCcRecipients:split_NSArray(cc)];
  [mail setBccRecipients:split_NSArray(bcc)];
  [mail setMessageBody:[NSString stringWithUTF8String:*body] isHTML:NO];
  mail.mailComposeDelegate = ios_app;
  dispatch_async(dispatch_get_main_queue(), ^{
    [ios_app.root_ctr presentViewController:mail animated:YES completion:nil];
  });
}

/**
 * @func initialize(options)
 */
void AppInl::initialize(const Map<String, int>& options) {
  XX_ASSERT(!ios_draw_core);
  ios_draw_core = IOSGLDrawCore::create(this, options);
  m_draw_ctx = ios_draw_core->host();
}

/**
 * @func ime_keyboard_open
 */
void AppInl::ime_keyboard_open(KeyboardOptions options) {
  dispatch_async(dispatch_get_main_queue(), ^{
    [ios_app.receiver input_keyboard_type:options.type];
    [ios_app.receiver input_keyboard_return_type:options.return_type];
    if ( options.is_clear ) {
      [ios_app.receiver clear];
    }
    [ios_app.receiver open];
  });
}

/**
 * @func ime_keyboard_can_backspace
 */
void AppInl::ime_keyboard_can_backspace(bool can_backspace, bool can_delete) {
  dispatch_async(dispatch_get_main_queue(), ^{
    [ios_app.receiver input_keyboard_can_backspace:can_backspace];
  });
}

/**
 * @func ime_keyboard_close
 */
void AppInl::ime_keyboard_close() {
  dispatch_async(dispatch_get_main_queue(), ^{
    [ios_app.receiver close];
  });
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
      ios_app.host.idleTimerDisabled = YES;
    } else {
      ios_app.host.idleTimerDisabled = NO;
    }
  });
}

/**
 * @func status_bar_height()
 */
float DisplayPort::status_bar_height() {
  ::CGRect rect = ios_app.host.statusBarFrame;
  return XX_MIN(rect.size.height, 20) * UIScreen.mainScreen.scale / m_scale_value[1];
}

/**
 * @func set_visible_status_bar(visible)
 */
void DisplayPort::set_visible_status_bar(bool visible) {
  
  if ( visible != ios_app.visible_status_bar ) {
    ios_app.visible_status_bar = visible;
    dispatch_async(dispatch_get_main_queue(), ^{
      //if ( visible ) {
      //  [ios_app.host setStatusBarHidden:NO withAnimation:UIStatusBarAnimationSlide];
      //} else {
      //  [ios_app.host setStatusBarHidden:YES withAnimation:UIStatusBarAnimationSlide];
      //}
      [ios_app refresh_status];
      
      ::CGRect rect = ios_app.glview.frame;
      m_host->render_loop()->post(Cb([this, rect](Se& ev) {
        if ( !ios_draw_core->refresh_surface_size(rect) ) {
          // 绘图表面尺寸没有改变，表示只是单纯状态栏改变，这个改变也当成change通知给用户
          main_loop()->post(Cb([this](Se& e){
            XX_TRIGGER(change);
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
  if ( ios_app.status_bar_style != style_2 ) {
    ios_app.status_bar_style = style_2;
    dispatch_async(dispatch_get_main_queue(), ^{
      //[ios_app.host setStatusBarStyle:ios_app.status_bar_style];
      [ios_app refresh_status];
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
  switch ( ios_app.host.statusBarOrientation ) {
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
  if ( ios_app.setting_orientation != orientation ) {
    ios_app.setting_orientation = orientation;
    dispatch_async(dispatch_get_main_queue(), ^{
      [ios_app refresh_status];
    });
  }
}

extern "C" int main(int argc, char* argv[]) {
  AppInl::run_main(argc, argv);
  
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
