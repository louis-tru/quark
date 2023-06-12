// @private head
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

#include "quark/util/macros.h"
#if Qk_APPLE
#import "../../app.h"
#import "../../render/render.h"
#if Qk_OSX
#import <AppKit/AppKit.h>
#define UIResponder NSResponder
#define UIApplicationDelegate NSApplicationDelegate
#define UIWindow NSWindow
#define UIKit NSView
#define UIView NSView
#define CGRect NSRect
#define UIApplication NSApplication
#define UIColor NSColor
#define UIScreen NSScreen
#define UIViewController NSViewController
#else
#import <UIKit/UIKit.h>
#endif

class QkAppleRender {
public:
	virtual UIView* make_surface_view(CGRect rect) = 0;
	virtual qk::Render* render() = 0;
};

@protocol QkIMEHelprt<NSObject>
	- (void)open;
	- (void)close;
	- (void)clear;
	- (void)set_keyboard_can_backspace:(bool)can_backspace
													can_delete:(bool)can_delete;
	- (void)set_keyboard_type:(qk::KeyboardType)type;
	- (void)set_keyboard_return_type:(qk::KeyboardReturnType)type;
	- (UIView*) view; // only ios return view
@end

QkAppleRender*  qk_make_apple_render(qk::Render::Options opts);
id<QkIMEHelprt> qk_ime_helper_new(qk::Application *host);

@interface QkRootViewController: UIViewController
@end

@interface QkApplicationDelegate: UIResponder<UIApplicationDelegate>
	@property (assign, nonatomic, readonly) UIApplication *app; // strong
	@property (assign, nonatomic, readonly) qk::Application *host;
	@property (assign, nonatomic, readonly) QkAppleRender *render;
	@property (strong, nonatomic) QkRootViewController *root_ctr;
	@property (strong, nonatomic) UIWindow *window;
	@property (strong, nonatomic) UIView *surface_view; // strong
	@property (strong, nonatomic) id<QkIMEHelprt> ime; // strong
@end

#endif
