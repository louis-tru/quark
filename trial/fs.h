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

#ifndef __quark__trial__fs__
#define __quark__trial__fs__

#include "src/util/fs.h"

namespace qk {

	/**
	* @class FileSearch # Resources files search
	*/
	class Qk_EXPORT FileSearch: public Object {
		Qk_DISABLE_COPY(FileSearch);
	public:
		FileSearch();
		~FileSearch();

		/**
		* @method add_search_path()
		* Add a search path, which must exist
		*/
		void add_search_path(cString& path);

		/**
		* @method add_zip_search_path()
		* Add a search path within a zip package, only unencrypted zip packages can be added
		*/
		void add_zip_search_path(cString& zip_path, cString& path);

		/**
		* @method get_search_paths() Gets the array of search paths.
		*/
		Array<String> get_search_paths() const;

		/**
		* @method remove_search_path() remove search path
		*/
		void remove_search_path(cString& path);

		/**
		* @method remove_all_search_path() Removes all search paths.
		*/
		void remove_all_search_path();

		/**
		* @method exists() # Find the file exists
		*/
		bool exists(cString& path) const;

		/**
		*
		* To obtain the absolute path to the file already exists.
		* If no such file returns the empty string ""
		* If it is a zip package path, will return with the prefix "zip:///home/xxx/test.apk@/assets/bb.jpg"
		* @method get_absolute_path()
		*/
		String get_absolute_path(cString& path) const;

		/**
		* @method read() Read the all file data and return Data
		*/
		Buffer read(cString& path) const;

		/**
		* @method share # Gets the instance of FileSearch.
		*/
		static FileSearch* shared();

	private:
		class SearchPath;
		class ZipInSearchPath;
		bool searchPath(cString& path, String *outAbsolute) const;
		List<SearchPath*> m_search_paths; // Search path list
	};

	inline FileSearch* fs_search() {
		return FileSearch::shared();
	}

}
#endif
