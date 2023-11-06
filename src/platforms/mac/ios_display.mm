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

#import "./ios_app.h"
#import "../../display.h"

using namespace qk;

// ***************** D i s p l a y *****************

typedef Display::StatusBarStyle StatusBarStyle;

extern QkApplicationDelegate *__app;

float Display::default_atom_pixel() {
	return 1.0 / UIScreen.mainScreen.scale;
}

void Display::keep_screen(bool keep) {
	dispatch_async(dispatch_get_main_queue(), ^{
		if ( keep ) {
			__app.app.idleTimerDisabled = YES;
		} else {
			__app.app.idleTimerDisabled = NO;
		}
	});
}

float Display::status_bar_height() {
	CGRect rect = __app.app.statusBarFrame;
	return Qk_MIN(rect.size.height, 20) * UIScreen.mainScreen.scale / _scale;
}

float Display::default_status_bar_height() { // static method
	if (__app && __app.host) {
		return __app.host->display()->status_bar_height();
	} else {
		return 20;
	}
}

void Display::set_visible_status_bar(bool visible) {
	if ( visible == __app.visible_status_bar )
		return;
	__app.visible_status_bar = visible;

	dispatch_async(dispatch_get_main_queue(), ^{
		//if ( visible ) {
		//  [__app.app setStatusBarHidden:NO withAnimation:UIStatusBarAnimationSlide];
		//} else {
		//  [__app.app setStatusBarHidden:YES withAnimation:UIStatusBarAnimationSlide];
		//}
		[__app refresh_status];
		// TODO 延时16ms(一帧画面时间),给足够的时间让RootViewController重新刷新状态 ?
		__app.host->render()->reload();

		// TODO 绘图表面尺寸没有改变? 表示只是单纯状态栏改变? 这个改变也当成change通知给用户
		__app.host->loop()->post(Cb([this](Cb::Data& e) {
			Qk_Trigger(Change);
		}));
	});
}

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
	if ( __app && __app.status_bar_style != style_2 ) {
		dispatch_async(dispatch_get_main_queue(), ^{
			[__app refresh_status];
		});
		__app.status_bar_style = style_2;
	}
}

void Display::request_fullscreen(bool fullscreen) {
	set_visible_status_bar(!fullscreen);
}

Display::Orientation Display::orientation() {
	Orientation r = ORIENTATION_INVALID;
	switch ( __app.app.statusBarOrientation ) {
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

void Display::set_orientation(Orientation orientation) {
	if ( __app.setting_orientation != orientation ) {
		dispatch_async(dispatch_get_main_queue(), ^{
			[__app refresh_status];
		});
		__app.setting_orientation = orientation;
	}
}
