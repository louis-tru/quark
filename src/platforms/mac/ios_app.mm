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

#import "../../util/loop.h"
#import "../../app.h"
#import "../../event.h"
#import "../../display.h"
#import "./mac_app.h"

using namespace qk;

// ***************** Q k . A p p l i c a t i o n . D e l e g a t e *****************

QkApplicationDelegate *__app = nil; //

@implementation QkApplicationDelegate

	- (void)refresh_status {
		if ( self.window.rootViewController == self.root_ctr ) {
			self.window.rootViewController = nil;
			self.window.rootViewController = self.root_ctr;
		}
	}

	- (BOOL)application:(UIApplication*)app didFinishLaunchingWithOptions:(NSDictionary*)options {
		Qk_ASSERT(!__app);
		Qk_ASSERT(Application::shared());
		__app = self;
		_host = Application::shared();
		_app = app;
		_render = dynamic_cast<QkMacRender*>(_host->render());

		//[app setStatusBarStyle:UIStatusBarStyleLightContent];
		//[app setStatusBarHidden:NO];
		_is_background = NO;

		self.setting_orientation = Orientation::ORIENTATION_USER;
		self.current_orientation = Orientation::ORIENTATION_INVALID;
		self.visible_status_bar = YES;
		self.status_bar_style = UIStatusBarStyleDefault;

		self.window = [[UIWindow alloc] initWithFrame:[[UIScreen mainScreen] bounds]];
		self.root_ctr = [[QkRootViewController alloc] init];
		self.window.backgroundColor = [UIColor blackColor];
		self.window.rootViewController = self.root_ctr;

		[self.window makeKeyAndVisible];

		UIView *rootView = self.window.rootViewController.view;

		self.surface_view = self.render->make_surface_view(rootView.bounds);
		self.surface_view.contentScaleFactor = UIScreen.mainScreen.scale;
		self.surface_view.translatesAutoresizingMaskIntoConstraints = NO;
		self.surface_view.multipleTouchEnabled = YES;
		self.surface_view.userInteractionEnabled = YES;

		self.ime = qk_ime_helper_new(_host);

		[rootView addSubview:self.surface_view];
		[rootView addSubview:self.ime.view];
		[rootView addConstraint:[NSLayoutConstraint
												constraintWithItem:self.surface_view
												attribute:NSLayoutAttributeWidth
												relatedBy:NSLayoutRelationEqual
												toItem:rootView
												attribute:NSLayoutAttributeWidth
												multiplier:1
												constant:0]];
		[rootView addConstraint:[NSLayoutConstraint
												constraintWithItem:self.surface_view
												attribute:NSLayoutAttributeHeight
												relatedBy:NSLayoutRelationEqual
												toItem:rootView
												attribute:NSLayoutAttributeHeight
												multiplier:1
												constant:0]];

		self.host->render()->reload(); // set size

		Inl_Application(_host)->triggerLoad();
		Qk_DEBUG("application,triggerLoad");

		return YES;
	}

	- (void)application:(UIApplication*)app didChangeStatusBarFrame:(CGRect)frame {
		if ( __app && !_is_background ) {
			self.host->render()->reload(); // set size
		}
	}

	- (void)applicationWillResignActive:(UIApplication*) application {
		Inl_Application(_host)->triggerPause();
		Qk_DEBUG("applicationWillResignActive,triggerPause");
	}

	- (void)applicationDidBecomeActive:(UIApplication*) application {
		Inl_Application(_host)->triggerResume();
		self.host->render()->reload(); // set size
		Qk_DEBUG("applicationDidBecomeActive,triggerResume");
	}

	- (void)applicationDidEnterBackground:(UIApplication*) application {
		_is_background = YES;
		Inl_Application(_host)->triggerBackground();
		Qk_DEBUG("applicationDidEnterBackground,triggerBackground");
	}

	- (void)applicationWillEnterForeground:(UIApplication*) application {
		_is_background = NO;
		Inl_Application(_host)->triggerForeground();
		Qk_DEBUG("applicationWillEnterForeground,triggerForeground");
	}

	- (void)applicationDidReceiveMemoryWarning:(UIApplication*) application {
		Inl_Application(_host)->triggerMemorywarning();
		Qk_DEBUG("applicationDidReceiveMemoryWarning,triggerMemorywarning");
	}

	- (void)applicationWillTerminate:(UIApplication*)application {
		Inl_Application(_host)->triggerUnload();
		Qk_DEBUG("applicationWillTerminate,triggerUnload");
	}

	- (void) mailComposeController:(MFMailComposeViewController*)controller
						didFinishWithResult:(MFMailComposeResult)result
													error:(NSError*)error {
		[controller dismissViewControllerAnimated:YES completion:nil];
		Qk_DEBUG("mailComposeController");
	}

@end


// ***************** A p p l i c a t i o n *****************

static NSArray<NSString*>* split_ns_array(cString& str) {
	auto arr = [NSMutableArray<NSString*> new];
	for (auto& i : str.split(','))
		[arr addObject: [NSString stringWithUTF8String:i.c_str()]];
	return arr;
}

void Application::pending() {
	// exit(0);
}

void Application::open_url(cString& url) {
	NSURL* url_ = [NSURL URLWithString:[NSString stringWithUTF8String:*url]];
	dispatch_async(dispatch_get_main_queue(), ^{
		[__app.app openURL:url_ options:@{} completionHandler:nil];
	});
}

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
		mail.mailComposeDelegate = __app;
		[__app.root_ctr presentViewController:mail animated:YES completion:nil];
	});
}

// ***************** E v e n t . D i s p a t c h *****************

void EventDispatch::set_volume_up() {
	// TODO ..
}

void EventDispatch::set_volume_down() {
	// TODO ..
}

void EventDispatch::set_ime_keyboard_open(KeyboardOptions options) {
	dispatch_async(dispatch_get_main_queue(), ^{
		[__app.ime set_keyboard_type:options.type];
		[__app.ime set_keyboard_return_type:options.return_type];
		if ( options.is_clear ) {
			[__app.ime clear];
		}
		[__app.ime open];
	});
}

void EventDispatch::set_ime_keyboard_can_backspace(bool can_backspace, bool can_delete) {
	dispatch_async(dispatch_get_main_queue(), ^{
		[__app.ime set_keyboard_can_backspace:can_backspace can_delete:can_delete];
	});
}

void EventDispatch::set_ime_keyboard_close() {
	dispatch_async(dispatch_get_main_queue(), ^{
		[__app.ime close];
	});
}

void EventDispatch::set_ime_keyboard_spot_location(Vec2 location) {
	// noop
}
