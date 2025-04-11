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

#ifndef __quark__apple__apple_app__
#define __quark__apple__apple_app__
#include "../../render/apple/apple_render.h"
#include "../../ui/types.h"

#if Qk_APPLE
@class QkWindowDelegate;
namespace qk {
	class Application;
	class Window;
	class WindowImpl {
	public:
		QkWindowDelegate* delegate();
	};
}

/**
 * @protocol QkIMEHelprt
*/
@protocol QkIMEHelprt<NSObject>
- (void)activate:(bool)clear;
- (void)deactivate;
- (void)set_keyboard_can_backspace:(bool)can_backspace
												can_delete:(bool)can_delete;
- (void)set_keyboard_type:(qk::KeyboardType)type;
- (void)set_keyboard_return_type:(qk::KeyboardReturnType)type;
- (void)set_spot_rect:(qk::Rect)rect;
- (UIView*)view; // ime view
@end

/**
 * @interface QkWindowDelegate
*/
@interface QkWindowDelegate: UIViewController
#if Qk_MacOS
<NSWindowDelegate>
#endif
@property (assign, nonatomic) qk::Window *qkwin;
@property (strong, nonatomic) UIWindow   *uiwin;
@property (strong, nonatomic) id<QkIMEHelprt> ime;
@end

/**
 * @interface QkApplicationDelegate
*/
@interface QkApplicationDelegate:
#if Qk_MacOS
NSObject<UIApplicationDelegate>
#else
UIResponder<UIApplicationDelegate>
#endif
@property (assign, nonatomic, readonly) qk::Application *host;
@property (assign, nonatomic, readonly) UIApplication *app;
@end

id<QkIMEHelprt> qk_make_ime_helper(qk::Window *win);

#endif // #if Qk_APPLE
#endif
