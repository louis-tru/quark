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

#import <AppKit/AppKit.h>
#import "../../ui/ui.h"
#import "../../render/apple/apple_render.h"
#import "./apple_app.h"

using namespace qk;

// ***************** W i n d o w *****************

QkWindowDelegate* WindowImpl::delegate() {
	return ((__bridge QkWindowDelegate*)(this));
}

@interface QkWindowDelegate() {
	@private
	id _mouseMovedId;
	id _keyDownId;
}
@property (assign, atomic) BOOL isClose;
@end

@implementation QkWindowDelegate

- (id) init:(Window::Options&)opts win:(Window*)win render:(Render*)render {
	if ( !(self = [super init]) )
		return nil;

	NSWindowStyleMask style = NSWindowStyleMaskBorderless |
		NSWindowStyleMaskTitled | NSWindowStyleMaskClosable |
		NSWindowStyleMaskResizable | NSWindowStyleMaskMiniaturizable;
	UIScreen* screen = UIScreen.mainScreen;
	//CGFloat scale = screen.backingScaleFactor;

	float w = opts.frame.size.x() > 0 ?
		opts.frame.size.x(): screen.frame.size.width / 2;
	float h = opts.frame.size.y() > 0 ?
		opts.frame.size.y(): screen.frame.size.height / 2;
	float x = opts.frame.origin.x() > 0 ?
		opts.frame.origin.x(): (screen.frame.size.width - w) / 2.0;
	float y = opts.frame.origin.y() > 0 ?
		opts.frame.origin.y(): (screen.frame.size.height - h) / 2.0;

	NSRect rect = NSMakeRect(x, y, w, h);

	UIWindow *uiwin = [[UIWindow alloc] initWithContentRect:rect
																						styleMask:style
																							backing:NSBackingStoreBuffered
																								defer:NO
																								screen:screen];
	uiwin.title = [NSString stringWithUTF8String:opts.title.c_str()];
	uiwin.delegate = self;

	self.isClose = NO;
	self.qkwin = win;
	self.uiwin = uiwin;
	self.uiwin.contentViewController = self;
	self.ime = qk_make_ime_helper(win);

	if (opts.frame.origin.x() < 0 && opts.frame.origin.y() < 0) {
		[uiwin center];
	}

	[uiwin setFrame:rect display:NO];

	UIView *rootView = self.view;
	UIView *view = render->surface()->surfaceView();

	view.frame = rootView.bounds;
	view.translatesAutoresizingMaskIntoConstraints = NO;

	[rootView addSubview:view];
	[rootView addSubview:self.ime.view];
	[rootView addConstraint:[NSLayoutConstraint
													constraintWithItem:view
													attribute:NSLayoutAttributeWidth
													relatedBy:NSLayoutRelationEqual
													toItem:rootView
													attribute:NSLayoutAttributeWidth
													multiplier:1
													constant:0]];
	[rootView addConstraint:[NSLayoutConstraint
													constraintWithItem:view
													attribute:NSLayoutAttributeHeight
													relatedBy:NSLayoutRelationEqual
													toItem:rootView
													attribute:NSLayoutAttributeHeight
													multiplier:1
													constant:0]];
	return self;
}

// UIViewController
// -------------------------------------------------------------------------

//- (void)doCommandBySelector:(SEL)selector {
//	[super doCommandBySelector:selector];
//}

- (BOOL) acceptsFirstResponder {
	return YES;
}

- (Vec2)location:(NSEvent *)e {
	auto scale = self.qkwin->defaultScale() / self.qkwin->scale();
	return Vec2(e.locationInWindow.x,e.window.contentView.frame.size.height-e.locationInWindow.y) * scale;
}

- (void) LogMouse:(const char*)name event:(NSEvent *)e {
	/*
		Qk_DLog("%s,type:%d,modifierFlags:%d,eventNumber:%d,\
pressure:%f,locationInWindow:%f %f,delta:%f %f,defaultScale:%f,scale:%f\
",
		name, e.type,
		e.modifierFlags,
		e.eventNumber,
		e.pressure,
		e.locationInWindow.x,e.locationInWindow.y,
		e.deltaX,e.deltaY,self.qkwin->defaultScale(),self.qkwin->scale()
	);*/
}

