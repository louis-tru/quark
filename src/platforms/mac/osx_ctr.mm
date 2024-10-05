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
#import "../../ui/app.h"
#import "../../ui/window.h"
#import "../../ui/event.h"
#import "./mac_app.h"

using namespace qk;

@interface QkRootViewController() {
	@private
	id _mouseMovedId;
	id _keyDownId;
}
@end

@implementation QkRootViewController

- (void)doCommandBySelector:(SEL)selector {
	// [super doCommandBySelector:selector];
}

- (BOOL) acceptsFirstResponder {
	return YES;
}

- (Vec2)location:(NSEvent *)e {
	auto scale = self.win->defaultScale() / self.win->scale();
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
		e.deltaX,e.deltaY,self.win->defaultScale(),self.win->scale()
	);*/
}
- (void)viewDidAppear {
	auto _uiwin = self.win->impl()->delegate().uiwin;
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
	_win->dispatch()->onMousepress(KEYCODE_MOUSE_LEFT, true, &pos);
}
- (void)rightMouseDown:(NSEvent *)e{
	[self LogMouse:"rightMouseDown" event:e];
	auto pos = [self location:e];
	_win->dispatch()->onMousepress(KEYCODE_MOUSE_RIGHT, true, &pos);
}
- (void)otherMouseDown:(NSEvent *)e{
	[self LogMouse:"otherMouseDown" event:e];
	auto pos = [self location:e];
	_win->dispatch()->onMousepress(KEYCODE_MOUSE_CENTER, true, &pos);
}
- (void)mouseUp:(NSEvent *)e{
	[self LogMouse:"mouseUp" event:e];
	auto pos = [self location:e];
	_win->dispatch()->onMousepress(KEYCODE_MOUSE_LEFT, false, &pos);
}
- (void)rightMouseUp:(NSEvent *)e{
	[self LogMouse:"rightMouseUp" event:e];
	auto pos = [self location:e];
	_win->dispatch()->onMousepress(KEYCODE_MOUSE_RIGHT, false, &pos);
}
- (void)otherMouseUp:(NSEvent *)e{
	[self LogMouse:"otherMouseUp" event:e];
	auto pos = [self location:e];
	_win->dispatch()->onMousepress(KEYCODE_MOUSE_CENTER, false, &pos);
}
- (void)_mouseMoved:(NSEvent *)e{
	[self LogMouse:"mouseMoved" event:e];
	auto pos = [self location:e];
	_win->dispatch()->onMousemove(pos.x(), pos.y());
}
- (void)mouseDragged:(NSEvent *)e{
	[self LogMouse:"mouseDragged" event:e];
	auto pos = [self location:e];
	_win->dispatch()->onMousemove(pos.x(), pos.y());
}
- (void)rightMouseDragged:(NSEvent *)e{
	[self LogMouse:"rightMouseDragged" event:e];
	auto pos = [self location:e];
	_win->dispatch()->onMousemove(pos.x(), pos.y());
}
- (void)otherMouseDragged:(NSEvent *)e{
	[self LogMouse:"otherMouseDragged" event:e];
	auto pos = [self location:e];
	_win->dispatch()->onMousemove(pos.x(), pos.y());
}
- (void)scrollWheel:(NSEvent *)e{
	Qk_DLog("scrollWheel,type:%d,modifierFlags:%d,delta:%f %f", e.type, e.modifierFlags, e.deltaX,e.deltaY);
	auto delta = Vec2(e.deltaX,e.deltaY);
	_win->dispatch()->onMousepress(KEYCODE_MOUSE_WHEEL, true, &delta);
}
- (void)_keyDown:(NSEvent *)e {
	//NSLog(@"keyDown,%@", e);
	BOOL isCapsLock = e.modifierFlags & NSEventModifierFlagCapsLock;
	_win->dispatch()->keyboard()->dispatch(e.keyCode, false, true, isCapsLock, e.ARepeat, -1, 0);
}
- (void)keyUp:(NSEvent *)e {
	//NSLog(@"keyUp,%@", e);
	BOOL isCapsLock = e.modifierFlags & NSEventModifierFlagCapsLock;
	_win->dispatch()->keyboard()->dispatch(e.keyCode, false, false, isCapsLock, 0, -1, 0);
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
	_win->dispatch()->keyboard()->dispatch(e.keyCode, false, isDown, isCapsLock, false, -1, 0);
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

@end
