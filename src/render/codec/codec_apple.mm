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
#if Qk_MacOS
#import <AppKit/AppKit.h>
#define UIImage NSImage
#else
#import <UIKit/UIKit.h>
#endif
#include "../font/priv/util.h"

namespace qk {

	bool mac_img_decode1(cBuffer& data, Array<Pixel> *out) {
		NSData* nsData = [NSData dataWithBytesNoCopy:(void*)*data length:data.length() freeWhenDone:NO];
		UIImage* img = [[UIImage alloc] initWithData:nsData];
#if Qk_MacOS
		CGImageRef image = [img CGImageForProposedRect:nil context:nil hints:nil];
#else
		CGImageRef image = [img CGImage];
#endif
		if (!image) return false;

		int width = (int)CGImageGetWidth(image);
		int height = (int)CGImageGetHeight(image);
		int rowBytes = width * 4;
		ColorType colorType;
		AlphaType alphaType = kUnpremul_AlphaType;
		CGImageAlphaInfo alpha, inAlpha = CGImageGetAlphaInfo(image);

		switch (inAlpha) {
			case kCGImageAlphaNone:               /* For example, RGB. */
			case kCGImageAlphaNoneSkipLast:       /* For example, RGBX. */
			case kCGImageAlphaNoneSkipFirst:      /* For example, XRGB. */
			case kCGImageAlphaOnly:               /* No color data, alpha data only */
				alpha = kCGImageAlphaNoneSkipLast;
				colorType = kRGB_888X_ColorType;
				alphaType = kOpaque_AlphaType;
				break;
			case kCGImageAlphaPremultipliedLast:  /* For example, premultiplied RGBA */
			case kCGImageAlphaPremultipliedFirst: /* For example, premultiplied ARGB */
			case kCGImageAlphaLast:               /* For example, non-premultiplied RGBA */
			case kCGImageAlphaFirst:              /* For example, non-premultiplied ARGB */
				alpha = kCGImageAlphaPremultipliedLast;
				colorType = kRGBA_8888_ColorType;
				alphaType = kPremul_AlphaType;
				break;
		}
		CGColorSpaceRef space;
		if (!(space = CGImageGetColorSpace(image))) return false;
		CGBitmapInfo cgInfo = kCGBitmapByteOrder32Big | alpha;
		Buffer pixel(rowBytes * height);
		memset(*pixel, 0, pixel.length()); // reset storage
		QkUniqueCFRef<CGContextRef> ctx(CGBitmapContextCreate(*pixel, width, height, 8, rowBytes, space, cgInfo));
		CGContextDrawImage(ctx.get(), CGRectMake(0, 0, width, height), image);
		PixelInfo info(width, height, colorType, alphaType);
		out->push(Pixel(info, pixel));
		return true;
	}

