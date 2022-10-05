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

#include <unistd.h>
#include "../fs.h"
#include "../android-jni.h"

namespace quark {

	static String documents_path, temp_path, resources_path, executable_path;

	String fs_executable() {
		return executable_path;
	}

	String fs_documents(cString& child) {
		if ( child.is_empty() ) {
			return documents_path;
		}
		return fs_format("%s/%s", *documents_path, *child);
	}

	String fs_temp(cString& child) {
		if ( child.is_empty() ) {
			return temp_path;
		}
		return fs_format("%s/%s", *temp_path, *child);
	}

	/**
	* Get the resoures dir
	*/
	String fs_resources(cString& child) {
		if ( child.is_empty() ) {
			return resources_path;
		}
		return fs_format("%s/%s", *resources_path, *child);
	}

	extern "C" {

		Qk_EXPORT void Java_org_quark_API_setPaths(JNIEnv* env, jclass clazz, jstring package, jstring files_dir, jstring cache_dir) {
			documents_path = JNI::jstring_to_string(files_dir);
			temp_path = JNI::jstring_to_string(cache_dir);
			resources_path = fs_format("zip://%s@/assets", JNI::jstring_to_string(package));
			
			Char dir[PATH_MAX] = { 0 };
			readlink("/proc/self/exe", dir, PATH_MAX);
			executable_path = fs_format("%s", dir);
		}
	}

}

