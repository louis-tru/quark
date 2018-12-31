/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright (c) 2015, xuewen.chu
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of xuewen.chu nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL xuewen.chu BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * ***** END LICENSE BLOCK ***** */

#import "ios-ime-helper-1.h"
#import "ngui/utils/util.h"
#import "ngui/event.h"
#import "ngui/app-1.h"

using namespace ngui;
using namespace ngui;

@interface IOSTextPosition: UITextPosition
@property (assign, nonatomic) NSUInteger value;
@end

@implementation IOSTextPosition
+ (id)new { return [IOSTextPosition new:0]; }
+ (id)new:(NSUInteger)value {
	IOSTextPosition* obj = [[IOSTextPosition alloc] init];
	obj.value = value;
	return obj;
}
@end

@interface IOSTextRange: UITextRange<NSCopying> {
	@private
	NSUInteger _start;
	NSUInteger _end;
}
@end

@implementation IOSTextRange

+ (id)new {
	return [IOSTextRange new:[IOSTextPosition new] end:[IOSTextPosition new]];
}

+ (id)new:(IOSTextPosition*)start end:(IOSTextPosition*)end {
	IOSTextRange* range = [[IOSTextRange alloc] init];
	range->_start = start.value;
	range->_end = end.value;
	return range;
}

+ (id)new_with_uint:(NSUInteger)start end:(NSUInteger)end {
	IOSTextRange* range = [[IOSTextRange alloc] init];
	range->_start = start;
	range->_end = end;
	return range;
}

- (BOOL)isEmpty {
	return _end == _start;
}

- (UITextPosition*)start {
	return [IOSTextPosition new:_start];
}

- (UITextPosition*)end {
	return [IOSTextPosition new:_end];
}

- (id)copyWithZone:(NSZone*)zone {
	IOSTextRange* copy = [[[self class] allocWithZone: zone] init];
	copy->_start = _start; copy->_end = _end;
	return copy;
}

@end


@interface IOSIMEHelprt() {
	@private
	NSString* _marked_text;
	UITextInputStringTokenizer* _tokenizer;
	BOOL _can_backspace;
	AppInl* _app;
	uint16  _keyboard_up_keycode;
	BOOL _clearing;
	bool _has_open;
}
@end

@implementation IOSIMEHelprt

#pragma mark UITextInputTraits protocol
@synthesize autocapitalizationType;
@synthesize autocorrectionType;
@synthesize keyboardType;
@synthesize keyboardAppearance;
@synthesize returnKeyType;
@synthesize spellCheckingType;
@synthesize enablesReturnKeyAutomatically;

#pragma mark UITextInput protocol
@synthesize selectedTextRange;
@synthesize markedTextRange;
@synthesize markedTextStyle;
@synthesize beginningOfDocument;
@synthesize endOfDocument;
@synthesize inputDelegate;
@synthesize tokenizer;

- (id)initWithApplication:(GUIApplication*)app {
	self = [super initWithFrame:CGRectMake(0, -1000, 0, 0)];
	if (self) {
		self.autocapitalizationType = UITextAutocapitalizationTypeNone;
		self.autocorrectionType = UITextAutocorrectionTypeNo;
		self.keyboardType = UIKeyboardTypeDefault;
		self.returnKeyType = UIReturnKeyDefault;
		self.backgroundColor = [UIColor whiteColor];
		_marked_text = @"";
		_tokenizer = [[UITextInputStringTokenizer alloc] initWithTextInput:self];
		_can_backspace = NO;
		_app = _inl_app(app);
		_keyboard_up_keycode = 0;
		_clearing = NO;
		_has_open = NO;
		
		NSNotificationCenter* notification = [NSNotificationCenter defaultCenter];
		
		[notification addObserver:self
										 selector:@selector(UIKeyboardWillShowNotification:)
												 name:UIKeyboardWillShowNotification
											 object:nil];
		[notification addObserver:self
										 selector:@selector(UIKeyboardDidShowNotification:)
												 name:UIKeyboardDidShowNotification
											 object:nil];
		[notification addObserver:self
										 selector:@selector(UIKeyboardWillHideNotification:)
												 name:UIKeyboardWillHideNotification
											 object:nil];
		[notification addObserver:self
										 selector:@selector(UIKeyboardDidHideNotification:)
												 name:UIKeyboardDidHideNotification
											 object:nil];
		[notification addObserver:self
										 selector:@selector(UITextInputCurrentInputModeDidChangeNotification:)
												 name:UITextInputCurrentInputModeDidChangeNotification
											 object:nil];
	}
	return self;
}

- (void)dealloc {
	[[NSNotificationCenter defaultCenter] removeObserver:self];
}

- (void)UIKeyboardWillShowNotification:(NSNotification*)note {
	
}