	bool mac_img_decode(cBuffer& data, Array<Pixel> *out) {
		NSData* nsData = [NSData dataWithBytesNoCopy:(void*)*data length:data.length() freeWhenDone:NO];
		UIImage* img = [[UIImage alloc] initWithData:nsData];
#if Qk_MacOS
		CGImageRef image = [img CGImageForProposedRect:nil context:nil hints:nil];
#else
		CGImageRef image = [img CGImage];
#endif
		if (!image) return false;
		QkUniqueCFRef<CFDataRef> rawData(CGDataProviderCopyData(CGImageGetDataProvider(image)));
		if (!rawData) return false;

		int width  = (int)CGImageGetWidth(image);
		int height = (int)CGImageGetHeight(image);
		int bytesPerPixel = (int)CGImageGetBitsPerPixel(image) / 8;
		int rowBytes = (int)CGImageGetBytesPerRow(image);
		if (rowBytes == 0)
			rowBytes = width * bytesPerPixel;

		ColorType colorType;
		AlphaType alphaType;
		// CGColorSpaceRef space = CGImageGetColorSpace(image);
		// if (!space) return false;
		// CGColorSpaceModel model = CGColorSpaceGetModel(space);
		// CGBitmapInfo bitmapInfo = CGImageGetBitmapInfo(image);

		if (bytesPerPixel == 1) { // 单通道灰度
			colorType = kGray_8_ColorType;
			alphaType = kOpaque_AlphaType;
		} else if (bytesPerPixel == 2) { // 灰度 + alpha
			// Qk_ASSERT_EQ(space, kCGColorSpaceModelMonochrome, "invalid color space for 2 bytes per pixel");
			colorType = kLuminance_Alpha_88_ColorType; 
			alphaType = kUnpremul_AlphaType;
		} else if (bytesPerPixel == 4) {
			CGImageAlphaInfo alphaInfo = CGImageGetAlphaInfo(image);
			switch (alphaInfo) {
				// case kCGImageAlphaOnly:               /* No color data, alpha data only */
				case kCGImageAlphaNone:               /* For example, RGB. */
				case kCGImageAlphaNoneSkipLast:       /* For example, RGBX. */
				case kCGImageAlphaNoneSkipFirst:      /* For example, XRGB. */
					colorType = kRGB_888X_ColorType;    // no alpha，RGBX
					alphaType = kOpaque_AlphaType;
					break;
				case kCGImageAlphaPremultipliedLast:  /* For example, premultiplied RGBA */
				case kCGImageAlphaPremultipliedFirst: /* For example, premultiplied ARGB */
				case kCGImageAlphaLast:               /* For example, non-premultiplied RGBA */
				case kCGImageAlphaFirst:              /* For example, non-premultiplied ARGB */
					colorType = kRGBA_8888_ColorType;   // have alpha，RGBA
					alphaType = kUnpremul_AlphaType;
					break;
				default:
					return false; // error
			}
		} else { // 3 字节模式暂不支持
			return false; // error
		}

		const UInt8* src = CFDataGetBytePtr(rawData.get());
		Buffer dst(rowBytes * height);
		memcpy(dst.val(), src, dst.length());

		PixelInfo info(width, height, colorType, alphaType);
		out->push(Pixel(info, dst));
		return true;
	}

	bool mac_img_test(cBuffer& data, PixelInfo* out) {
		NSData* nsData = [NSData dataWithBytesNoCopy:(void*)*data length:data.length() freeWhenDone:NO];
		UIImage* img = [[UIImage alloc] initWithData:nsData];
	#if Qk_MacOS
		CGImageRef image = [img CGImageForProposedRect:nil context:nil hints:nil];
	#else
		CGImageRef image = [img CGImage];
	#endif
		if (!image) return false;

		int width = (int)CGImageGetWidth(image);
		int height = (int)CGImageGetHeight(image);
		int bytesPerPixel = (int)CGImageGetBitsPerPixel(image) / 8;

		ColorType colorType;
		AlphaType alphaType;

		if (bytesPerPixel == 1) { // 单通道灰度
			colorType = kGray_8_ColorType;
			alphaType = kOpaque_AlphaType;
		} else if (bytesPerPixel == 2) { // 灰度 + alpha
			colorType = kLuminance_Alpha_88_ColorType; 
			alphaType = kUnpremul_AlphaType;
		} else if (bytesPerPixel == 4) {
			CGImageAlphaInfo alphaInfo = CGImageGetAlphaInfo(image);
			switch (alphaInfo) {
				// case kCGImageAlphaOnly:               /* No color data, alpha data only */
				case kCGImageAlphaNone:               /* For example, RGB. */
				case kCGImageAlphaNoneSkipLast:       /* For example, RGBX. */
				case kCGImageAlphaNoneSkipFirst:      /* For example, XRGB. */
					colorType = kRGB_888X_ColorType;    // no alpha，RGBX
					alphaType = kOpaque_AlphaType;
					break;
				case kCGImageAlphaPremultipliedLast:  /* For example, premultiplied RGBA */
				case kCGImageAlphaPremultipliedFirst: /* For example, premultiplied ARGB */
				case kCGImageAlphaLast:               /* For example, non-premultiplied RGBA */
				case kCGImageAlphaFirst:              /* For example, non-premultiplied ARGB */
					colorType = kRGBA_8888_ColorType;   // have alpha，RGBA
					alphaType = kUnpremul_AlphaType;
					break;
				default:
					return false; // error
			}
		} else { // 3 字节模式暂不支持
			return false; // error
		}

		*out = PixelInfo(width, height, colorType, alphaType);
		return true;
	}

}
