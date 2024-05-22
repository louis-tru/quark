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

#import "./mac_app.h"
#import "../../ui/screen.h"
#import "IOKit/pwr_mgt/IOPMLib.h"

using namespace qk;

// ***************** D i s p l a y *****************

typedef Screen::StatusBarStyle StatusBarStyle;

extern QkApplicationDelegate *qkappdelegate;
static IOPMAssertionID prevent_screen_sleep_assertionID = 0;

float Screen::main_screen_scale() {
	return UIScreen.mainScreen.backingScaleFactor;
}

void Screen::prevent_screen_sleep(bool prevent) {
	if (prevent) {
		if (!prevent_screen_sleep_assertionID) {
			CFStringRef reasonForActivity = CFSTR("Preventing screen sleep due to my application");
			IOPMAssertionCreateWithName(
				kIOPMAssertionTypeNoIdleSleep,
				kIOPMAssertionLevelOn, reasonForActivity, &prevent_screen_sleep_assertionID
			);
		}
	} else {
		if (prevent_screen_sleep_assertionID) {
			IOPMAssertionRelease(prevent_screen_sleep_assertionID);
			prevent_screen_sleep_assertionID = 0;
		}
	}
}

float Screen::status_bar_height() const {
	return 0;
}

void Screen::set_visible_status_bar(bool visible) {
}

void Screen::set_status_bar_style(StatusBarStyle style) {
}

Screen::Orientation Screen::orientation() const {
	return kInvalid;
}

void Screen::set_orientation(Orientation orientation) {
}
