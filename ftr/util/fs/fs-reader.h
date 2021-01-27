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

#ifndef __ftr__util__fs__fs_reaser__
#define __ftr__util__fs__fs_reaser__

#include <ftr/util/fs/fs.h>
#include <ftr/util/fs/stream.h>

namespace ftr {

	/**
	* @class FileReader
	*/
	class FX_EXPORT FileReader: public Object {
			FX_HIDDEN_ALL_COPY(FileReader);
		public:
			FileReader();
			FileReader(FileReader&& reader);
			virtual ~FileReader();
			virtual uint32_t read_file(cString& path, Cb cb = 0);
			virtual uint32_t read_stream(cString& path, Callback<StreamResponse> cb = 0);
			virtual Buffer read_file_sync(cString& path) throw(Error);
			virtual void abort(uint32_t id);
			virtual bool exists_sync(cString& path);
			virtual bool is_file_sync(cString& path);
			virtual bool is_directory_sync(cString& path);
			virtual std::vector<Dirent> readdir_sync(cString& path);
			virtual bool is_absolute(cString& path);
			virtual String format(cString& path);
			virtual void clear();
			static void set_shared_instance(FileReader* reader);
			static FileReader* shared();
		private:
			class Core;
			Core* _core;
	};

	inline FileReader* fs_reader() {
		return FileReader::shared();
	}
}
#endif
