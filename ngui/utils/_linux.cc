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

#include <stdlib.h>
#include <unistd.h>
#include <linux/limits.h>
#include <sys/utsname.h>
#include "fs.h"

XX_NS(ngui)

String Path::executable() {
	static cString rv([]() -> String { 
		char dir[PATH_MAX] = { 0 };
		int n = readlink("/proc/self/exe", dir, PATH_MAX);
		return Path::format("%s", dir);
	}());
	return rv;
}

String Path::documents(cString& path) {
	static cString rv( Path::format("%s/%s", getenv("HOME"), "Documents") );
	if ( path.is_empty() ) {
		return rv;
	}
	return Path::format("%s/%s", *rv, *path);
}

String Path::temp(cString& path) {
	static cString rv( Path::format("%s/%s", getenv("HOME"), ".cache") );
	if (path.is_empty()) {
		return rv;
	}
	return Path::format("%s/%s", *rv, *path);
}

/**
 * Get the resoures dir
 */
String Path::resources(cString& path) {
	static cString rv( Path::dirname(executable()) );
	if (path.is_empty()) {
		return rv;
	}
	return Path::format("%s/%s", *rv, *path);
}

namespace sys {

	static struct utsname* utsn = NULL;

	utsname* _uname() {
		if (!utsn) {
			utsn = new utsname();
			uname(utsn);
		}
		return utsn;
	}

	String version() {
		return _uname()->release;
	}

	String brand() {
		return "Linux";
	}

	String subsystem() {
		return _uname()->version;
	}

}

XX_END
