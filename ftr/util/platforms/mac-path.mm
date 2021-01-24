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

#include "ftr/util/io/fs-path.h"
#include <Foundation/Foundation.h>
#if FX_IOS
# import <UIKit/UIKit.h>
#else
# import <AppKit/AppKit.h>
#endif

namespace ftr {

	String Path::executable() {
		static cString path( format([[[NSBundle mainBundle] executablePath] UTF8String]) );
		return path;
	}

	SString Path::documents(cString& child) {
		static SString path(
			Path::format([NSSearchPathForDirectoriesInDomains(
				NSDocumentDirectory,
				NSUserDomainMask,
				YES
			) objectAtIndex:0].UTF8String)
		);
		if (child.is_empty()) {
			return path.copy();
		}
		return Path::format("%s/%s", *path, *child);
	}

	SString Path::temp(cString& child) {
		static cString path( Path::format("%s", [NSTemporaryDirectory() UTF8String]) );
		if (child.is_empty()) {
			return path.copy();
		}
		return Path::format("%s/%s", *path, *child);
	}

	/**
	 * Get the resoures dir
	 */
	SString Path::resources(cString& child) {
		static cString path( Path::format("%s", [[[NSBundle mainBundle] resourcePath] UTF8String]) );
		if (child.is_empty()) {
			return path.copy();
		}
		return Path::format("%s/%s", *path, *child);
	}

}