- (void)viewDidAppear {
	auto _uiwin = self.qkwin->impl()->delegate().uiwin;
	_mouseMovedId = [NSEvent addLocalMonitorForEventsMatchingMask:NSEventMaskMouseMoved handler:^NSEvent *(NSEvent *event) {
		if (event.window == _uiwin) {
			[self _mouseMoved:event];
		}
		return event;
	}];
	_keyDownId = [NSEvent addLocalMonitorForEventsMatchingMask:NSEventMaskKeyDown | NSEventMaskFlagsChanged handler:^NSEvent *(NSEvent *e) {
		if (e.window == _uiwin) {
			if (e.type == NSEventTypeFlagsChanged) {
				[self _flagsChanged:e];
			} else {
				[self _keyDown:e];
			}
		}
		return e;
	}];
}

-(void)viewDidDisappear {
	[NSEvent removeMonitor:_mouseMovedId];
	[NSEvent removeMonitor:_keyDownId];
}

- (void)mouseDown:(NSEvent *)e{
	[self LogMouse:"mouseDown" event:e];
	auto pos = [self location:e];
	_qkwin->dispatch()->onMousepress(KEYCODE_MOUSE_LEFT, true, &pos);
}

- (void)rightMouseDown:(NSEvent *)e{
	[self LogMouse:"rightMouseDown" event:e];
	auto pos = [self location:e];
	_qkwin->dispatch()->onMousepress(KEYCODE_MOUSE_RIGHT, true, &pos);
}

- (void)otherMouseDown:(NSEvent *)e{
	[self LogMouse:"otherMouseDown" event:e];
	auto pos = [self location:e];
	_qkwin->dispatch()->onMousepress(KEYCODE_MOUSE_CENTER, true, &pos);
}

- (void)mouseUp:(NSEvent *)e{
	[self LogMouse:"mouseUp" event:e];
	auto pos = [self location:e];
	_qkwin->dispatch()->onMousepress(KEYCODE_MOUSE_LEFT, false, &pos);
}

- (void)rightMouseUp:(NSEvent *)e{
	[self LogMouse:"rightMouseUp" event:e];
	auto pos = [self location:e];
	_qkwin->dispatch()->onMousepress(KEYCODE_MOUSE_RIGHT, false, &pos);
}

- (void)otherMouseUp:(NSEvent *)e{
	[self LogMouse:"otherMouseUp" event:e];
	auto pos = [self location:e];
	_qkwin->dispatch()->onMousepress(KEYCODE_MOUSE_CENTER, false, &pos);
}

- (void)_mouseMoved:(NSEvent *)e{
	[self LogMouse:"mouseMoved" event:e];
	auto pos = [self location:e];
	_qkwin->dispatch()->onMousemove(pos.x(), pos.y());
}

- (void)mouseDragged:(NSEvent *)e{
	[self LogMouse:"mouseDragged" event:e];
	auto pos = [self location:e];
	_qkwin->dispatch()->onMousemove(pos.x(), pos.y());
}

- (void)rightMouseDragged:(NSEvent *)e{
	[self LogMouse:"rightMouseDragged" event:e];
	auto pos = [self location:e];
	_qkwin->dispatch()->onMousemove(pos.x(), pos.y());
}

- (void)otherMouseDragged:(NSEvent *)e{
	[self LogMouse:"otherMouseDragged" event:e];
	auto pos = [self location:e];
	_qkwin->dispatch()->onMousemove(pos.x(), pos.y());
}

- (void)scrollWheel:(NSEvent *)e{
	Qk_DLog("scrollWheel,type:%d,modifierFlags:%d,delta:%f %f", e.type, e.modifierFlags, e.deltaX,e.deltaY);
	KeyboardKeyCode code = e.deltaX != 0 ?
		e.deltaX > 0 ? KEYCODE_MOUSE_WHEEL_RIGHT: KEYCODE_MOUSE_WHEEL_LEFT:
		e.deltaY > 0 ? KEYCODE_MOUSE_WHEEL_UP: KEYCODE_MOUSE_WHEEL_DOWN;
	Vec2 delta(e.deltaX, e.deltaY);
	_qkwin->dispatch()->onMousepress(code, true, &delta);
}

