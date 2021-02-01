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
#include "../fs.h"

namespace ftr {

String Path::executable() {
	static cString path([]() -> String { 
		Char dir[PATH_MAX] = { 0 };
		int n = readlink("/proc/self/exe", dir, PATH_MAX);
		return Path::format("%s", dir);
	}());
	return path;
}

String Path::documents(cString& child) {
	static String documentsPath([]() -> String { 
		String s = Path::format("%s/%s", getenv("HOME"), "Documents");
		FileHelper::mkdir_p_sync(s);
		return s;
	}());
	if ( child.is_empty() ) {
		return documentsPath;
	}
	return Path::format("%s/%s", *documentsPath, *child);
}

String Path::temp(cString& child) {
	static String tempPath([]() -> String {
		String s = Path::format("%s/%s", getenv("HOME"), ".cache");
		FileHelper::mkdir_p_sync(s);
		return s;
	}());
	if (child.is_empty()) {
		return tempPath;
	}
	return Path::format("%s/%s", *tempPath, *child);
}

/**
 * Get the resoures dir
 */
String Path::resources(cString& child) {
	static String resourcesPath([]() -> String {
		return Path::dirname(executable());
	}());
	if (child.is_empty()) {
		return resourcesPath;
	}
	return Path::format("%s/%s", *resourcesPath, *child);
}

}

