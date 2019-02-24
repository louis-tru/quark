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

#include <sys/stat.h>
#include "qgr/utils/fs.h"

#if XX_WIN
	#include <io.h>
	#include <direct.h>
#else
	#include <unistd.h>
#endif

XX_NS(qgr)

#if XX_WIN
const uint FileHelper::default_mode(0);
#else
const uint FileHelper::default_mode([]() {
	uint mask = umask(0);
	umask(mask);
	return 0777 & ~mask;
}());
#endif

// Path implementation

static String split_path(cString& path, bool basename) {
	String s = path;
#if XX_WIN
	s = s.replace_all('\\', '/');
#endif 
	int start = path.length();
	if (path[start - 1] == '/') {
		start -= 2;
	}
	
	int index = s.last_index_of('/', start);
	if (index != -1) {
		if (basename) {
			return s.substring(index + 1, start);
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

String Path::basename(cString& path) {
	return split_path(path, true);
}

String Path::dirname(cString& path) {
	return split_path(path, false);
}

String Path::extname(cString& path) {
	String s = split_path(path, true);
	int index = s.last_index_of('.');
	if (index != -1) {
		return s.substr(index);
	}
	return String();
}

String Path::cwd() {
#if XX_WIN
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

bool Path::chdir(cString& path) {
	String str = format("%s", *path);
#if XX_WIN
	return _chdir(*str.substr(8)) == 0;
#else 
	return ::chdir(*str.substr(7)) == 0;
#endif 
}

static const String chars("ABCDEFGHIJKMLNOPQRSTUVWXYZabcdefghijkmlnopqrstuvwxyz");

bool Path::is_local_absolute(cString& path) {
#if XX_WIN
	if (chars.index_of(s[0]) != -1 && path[1] == ':') {
		return true;
	}
#else 
	if (path[0] == '/') {
		return true;
	}
#endif 
	else if ( is_local_zip(path) || is_local_file( path ) ) {
		return true;
	}
	return false;
}

bool Path::is_local_zip(cString& path) {
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

bool Path::is_local_file(cString& path) {
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

String inl__format_part_path(cString& path) {
	
	Array<String> ls = path.split('/');
	Array<String> rev;
	int up = 0;
	for (int i = ls.length() - 1; i > -1; i--) {
		String& v = ls[i];
		if (!v.is_empty() && v != '.') {
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
	
	String s = String();
	
	if (rev.length()) {
		// reverse
		for (int i = rev.length() - 1; i > 0; i--) {
			s.push(rev[i]).push('/');
		}
		s.push(rev[0]);
	}
	return s;
}

String Path::format(cString& path) {
	
	String s = path;
	
#if XX_WIN
	// TODO wondows ...
	
	s = s.replace_all('\\', '/');
	
	String prefix;
	
	if (s[0] == '/') { // absolute path
		// add windows drive letter
		// file:///c:/
		prefix = cwd().substr(0, 11);
	}
	else if ( s.length() > 7 && is_local_zip(s) ) {
		
		if (chars.index_of(s[7]) != -1 && s[8] == ':') { // zip:///c:
			if (s.length() < 10) {
				return s.substr(0, 9).push('/');
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
		
		if (chars.index_of(s[8]) != -1 && s[9] == ':') { // file:///c:
			if (s.length() < 11) {
				return s.substr(0, 10).push('/');
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
				chars.index_of(s[0]) != -1 && s[1] == ':' &&
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
		if ( s.length() > 7 && is_local_zip(s) ) {
			prefix = "zip:///";
			s = s.substr(7);
		}
		else if (s.length() >= 8 && is_local_file( s ) ) {
			s = s.substr(8);
		} else { // Relative path
			s = cwd().substr(8).push('/').push(s);
		}
	}
#endif
	
	s = inl__format_part_path(s);
	
	return prefix.push( s );
}

String Path::format(cchar* path, ...) {
	
	XX_STRING_FORMAT(path, s);
	
	return format(s);
}

String Path::fallback(cString& path) {
	return fallback_c(path);
}

cchar* Path::fallback_c(cString& path) {
#if XX_WIN
	if ( is_local_zip(path) ) {
		return path.c() + 7;
	}
	else if ( is_local_file(path) ) {
		return path.c() + 8;
	}
#else
	if ( is_local_zip(path) ) {
		return path.c() + 6;
	}
	else if ( is_local_file(path) ) {
		return path.c() + 7;
	}
#endif
	return *path;
}

XX_END
