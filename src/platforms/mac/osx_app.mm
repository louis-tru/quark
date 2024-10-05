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

#import <MacTypes.h>
#import <AppKit/AppKit.h>
#import "../../util/loop.h"
#import "../../util/http.h"
#import "../../ui/app.h"
#import "../../ui/window.h"
#import "../../ui/event.h"
#import "../../ui/screen.h"
#import "./mac_app.h"

using namespace qk;

// ***************** Q k . A p p l i c a t i o n . D e l e g a t e *****************

QkApplicationDelegate* qkappdelegate = nil;

@interface QkApplicationDelegate()
@property (assign, nonatomic) BOOL isPause;
@end

@implementation QkApplicationDelegate

- (void)applicationDidFinishLaunching:(NSNotification*) notification {
	Qk_Assert(!qkappdelegate);
	Qk_Assert(Application::shared());
	qkappdelegate = self;
	_host = Application::shared();
	_app = UIApplication.sharedApplication;
	self.isPause = YES;
	Inl_Application(_host)->triggerLoad();
}

- (void)applicationWillResignActive:(NSNotification*)notification {
	if (self.isPause) return;
	self.isPause = YES;
	Inl_Application(_host)->triggerPause();
	Qk_DLog("applicationWillResignActive, triggerPause");
}

- (void)applicationDidBecomeActive:(NSNotification*)notification {
	if (!self.isPause) return;
	self.isPause = NO;
	Inl_Application(_host)->triggerResume();
	Qk_DLog("applicationDidBecomeActive, triggerResume");
}

- (void)applicationDidHide:(NSNotification*)notification {
	Qk_DLog("applicationDidHide, onBackground");
}

- (void)applicationWillUnhide:(NSNotification*)notification {
	Qk_DLog("applicationWillUnhide, onForeground");
}

- (void)applicationWillTerminate:(NSNotification*)notification {
	Inl_Application(_host)->triggerUnload();
	Qk_DLog("applicationWillTerminate, triggerUnload");
}

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication*)sender {
	Qk_DLog("applicationShouldTerminateAfterLastWindowClosed, exit application");
	return YES;
}

- (BOOL)applicationShouldHandleReopen:(NSApplication*)sender hasVisibleWindows:(BOOL)flag {
	Qk_DLog("applicationShouldHandleReopen");
	return YES;
}

@end

// ***************** A p p l i c a t i o n *****************

void Application::openURL(cString& url) {
	[[NSWorkspace sharedWorkspace] openURL:[NSURL URLWithString:[NSString stringWithUTF8String:*url]]];
}

void Application::sendEmail(cString& recipient,
														 cString& subject,
														 cString& body, cString& cc, cString& bcc)
{
	NSString *url = [NSString stringWithFormat:@"mailto:%s?subject=%s&body=%s&cc=%s&bcc=%s",
		*recipient,*URI::encode(subject, true),*URI::encode(body, true),*cc,*bcc
	];
	[[NSWorkspace sharedWorkspace] openURL:[NSURL URLWithString:url]];
}

// ***************** E v e n t . D i s p a t c h *****************

void EventDispatch::setVolumeUp() {
}

void EventDispatch::setVolumeDown() {
}

void EventDispatch::setImeKeyboardCanBackspace(bool can_backspace, bool can_delete) {
}

void EventDispatch::setImeKeyboardOpen(KeyboardOptions options) {
	auto delegate = window()->impl()->delegate();
	qk_post_messate_main(Cb([options,delegate](auto e) {
		[delegate.ime set_keyboard_type:options.type];
		[delegate.ime set_keyboard_return_type:options.return_type];
		[delegate.ime activate: options.is_clear];
		[delegate.ime set_spot_rect:options.spot_rect];
	}), false);
}

void EventDispatch::setImeKeyboardClose() {
	auto delegate = window()->impl()->delegate();
	qk_post_messate_main(Cb([delegate](auto e) {
		[delegate.ime deactivate];
	}), false);
}

void EventDispatch::setImeKeyboardSpotRect(Rect rect) {
	auto delegate = window()->impl()->delegate();
	qk_post_messate_main(Cb([delegate,rect](auto e) {
		[delegate.ime set_spot_rect:rect];
	}), false);
}
