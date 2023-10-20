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

#include <sys/stat.h>
#include "./fs.h"

#if Qk_WIN
	#include <io.h>
	#include <direct.h>
#else
	#include <unistd.h>
#endif

namespace qk {

	String string_format(cChar* f, va_list arg);

	static String fs_split_path(cString& path, bool basename) {
		String s = path;
#if Qk_WIN
			s = s.replace_all('\\', '/');
#endif
		int start = path.length();
		if (path[start - 1] == '/') {
			s = s.substr(0, start - 1);
			start -= 2;
		}
		
		int index = s.lastIndexOf("/", start);
		if (index != -1) {
			if (basename) {
				return s.substring(index + 1);
			} else { // dirname
				return s.substring(0, index);
			}
		} else {
			if (basename) {
				return s;
			} else {
				return String();
			}
		}
	}

	String fs_basename(cString& path) {
		return fs_split_path(path, true);
	}

	String fs_dirname(cString& path) {
		return fs_split_path(path, false);
	}

	String fs_extname(cString& path) {
		String s = fs_split_path(path, true);
		int index = s.lastIndexOf(".");
		if (index != -1) {
			return s.substr(index);
		}
		return String();
	}

	String fs_cwd() {
#if Qk_WIN
			Char cwd[1100] = { 'f', 'i', 'l', 'e', ':', '/', '/', '/' };
			_getcwd(cwd + 8, 1024);
			String str = String(cwd).replace_all('\\', '/');
			if (str.length() == 10)
				str.push('/'); //
			return str;
#else
			Char cwd[1100] = { 'f', 'i', 'l', 'e', ':', '/', '/' };
			getcwd(cwd + 7, 1024);
			return cwd;
#endif
	}

	bool fs_chdir(cString& path) {
		String str = fs_format("%s", path.c_str());
#if Qk_WIN
			return _chdir(str.substr(8).c_str()) == 0;
#else
			return ::chdir(str.substr(7).c_str()) == 0;
#endif
	}

	static cString Chars("ABCDEFGHIJKMLNOPQRSTUVWXYZabcdefghijkmlnopqrstuvwxyz");

	bool fs_is_local_absolute(cString& path) {
#if Qk_WIN
			if (Chars.index_of(s[0]) != -1 && path[1] == ':') {
				return true;
			}
#else
			if (path[0] == '/') {
				return true;
			}
#endif
		else if ( fs_is_local_zip(path) || fs_is_local_file( path ) ) {
			return true;
		}
		return false;
	}

	bool fs_is_local_zip(cString& path) {
		if (
				(path[0] == 'z' || path[0] == 'Z') &&
				(path[1] == 'i' || path[1] == 'I') &&
				(path[2] == 'p' || path[2] == 'P') &&
				path[3] == ':' &&
				path[4] == '/' &&
				path[5] == '/' &&
				path[6] == '/') {
			return true;
		}
		return false;
	}

	bool fs_is_local_file(cString& path) {
		if (
				(path[0] == 'f' || path[0] == 'F') &&
				(path[1] == 'i' || path[1] == 'I') &&
				(path[2] == 'l' || path[2] == 'L') &&
				(path[3] == 'e' || path[3] == 'E') &&
				path[4] == ':' &&
				path[5] == '/' &&
				path[6] == '/' &&
				path[7] == '/') {
			return true;
		}
		return false;
	}

	String fs_format_part_path(cString& path) {

		Array<String> ls = path.split("/");
		Array<String> rev;
		int up = 0;
		for (int i = ls.length() - 1; i > -1; i--) {
			cString& v = ls[i];
			if (!v.isEmpty() && v != ".") {
				if (v[0] == '.' && v[1] == '.') { // set up ../
					up++;
				}
				else if (up == 0) { // no up item
					rev.push(v);
				}
				else { // un up
					up--;
				}
			}
		}
		
		String s;
		
		if (rev.length()) {
			// reverse
			for (int i = rev.length() - 1; i > 0; i--) {
				s.append(rev[i]).append('/');
			}
			s.append(rev[0]);
		}
		return s;
	}

