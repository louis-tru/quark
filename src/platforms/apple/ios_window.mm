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
typedef UIEvent MacUIEvent;
#import "../../ui/app.h"
#import "../../ui/event.h"
#include "../../ui/window.h"
#include "../../ui/screen.h"
#include "../../render/apple/apple_render.h"
#include "./apple_app.h"

using namespace qk;

typedef Screen::Orientation Orientation;

// ***************** W i n d o w *****************

QkWindowDelegate* WindowImpl::delegate() {
	return ((__bridge QkWindowDelegate*)(this));
}

@interface QkWindowDelegate()
@property (assign, nonatomic) Orientation      setting_orientation;
@property (assign, nonatomic) Orientation      current_orientation;
@property (assign, nonatomic) bool             visible_status_bar;
@property (assign, nonatomic) UIStatusBarStyle status_bar_style;
@end

@implementation QkWindowDelegate

- (id) init:(Window::Options&)opts win:(Window*)win render:(Render*)render {
	if ( !(self = [super init]) )
		return nil;

	self.setting_orientation = Orientation::kUser;
	self.current_orientation = Orientation::kInvalid;
	self.visible_status_bar = YES;
	self.status_bar_style = UIStatusBarStyleDefault;

	UIScreen *screen = UIScreen.mainScreen;
	UIWindow *uiwin = [[UIWindow alloc] initWithFrame:screen.bounds];

	self.qkwin = win;
	self.uiwin = uiwin;
	self.uiwin.rootViewController = self;
	self.ime = qk_make_ime_helper(win);

	UIView *rootView = self.view;
	UIView *view = render->surface()->surfaceView();

	view.frame = rootView.bounds;
	view.translatesAutoresizingMaskIntoConstraints = NO;
	view.contentScaleFactor = screen.scale;
	view.multipleTouchEnabled = YES;
	view.userInteractionEnabled = YES;

	[rootView addSubview:view];
	[rootView addSubview:self.ime.view];
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

	_qkwin->render()->reload();
	return self;
}

// UIViewController
// -------------------------------------------------------------------------

- (BOOL)shouldAutorotate {
	return YES;
}

- (UIInterfaceOrientationMask)supportedInterfaceOrientations {
	switch ( self.setting_orientation ) {
		case Orientation::kPortrait:
			return UIInterfaceOrientationMaskPortrait;
		case Orientation::kLandscape:
			return UIInterfaceOrientationMaskLandscapeRight;
		case Orientation::kReverse_Portrait:
			return UIInterfaceOrientationMaskPortraitUpsideDown;
		case Orientation::kReverse_Landscape:
			return UIInterfaceOrientationMaskLandscapeLeft;
		case Orientation::kUser: default:
			return UIInterfaceOrientationMaskAll;
		case Orientation::kUser_Portrait:
			return UIInterfaceOrientationMaskPortrait | UIInterfaceOrientationMaskPortraitUpsideDown;
		case Orientation::kUser_Landscape:
			return UIInterfaceOrientationMaskLandscape;
		case Orientation::kUser_Locked: {
			switch (self.current_orientation) {
				default:
				case Orientation::kInvalid:
					return UIInterfaceOrientationMaskAll;
				case Orientation::kPortrait:
					return UIInterfaceOrientationMaskPortrait;
				case Orientation::kLandscape:
					return UIInterfaceOrientationMaskLandscapeRight;
				case Orientation::kReverse_Portrait:
					return UIInterfaceOrientationMaskPortraitUpsideDown;
				case Orientation::kReverse_Landscape:
					return UIInterfaceOrientationMaskLandscapeLeft;
			}
		}
	}
	return UIInterfaceOrientationMaskAll;
}

- (void)viewWillTransitionToSize:(CGSize)size
			withTransitionCoordinator:(id<UIViewControllerTransitionCoordinator>)coordinator
{
	[coordinator animateAlongsideTransition:^(id context) {
		_qkwin->render()->reload();

		auto orient = _qkwin->host()->screen()->orientation();
		if (orient != self.current_orientation) {
			self.current_orientation = orient;
			Inl_Application(_qkwin->host())->triggerOrientation();
		}
	} completion:nil];
	[super viewWillTransitionToSize:size withTransitionCoordinator:coordinator];
}

- (UIStatusBarStyle)preferredStatusBarStyle {
	return self.status_bar_style;
}

- (UIStatusBarAnimation)preferredStatusBarUpdateAnimation {
	// UIApplicationWillChangeStatusBarFrameNotification
	return UIStatusBarAnimationSlide;
}

- (BOOL)prefersStatusBarHidden {
	return !self.visible_status_bar;
}

