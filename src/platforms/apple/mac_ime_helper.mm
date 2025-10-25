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

#import "./apple_app.h"
#import "../../util/util.h"
#import "../../ui/event.h"
#import "../../ui/app.h"
#import "../../ui/window.h"
#import <AppKit/AppKit.h>

using namespace qk;

//#define Qk_NSLog(...) NSLog(__VA_ARGS__)
#define Qk_NSLog(...)

static const NSRange kEmptyRange = {NSNotFound, 0};

@interface QkMacIMEHelprt: NSView<QkIMEHelprt,NSTextInputClient,NSTextInput> {
	@private
	Window      *_win;
	BOOL         _isOpen;
	CGRect       _spot_rect;
	NSRange      _selectedRange, _markedRange;
}
@property (nonatomic, strong) NSString *markedText;
- (id)initIME:(Window*)win;
@end

@implementation QkMacIMEHelprt

- (id)initIME:(Window*)win {
	self = [super initWithFrame:CGRectZero];
	if (self) {
		_win = win;
		_isOpen = NO;
		_selectedRange = _markedRange = kEmptyRange;
		self.markedText = @"";
	}
	return self;
}

// ----------------------------------------------------------------
// QkIMEHelprt
// ----------------------------------------------------------------
- (void)activate:(bool)isClear {
	if (isClear) {
		[self removeMarkedText];
	}
	if (!_isOpen) {
		_isOpen = YES;
		[self.inputContext activate];
		[self becomeFirstResponder];
	}
}

- (void)deactivate {
	if (_isOpen) {
		_isOpen = NO;
		[self removeMarkedText];
		[self resignFirstResponder];
		[self.inputContext deactivate];
	}
}

- (void)set_keyboard_can_backspace:(bool)can_backspace can_delete:(bool)can_delete {
}

- (void)set_keyboard_type:(KeyboardType)type {
}

- (void)set_keyboard_return_type:(KeyboardReturnType)type {
}

- (void)set_spot_rect:(qk::Rect)rect {
	auto uiwin = _win->impl()->delegate().uiwin;
	auto offset = [uiwin convertRectToScreen:NSZeroRect].origin;
	//Qk_NSLog(@"set_ime_keyboard_spot_location %f,%f,%f,%f", rect.origin.x, rect.origin.y, rect.size.width, rect.size.height);
	auto begin = rect.begin;
	auto win_h = uiwin.frame.size.height;
	auto x = begin.x() + offset.x;
	auto y = win_h - begin.y() + offset.y - rect.size.y() - 30;
	_spot_rect = CGRectMake(x, y, rect.size.x(), rect.size.y());
}

- (UIView*)view {
	return self;
}

- (void) removeMarkedText {
	if (_markedText.length) {
		_selectedRange = _markedRange = kEmptyRange;
		self.markedText = @"";
	}
}

// ----------------------------------------------------------------
// NSResponder
// ----------------------------------------------------------------
- (BOOL) acceptsFirstResponder {
	return YES;
}

- (void) keyDown: (NSEvent *) e {
	if (_isOpen) {
		[self interpretKeyEvents: @[e]];
		Qk_NSLog(@"keyDown keyCode,%@,%d", e.characters, e.keyCode);
	}
}

- (void)deleteForward:(nullable id)sender {
	_win->dispatch()->onImeDelete(1);
	Qk_NSLog(@"deleteForward");
}

- (void) deleteBackward: (nullable id) sender {
	_win->dispatch()->onImeDelete(-1);
	Qk_NSLog(@"deleteBackward");
}

- (void) insertNewline: (id) sender {
	[self insertText: @"\n" replacementRange:kEmptyRange];
}

- (void) insertTab: (id) sender {
	[self insertText: @"\t" replacementRange:kEmptyRange];
}

- (void) insertText: (id) aString {
	[self insertText: aString replacementRange:kEmptyRange];
}

// ----------------------------------------------------------------
// NSTextInputClient (Mac OS X 10.5)
// ----------------------------------------------------------------
- (void) doCommandBySelector: (SEL) aSelector { // copy,cut..
	[super doCommandBySelector: aSelector];
}

- (void) insertText: (NSString*) aString replacementRange: (NSRange) replacementRange {
	if (_markedRange.length) {
		_win->dispatch()->onImeUnmark(aString.UTF8String);
		[self removeMarkedText];
	} else {
		_win->dispatch()->onImeInsert(aString.UTF8String);
	}
	Qk_NSLog(@"insertText,%@", aString);
}

- (void) setMarkedText: (id) aString
					selectedRange: (NSRange) selectedRange
			replacementRange: (NSRange) replacementRange
{
	if ([aString isKindOfClass: [NSAttributedString class]]) {
		NSAttributedString *str = aString;
		self.markedText = str.string;
	} else {
		self.markedText = [NSString stringWithFormat:@"%@", aString];
	}

	_selectedRange = selectedRange;
	_markedRange = selectedRange.location == NSNotFound ? kEmptyRange: NSRange{0,_markedText.length};

	Qk_NSLog(@"setMarkedText,%@", _markedText);

	_win->dispatch()->onImeMarked(_markedText.UTF8String);

	if ([aString length] == 0) {
		[self removeMarkedText];
	}
}

- (void) unmarkText {
	_win->dispatch()->onImeUnmark("");
	[self removeMarkedText];
	Qk_NSLog(@"unmarkText");
}

- (BOOL) hasMarkedText {
	return _markedRange.length;
}

- (NSRange) selectedRange {
	return _selectedRange;
}

- (NSRange) markedRange {
	return _markedRange;
}

- (NSAttributedString *) attributedSubstringForProposedRange: (NSRange) aRange
																									actualRange: (NSRangePointer) actualRange
{
  return [[NSAttributedString alloc] init];
}

- (NSRect) firstRectForCharacterRange: (NSRange) aRange
													actualRange: (NSRangePointer) actualRange
{
	return _spot_rect;
}

- (NSUInteger) characterIndexForPoint: (NSPoint) thePoint {
	return 0;
}

- (NSArray *) validAttributesForMarkedText {
	return @[];
}

// ----------------------------------------------------------------
// NSTextInput
// ----------------------------------------------------------------
- (void) setMarkedText: (id) aString selectedRange: (NSRange) selRange {
	[self setMarkedText: aString selectedRange: selRange replacementRange: kEmptyRange];
}

- (NSInteger) conversationIdentifier {
	return (NSInteger) self;
}

- (NSAttributedString *) attributedSubstringFromRange: (NSRange) theRange {
	return [self attributedSubstringForProposedRange:theRange actualRange:NULL];
}

- (NSRect) firstRectForCharacterRange: (NSRange) theRange {
	return [self firstRectForCharacterRange:theRange actualRange:NULL];
}
@end


id<QkIMEHelprt> qk_make_ime_helper(Window *win) {
	return [[QkMacIMEHelprt alloc] initIME:win];
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
	post_messate_main(Cb([options,delegate](auto e) {
		[delegate.ime set_keyboard_type:options.type];
		[delegate.ime set_keyboard_return_type:options.return_type];
		[delegate.ime activate: options.clear];
		[delegate.ime set_spot_rect:options.spot_rect];
	}), false);
}

void EventDispatch::setImeKeyboardClose() {
	auto delegate = window()->impl()->delegate();
	post_messate_main(Cb([delegate](auto e) {
		[delegate.ime deactivate];
	}), false);
}

void EventDispatch::setImeKeyboardSpotRect(Rect rect) {
	auto delegate = window()->impl()->delegate();
	post_messate_main(Cb([delegate,rect](auto e) {
		[delegate.ime set_spot_rect:rect];
	}), false);
}