	String fs_format(cString& path) {
		String s = path.copy();
		
#if Qk_WIN
			// TODO wondows ...
			s = path.replace_all('\\', '/');
			
			String prefix;
			
			if (s[0] == '/') { // absolute path
				// add windows drive letter
				// file:///c:/
				prefix = cwd().substr(0, 11);
			}
			else if ( s.length() > 7 && is_local_zip(s) ) {
				
				if (Chars.index_of(s[7]) != -1 && s[8] == ':') { // zip:///c:
					if (s.length() < 10) {
						return s.substr(0, 9).append('/');
					} else if (s[9] == '/') { // zip:///c:/
						prefix = s.substr(0, 10);
						s = s.substr(10);
					} else { // invalid windows path
						prefix = "zip:///"; // unix path ?
						s = s.substr(7);
					}
				} else {
					prefix = "zip:///"; // unix path ?
					s = s.substr(7);
				}
				
			} else if ( s.length() >= 8 && is_local_file( s ) ) { // file:///
				
				if (Chars.index_of(s[8]) != -1 && s[9] == ':') { // file:///c:
					if (s.length() < 11) {
						return s.substr(0, 10).append('/');
					} else if (s[10] == '/') { // file:///c:/
						prefix = s.substr(0, 11);
						s = s.substr(11);
					} else { // invalid windows path
						prefix = "file:///"; // unix path ?
						s = s.substr(8);
					}
				} else {
					prefix = "file:///"; // unix path ?
					s = s.substr(8);
				}
			} else { // Relative path
				if (s.length() >= 2 &&
						Chars.index_of(s[0]) != -1 && s[1] == ':' &&
						(s.length() < 3 || s[2] == '/')
						) { // Windows absolute path
					prefix = String("file:///").push(*s, 2).push('/');
					s = s.substr(2);
				}
				else {
					// file:///c:/
					prefix = cwd().substr(0, 11);
					s = cwd().substr(11).push('/').push(s);
				}
			}
#else
			String prefix = "file:///";
			if (s[0] == '/') { // absolute path
				//
			} else {
				if ( s.length() > 7 && fs_is_local_zip(s) ) {
					prefix = "zip:///";
					s = s.substr(7);
				}
				else if (s.length() >= 8 && fs_is_local_file( s ) ) {
					s = s.substr(8);
				} else { // Relative path
					s = fs_cwd().substr(8).append('/').append(s);
				}
			}
#endif
		
		s = fs_format_part_path(s);
		
		return prefix.append( s );
	}

	String fs_format(cChar* path, ...) {
		va_list arg;
		va_start(arg, path);
		String str = string_format(path, arg);
		va_end(arg);
		return fs_format(str);
	}

	int fallback_indexOf(cString& path) {
#if Qk_WIN
			if ( fs_is_local_zip(path) ) {
				return 7;
			}
			else if ( fs_is_local_file(path) ) {
				return 8;
			}
#else
			if ( fs_is_local_zip(path) ) {
				return 6;
			}
			else if ( fs_is_local_file(path) ) {
				return 7;
			}
#endif
		return 0;
	}

	String fs_fallback(cString& path) {
		return path.substr(fallback_indexOf(path));
	}

	cChar* fs_fallback_c(cString& path) {
		return path.c_str() + fallback_indexOf(path);
	}

}

#if Qk_LINUX
#include <sys/utsname.h>
#include <stdlib.h>

namespace qk {
	static String executable_path, documents_path, temp_path, resources_path;

	String fs_executable() {
		if (executable_path.is_empty()) {
			char dir[PATH_MAX] = { 0 };
			int n = readlink("/proc/self/exe", dir, PATH_MAX);
			executable_path = fs_format("%s", dir);
		}
		return executable_path;
	}

	String fs_documents(cString& child) {
		if (documents_path.is_empty()) {
			documents_path = fs_format("%s/%s", getenv("HOME"), "Documents");
			fs_mkdir_p_sync(documents_path);
		}
		if ( child.is_empty() )
			return documents_path;
		return fs_format("%s/%s", *documents_path, *child);
	}

	String fs_temp(cString& child) {
		if (temp_path.is_empty()) {
			temp_path = fs_format("%s/%s", getenv("HOME"), ".cache");
			fs_mkdir_p_sync(temp_path);
		}
		if (child.is_empty())
			return temp_path;
		return fs_format("%s/%s", *temp_path, *child);
	}

	String fs_resources(cString& child) {
		if (resources_path.is_empty()) {
			resources_path = fs_dirname(fs_executable());
		}
		if (child.is_empty())
			return resources_path;
		return fs_format("%s/%s", *resources_path, *child);
	}

}
#endif

#if Qk_ANDROID
#include "./jni.h"

extern "C" {

	Qk_EXPORT void Java_org_quark_API_setPaths(JNIEnv* env, jclass clazz, jstring package, jstring files_dir, jstring cache_dir) {
		using namespace qk;
		documents_path = JNI::jstring_to_string(files_dir);
		temp_path = JNI::jstring_to_string(cache_dir);
		resources_path = fs_format("zip://%s@/assets", JNI::jstring_to_string(package));
		
		char dir[PATH_MAX] = { 0 };
		readlink("/proc/self/exe", dir, PATH_MAX);
		executable_path = fs_format("%s", dir);
	}
}
#endif
