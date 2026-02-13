/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright (c) 2015, Louis.chu
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of Louis.chu nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL Louis.chu BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * ***** END LICENSE BLOCK ***** */

#import "../../ui/clipboard.h"
#import "./apple_app.h"
#import "../../ui/app.h"
#if Qk_MacOS
#define UIPasteboard NSPasteboard
#endif

namespace qk {

	String Clipboard::get_text() {
		@autoreleasepool {
			UIPasteboard *pb = [UIPasteboard generalPasteboard];
#if Qk_MacOS
			NSString *str = [pb stringForType:NSPasteboardTypeString];
	#else
			NSString *str = pb.string;
#endif
			if (!str)
				return String();
			return String([str UTF8String]);
		}
	}

	void Clipboard::set_text(cString& text) {
		@autoreleasepool {
			post_message_main(Cb([text](auto e) {
				UIPasteboard *pb = [UIPasteboard generalPasteboard];
				NSString *ns = [NSString stringWithUTF8String:text.c_str()];
#if Qk_MacOS
				[pb clearContents];
				[pb setString:ns forType:NSPasteboardTypeString];
	#else
				pb.string = ns;
#endif
			}), true);
		}
	}

	bool Clipboard::has_text() {
		@autoreleasepool {
			UIPasteboard *pb = [UIPasteboard generalPasteboard];
#if Qk_MacOS
			NSString *str = [pb stringForType:NSPasteboardTypeString];
	#else
			NSString *str = pb.string;
#endif
			return str != nil;
		}
	}

	void Clipboard::clear() {
		@autoreleasepool {
			post_message_main(Cb([](auto e) {
				UIPasteboard *pb = [UIPasteboard generalPasteboard];
#if Qk_MacOS
				[pb clearContents];
#else
				pb.string = @"";
#endif
			}), true);
		}
	}
}