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

#import "./apple_app.h"
#import "../../ui/screen.h"
#import "../../ui/app.h"
#import "../../ui/window.h"

using namespace qk;

extern QkApplicationDelegate *qkappdelegate;

@interface QkWindowDelegate()
- (void)set_visible_status_bar:(bool)visible;
- (void)set_status_bar_style:(Screen::StatusBarStyle)style;
- (void)set_orientation:(Screen::Orientation)orientation;
@end

QkWindowDelegate* getWindowDelegate();

float Screen::main_screen_scale() {
	return UIScreen.mainScreen.scale;
}

void Screen::prevent_screen_sleep(bool prevent) {
	dispatch_async(dispatch_get_main_queue(), ^{
		if (prevent) {
			qkappdelegate.app.idleTimerDisabled = YES;
		} else {
			qkappdelegate.app.idleTimerDisabled = NO;
		}
	});
}

float Screen::status_bar_height() const {
	CGRect rect = qkappdelegate.app.statusBarFrame;
	auto qkwin = qkappdelegate.host->activeWindow();
	return Qk_Min(rect.size.height, 20) * UIScreen.mainScreen.scale / qkwin->scale();
}

void Screen::set_visible_status_bar(bool visible) {
	[getWindowDelegate() set_visible_status_bar:visible];
}

void Screen::set_status_bar_style(StatusBarStyle style) {
	[getWindowDelegate() set_status_bar_style:style];
}

Screen::Orientation Screen::orientation() const {
	Screen::Orientation r;
	switch ( qkappdelegate.app.statusBarOrientation ) {
		case UIInterfaceOrientationPortrait:           r = kPortrait; break;
		case UIInterfaceOrientationPortraitUpsideDown: r = kReverse_Portrait; break;
		case UIInterfaceOrientationLandscapeLeft:      r = kReverse_Landscape; break;
		case UIInterfaceOrientationLandscapeRight:     r = kLandscape; break;
		default:                                       r = kInvalid; break;
	}
	return r;
}

void Screen::set_orientation(Screen::Orientation orientation) {
	[getWindowDelegate() set_orientation:orientation];
}
