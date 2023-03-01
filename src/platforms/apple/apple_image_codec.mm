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

#import "../../render/codec/codec.h"
#import <Foundation/Foundation.h>
#import <CoreGraphics/CoreGraphics.h>

#if Qk_OSX
# import <AppKit/AppKit.h>
# define UIImage NSImage
#else
# import <UIKit/UIKit.h>
#endif

namespace qk {

	bool apple_img_decode(cBuffer& data, Array<Pixel> *out) {

		NSData* nsdata = [NSData dataWithBytesNoCopy:(void*)*data
																					length:data.length()
																		freeWhenDone:NO];
		UIImage* img = [[UIImage alloc] initWithData:nsdata];
	#if Qk_OSX
		CGImageRef image = [img CGImageForProposedRect:nil context:nil hints:nil];
	#else 
		CGImageRef image = [img CGImage];
	#endif

		if (image) {
			CGColorSpaceRef color_space = CGImageGetColorSpace(image);
			if (color_space) {
				int width = (int)CGImageGetWidth(image);
				int height = (int)CGImageGetHeight(image);
				int pixel_size = width * height * 4;

				CGImageAlphaInfo info = kCGImageAlphaLast | kCGBitmapByteOrder32Big;
				// info = CGImageGetAlphaInfo(image);

				auto pixel_data = Buffer::alloc(pixel_size);
				color_space = CGColorSpaceCreateDeviceRGB();
				CGContextRef ctx =
				CGBitmapContextCreate(*pixel_data, width, height, 8, width * 4, color_space, info);
				CGContextDrawImage(ctx, CGRectMake(0, 0, width, height), image);
				CGContextRelease(ctx);
				CFRelease(color_space);

				PixelInfo info(width, height, kColor_Type_RGBA_8888, kAlphaType_Unpremul);

				out->push(Pixel(info, pixel_data));

				return true;
			}
		}

		return false;
	}

	bool apple_img_test(cBuffer& data, PixelInfo* out) {
		
		NSData* nsdata = [NSData dataWithBytesNoCopy:(void*)*data
																					length:data.length()
																		freeWhenDone:NO];
		UIImage* img = [[UIImage alloc] initWithData:nsdata];
	#if Qk_OSX
		CGImageRef image = [img CGImageForProposedRect:nil context:nil hints:nil];
	#else
		CGImageRef image = [img CGImage];
	#endif
		
		if (!image)
			return false;

		int width = (int)CGImageGetWidth(image);
		int height = (int)CGImageGetHeight(image);

		*out = PixelInfo(width, height, kColor_Type_RGBA_8888, kAlphaType_Unpremul);

		return true;
	}

}