- (List<TouchPoint>)touchsList:(NSSet<UITouch*>*)touches {
	List<TouchPoint> rv;

	Vec2 size = _qkwin->size();

	float scale_x = size.x() /  self.view.frame.size.width;
	float scale_y = size.y() /  self.view.frame.size.height;

	for (UITouch* touch in [touches objectEnumerator]) {
		CGPoint point = [touch locationInView:touch.view];
		// CGFloat angle = touch.altitudeAngle;
		// CGFloat max_force = touch.maximumPossibleForce;
		rv.pushBack({
			uint32_t((size_t)touch % Uint32::limit_max), 0, 0,
			float(point.x * scale_x), float(point.y * scale_y),
			float(touch.force), false, nullptr,
		});
	}
	return rv;
}

-(void)touchesBegan:(NSSet<UITouch *> *)touches withEvent:(nullable MacUIEvent *)event {
	_qkwin->dispatch()->onTouchstart( [self touchsList:touches] );
}

-(void)touchesMoved:(NSSet<UITouch *> *)touches withEvent:(nullable MacUIEvent *)event {
	// Qk_DLog("touchesMoved, count: %d", touches.count);
	_qkwin->dispatch()->onTouchmove( [self touchsList:touches] );
}

-(void)touchesEnded:(NSSet<UITouch *> *)touches withEvent:(nullable MacUIEvent *)event{
	_qkwin->dispatch()->onTouchend( [self touchsList:touches] );
}

-(void)touchesCancelled:(NSSet<UITouch *> *)touches withEvent:(nullable MacUIEvent *)event {
	_qkwin->dispatch()->onTouchcancel( [self touchsList:touches] );
}

// Screen
// -------------------------------------------------------------------------

-(void)refresh {
	if ( _uiwin.rootViewController == self ) {
		_uiwin.rootViewController = nil;
		_uiwin.rootViewController = self;
	}
}

- (void)set_visible_status_bar:(bool)visible {
	if (visible != self.visible_status_bar) {
		dispatch_async(dispatch_get_main_queue(), ^{
			[self refresh];
		});
		self.visible_status_bar = visible;
	}
}

- (void)set_status_bar_style:(Screen::StatusBarStyle)style {
	UIStatusBarStyle barStyle;
	if ( style == Screen::kWhite ) {
		barStyle = UIStatusBarStyleLightContent;
	} else {
		if (@available(iOS 13.0, *)) {
			barStyle = UIStatusBarStyleDarkContent;
		} else {
			barStyle = UIStatusBarStyleDefault;
		}
	}
	if ( self.status_bar_style != barStyle ) {
		dispatch_async(dispatch_get_main_queue(), ^{
			[self refresh];
		});
		self.status_bar_style = barStyle;
	}
}

- (void)set_orientation:(Screen::Orientation)orientation {
		if ( self.setting_orientation != orientation ) {
		dispatch_async(dispatch_get_main_queue(), ^{
			[self refresh];
		});
		self.setting_orientation = orientation;
	}
}

@end

void Window::openImpl(Options &opts) {
	post_messate_main(Cb([&opts,this](auto e) {
		auto impl = [[QkWindowDelegate alloc]
								 init:opts win:this render:_render];
		CFBridgingRetain(impl); // Retain
		_impl = (__bridge WindowImpl*)impl;
		set_backgroundColor(opts.backgroundColor);
		activate();
	}), true);
}

void Window::closeImpl() {
	post_messate_main(Cb([this](auto e) {
		Qk_ASSERT_NE(_impl, nullptr);
		CFBridgingRelease(_impl);
		_impl = nullptr;
		if (!_host->activeWindow())
			thread_exit(0); // Exit process
	}, this), false);
}

void Window::beforeClose() {}

void Window::set_backgroundColor(Color val) {
	post_messate_main(Cb([this,val](auto e) {
		if (!_impl) return;
		auto color = val.to_color4f();
		_impl->delegate().uiwin.backgroundColor = [
			UIColor colorWithRed:color.r() green:color.g() blue:color.b() alpha:color.a()
		];
	}, this), false);
	_backgroundColor = val;
}

void Window::activate() {
	auto awin = _host->activeWindow();
	if (awin == this) return;

	post_messate_main(Cb([this](auto e) {
		if (!_impl) return;
		[_impl->delegate().uiwin makeKeyAndVisible];
	}, this), false);

	Inl_Application(_host)->setActiveWindow(this);
	if (awin)
		Inl_Application(_host)->triggerBackground(awin);
	Inl_Application(_host)->triggerForeground(this);
}

Region Window::getDisplayRegion(Vec2 size) {
	return {{0}, size};
}

void Window::afterDisplay() {
	// Noop
}

float Window::getDefaultScale() {
	return _platform(_impl)->_xwin_scale;
}

void Window::pending() {
	// Exit app
}

void Window::setFullscreen(bool fullscreen) {
	[_impl->delegate() set_visible_status_bar:!fullscreen];
}

void Window::setCursorStyle(CursorStyle cursor, bool low) {
}
