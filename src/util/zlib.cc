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

#include "./handle.h"
#include "./error.h"
#include "./zlib.h"
#include "./fs.h"
#include <zip.h>
#include <zlib.h>
#include <unzip.h>

namespace quark {

	/*
	 gzip是一种文件压缩工具（或该压缩工具产生的压缩文件格式），它的设计目标是处理单个
	 的文件。gzip在压缩文件中的数据时使用的就是zlib。为了保存与文件属性有关的信息，gz
	 ip需要在压缩文件（*.gz）中保存更多的头信息内容，而zlib不用考虑这一点。但gzip只适
	 用于单个文件，所以我们在UNIX/Linux上经常看到的压缩包后缀都是*.tar.gz或*.tgz，也
	 就是先用tar把多个文件打包成单个文件，再用gzip压缩的结果。
	 */
	/*
	 zip是适用于压缩多个文件的格式（相应的工具有PkZip和WinZip等），因此，zip文件还要
	 进一步包含文件目录结构的信息，比gzip的头信息更多。但需要注意，zip格式可采用多种
	 压缩算法，我们常见的zip文件大多不是用zlib的算法压缩的，其压缩数据的格式与gzip大
	 不一样。
	 */

	static Buffer _compress(cChar* data, uint32_t len, int level) {
		Buffer rev;
		auto tmp = Buffer::alloc(16384); // 16k
		z_stream strm;
		strm.zalloc = Z_NULL;
		strm.zfree = Z_NULL;
		strm.opaque = Z_NULL;
		strm.next_in = Z_NULL;
		strm.avail_in = 0;
		
		if ( deflateInit(&strm, level) != Z_OK ) {
			return rev;
		}
		
		strm.next_in = (uint8_t*)data;
		strm.avail_in = len;
		do {
			strm.next_out = (uint8_t*)*tmp;
			strm.avail_out = tmp.length();
			deflate(&strm, Z_FINISH);
			rev.write(tmp, rev.length(), tmp.length() - strm.avail_out);
		} while(strm.avail_out == 0);
		
		deflateEnd(&strm);
		return rev;
	}

	static Buffer _uncompress(cChar* data, uint32_t len) {
		Buffer rev;
		auto tmp = Buffer::alloc(16384); // 16k
		z_stream strm;
		strm.zalloc = Z_NULL;
		strm.zfree = Z_NULL;
		strm.opaque = Z_NULL;
		strm.next_in = Z_NULL;
		strm.avail_in = 0;
		
		if ( inflateInit(&strm) != Z_OK ) {
			return rev;
		}
		
		strm.next_in = (uint8_t*)data;
		strm.avail_in = len;
		do {
			strm.next_out = (uint8_t*)*tmp;
			strm.avail_out = tmp.length();
			inflate(&strm, Z_FINISH);
			rev.write(tmp, rev.length(), tmp.length() - strm.avail_out);
		} while(strm.avail_out == 0);
		
		inflateEnd(&strm);
		return rev;
	}

	/**
	 * @func compress
	 */
	Buffer zlib_compress(WeakBuffer buff, int level) {
		return _compress(buff.val(), (uint32_t)buff.length(), level);
	}

	/**
	 * @func uncompress
	 */
	Buffer zlib_uncompress(WeakBuffer buff) {
		return _uncompress(buff.val(), (uint32_t)buff.length());
	}

	GZip::~GZip() {
		if(_gzfp) {
			gzclose((gzFile)_gzfp);
		}
	}

	// Override
	bool GZip::is_open() {
		return _gzfp != NULL;
	}

	extern cChar* inl__file_flag_str(int flag);

	// Override
	int GZip::open(int flag) {
		Qk_ASSERT(!_gzfp);
		if (_gzfp) // 已经打开了
			return 0;
		_gzfp = gzopen(fs_fallback_c(_path), inl__file_flag_str(flag));
		if (_gzfp) {
			return 0;
		}
		return -1;
	}

	// Override
	int GZip::close() {
		if (!_gzfp) return 0;
		int i = gzclose((gzFile)_gzfp);
		if (i == 0) {
			_gzfp = NULL;
			return 0;
		}
		return i;
	}

	// Override
	int GZip::read(void* buffer, int64_t size, int64_t offset) {
		if ( offset > -1 ) {
			gzseek((gzFile)_gzfp, offset, SEEK_SET);
		}
		return gzread((gzFile)_gzfp, buffer, uint32_t(size));
	}

	// Override
	int GZip::write(const void* buffer, int64_t size, int64_t offset) {
		if ( offset > -1 ) {
			gzseek((gzFile)_gzfp, offset, SEEK_SET);
		}
		return gzwrite((gzFile)_gzfp, buffer, uint32_t(size));
	}