- (void)_keyDown:(NSEvent *)e {
	//NSLog(@"keyDown,%@", e);
	BOOL isCapsLock = e.modifierFlags & NSEventModifierFlagCapsLock;
	_qkwin->dispatch()->keyboard()->dispatch(e.keyCode, false, true, isCapsLock, e.ARepeat, -1, 0);
}

- (void)keyUp:(NSEvent *)e {
	//NSLog(@"keyUp,%@", e);
	BOOL isCapsLock = e.modifierFlags & NSEventModifierFlagCapsLock;
	_qkwin->dispatch()->keyboard()->dispatch(e.keyCode, false, false, isCapsLock, 0, -1, 0);
}

- (void)_flagsChanged:(NSEvent *)e{
	//NSLog(@"flagsChanged,%@", e);

	auto flags = e.modifierFlags & NSEventModifierFlagDeviceIndependentFlagsMask;
	BOOL isCapsLock = flags & NSEventModifierFlagCapsLock;
	BOOL isDown;

	switch (e.keyCode) {
		case 55: // Command
		case 54: // Command right
			isDown = flags & NSEventModifierFlagCommand; break;
		case 57: // CapsLock
			isDown = isCapsLock; break;
		case 56: // shift
		case 60: // right shift
			isDown = flags & NSEventModifierFlagShift; break;
		case 58: // Option
		case 61: // right Option
			isDown = flags & NSEventModifierFlagOption; break;
		case 59: // Control
		case 62: // right Control
			isDown = flags & NSEventModifierFlagControl; break;
		case 63: // Function
			isDown = flags & NSEventModifierFlagFunction; break;
		case 114: // help
			isDown = flags & NSEventModifierFlagHelp; break;
		default: return;
	}
	_qkwin->dispatch()->keyboard()->dispatch(e.keyCode, false, isDown, isCapsLock, false, -1, 0);
}

//  - (void)mouseEntered:(NSEvent *)e{
//  	[self LogMouse:"mouseEntered" event:e];
//  }
//  - (void)mouseExited:(NSEvent *)e{
//  	[self LogMouse:"mouseExited" event:e];
//  }
// - (void)tabletPoint:(NSEvent *)e{
// 	[self LogMouse:"tabletPoint" event:e];
// }
// - (void)tabletProximity:(NSEvent *)e{
// 	[self LogMouse:"tabletProximity" event:e];
// }

// NSWindowDelegate
// -------------------------------------------------------------------------

- (void)windowWillClose:(NSNotification *)notification {
	//NSWindow *closingWindow = notification.object;
}

- (BOOL)windowShouldClose:(NSWindow*)sender {
	if (!self.isClose) {
		self.isClose = YES;
		self.qkwin->host()->loop()->post(Cb([self](auto&e){
			self.qkwin->close(); // close destroy window
		}));
	}
	return YES;
}

- (NSSize)windowWillResize:(NSWindow*)sender toSize:(NSSize)size {
	return size;
}

- (void)windowDidMiniaturize:(NSNotification*)notification {
	Inl_Application(self.qkwin->host())->triggerBackground(self.qkwin);
	Qk_DLog("windowDidMiniaturize, triggerBackground");
}

- (void)windowDidDeminiaturize:(NSNotification*)notification {
	Inl_Application(self.qkwin->host())->triggerForeground(self.qkwin);
	Qk_DLog("windowDidDeminiaturize,triggerForeground");
}

@end

void Window::openImpl(Options &opts) {
	post_messate_main(Cb([&opts,this](auto e) {
		auto impl = [[QkWindowDelegate alloc]
								 init:opts win:this render:_render];
		CFBridgingRetain(impl);
		_impl = (__bridge WindowImpl*)impl;
		set_backgroundColor(opts.backgroundColor);
		activate();
	}), true);
}

void Window::closeImpl() {
	post_messate_main(Cb([this](auto e) {
		Qk_ASSERT_NE(_impl, nullptr);
		if (_impl && !_impl->delegate().isClose) {
			[_impl->delegate().uiwin close]; /* close platform window */
		}
		CFBridgingRelease(_impl);
		_impl = nullptr;
	}, this), false);
}

