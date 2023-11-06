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

// @private head

#ifndef __quark__mac__mac_app__
#define __quark__mac__mac_app__
#include "../../render/render_mac.h"
#include "../../types.h"
#if Qk_MAC

@class QkWindowDelegate;

namespace qk {
	class Application;
	class WindowImpl {
	public:
		QkWindowDelegate* delegate();
		UIWindow*         window();
	};
}

@protocol QkIMEHelprt<NSObject>
	- (void)open;
	- (void)close;
	- (void)clear;
	- (void)set_keyboard_can_backspace:(bool)can_backspace
													can_delete:(bool)can_delete;
	- (void)set_keyboard_type:(qk::KeyboardType)type;
	- (void)set_keyboard_return_type:(qk::KeyboardReturnType)type;
	- (UIView*)view; // only ios return view
@end

id<QkIMEHelprt> qk_make_ime_helper(qk::Application *host);
void            qk_post_messate_sync_main(qk::Cb cb);

@interface QkRootViewController: UIViewController
@end

@interface QkWindowDelegate: NSObject
#if Qk_OSX
<NSWindowDelegate>
#endif
	@property (strong, nonatomic) UIWindow *window; // strong
	@property (strong, nonatomic) QkRootViewController *root_ctr;
@end

@interface QkApplicationDelegate: UIResponder<UIApplicationDelegate>
	@property (assign, nonatomic, readonly) UIApplication *app; // strong
	@property (assign, nonatomic, readonly) qk::Application *host;
	@property (strong, nonatomic) id<QkIMEHelprt> ime; // strong
@end

#endif // #if Qk_MAC

#if Qk_iOS
#include "../../display.h"
#import <MessageUI/MFMailComposeViewController.h>

typedef qk::Screen::Orientation Orientation;

@interface QkApplicationDelegate()<MFMailComposeViewControllerDelegate>
	{
		BOOL _is_background;
	}
	@property (assign, nonatomic) Orientation setting_orientation;
	@property (assign, nonatomic) Orientation current_orientation;
	@property (assign, nonatomic) bool        visible_status_bar;
	@property (assign, nonatomic) UIStatusBarStyle status_bar_style;
	// methods
	- (void)refresh_status;
@end
#endif // #if Qk_iOS

#endif