	Qk_DEFINE_INLINE_MEMBERS(ZipReader, Inl) {
	public:
		#define _inl_reader(self) static_cast<ZipReader::Inl*>(self)
		
		bool _open_current_file() {
			if ( _is_open ) {
				return true;
			}
			if ( _unzp ) {
				int code;
				if ( _passwd.is_empty() ) {
					code = unzOpenCurrentFile((unzFile)_unzp);
				} else {
					code = unzOpenCurrentFilePassword((unzFile)_unzp, _passwd.c_str());
				}
				if ( code == UNZ_OK ) {
					_is_open = true;
					return true;
				}
			}
			return false;
		}
		
		bool _close_current_file() {
			if ( _is_open ) {
				if ( unzCloseCurrentFile((unzFile)_unzp) != UNZ_OK ) {
					return false;
				}
				_is_open = false;
			}
			return true;
		}
		
		bool _go_to_cur_pos() {
			if ( _unzp ) {
				unz_entry_info& info = _cur_it->value;
				unz_file_pos pos = { info.pos.pos_in_zip_directory, info.pos.num_of_file };
				int code = unzGoToFilePos((unzFile)_unzp, &pos);
				return code == UNZ_OK;
			}
			return false;
		}
		
		void add_dir_info_item(cString& pathname, FileType type) {
			
			String dirname = fs_dirname(pathname);
			String compatible_path = _compatible_path + "/" + pathname;
			
			if ( dirname.is_empty() ) {
				Dirent dir{pathname, compatible_path, type};
				auto l = _dir_info[dirname];
				l.push(dir);
			} else {
				String basename = pathname.substr(dirname.length() + 1);
				auto it = _dir_info.find(dirname);

				if ( it == _dir_info.end() ) {
					add_dir_info_item(dirname, FTYPE_DIR);
					_dir_info[dirname].push(Dirent{basename, compatible_path, type});
				} else {
					it->value.push(Dirent{basename, compatible_path, type});
				}
			}
		}
		
	};

	ZipReader::ZipReader(cString& path, cString& passwd)
		: _path(fs_format(path)), _passwd(passwd.copy())
		, _unzp(nullptr)
		, _is_open(false)
	{
		if ( fs_is_local_zip(_path) ) { // zip:///
			_compatible_path = _path + "?";
		} else if ( fs_is_local_file(_path) ) { // file:///
			_compatible_path = String::format("zip:///%s?", _path.substr(8).c_str());
		}
	}

	ZipReader::~ZipReader() {
		close();
	}

	/**
	 * @func
	 */
	bool ZipReader::open() {
		
		if ( _unzp ) {
			Qk_ERR("First close the open file");
			return false;
		}
		
		unzFile unzp = unzOpen(fs_fallback_c(_path));
		if ( !unzp ) {
			Qk_ERR("Cannot open file ZipReader, %s", _path.c_str());
			return false;
		}
		
		CPointer<void> clear(unzp, [](unzFile unzp) {
			if ( unzClose((unzFile) unzp) != UNZ_OK ) {
				//Qk_ERR("Cannot close file ZipReader, %s", _path.c_str());
			}
		});
		
		// Init
		Char name[256 + 1];
		unz_file_info unzfi;
		unz_file_pos pos;
		int code;
		
		do {
			code = unzGetFilePos(unzp, &pos);
			if ( code ) {
				Qk_ERR("Open current file pos info error"); return false;
			}
			_unz_file_pos _pos = { pos.pos_in_zip_directory, pos.num_of_file };
			code = unzGetCurrentFileInfo(unzp, &unzfi, name, 256, NULL, 0, NULL, 0);
			if ( code ) {
				Qk_ERR("Get current file info error"); return false;
			}
			String pathname = name;
			uint32_t compressed_size = (uint32_t)unzfi.compressed_size;
			uint32_t uncompressed_size = (uint32_t)unzfi.uncompressed_size;
			unz_entry_info info = { _pos, pathname, compressed_size, uncompressed_size };
			_inl_reader(this)->add_dir_info_item(pathname, FTYPE_FILE);
			_file_info[info.pathname] = info;
		} while(unzGoToNextFile(unzp) == UNZ_OK);
		
		first();
		
		clear.collapse(); // 安全通过后取消
		
		_unzp = unzp;
		return true;
	}

	/**
	 * @func close
	 */
	bool ZipReader::close() {
		if ( _unzp ) {
			if ( !_inl_reader(this)->_close_current_file() ) {
				Qk_ERR("Cannot close file reader internal documents, %s, %s",
							 _path.c_str(), _cur_it->value.pathname.c_str());
			}
			if ( unzClose((unzFile)_unzp) == UNZ_OK ) {
				_unzp = nullptr;
			} else {
				Qk_ERR("Cannot close file ZipReader, %s", _path.c_str());
			}
			_file_info.clear();
			_dir_info.clear();
		}
		return !_unzp;
	}

