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

#import "qgr/image-codec.h"
#import <Foundation/Foundation.h>
#import <CoreGraphics/CoreGraphics.h>
#import <GLKit/GLKit.h>

#if XX_OSX
# import <AppKit/AppKit.h>
# define UIImage NSImage
#else
# import <UIKit/UIKit.h>
#endif

XX_NS(qgr)

static PixelData image_decode(cBuffer& data) {

	NSData* nsdata = [NSData dataWithBytesNoCopy:(void*)*data
                                        length:data.length()
                                  freeWhenDone:NO];
  UIImage* uiimg = [[UIImage alloc] initWithData:nsdata];
 #if XX_OSX
  CGImageRef image = [uiimg CGImageForProposedRect:nil context:nil hints:nil];
 #else
	CGImageRef image = [uiimg CGImage];
 #endif
  
	if (image) {
		CGColorSpaceRef color_space = CGImageGetColorSpace(image);
		if (color_space) {
			
			int width = (int)CGImageGetWidth(image);
			int height = (int)CGImageGetHeight(image);
			int pixel_size = width * height * 4;
			
			CGImageAlphaInfo info;
			PixelData::Format format;
			bool alpha = true;
			
			switch (CGImageGetAlphaInfo(image)) {
				case kCGImageAlphaPremultipliedLast:
				case kCGImageAlphaPremultipliedFirst:
				case kCGImageAlphaLast:
				case kCGImageAlphaFirst:
					format = PixelData::RGBA8888;
					break;
				default:
					format = PixelData::RGBX8888;
					alpha = false;
					break;
			}
			
			bool isPremultipliedAlpha;
			
			if (alpha) {
				info = kCGImageAlphaPremultipliedLast;
				isPremultipliedAlpha = true;
			} else {
				info = kCGImageAlphaNoneSkipLast;
				isPremultipliedAlpha = false;
			}
			
			Buffer pixel_data(pixel_size);
			color_space = CGColorSpaceCreateDeviceRGB();
			CGContextRef context =
			CGBitmapContextCreate(*pixel_data, width, height, 8,
														width * 4, color_space, info | kCGBitmapByteOrder32Big);
			CGContextDrawImage(context, CGRectMake(0, 0, width, height), image);
			CGContextRelease(context);
			CFRelease(color_space);
			return PixelData(pixel_data, width, height, PixelData::RGBA8888, isPremultipliedAlpha);
		}
	}
	return PixelData();
}

static PixelData image_decode_header(cBuffer& data) {
	
	NSData* nsdata = [NSData dataWithBytesNoCopy:(void*)*data
                                        length:data.length()
                                  freeWhenDone:NO];
  UIImage* uiimg = [[UIImage alloc] initWithData:nsdata];
 #if XX_OSX
	CGImageRef image = [uiimg CGImageForProposedRect:nil context:nil hints:nil];
 #else
	CGImageRef image = [uiimg CGImage];
 #endif
	
	if (image) {
		int width = (int)CGImageGetWidth(image);
		int height = (int)CGImageGetHeight(image);
		CGImageAlphaInfo alpha = CGImageGetAlphaInfo(image);
		PixelData::Format format;
		
		if (alpha == kCGImageAlphaPremultipliedLast ||
				alpha == kCGImageAlphaPremultipliedFirst ||
				alpha == kCGImageAlphaLast ||
				alpha == kCGImageAlphaFirst) {
			format = PixelData::RGBA8888;
		} else {
			format = PixelData::RGBX8888;
		}
		return PixelData(Buffer(), width, height, format, false);
	}
	return PixelData();
}

Array<PixelData> JPEGImageCodec::decode(cBuffer& data) {
	Array<PixelData> rv; rv.push(image_decode(data));
	return rv;
}

PixelData JPEGImageCodec::decode_header(cBuffer& data) {
	return image_decode_header(data);
}

Buffer JPEGImageCodec::encode(cPixelData& data) {
	XX_UNIMPLEMENTED();
	return Buffer();
}

Array<PixelData> GIFImageCodec::decode(cBuffer& data) {
	Array<PixelData> rv; rv.push(image_decode(data));
	return rv;
}

PixelData GIFImageCodec::decode_header(cBuffer& data) {
	return image_decode_header(data);
}

Buffer GIFImageCodec::encode(cPixelData& data) {
	XX_UNIMPLEMENTED();
	return Buffer();
}

Array<PixelData> PNGImageCodec::decode(cBuffer& data) {
	Array<PixelData> rv; rv.push(image_decode(data));
	return rv;
}

PixelData PNGImageCodec::decode_header(cBuffer& data) {
	return image_decode_header(data);
}

Buffer PNGImageCodec::encode(cPixelData& data) {
	XX_UNIMPLEMENTED();
	return Buffer();
}

Array<PixelData> WEBPImageCodec::decode(cBuffer& data) {
	Array<PixelData> rv; rv.push(image_decode(data));
	return rv;
}

PixelData WEBPImageCodec::decode_header(cBuffer& data) {
	return image_decode_header(data);
}

Buffer WEBPImageCodec::encode(cPixelData& data) {
	XX_UNIMPLEMENTED();
	return Buffer();
}

XX_END
