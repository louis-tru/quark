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

#import "../../ui/app.h"
#import "../../ui/window.h"
#import "../../ui/event.h"
#import "../../ui/screen.h"
#import "./mac_app.h"
#import <MessageUI/MFMailComposeViewController.h>

using namespace qk;

// ***************** Q k . A p p l i c a t i o n . D e l e g a t e *****************

QkApplicationDelegate* qkappdelegate = nil;

QkWindowDelegate* getActiveDelegate() {
	auto qkwin = qkappdelegate.host->activeWindow();
	if (qkwin)
		return qkwin->impl()->delegate();
	return nil;
}

@interface QkApplicationDelegate()<MFMailComposeViewControllerDelegate>
@end

@implementation QkApplicationDelegate

	- (BOOL)application:(UIApplication*)app didFinishLaunchingWithOptions:(NSDictionary*)options {
		Qk_Assert(!qkappdelegate);
		Qk_Assert(Application::shared());
		qkappdelegate = self;
		_host = Application::shared();
		_app = app;
		Inl_Application(_host)->triggerLoad();
		//[app setStatusBarStyle:UIStatusBarStyleLightContent];
		//[app setStatusBarHidden:NO];
		return YES;
	}

	- (void)application:(UIApplication*)app didChangeStatusBarFrame:(CGRect)frame {
		_host->activeWindow()->render()->reload();
	}

	- (void)applicationWillResignActive:(UIApplication*) application {
		Inl_Application(_host)->triggerPause();
		Qk_DLog("applicationWillResignActive,triggerPause");
	}

	- (void)applicationDidBecomeActive:(UIApplication*) application {
		Inl_Application(_host)->triggerResume();
		auto win = _host->activeWindow();
		if (win)
			win->render()->reload();
		Qk_DLog("applicationDidBecomeActive,triggerResume");
	}

	- (void)applicationDidEnterBackground:(UIApplication*) application {
		Inl_Application(_host)->triggerBackground(_host->activeWindow());
		Qk_DLog("applicationDidEnterBackground,triggerBackground");
	}

	- (void)applicationWillEnterForeground:(UIApplication*) application {
		Inl_Application(_host)->triggerForeground(_host->activeWindow());
		Qk_DLog("applicationWillEnterForeground,triggerForeground");
	}

	- (void)applicationDidReceiveMemoryWarning:(UIApplication*) application {
		Inl_Application(_host)->triggerMemorywarning();
		Qk_DLog("applicationDidReceiveMemoryWarning,triggerMemorywarning");
	}

	- (void)applicationWillTerminate:(UIApplication*)application {
		Inl_Application(_host)->triggerUnload();
		Qk_DLog("applicationWillTerminate,triggerUnload");
	}

	- (void) mailComposeController:(MFMailComposeViewController*)controller
						didFinishWithResult:(MFMailComposeResult)result
													error:(NSError*)error {
		[controller dismissViewControllerAnimated:YES completion:nil];
		Qk_DLog("mailComposeController");
	}

@end


// ***************** A p p l i c a t i o n *****************

static NSArray<NSString*>* split_ns_array(cString& str) {
	auto arr = [NSMutableArray<NSString*> new];
	for (auto& i : str.split(','))
		[arr addObject: [NSString stringWithUTF8String:i.c_str()]];
	return arr;
}

void Application::openURL(cString& url) {
	NSURL* url_ = [NSURL URLWithString:[NSString stringWithUTF8String:*url]];
	dispatch_async(dispatch_get_main_queue(), ^{
		[qkappdelegate.app openURL:url_ options:@{} completionHandler:nil];
	});
}

void Application::sendEmail(cString& recipient,
														cString& subject,
														cString& cc, cString& bcc, cString& body) {
	auto delegate = getActiveDelegate();
	if (!delegate) return;
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
		mail.mailComposeDelegate = qkappdelegate;

		[delegate presentViewController:mail animated:YES completion:nil];
	});
}

// ***************** E v e n t . D i s p a t c h *****************

void EventDispatch::setVolumeUp() {
	// TODO ..
}

void EventDispatch::setVolumeDown() {
	// TODO ..
}

void EventDispatch::setImeKeyboardCanBackspace(bool can_backspace, bool can_delete) {
	auto delegate = window()->impl()->delegate();
	dispatch_async(dispatch_get_main_queue(), ^{
		[delegate.ime set_keyboard_can_backspace:can_backspace can_delete:can_delete];
	});
}

void EventDispatch::setImeKeyboardOpen(KeyboardOptions options) {
	auto delegate = window()->impl()->delegate();
	dispatch_async(dispatch_get_main_queue(), ^{
		[delegate.ime set_keyboard_type:options.type];
		[delegate.ime set_keyboard_return_type:options.return_type];
		[delegate.ime activate: options.is_clear];
	});
}

void EventDispatch::setImeKeyboardClose() {
	auto delegate = window()->impl()->delegate();
	dispatch_async(dispatch_get_main_queue(), ^{
		[delegate.ime deactivate];
	});
}

void EventDispatch::setImeKeyboardSpotRect(Rect rect) {
}