	bool ZipReader::exists(cString& path) const {
		if (_file_info.find(path) != _file_info.end())
			return true;
		if (_dir_info.find(path) != _dir_info.end())
			return true;
		return false;
	}

	bool ZipReader::is_file(cString& path) const {
		return _file_info.find(path) != _file_info.end();
	}

	bool ZipReader::is_directory(cString& path) const {
		return _dir_info.find(path) != _dir_info.end();
	}

	/**
	 * @func readdir(path)
	 */
	Array<Dirent> ZipReader::readdir(cString& path) const {
		auto it = _dir_info.find(path);
		if ( it == _dir_info.end() ) {
			return Array<Dirent>();
		} else {
			return it->value;
		}
	}

	bool ZipReader::jump(cString& path) {
		auto it = _file_info.find(path);
		if ( it == _file_info.end() ) {
			return false;
		}
		if (_inl_reader(this)->_close_current_file() ) {
			_cur_it = it;
			return _inl_reader(this)->_go_to_cur_pos();
		}
		return false;
	}

	bool ZipReader::first() {
		if ( _inl_reader(this)->_close_current_file() ) {
			_cur_it = _file_info.begin();
			return _inl_reader(this)->_go_to_cur_pos();
		}
		return false;
	}

	bool ZipReader::next() {
		if (_inl_reader(this)->_close_current_file() ) {
			if ( ++_cur_it == _file_info.end() ) {
				_cur_it--;
				return false;
			}
			return _inl_reader(this)->_go_to_cur_pos();
		}
		return false;
	}

	int ZipReader::read(void* buffer, int size) {
		if ( _inl_reader(this)->_open_current_file() ) {
			return unzReadCurrentFile((unzFile)_unzp, buffer, size);
		}
		return -1;
	}

	Buffer ZipReader::read() {
		int size = uncompressed_size();
		Buffer buffer = Buffer::alloc(size, size + 1);
		size = read(*buffer, size);
		if (size < 0) { // err
			return Buffer();
		}
		*(*buffer + size) = '\0';
		return buffer;
	}

	Buffer ZipReader::read(uint32_t size) {
		Buffer buffer = Buffer::alloc(size, size + 1);
		int length = read(*buffer, size);
		if (length < 0) { // err
			return Buffer();
		}
		*(*buffer + length) = '\0';
		return buffer;
	}

	// ZipWriter

	ZipWriter::ZipWriter(cString& path, cString& passwd)
		: _path(fs_format(path))
		, _passwd(passwd.copy())
		, _open_mode(OPEN_MODE_CREATE)
		, _level(-1)
		, _zipp(nullptr)
		, _new_name() {
			
	}

	ZipWriter::~ZipWriter() {
		close();
	}

	bool ZipWriter::open(OpenMode mode) {
		
		if ( _zipp ) {
			Qk_ERR("First close the open file");
			return false;
		}
		
		_open_mode = mode;
		_zipp = zipOpen(fs_fallback(_path).c_str(), _open_mode);
		
		if ( !_zipp ) {
			Qk_ERR("Cannot open file ZipWriter, %s", _path.c_str());
			return false;
		}
		return true;
	}

	/**
	 * @func close
	 */
	bool ZipWriter::close() {
		if ( _zipp ) {
			close_current_file();
			if ( zipClose((zipFile*)_zipp, NULL) == ZIP_OK ) {
				_zipp = nullptr;
			} else {
				Qk_ERR("Cannot close zip ZipWriter, %s", _path.c_str());
			}
		}
		return !_zipp;
	}

	bool ZipWriter::add_file(cString& path) {
		if ( close_current_file() ) {
			zip_fileinfo zipfi;
			
			int i = zipOpenNewFileInZip3((zipFile*)_zipp,
																	path.c_str(),
																	&zipfi,
																	NULL,
																	0,
																	NULL,
																	0,
																	NULL,
																	Z_DEFLATED,
																	_level,
																	0,
																	-MAX_WBITS,
																	DEF_MEM_LEVEL,
																	Z_DEFAULT_STRATEGY,
																	_passwd == "" ? NULL: _passwd.c_str(),
																	0);
			if ( i == ZIP_OK ) {
				return true;
			} else {
				Qk_ERR("add zip file error, `%s, %s`", _path.c_str(), path.c_str());
			}
		}
		return false;
	}

	bool ZipWriter::write(WeakBuffer data) {
		return zipWriteInFileInZip((zipFile*)_zipp, data.val(), data.length()) == 0;
	}

	bool ZipWriter::close_current_file() {
		if ( ! _new_name.is_empty() ) { // 当前有打开的新文件
			int code = zipCloseFileInZip((zipFile*)_zipp);
			if ( code != ZIP_OK ) {
				Qk_ERR("Cannot close file writer internal documents, %s, %s", _path.c_str(), _new_name.c_str());
				return false;
			}
			_new_name = String();
		}
		return true;
	}

}