- (void)UIKeyboardDidShowNotification:(NSNotification*)note {
	
}

- (void)UIKeyboardWillHideNotification:(NSNotification*)note {
	
}

- (void)UIKeyboardDidHideNotification:(NSNotification*)note {
	
}

- (void)UITextInputCurrentInputModeDidChangeNotification:(NSNotification*)note {
	
}

- (void)open {
	_has_open = YES;
	[self becomeFirstResponder];
}
- (void)close {
	_has_open = NO;
	[self resignFirstResponder];
}

- (void)clear {
	_marked_text = @"";
	_keyboard_up_keycode = 0;
	_clearing = YES;
	if ( self.isFirstResponder ) {
		[self resignFirstResponder];
		[self becomeFirstResponder];
	}
	_clearing = NO;
}

- (void)set_keyboard_can_backspace:(bool)can_backspace can_delete:(bool)can_delete {
	_can_backspace = can_backspace;
}

- (void)set_keyboard_type:(KeyboardType)type {
		UIKeyboardType type2 = UIKeyboardTypeDefault;
		switch ( type ) {
			default: break;
			case KeyboardType::ASCII: type2 = UIKeyboardTypeASCIICapable; break;
			case KeyboardType::NUMBER: type2 = UIKeyboardTypeNumbersAndPunctuation; break;
			case KeyboardType::URL: type2 = UIKeyboardTypeURL; break;
			case KeyboardType::NUMBER_PAD: type2 = UIKeyboardTypeNumberPad; break;
			case KeyboardType::PHONE: type2 = UIKeyboardTypePhonePad; break;
			case KeyboardType::NAME_PHONE: type2 = UIKeyboardTypeNamePhonePad; break;
			case KeyboardType::EMAIL: type2 = UIKeyboardTypeEmailAddress; break;
			case KeyboardType::DECIMAL: type2 = UIKeyboardTypeDecimalPad; break;
			case KeyboardType::TWITTER: type2 = UIKeyboardTypeTwitter; break;
			case KeyboardType::SEARCH: type2 = UIKeyboardTypeWebSearch; break;
			case KeyboardType::ASCII_NUMBER: type2 = UIKeyboardTypeASCIICapableNumberPad; break;
		}
		self.keyboardType = type2;
}

- (void)set_keyboard_return_type:(KeyboardReturnType)type {
	UIReturnKeyType type2 = UIReturnKeyDefault;
	switch ( type ) {
		default: break;
		case KeyboardReturnType::GO: type2 = UIReturnKeyGo; break;
		//case KeyboardReturnType::GOOGLE: type2 = UIReturnKeyGoogle; break;
		case KeyboardReturnType::JOIN: type2 = UIReturnKeyJoin; break;
		case KeyboardReturnType::NEXT: type2 = UIReturnKeyNext; break;
		case KeyboardReturnType::ROUTE: type2 = UIReturnKeyRoute; break;
		case KeyboardReturnType::SEARCH: type2 = UIReturnKeySearch; break;
		case KeyboardReturnType::SEND: type2 = UIReturnKeySend; break;
		//case KeyboardReturnType::YAHOO: type2 = UIReturnKeyYahoo; break;
		case KeyboardReturnType::DONE: type2 = UIReturnKeyDone; break;
		case KeyboardReturnType::EMERGENCY: type2 = UIReturnKeyEmergencyCall; break;
		case KeyboardReturnType::CONTINUE: type2 = UIReturnKeyContinue; break;
	}
	self.returnKeyType = type2;
}

- (BOOL)canBecomeFirstResponder {
	return YES;
}

- (BOOL)hasText {
	return _marked_text.length > 0;
}

- (void)insertText:(NSString*)text {
	if ( text.length == 1 && _marked_text.length == 0 ) {
		uint16 keycode = [text characterAtIndex:0];
		if ( _keyboard_up_keycode == 0 ) {
			_app->dispatch()->keyboard_adapter()->dispatch(keycode, 1, 1, 0, -1, 0);
		} else {
			XX_ASSERT( keycode == _keyboard_up_keycode );
		}
		_app->dispatch()->dispatch_ime_insert([text UTF8String]);
		_app->dispatch()->keyboard_adapter()->dispatch(keycode, 1, 0, 0, -1, 0);
		_keyboard_up_keycode = 0;
	} else {
		_app->dispatch()->dispatch_ime_insert([text UTF8String]);
	}
}

- (void)deleteBackward {
	_keyboard_up_keycode = 0;
	_app->dispatch()->keyboard_adapter()->dispatch(KEYCODE_BACK_SPACE, 1, 1, 0, -1, 0);
	_app->dispatch()->dispatch_ime_delete(-1);
	_app->dispatch()->keyboard_adapter()->dispatch(KEYCODE_BACK_SPACE, 1, 0, 0, -1, 0);
}

