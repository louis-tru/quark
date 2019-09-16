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

#include "niutils/fs.h"
#include <Foundation/Foundation.h>
#if XX_IOS
# import <UIKit/UIKit.h>
#else
# import <AppKit/AppKit.h>
#endif

XX_NS(ngui)

String Path::executable() {
	static cString path( format([[[NSBundle mainBundle] executablePath] UTF8String]) );
	return path;
}

String Path::documents(cString& child) {
	static cString path(
		Path::format([NSSearchPathForDirectoriesInDomains(
			NSDocumentDirectory,
			NSUserDomainMask,
			YES
		) objectAtIndex:0].UTF8String)
	);
	if (child.is_empty()) {
		return path;
	}
	return Path::format("%s/%s", *path, *child);
}

String Path::temp(cString& child) {
	static cString path( Path::format("%s", [NSTemporaryDirectory() UTF8String]) );
	if (child.is_empty()) {
		return path;
	}
	return Path::format("%s/%s", *path, *child);
}

/**
 * Get the resoures dir
 */
String Path::resources(cString& child) {
	static cString path( Path::format("%s", [[[NSBundle mainBundle] resourcePath] UTF8String]) );
	if (child.is_empty()) {
		return path;
	}
	return Path::format("%s/%s", *path, *child);
}

namespace sys {

	String brand() {
		return "Apple";
	}

#if XX_IOS

	String version() {
		return [[[UIDevice currentDevice] systemVersion] UTF8String];
	}

	String subsystem() {
		return [[[UIDevice currentDevice] model] UTF8String];
	}

#else 

	String version() {
		return String();
	}

	String subsystem() {
		static String name("MacOSX");
		return name;
	}

#endif 

	void __get_languages__(String& langs, String& lang) {
		NSArray* languages = [NSLocale preferredLanguages];
		for ( int i = 0; i < [languages count]; i++ ) {
			NSString* str = [languages objectAtIndex:0];
			if (i == 0) {
				lang = [str UTF8String];
			} else {
				langs += ',';
			}
			langs += [str UTF8String];
		}
	}
	
}

XX_END
