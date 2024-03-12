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

#include "trial/fs.h"
#include "quark/util/zlib.h"
#include "quark/util/handle.h"
#include "quark/util/error.h"

namespace qk {

	String fs_format_part_path(cString& path);

	class FileSearch::SearchPath {
	public:
		virtual ZipInSearchPath* as_zip() {
			return nullptr;
		}
		virtual String get_absolute_path(cString& path) {
			return fs_format("%s/%s", *m_searchPath, *path);
		}
		virtual bool exists(cString& path) {
			return fs_exists_sync(fs_format("%s/%s", *m_searchPath, *path));
		}
		virtual Buffer read(cString& path) {
			String p = fs_format("%s/%s", *m_searchPath, *path);
			return fs_read_file_sync( p );
		}
		inline String searchPath() {
			return m_searchPath;
		}

	protected:
		String m_searchPath;
		friend class ZipInSearchPath;
	private:
		SearchPath(cString& path): m_searchPath(path) { }
		virtual ~SearchPath() {}
		friend class FileSearch;
	};

	class FileSearch::ZipInSearchPath: public FileSearch::SearchPath {
	public:
		String get_absolute_path(cString& path) override {
			String s = formatPath(m_searchPath, path);
			return String::format("zip://%s@/%s", /*file:// */ *m_zip_path.substr(7), *s);
		}
		bool exists(cString& path) override {
			return m_zip.exists(formatPath(m_searchPath, path));
		}
		Buffer read(cString& path) override {
			return read_by_in_path(formatPath(m_searchPath, path));
		}
		Buffer read_by_in_path(cString& path) {
			if (m_zip.jump(path)) {
				return m_zip.read();
			}
			return Buffer();
		}
		bool exists_by_full(cString& path) { // searchPath + path
			return m_zip.exists(path);
		}
		inline String zip_path() {
			return m_zip_path;
		}
		ZipInSearchPath* as_zip() override {
			return this;
		}
		static String formatPath(cString& path1, cString& path2) {
			return fs_format_part_path(String(path1).append('/').append(path2));
		}
	private:
		ZipInSearchPath(cString& zip_path, cString& path)
			: SearchPath(path)
			, m_zip_path(zip_path)
			, m_zip (zip_path)
		{
			auto isOpen = m_zip.open();
			Qk_ASSERT( isOpen, "Cannot open zip file, `%s`", *zip_path );
		}
		~ZipInSearchPath() = default;
		String m_zip_path;
		ZipReader m_zip;
		friend class FileSearch;
	};

	// ------------------ F i l e . S e a r c h ------------------

	FileSearch::FileSearch() {
		cString& res = fs_resources(); // 把资源目录加入进来
		
		if (fs_is_local_zip(res)) { // zip pkg
			int i = res.indexOf("@/");
			if (i != -1) {
				add_zip_search_path(/*zip://*/res.substring(6, i), res.substr(i + 2));
			} else if (res[res.length() - 1] == '@') {
				add_zip_search_path(res.substr(0, res.length() - 1), String());
			} else {
				Qk_WARN("SEARCH", "Invalid path, %s", *res);
			}
		} else {
			if (fs_exists_sync(res)) {
				add_search_path(res);
			} else {
				Qk_WARN("SEARCH", "Resource directory does not exists, %s", *res);
			}
		}
	}

	FileSearch::~FileSearch() {
		remove_all_search_path();
	}

	void FileSearch::add_search_path(cString& path) {
		String str = fs_format(*path);
		
		auto it = m_search_paths.begin();
		auto end = m_search_paths.end();
		for (; it != end; it++) {
			FileSearch::SearchPath* s = *it;
			if (!s->as_zip()) {
				if (s->searchPath() == str) {
					Qk_WARN("SEARCH", "The repetitive path, \"%s\"", *path);
					// Fault tolerance, skip the same path
					return;
				}
			}
		}
		m_search_paths.pushBack(new FileSearch::SearchPath(str));
	}

	void FileSearch::add_zip_search_path(cString& zip_path, cString& path) {
		String _zip_path = fs_format("%s", *zip_path);
		String _path = path;
#if Qk_WIN
			_path = path.replace_all('\\', '/');
#endif
		_path = fs_format_part_path(path);
		
		auto it = m_search_paths.begin();
		auto end = m_search_paths.end();
		for (; it != end; it++) {
			if ((*it)->as_zip()) {
				FileSearch::ZipInSearchPath* s = (*it)->as_zip();
				if (s->zip_path() == _zip_path && s->searchPath() == _path) {
					Qk_WARN("SEARCH", "The repetitive path, ZIP: %s, %s", *zip_path, *path);
					// Fault tolerance,skip the same path
					return;
				}
			}
		}
		m_search_paths.pushBack(new FileSearch::ZipInSearchPath(_zip_path, _path));
	}

	Array<String> FileSearch::get_search_paths() const {
		Array<String> result;
		for (auto i: m_search_paths)
			result.push(i->searchPath());
		Qk_ReturnLocal(result);
	}

	void FileSearch::remove_search_path(cString& path) {
		auto it = m_search_paths.begin();
		auto end = m_search_paths.end();
		for ( ; it != end; it++) {
			if ((*it)->searchPath() == path) {
				delete (*it);
				m_search_paths.erase(it);
				return;
			}
		}
	}

	void FileSearch::remove_all_search_path() {
		for (auto i: m_search_paths)
			delete i;
		m_search_paths.clear();
	}

	bool FileSearch::exists(cString& path) const {
		return searchPath(path, nullptr);
	}

	String FileSearch::get_absolute_path(cString& path) const {
		String restult;
		searchPath(path, &restult);
		Qk_ReturnLocal(restult);
	}

	bool FileSearch::searchPath(cString& path, String *outAbsolute) const {
		if (path.isEmpty()) {
			return false;
		}
		if (fs_is_local_zip(path)) { // abs path
			auto paths = path.substr(7).split("@/");
			if (paths.length() > 1) {
				for ( auto i: m_search_paths ) {
					auto zip = i->as_zip();
					if (zip && zip->zip_path() == paths[0]) {
						if (zip->exists_by_full(paths[1])) {
							if (outAbsolute)
								*outAbsolute = path;
							return true;
						}
					}
				}
			}
		} else if (fs_is_local_absolute(path)) {
			if (fs_exists_sync(path)) {
				if (outAbsolute)
					*outAbsolute = fs_format(path.c_str());
				return true;
			}
		} else {
			for ( auto i: m_search_paths ) {
				if (i->exists(path)) {
					if (outAbsolute)
						*outAbsolute = i->get_absolute_path(path);
					return true;
				}
			}
		}
		return false;
	}

	Buffer FileSearch::read(cString& path) const {
		if (path.isEmpty()) {
			return Buffer();
		}
		if (fs_is_local_zip(path)) { // zip pkg inner file
			auto paths = path.substr(7).split("@/");
			if (paths.length() > 1) {
				for ( auto i: m_search_paths ) {
					auto zip = i->as_zip();
					if (zip && zip->zip_path() == paths[0]) {
						return zip->read_by_in_path(paths[1]);
					}
				}
			}
		} else if (fs_is_local_absolute(path)) { // absolute path
			return fs_read_file_sync(path);
		} else {
			for ( auto i: m_search_paths ) {
				if (i->exists(path)) {
					return i->read(path);
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

}