- (NSString*)textInRange:(UITextRange*)range {
	return _marked_text;
}

- (void)replaceRange:(UITextRange*)range withText:(NSString*)text {
	
}

- (void)setSelectedTextRange:(UITextRange*)aSelectedTextRange {
	
}

- (UITextRange*)selectedTextRange {
	return [IOSTextRange new];
}

- (void)setMarkedTextStyle:(NSDictionary*)style {
	
}

- (NSDictionary*)markedTextStyle {
	return nil;
}

- (UITextRange*)markedTextRange {
	if ( _marked_text.length == 0 ) {
		return nil;
	}
	return [IOSTextRange new];
}

- (void)setMarkedText:(NSString*)markedText selectedRange:(NSRange)selectedRange {
	
	if ( !_clearing ) {
		_marked_text = markedText;
		_app->dispatch()->dispatch_ime_marked([_marked_text UTF8String]);
		
		if ( _keyboard_up_keycode ) {
			_app->dispatch()->keyboard_adapter()->dispatch(_keyboard_up_keycode, 1, 0, 0, -1, 0);
			_keyboard_up_keycode = 0;
		}
	}
}

- (void)unmarkText {
	if ( !_clearing ) {
		_app->dispatch()->dispatch_ime_unmark([_marked_text UTF8String]);
	}
	_marked_text = @"";
}

- (UITextPosition*)beginningOfDocument {
	return [IOSTextPosition new];
}

- (UITextPosition*)endOfDocument {
	return [IOSTextPosition new:_marked_text.length];
}

- (UITextRange*)textRangeFromPosition:(UITextPosition*)fromPosition
													 toPosition:(UITextPosition*)toPosition {
	return [IOSTextRange new:(IOSTextPosition*)fromPosition end:(IOSTextPosition*)toPosition];
}

- (UITextPosition*)positionFromPosition:(UITextPosition*)position
																 offset:(NSInteger)offset {
	IOSTextPosition* p = (IOSTextPosition*)position;
	return [IOSTextPosition new:p.value + offset];
}

- (UITextPosition*)positionFromPosition:(UITextPosition*)position
														inDirection:(UITextLayoutDirection)direction
																 offset:(NSInteger)offset {
	return [IOSTextPosition new];
}

- (NSComparisonResult)comparePosition:(UITextPosition*)position
													 toPosition:(UITextPosition*)other {
	return _can_backspace ? NSOrderedAscending: NSOrderedSame;
}

- (NSInteger)offsetFromPosition:(UITextPosition*)from
										 toPosition:(UITextPosition*)toPosition {
	NSUInteger a = [(IOSTextPosition*)from value];
	NSUInteger b = [(IOSTextPosition*)toPosition value];
	NSInteger result = b - a;
	return result;
}

- (id<UITextInputTokenizer>)tokenizer {
	return _tokenizer;
}

- (UITextPosition*)positionWithinRange:(UITextRange *)range
									 farthestInDirection:(UITextLayoutDirection)direction {
	return nil;
}

- (UITextRange*)characterRangeByExtendingPosition:(UITextPosition *)position
																			inDirection:(UITextLayoutDirection)direction; {
	return nil;
}

- (UITextWritingDirection)baseWritingDirectionForPosition:(UITextPosition *)position
																							inDirection:(UITextStorageDirection)direction; {
	return UITextWritingDirectionLeftToRight;
}

- (void)setBaseWritingDirection:(UITextWritingDirection)writingDirection
											 forRange:(UITextRange*)range {
	
}

- (::CGRect)firstRectForRange:(UITextRange*)range {
	return CGRectZero;
}

- (::CGRect)caretRectForPosition:(UITextPosition*)position {
	return CGRectZero;
}

- (NSArray*)selectionRectsForRange:(UITextRange*)range{
	return nil;
}

- (UITextPosition*)closestPositionToPoint:(CGPoint)point {
	return [IOSTextPosition new];
}

- (UITextPosition*)closestPositionToPoint:(CGPoint)point withinRange:(UITextRange*)range {
	return range.start;
}

- (UITextRange*)characterRangeAtPoint:(CGPoint)point {
	return [IOSTextRange new];
}

- (BOOL)shouldChangeTextInRange:(UITextRange*)range replacementText:(NSString*)text {
	
	_keyboard_up_keycode = 0;
	
	if ( text ) {
		if ( text.length == 1 ) {
			uint16 keycode = [text characterAtIndex:0];
			
			if ( ![text isEqualToString:_marked_text] ) {
				_keyboard_up_keycode = keycode;
				_app->dispatch()->keyboard_adapter()->dispatch(keycode, 1, 1, 0, -1, 0);
			}
		}
	}
	return YES;
}

@end
