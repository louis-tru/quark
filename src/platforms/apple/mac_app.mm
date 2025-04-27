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
#import "../../util/http.h"
#import "../../ui/ui.h"
#import "./apple_app.h"

using namespace qk;

// ***************** Q k . A p p l i c a t i o n . D e l e g a t e *****************

QkApplicationDelegate* qkappdelegate = nil;

@interface QkApplicationDelegate()
@end

@implementation QkApplicationDelegate

	- (void) initPlatform:(AppInl*)host {
	}

	- (AppInl*) hostInl {
		return Inl_Application(_host);
	}

- (void)applicationDidFinishLaunching:(NSNotification*) notification {
	Qk_ASSERT(!qkappdelegate);
	Qk_ASSERT(Application::shared());
	qkappdelegate = self;
	_host = Application::shared();
	_app = UIApplication.sharedApplication;
	[_app activateIgnoringOtherApps:YES];
	[_app setActivationPolicy:NSApplicationActivationPolicyRegular];
	Inl_Application(_host)->triggerLoad();
}

- (void)applicationWillResignActive:(NSNotification*)notification {
	self.hostInl->triggerPause();
	Qk_DLog("applicationWillResignActive, triggerPause");
}

- (void)applicationDidBecomeActive:(NSNotification*)notification {
	self.hostInl->triggerResume();
	Qk_DLog("applicationDidBecomeActive, triggerResume");
}

- (void)applicationDidHide:(NSNotification*)notification {
	Qk_DLog("applicationDidHide, onBackground");
}

- (void)applicationWillUnhide:(NSNotification*)notification {
	Qk_DLog("applicationWillUnhide, onForeground");
}

- (void)applicationWillTerminate:(NSNotification*)notification {
	self.hostInl->triggerUnload();
	Qk_DLog("applicationWillTerminate, triggerUnload");
}

- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication*)sender {
	Qk_DLog("applicationShouldTerminate");
	return NSTerminateNow;
}

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication*)sender {
	Qk_DLog("applicationShouldTerminateAfterLastWindowClosed, exit application");
	return YES;
}

- (BOOL)applicationShouldHandleReopen:(NSApplication*)sender hasVisibleWindows:(BOOL)flag {
	Qk_DLog("applicationShouldHandleReopen");
	return YES;
}

- (BOOL)applicationSupportsSecureRestorableState:(NSApplication *)app {
	Qk_DLog("applicationSupportsSecureRestorableState");
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
	NSString *url = [NSString stringWithFormat:@"mailto:%s?cc=%s&bcc=%s&subject=%s&body=%s",
		*recipient,
		*URI::encode(cc, true),
		*URI::encode(bcc, true),
		*URI::encode(subject, true), *URI::encode(body, true)
	];
	[[NSWorkspace sharedWorkspace] openURL:[NSURL URLWithString:url]];
}

void AppInl::initPlatform() {
	[qkappdelegate initPlatform: this];
}
