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

#if Qk_LINUX
#include <sys/utsname.h>
#include <stdlib.h>
#include <limits.h>
#endif

#if Qk_ANDROID
#include "../platforms/android/android.h"
#endif

namespace qk {

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
		char cwd[1100] = { 'f', 'i', 'l', 'e', ':', '/', '/', '/' };
		_getcwd(cwd + 8, 1024);
		String str = String(cwd).replace_all('\\', '/');
		if (str.length() == 10)
			str.push('/'); //
		return str;
#else
		char cwd[1100] = { 'f', 'i', 'l', 'e', ':', '/', '/' };
		getcwd(cwd + 7, 1024);
		return cwd;
#endif
	}

	bool fs_chdir(cString& path) {
		String str = fs_format("%s", path.c_str());
#if Qk_WIN
		return _chdir(str.substr(8).c_str()) == 0;
#else
		return chdir(str.substr(7).c_str()) == 0;
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
		auto c = path.c_str();
		if (
				(c[0] == 'z' || c[0] == 'Z') &&
				(c[1] == 'i' || c[1] == 'I') &&
				(c[2] == 'p' || c[2] == 'P') &&
				c[3] == ':' &&
				c[4] == '/' &&
				c[5] == '/' &&
				c[6] == '/') {
			return true;
		}
		return false;
	}

	bool fs_is_local_file(cString& path) {
		auto c = path.c_str();
		if (
				(c[0] == 'f' || c[0] == 'F') &&
				(c[1] == 'i' || c[1] == 'I') &&
				(c[2] == 'l' || c[2] == 'L') &&
				(c[3] == 'e' || c[3] == 'E') &&
				c[4] == ':' &&
				c[5] == '/' &&
				c[6] == '/' &&
				c[7] == '/') {
			return true;
		}
		return false;
	}

	String fs_format_part_path(cString& path) {

		Array<String> ls = path.split("/");
		Array<String> rev;
		int up = 0;
		for (int i = Qk_Minus(ls.length(), 1); i > -1; i--) {
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
			for (int i = Qk_Minus(rev.length(), 1); i > 0; i--) {
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
		String str = _Str::printfv(path, arg);
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

#if Qk_LINUX || Qk_ANDROID
	static String path_home_dir, path_executable, path_documents, path_temp, path_resources;

	String fs_home_dir(cChar *child, ...) {
		if (path_home_dir.isEmpty()) {
			path_home_dir = fs_format("%s", getenv("HOME"));
		}
		if (child) {
			va_list arg;
			va_start(arg, child);
			auto str = _Str::printfv(child, arg);
			va_end(arg);
			return fs_format("%s/%s", path_home_dir.c_str(), str.c_str());
		} else {
			return path_home_dir;
		}
	}

	String fs_executable() {
		if (path_executable.isEmpty()) {
			char dir[PATH_MAX] = { 0 };
			int n = readlink("/proc/self/exe", dir, PATH_MAX);
			path_executable = fs_format("%s", dir);
		}
		return path_executable;
	}

	String fs_documents(cString& child) {
		if (path_documents.isEmpty()) {
			path_documents = fs_home_dir("Documents");
			fs_mkdirs_sync(path_documents);
		}
		return child.isEmpty() ? path_documents: fs_format("%s/%s", *path_documents, *child);
	}

	String fs_temp(cString& child) {
		if (path_temp.isEmpty()) {
			path_temp = fs_home_dir(".cache");
			fs_mkdirs_sync(path_temp);
		}
		return child.isEmpty() ? path_temp: fs_format("%s/%s", *path_temp, *child);
	}

	String fs_resources(cString& child) {
		if (path_resources.isEmpty()) {
			path_resources = fs_dirname(fs_executable());
		}
		return child.isEmpty() ? path_resources: fs_format("%s/%s", *path_resources, *child);
	}
#endif

#if Qk_ANDROID
	void Android_fs_init_paths(jstring pkg, jstring files_dir, jstring cache_dir) {
		path_documents = JNI::jstring_to_string(files_dir);
		path_home_dir = fs_format("%s/..", path_documents.c_str());
		path_temp = JNI::jstring_to_string(cache_dir);
		path_resources = fs_format("zip://%s@/assets", JNI::jstring_to_string(pkg).c_str());
		char dir[PATH_MAX] = { 0 };
		readlink("/proc/self/exe", dir, PATH_MAX);
		path_executable = fs_format("%s", dir);
	}
#endif
}
