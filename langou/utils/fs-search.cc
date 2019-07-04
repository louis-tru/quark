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

#include "langou/utils/fs.h"
#include "langou/utils/zlib.h"
#include "langou/utils/handle.h"
#include "langou/utils/error.h"

XX_NS(langou)

// FileSearch implementation

String inl__format_part_path(cString& path);

class FileSearch::SearchPath {
public:
	virtual ZipInSearchPath* as_zip() { return NULL; }
	virtual String get_absolute_path(cString& path);
	virtual Buffer read(cString& path);
	inline String path() { return m_path; }
protected:
	String m_path;
	friend class ZipInSearchPath;
private:
	SearchPath(cString& path): m_path(path) { }
	virtual ~SearchPath() { }
	friend class FileSearch;
};

class FileSearch::ZipInSearchPath: public FileSearch::SearchPath {
public:
	ZipInSearchPath* as_zip() { return this; }
	String get_absolute_path(cString& path);
	Buffer read(cString& path);
	Buffer read_by_in_path(cString& path);
	bool exists_by_abs(cString& path);
	inline String zip_path() { return m_zip_path; }
	static String formatPath(cString& path1, cString& path2);
private:
	ZipInSearchPath(cString& zip_path, cString& path);
	~ZipInSearchPath();
	String m_zip_path;
	ZipReader m_zip;
	friend class FileSearch;
};

String FileSearch::SearchPath::get_absolute_path(cString& path) {
	String p = Path::format("%s/%s", *m_path, *path);
	if (FileHelper::exists_sync(p)) {
		return p;
	}
	return String();
}

Buffer FileSearch::SearchPath::read(cString& path) {
	String p = Path::format("%s/%s", *m_path, *path);
	return FileHelper::read_file_sync( p );
}

String FileSearch::ZipInSearchPath::formatPath(cString& path1, cString& path2) {
	return inl__format_part_path(String(path1).push('/').push(path2));
}

String FileSearch::ZipInSearchPath::get_absolute_path(cString& path) {
	String s = formatPath(m_path, path);
	if (m_zip.exists(s)) {
		return String::format("zip://%s@/%s", *m_zip_path.substr(7), *s);
	}
	return String();
}

Buffer FileSearch::ZipInSearchPath::read(cString& path) {
	return read_by_in_path(formatPath(m_path, path));
}

Buffer FileSearch::ZipInSearchPath::read_by_in_path(cString& path) {
	if (m_zip.jump(path)) {
		return m_zip.read();
	}
	return Buffer();
}

bool FileSearch::ZipInSearchPath::exists_by_abs(cString& path) {
	return m_zip.exists(path);
}

FileSearch::ZipInSearchPath::ZipInSearchPath(cString& zip_path, cString& path)
: SearchPath(path)
, m_zip_path(zip_path)
, m_zip (zip_path) {
	XX_ASSERT_ERR( m_zip.open(), "Cannot open zip file, `%s`", *zip_path );
}

FileSearch::ZipInSearchPath::~ZipInSearchPath() {
	
}

FileSearch::FileSearch() {
	// 把资源目录加入进来
	cString& res = Path::resources();
	
	if (Path::is_local_zip(res)) { // zip pkg
		int i = res.index_of('@');
		if (i != -1) {
			add_zip_search_path(res, res.substr(i + 1));
		} else {
			XX_WARN("Invalid path, %s", *res);
		}
	} else {
		if (FileHelper::exists_sync(res)) {
			add_search_path(res);
		} else {
			XX_WARN("Resource directory does not exists, %s", *res);
		}
	}
}

FileSearch::~FileSearch() {
	remove_all_search_path();
}

void FileSearch::add_search_path(cString& path) {
	String str = Path::format(*path);
	
	auto it = m_search_paths.begin();
	auto end = m_search_paths.end();
	for (; it != end; it++) {
		FileSearch::SearchPath* s = it.value();
		if (!s->as_zip()) {
			if (s->path() == str) {
				XX_WARN("The repetitive path, \"%s\"", *path);
				// Fault tolerance, skip the same path
				return;
			}
		}
	}
	m_search_paths.push(new FileSearch::SearchPath(str));
}