void Window::beforeClose() {}

float Window::getDefaultScale() {
	return _impl->delegate().uiwin.backingScaleFactor;
}

Region Window::getDisplayRegion(Vec2 size) {
	return {{0}, size};
}

void Window::afterDisplay() {
	// Noop
}

void Window::set_backgroundColor(Color val) {
	post_messate_main(Cb([this,val](auto e) {
		if (!_impl) return;
		auto color = val.to_color4f();
		_impl->delegate().uiwin.backgroundColor =
			[UIColor colorWithSRGBRed:color.r() green:color.g() blue:color.b() alpha:color.a()];
	}, this), false);
	_backgroundColor = val;
}

void Window::activate() {
	post_messate_main(Cb([this](auto e) {
		if (!_impl) return;
		[_impl->delegate().uiwin makeKeyAndOrderFront:nil];
	}, this), false);
	Inl_Application(_host)->setActiveWindow(this);
}

void Window::pending() {
	// Noop
}

void Window::setFullscreen(bool fullscreen) {
	post_messate_main(Cb([this,fullscreen](auto e) {
		if (!_impl) return;
		auto uiwin = _impl->delegate().uiwin;
		auto screenSize = uiwin.screen.frame.size;
		auto size = uiwin.frame.size;
		if (screenSize.width == size.width && screenSize.height == size.height) {
			if (!fullscreen) [uiwin toggleFullScreen:nil];
		} else {
			if (fullscreen) [uiwin toggleFullScreen:nil];
		}
	}, this), false);
}

void Window::setCursorStyle(CursorStyle cursor, bool isBase) {
	static CursorStyle current_cursor_base = CursorStyle::Arrow;
	static CursorStyle current_cursor_user = CursorStyle::Normal;

	if (isBase) {
		current_cursor_base = cursor;
	} else {
		current_cursor_user = cursor;
	}
	cursor = current_cursor_user == CursorStyle::Normal ? current_cursor_base: current_cursor_user;

	if (cursor == CursorStyle::None) {
		[NSCursor hide];
	} else if (cursor == CursorStyle::NoneUntilMouseMoves) {
		[NSCursor setHiddenUntilMouseMoves:YES];
	} else {
		switch (cursor) {
			case CursorStyle::Normal:
			case CursorStyle::Arrow: [NSCursor.arrowCursor set]; break;
			case CursorStyle::Ibeam: [NSCursor.IBeamCursor set]; break;
			case CursorStyle::PointingHand: [NSCursor.pointingHandCursor set]; break;
			case CursorStyle::ClosedHand: [NSCursor.closedHandCursor set]; break;
			case CursorStyle::OpenHand: [NSCursor.openHandCursor set]; break;
			case CursorStyle::ResizeLeft: [NSCursor.resizeLeftCursor set]; break;
			case CursorStyle::ResizeRight: [NSCursor.resizeRightCursor set]; break;
			case CursorStyle::ResizeLeftRight: [NSCursor.resizeLeftRightCursor set]; break;
			case CursorStyle::ResizeUp: [NSCursor.resizeUpCursor set]; break;
			case CursorStyle::ResizeDown: [NSCursor.resizeDownCursor set]; break;
			case CursorStyle::ResizeUpDown: [NSCursor.resizeUpDownCursor set]; break;
			case CursorStyle::Crosshair: [NSCursor.crosshairCursor set]; break;
			case CursorStyle::DisappearingItem: [NSCursor.disappearingItemCursor set]; break;
			case CursorStyle::OperationNotAllowed: [NSCursor.operationNotAllowedCursor set]; break;
			case CursorStyle::DragLink: [NSCursor.dragLinkCursor set]; break;
			case CursorStyle::DragCopy: [NSCursor.dragCopyCursor set]; break;
			case CursorStyle::ContextualMenu: [NSCursor.contextualMenuCursor set]; break;
			case CursorStyle::IbeamForVertical: [NSCursor.IBeamCursorForVerticalLayout set]; break;
			case CursorStyle::Cross: [NSCursor.crosshairCursor set]; break;
			default: break;
		}
		[NSCursor unhide];
	}
}
