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

#import "./codec.h"
#import <Foundation/Foundation.h>
#import <CoreGraphics/CoreGraphics.h>
#if Qk_OSX
#import <AppKit/AppKit.h>
#define UIImage NSImage
#else
#import <UIKit/UIKit.h>
#endif
#include "../font/util_mac.h"

namespace qk {

	bool mac_img_decode(cBuffer& data, Array<Pixel> *out) {
		NSData* nsData = [NSData dataWithBytesNoCopy:(void*)*data
																					length:data.length()
																		freeWhenDone:NO];
		UIImage* img = [[UIImage alloc] initWithData:nsData];
		CGColorSpaceRef space;

#if Qk_OSX
		CGImageRef image = [img CGImageForProposedRect:nil context:nil hints:nil];
#else
		CGImageRef image = [img CGImage];
#endif
		if (!image) return false;
		if (!(space = CGImageGetColorSpace(image))) return false;

		int width = (int)CGImageGetWidth(image);
		int height = (int)CGImageGetHeight(image);
		int pixel_size = width * height * 4;

		ColorType colorType;
		AlphaType alphaType;
		CGImageAlphaInfo cgAlpha;
		QkUniqueCFRef<CGColorSpaceRef> spaceHold;

		switch (CGImageGetAlphaInfo(image)) {
			case kCGImageAlphaPremultipliedLast:
			case kCGImageAlphaPremultipliedFirst:
			case kCGImageAlphaLast:
			case kCGImageAlphaFirst:
				colorType = kRGBA_8888_ColorType;
				alphaType = kPremul_AlphaType;
				cgAlpha = kCGImageAlphaPremultipliedLast;
				break;
			default:
				colorType = kRGB_888X_ColorType;
				alphaType = kUnpremul_AlphaType;
				cgAlpha = kCGImageAlphaNoneSkipLast;
				space = CGColorSpaceCreateDeviceRGB(); // create rgb
				spaceHold.reset(space);
				break;
		}

		CGBitmapInfo cgInfo = kCGBitmapByteOrder32Host | cgAlpha;
		auto pixel = Buffer::alloc(pixel_size);

		QkUniqueCFRef<CGContextRef> ctx(
			CGBitmapContextCreate(*pixel, width, height, 8, width * 4, space, cgInfo)
		);
		CGContextDrawImage(ctx.get(), CGRectMake(0, 0, width, height), image);

		PixelInfo info(width, height, kRGBA_8888_ColorType, kUnpremul_AlphaType);
		out->push(Pixel(info, pixel));

		return true;
	}

	bool mac_img_test(cBuffer& data, PixelInfo* out) {
		
		NSData* nsData = [NSData dataWithBytesNoCopy:(void*)*data
																					length:data.length()
																		freeWhenDone:NO];
		UIImage* img = [[UIImage alloc] initWithData:nsData];
	#if Qk_OSX
		CGImageRef image = [img CGImageForProposedRect:nil context:nil hints:nil];
	#else
		CGImageRef image = [img CGImage];
	#endif
		if (!image) return false;

		int width = (int)CGImageGetWidth(image);
		int height = (int)CGImageGetHeight(image);
		ColorType colorType;
		AlphaType alphaType;

		switch (CGImageGetAlphaInfo(image)) {
			case kCGImageAlphaPremultipliedLast:
			case kCGImageAlphaPremultipliedFirst:
			case kCGImageAlphaLast:
			case kCGImageAlphaFirst:
				colorType = kRGBA_8888_ColorType;
				alphaType = kPremul_AlphaType;
				break;
			default:
				colorType = kRGB_888X_ColorType;
				alphaType = kUnpremul_AlphaType;
				break;
		}

		*out = PixelInfo(width, height, colorType, alphaType);
		return true;
	}

}