void FileSearch::add_zip_search_path(cString& zip_path, cString& path) {
	String _zip_path = Path::format("%s", *zip_path);
	String _path = path;
#if XX_WIN
	_path = path.replace_all('\\', '/');
#endif
	_path = inl__format_part_path(path);
	
	auto it = m_search_paths.begin();
	auto end = m_search_paths.end();
	for (; it != end; it++) {
		if (it.value()->as_zip()) {
			FileSearch::ZipInSearchPath* s = it.value()->as_zip();
			if (s->zip_path() == _zip_path && s->path() == _path) {
				XX_WARN("The repetitive path, ZIP: %s, %s", *zip_path, *path);
				// Fault tolerance,skip the same path
				return;
			}
		}
	}
	m_search_paths.push(new FileSearch::ZipInSearchPath(_zip_path, _path));
}

/**
 *  Gets the array of search paths.
 *
 *  @ret {const Array<String>} The array of search paths.
 */
Array<String> FileSearch::get_search_paths() const {
	auto it = m_search_paths.begin();
	auto end = m_search_paths.end();
	Array<String> rest;
	for (; it != end; it++) {
		rest.push(it.value()->path());
	}
	return rest;
}

/**
 * remove search path
 */
void FileSearch::remove_search_path(cString& path) {
	auto it = m_search_paths.begin();
	auto end = m_search_paths.end();
	for ( ; it != end; it++) {
		if (it.value()->path() == path) {
			delete it.value();
			m_search_paths.del(it);
			return;
		}
	}
}

/**
 * Removes all search paths.
 */
void FileSearch::remove_all_search_path() {
	auto it = m_search_paths.begin();
	auto end = m_search_paths.end();
	for (; it != end; it++) {
		delete it.value();
	}
	m_search_paths.clear();
}

String FileSearch::get_absolute_path(cString& path) const {
	
	if (path.is_empty()) {
		XX_WARN("Search path cannot be a empty and null");
		return String();
	}
	
	if (Path::is_local_absolute(path)) {
		return FileHelper::exists_sync(path) ? Path::format(path.c()) : String();
	}
	
	auto it = m_search_paths.begin();
	auto end = m_search_paths.end();
	
	if (path.substr(0, 7).lower_case().index_of("zip:///") == 0) {
		
		String path_s = path.substr(7);
		Array<String> ls = path_s.split("@/");
		
		if (ls.length() > 1) {
			String zip_path = ls[0];
			path_s = ls[1];
			
			for ( ; it != end; it++ ) {
				auto zip = it.value()->as_zip();
				if (zip && zip->zip_path() == zip_path) {
					if (zip->exists_by_abs(path_s)) {
						return path;
					}
				}
			}
		}
	}
	else {
		for ( ; it != end; it++ ) {
			String abs_path = *it.value()->get_absolute_path(path);
			if (String() != abs_path) {
				return abs_path;
			}
		}
	}
	return String();
}

bool FileSearch::exists(cString& path) const {
	return !get_absolute_path(path).is_empty();
}

Buffer FileSearch::read(cString& path) const {
	
	if (path.is_empty()) {
		return Buffer();
	}
	
	if (Path::is_local_absolute(path)) { // absolute path
		return FileHelper::read_file_sync(path);
	}
	else {
		
		auto it = m_search_paths.begin();
		auto end = m_search_paths.end();
		if (path.substr(0, 7).lower_case().index_of("zip:///") == 0) { // zip pkg inner file
			
			String path_s = path.substr(7);
			Array<String> ls = path_s.split("@/");
			
			if (ls.length() > 1) {
				String zip_path = ls[0];
				path_s = ls[1];
				
				for ( ; it != end; it++ ) {
					auto zip = it.value()->as_zip();
					if (zip && zip->zip_path() == zip_path) {
						return zip->read_by_in_path(path_s);
					}
				}
			}
		}
		else {
			for ( ; it != end; it++ ) {
				Buffer data = it.value()->read(path);
				if (data.length()) {
					return data;
				}
			}
		}
	}
	
	return Buffer();
}

static FileSearch* search = NULL;

FileSearch* FileSearch::shared() {
	if ( ! search ) {
		search = new FileSearch();
	}
	return search;
}

XX_END
