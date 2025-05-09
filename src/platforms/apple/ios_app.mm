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

#import "../../ui/ui.h"
#import "../../ui/window.h"
#import "./apple_app.h"
#import <MessageUI/MFMailComposeViewController.h>

using namespace qk;

// ***************** Q k . A p p l i c a t i o n . D e l e g a t e *****************

QkApplicationDelegate* qkappdelegate = nil;

QkWindowDelegate* getWindowDelegate() {
	auto qkwin = qkappdelegate.host->activeWindow();
	if (qkwin)
		return qkwin->impl()->delegate();
	return nil;
}

@interface QkApplicationDelegate()<MFMailComposeViewControllerDelegate>
@end

@implementation QkApplicationDelegate

	- (void) initPlatform:(AppInl*)host {
		_host = host;
		host->triggerLoad();
	}

	- (AppInl*) hostInl {
		return Inl_Application(_host);
	}

	- (void) openSystemSettings {
		NSURL *url = [NSURL URLWithString:UIApplicationOpenSettingsURLString];
		if ([_app canOpenURL:url]) {
			[_app openURL:url options:@{} completionHandler:^(BOOL ok){}];
		}
	}

	- (BOOL)application:(UIApplication*)app didFinishLaunchingWithOptions:(NSDictionary*)options {
		Qk_ASSERT(!qkappdelegate);
		qkappdelegate = self;
		_app = app;
		return YES;
	}

	- (void)application:(UIApplication*)app didChangeStatusBarFrame:(CGRect)frame {
	}

	- (void)applicationWillResignActive:(UIApplication*) application {
		if (!_host) return;
		self.hostInl->triggerPause();
		Qk_DLog("applicationWillResignActive,triggerPause");
	}

	- (void)applicationDidBecomeActive:(UIApplication*) application {
		if (!_host) return;
		self.hostInl->triggerResume();
		auto win = _host->activeWindow();
		if (win)
			win->render()->reload();
		Qk_DLog("applicationDidBecomeActive,triggerResume");
	}

	- (void)applicationDidEnterBackground:(UIApplication*) application {
		if (!_host) return;
		auto win = _host->activeWindow();
		if (win)
			self.hostInl->triggerBackground(win);
		Qk_DLog("applicationDidEnterBackground,triggerBackground");
	}

	- (void)applicationWillEnterForeground:(UIApplication*) application {
		if (!_host) return;
		auto win = _host->activeWindow();
		if (win)
			self.hostInl->triggerForeground(win);
		Qk_DLog("applicationWillEnterForeground,triggerForeground");
	}

	- (void)applicationDidReceiveMemoryWarning:(UIApplication*) application {
		if (!_host) return;
		self.hostInl->triggerMemorywarning();
		Qk_DLog("applicationDidReceiveMemoryWarning,triggerMemorywarning");
	}

	- (void)applicationWillTerminate:(UIApplication*)application {
		if (!_host) return;
		self.hostInl->triggerUnload();
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

static NSArray<NSString*>* toNSArray(cString& str) {
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
	auto delegate = getWindowDelegate();
	if (!delegate) return;
	id recipient_ = toNSArray(recipient);
	id subject_ = [NSString stringWithUTF8String:*subject];
	id cc_ = toNSArray(cc);
	id bcc_ = toNSArray(bcc);
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

void AppInl::initPlatform() {
	[qkappdelegate initPlatform: this];
}
