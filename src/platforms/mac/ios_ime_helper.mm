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

#import "./mac_app.h"
#import "../../util/util.h"
#import "../../event.h"
#import "../../app.h"

using namespace qk;

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

@interface QkiOSIMEHelprt: UIView<UITextInput,QkIMEHelprt> {
	@private
	NSString*    _marked_text;
	UITextInputStringTokenizer* _tokenizer;
	Application* _host;
	uint16_t     _keyboard_up_keycode;
	BOOL         _can_backspace;
	BOOL         _clearing;
}
- (id)initIME:(Application*)host;
- (void)open;
- (void)close;
- (void)clear;
- (void)set_keyboard_can_backspace:(bool)can_backspace
												can_delete:(bool)can_delete;
- (void)set_keyboard_type:(KeyboardType)type;
- (void)set_keyboard_return_type:(KeyboardReturnType)type;
@end

@implementation QkiOSIMEHelprt
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

	- (id)initIME:(Application*)host {
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
			_host = host;
			_keyboard_up_keycode = 0;
			_clearing = NO;
			
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

	#pragma mark - QkIMEHelprt

	- (void)open {
		if ([self becomeFirstResponder]) {
			Qk_DEBUG("becomeFirstResponder ok");
		}
	}
	- (void)close {
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
			UIKeyboardType ktype = UIKeyboardTypeDefault;
			switch ( type ) {
				default: break;
				case KeyboardType::ASCII: ktype = UIKeyboardTypeASCIICapable; break;
				case KeyboardType::NUMBER: ktype = UIKeyboardTypeNumbersAndPunctuation; break;
				case KeyboardType::URL: ktype = UIKeyboardTypeURL; break;
				case KeyboardType::NUMBER_PAD: ktype = UIKeyboardTypeNumberPad; break;
				case KeyboardType::PHONE: ktype = UIKeyboardTypePhonePad; break;
				case KeyboardType::NAME_PHONE: ktype = UIKeyboardTypeNamePhonePad; break;
				case KeyboardType::EMAIL: ktype = UIKeyboardTypeEmailAddress; break;
				case KeyboardType::DECIMAL: ktype = UIKeyboardTypeDecimalPad; break;
				case KeyboardType::TWITTER: ktype = UIKeyboardTypeTwitter; break;
				case KeyboardType::SEARCH: ktype = UIKeyboardTypeWebSearch; break;
				case KeyboardType::ASCII_NUMBER: ktype = UIKeyboardTypeASCIICapableNumberPad; break;
			}
			self.keyboardType = ktype;
	}

	- (void)set_keyboard_return_type:(KeyboardReturnType)type {
		UIReturnKeyType ktype = UIReturnKeyDefault;
		switch ( type ) {
			default: break;
			case KeyboardReturnType::GO: ktype = UIReturnKeyGo; break;
			//case KeyboardReturnType::GOOGLE: ktype = UIReturnKeyGoogle; break;
			case KeyboardReturnType::JOIN: ktype = UIReturnKeyJoin; break;
			case KeyboardReturnType::NEXT: ktype = UIReturnKeyNext; break;
			case KeyboardReturnType::ROUTE: ktype = UIReturnKeyRoute; break;
			case KeyboardReturnType::SEARCH: ktype = UIReturnKeySearch; break;
			case KeyboardReturnType::SEND: ktype = UIReturnKeySend; break;
			//case KeyboardReturnType::YAHOO: ktype = UIReturnKeyYahoo; break;
			case KeyboardReturnType::DONE: ktype = UIReturnKeyDone; break;
			case KeyboardReturnType::EMERGENCY: ktype = UIReturnKeyEmergencyCall; break;
			case KeyboardReturnType::CONTINUE: ktype = UIReturnKeyContinue; break;
		}
		self.returnKeyType = ktype;
	}

	- (UIView*)view {
		return self;
	}

	#pragma mark - UITextInput

	- (BOOL)canBecomeFirstResponder {
		return YES;
	}

	- (BOOL)hasText {
		return _marked_text.length > 0;
	}

	- (void)insertText:(NSString*)text {
		if ( text.length == 1 && _marked_text.length == 0 ) {
			uint16_t keycode = [text characterAtIndex:0];
			if ( _keyboard_up_keycode == 0 ) {
				_host->dispatch()->keyboard()->onDispatch(keycode, 1, true/*down*/, 0, -1, 0);
			} else {
				Qk_ASSERT( keycode == _keyboard_up_keycode );
			}
			_host->dispatch()->onImeInsert([text UTF8String]);
			_host->dispatch()->keyboard()->onDispatch(keycode, 1, false/*up*/, 0, -1, 0);
			_keyboard_up_keycode = 0;
		} else {
			_host->dispatch()->onImeInsert([text UTF8String]);
		}
	}

	- (void)deleteBackward {
		_keyboard_up_keycode = 0;
		_host->dispatch()->keyboard()->onDispatch(KEYCODE_BACK_SPACE, 1, 1, 0, -1, 0);
		_host->dispatch()->onImeDelete(-1);
		_host->dispatch()->keyboard()->onDispatch(KEYCODE_BACK_SPACE, 1, 0, 0, -1, 0);
	}

	- (void)setMarkedText:(NSString*)markedText selectedRange:(NSRange)selectedRange {
		if ( _clearing ) return;
		_marked_text = markedText;
		_host->dispatch()->onImeMarked([_marked_text UTF8String]);

		if ( _keyboard_up_keycode ) {
			_host->dispatch()->keyboard()->onDispatch(_keyboard_up_keycode, 1, 0, 0, -1, 0);
			_keyboard_up_keycode = 0;
		}
	}

	- (void)unmarkText {
		if ( _clearing ) return;
		_host->dispatch()->onImeUnmark([_marked_text UTF8String]);
		_marked_text = @"";
	}

	- (BOOL)shouldChangeTextInRange:(UITextRange*)range replacementText:(NSString*)text {
		_keyboard_up_keycode = 0;
		if ( text ) {
			if ( text.length == 1 ) {
				uint16_t keycode = [text characterAtIndex:0];
				
				if ( ![text isEqualToString:_marked_text] ) {
					_keyboard_up_keycode = keycode;
					_host->dispatch()->keyboard()->onDispatch(keycode, 1, 1, 0, -1, 0);
				}
			}
		}
		return YES;
	}

	- (NSString*)textInRange:(UITextRange*)range {
		return _marked_text;
	}

	- (void)replaceRange:(UITextRange*)range withText:(NSString*)text {
	}

	- (UITextRange*)selectedTextRange {
		return [IOSTextRange new];
	}

	- (UITextRange*)markedTextRange {
		if ( _marked_text.length == 0 ) {
			return nil;
		}
		return [IOSTextRange new];
	}

	- (void)setSelectedTextRange:(UITextRange*)aSelectedTextRange {
	}

	- (NSDictionary*)markedTextSlant {
		return nil;
	}

	- (void)setMarkedTextSlant:(NSDictionary*)style {
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

	- (nullable UITextRange *)characterRangeByExtendingPosition:(nonnull UITextPosition *)position
																									inDirection:(UITextLayoutDirection)direction {
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

	- (nullable UITextRange *)characterRangeAtPoint:(CGPoint)point {
		return [IOSTextRange new];
	}

	- (void)encodeWithCoder:(nonnull NSCoder *)coder {
	}

	- (void)traitCollectionDidChange:(nullable UITraitCollection *)previousTraitCollection {
		Qk_DEBUG("traitCollectionDidChange");
	}

	- (CGPoint)convertPoint:(CGPoint)point fromCoordinateSpace:(nonnull id<UICoordinateSpace>)coordinateSpace {
		Qk_DEBUG("convertPoint:fromCoordinateSpace");
	}

	- (CGPoint)convertPoint:(CGPoint)point toCoordinateSpace:(nonnull id<UICoordinateSpace>)coordinateSpace {
		Qk_DEBUG("convertPoint:toCoordinateSpace");
	}

	- (CGRect)convertRect:(CGRect)rect fromCoordinateSpace:(nonnull id<UICoordinateSpace>)coordinateSpace {
		Qk_DEBUG("convertRect:fromCoordinateSpace");
	}

	- (CGRect)convertRect:(CGRect)rect toCoordinateSpace:(nonnull id<UICoordinateSpace>)coordinateSpace {
		Qk_DEBUG("convertRect:toCoordinateSpace");
	}

	- (void)didUpdateFocusInContext:(nonnull UIFocusUpdateContext *)context withAnimationCoordinator:(nonnull UIFocusAnimationCoordinator *)coordinator {
		Qk_DEBUG("didUpdateFocusInContext:withAnimationCoordinator");
	}

	- (void)setNeedsFocusUpdate {
		Qk_DEBUG("setNeedsFocusUpdate");
	}

	- (BOOL)shouldUpdateFocusInContext:(nonnull UIFocusUpdateContext *)context {
		Qk_DEBUG("shouldUpdateFocusInContext");
	}

	- (void)updateFocusIfNeeded {
		Qk_DEBUG("updateFocusIfNeeded");
	}

@end

id<QkIMEHelprt> qk_make_ime_helper(Application *host) {
	return [[QkiOSIMEHelprt alloc] initIME:host];
}