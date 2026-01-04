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

#import "../fs.h"
#if Qk_APPLE
#import <Foundation/Foundation.h>
#if Qk_iOS
#import <UIKit/UIKit.h>
#else
#import <AppKit/AppKit.h>
#endif

namespace qk {

	String fs_home_dir(cChar *child, ...) {
		static cString path(fs_format("%s", NSHomeDirectory().UTF8String));
		if (child) {
			va_list arg;
			va_start(arg, child);
			auto str = _Str::printfv(child, arg);
			va_end(arg);
			return fs_format("%s/%s", path.c_str(), str.c_str());
		} else {
			return path;
		}
	}

	String fs_executable() {
		static cString path(fs_format(NSBundle.mainBundle.executablePath.UTF8String));
		return path;
	}

	String fs_documents(cString& child) {
		static String path(
			fs_format([NSSearchPathForDirectoriesInDomains(
				NSDocumentDirectory, NSUserDomainMask, YES) objectAtIndex:0].UTF8String)
		);
		return child.is_empty() ? path: fs_format("%s/%s", path.c_str(), child.c_str());
	}

	String fs_temp(cString& child) {
		static cString path(fs_format("%s", NSTemporaryDirectory().UTF8String));
		return child.is_empty() ? path: fs_format("%s/%s", path.c_str(), child.c_str());;
	}

	/**
	 * Get the resoures dir
	 */
	String fs_resources(cString& child) {
		static cString path( fs_format("%s", NSBundle.mainBundle.resourcePath.UTF8String) );
		return child.is_empty()? path: fs_format("%s/%s", path.c_str(), child.c_str());
	}

}

#endif
