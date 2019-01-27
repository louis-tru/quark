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

#include <zlib.h>
#include <zip.h>
#include <unzip.h>
#include "./zlib.h"
#include "./handle.h"
#include "./error.h"

XX_NS(shark)

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

static Buffer _compress(cchar* data, uint len, int level) {
	Buffer rev, tmp(16384); // 16k
	z_stream strm;
	strm.zalloc = Z_NULL;
	strm.zfree = Z_NULL;
	strm.opaque = Z_NULL;
	strm.next_in = Z_NULL;
	strm.avail_in = 0;
	
	if ( deflateInit(&strm, level) != Z_OK ) {
		return rev;
	}
	
	strm.next_in = (byte*)data;
	strm.avail_in = len;
	do {
		strm.next_out = (byte*)*tmp;
		strm.avail_out = tmp.length();
		deflate(&strm, Z_FINISH);
		rev.write(tmp, rev.length(), tmp.length() - strm.avail_out);
	} while(strm.avail_out == 0);
	
	deflateEnd(&strm);
	return rev;
}

static Buffer _uncompress(cchar* data, uint len) {
	Buffer rev, tmp(16384); // 16k
	z_stream strm;
	strm.zalloc = Z_NULL;
	strm.zfree = Z_NULL;
	strm.opaque = Z_NULL;
	strm.next_in = Z_NULL;
	strm.avail_in = 0;
	
	if ( inflateInit(&strm) != Z_OK ) {
		return rev;
	}
	
	strm.next_in = (byte*)data;
	strm.avail_in = len;
	do {
		strm.next_out = (byte*)*tmp;
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
Buffer ZLib::compress(cString& str, int level) {
	return _compress(*str, str.length(), level);
}

/**
 * @func uncompress
 */
Buffer ZLib::uncompress(cString& str) {
	return _uncompress(*str, str.length());
}

/**
 * @func compress
 */
Buffer ZLib::compress(cBuffer& buff, int level) {
	return _compress(*buff, buff.length(), level);
}

/**
 * @func uncompress
 */
Buffer ZLib::uncompress(cBuffer& buff) {
	return _uncompress(*buff, buff.length());
}

GZip::~GZip() {
	if(m_gzfp) {
		gzclose((gzFile)m_gzfp);
	}
}

// Override
bool GZip::is_open() {
	return m_gzfp != NULL;
}

extern cchar* inl__file_flag_str(int flag);

// Override
bool GZip::open(int flag) {
	XX_ASSERT(!m_gzfp);
	if(m_gzfp){ // 已经打开了
		return true;
	}
	m_gzfp = gzopen(Path::fallback_c(m_path), inl__file_flag_str(flag));
	if(m_gzfp){
		return true;
	}
	return false;
}

// Override
bool GZip::close() {
	int i = gzclose((gzFile)m_gzfp);
	if(i == 0){
		m_gzfp = NULL;
		return true;
	}
	return false;
}

// Override
int GZip::read(void* buffer, int64 size, int64 offset) {
	if ( offset > -1 ) {
		gzseek((gzFile)m_gzfp, offset, SEEK_SET);
	}
	return gzread((gzFile)m_gzfp, buffer, uint(size));
}

// Override
int GZip::write(const void* buffer, int64 size, int64 offset) {
	if ( offset > -1 ) {
		gzseek((gzFile)m_gzfp, offset, SEEK_SET);
	}
	return gzwrite((gzFile)m_gzfp, buffer, uint(size));
}

XX_DEFINE_INLINE_MEMBERS(ZipReader, Inl) {
 public:
 #define _inl_reader(self) static_cast<ZipReader::Inl*>(self)
	
	bool m_open_current_file() {
		if ( m_is_open ) {
			return true;
		}
		if ( m_unzp ) {
			int code;
			if ( m_passwd.is_empty() ) {
				code = unzOpenCurrentFile((unzFile)m_unzp);
			} else {
				code = unzOpenCurrentFilePassword((unzFile)m_unzp, *m_passwd);
			}
			if ( code == UNZ_OK ) {
				m_is_open = true;
				return true;
			}
		}
		return false;
	}
	
	bool m_close_current_file() {
		if ( m_is_open ) {
			if ( unzCloseCurrentFile((unzFile)m_unzp) != UNZ_OK ) {
				return false;
			}
			m_is_open = false;
		}
		return true;
	}
	
	bool m_go_to_cur_pos() {
		if ( m_unzp ) {
			unz_entry_info& info = m_cur_it.value();
			unz_file_pos pos = { info.pos.pos_in_zip_directory, info.pos.num_of_file };
			int code = unzGoToFilePos((unzFile)m_unzp, &pos);
			return code == UNZ_OK;
		}
		return false;
	}
	
	void add_dir_info_item(cString& pathname, FileType type) {
		
		String dirname = Path::dirname(pathname);
		String compatible_path = m_compatible_path + '/' + pathname;
		
		if ( dirname.is_empty() ) {
			m_dir_info.get(dirname).push(Dirent(pathname, compatible_path, type));
		} else {
			String basename = pathname.substr(dirname.length() + 1);
			auto it = m_dir_info.find(dirname);
			if ( it.is_null() ) {
				add_dir_info_item(dirname, FILE_DIR);
				m_dir_info.get(dirname).push(Dirent(basename, compatible_path, type));
			} else {
				it.value().push(Dirent(basename, compatible_path, type));
			}
		}
	}
	
};

ZipReader::ZipReader(cString& path, cString& passwd)
	: m_path(Path::format(path)), m_passwd(passwd)
	, m_unzp(nullptr)
	, m_is_open(false)
{
	if ( Path::is_local_zip(m_path) ) { // zip:///
		m_compatible_path = m_path + '@';
	} else if ( Path::is_local_file(m_path) ) { // file:///
		m_compatible_path = String::format("zip:///%s@", *m_path.substr(8));
	}
}

ZipReader::~ZipReader() {
	close();
}

/**
 * @func
 */
bool ZipReader::open() {
	
	if ( m_unzp ) {
		XX_ERR("First close the open file");
		return false;
	}
	
	unzFile unzp = unzOpen(Path::fallback_c(m_path));
	if ( !unzp ) {
		XX_ERR("Cannot open file reader, %s", *m_path);
		return false;
	}
	
	ScopeClear clear([&]() {
		if ( unzClose((unzFile) unzp) != UNZ_OK ) {
			XX_ERR("Cannot close file reader, %s", *m_path);
		}
	});
	
	// Init
	char name[256 + 1];
	unz_file_info unzfi;
	unz_file_pos pos;
	int code;
	
	do {
		code = unzGetFilePos(unzp, &pos);
		if ( code ) {
			XX_ERR("Open current file pos info error"); return false;
		}
		m_unz_file_pos _pos = { pos.pos_in_zip_directory, pos.num_of_file };
		code = unzGetCurrentFileInfo(unzp, &unzfi, name, 256, NULL, 0, NULL, 0);
		if ( code ) {
			XX_ERR("Get current file info error"); return false;
		}
		String pathname = name;
		uint compressed_size = (uint)unzfi.compressed_size;
		uint uncompressed_size = (uint)unzfi.uncompressed_size;
		unz_entry_info info = { _pos, pathname, compressed_size, uncompressed_size };
		_inl_reader(this)->add_dir_info_item(pathname, FILE_FILE);
		m_file_info.set(info.pathname, info);
	} while(unzGoToNextFile(unzp) == UNZ_OK);
	
	first();
	
	clear.cancel(); // 安全通过后取消
	
	m_unzp = unzp;
	return true;
}

/**
 * @func close
 */
bool ZipReader::close() {
	if ( m_unzp ) {
		if ( !_inl_reader(this)->m_close_current_file() ) {
			XX_ERR("Cannot close file reader internal documents, %s, %s",
						 *m_path, *m_cur_it.value().pathname);
		}
		if ( unzClose((unzFile)m_unzp) == UNZ_OK ) {
			m_unzp = nullptr;
		} else {
			XX_ERR("Cannot close file reader, %s", *m_path);
		}
		m_file_info.clear();
		m_dir_info.clear();
	}
	return !m_unzp;
}

bool ZipReader::exists(cString& path) const {
	return m_file_info.has(path) || m_dir_info.has(path);
}

bool ZipReader::is_file(cString& path) const {
	return m_file_info.has(path);
}

bool ZipReader::is_directory(cString& path) const {
	return m_dir_info.has(path);
}

/**
 * @func readdir(path)
 */
Array<Dirent> ZipReader::readdir(cString& path) const {
	auto it = m_dir_info.find(path);
	if ( it.is_null() ) {
		return Array<Dirent>();
	} else {
		return it.value();
	}
}

bool ZipReader::jump(cString& path) {
	auto it = m_file_info.find(path);
	if ( it == m_file_info.end() ) {
		return false;
	}
	if (_inl_reader(this)->m_close_current_file() ) {
		m_cur_it = it;
		return _inl_reader(this)->m_go_to_cur_pos();
	}
	return false;
}

bool ZipReader::first() {
	if ( _inl_reader(this)->m_close_current_file() ) {
		m_cur_it = m_file_info.begin();
		return _inl_reader(this)->m_go_to_cur_pos();
	}
	return false;
}

bool ZipReader::next() {
	if (_inl_reader(this)->m_close_current_file() ) {
		if ( ++m_cur_it == m_file_info.end() ) {
			m_cur_it--;
			return false;
		}
		return _inl_reader(this)->m_go_to_cur_pos();
	}
	return false;
}

int ZipReader::read(void* buffer, int size) {
	if ( _inl_reader(this)->m_open_current_file() ) {
		return unzReadCurrentFile((unzFile)m_unzp, buffer, size);
	}
	return -1;
}

Buffer ZipReader::read() {
	int size = uncompressed_size();
	Buffer buffer(size, size + 1);
	size = read(*buffer, size);
	if (size < 0) { // err
		return Buffer();
	}
	*(*buffer + size) = '\0';
	return buffer;
}

Buffer ZipReader::read(uint size) {
	Buffer buffer(size, size + 1);
	int length = read(*buffer, size);
	if (length < 0) { // err
		return Buffer();
	}
	*(*buffer + length) = '\0';
	return buffer.realloc(length);
}

// ZipWriter

ZipWriter::ZipWriter(cString& path, cString& passwd)
	: m_path(path)
	, m_passwd(passwd)
	, m_open_mode(OPEN_MODE_CREATE)
	, m_level(-1)
	, m_zipp(nullptr)
	, m_new_name() {
		
}

ZipWriter::~ZipWriter() {
	close();
}

bool ZipWriter::open(OpenMode mode) {
	
	if ( m_zipp ) {
		XX_ERR("First close the open file");
		return false;
	}
	
	m_open_mode = mode;
	m_zipp = zipOpen(Path::fallback_c(m_path), m_open_mode);
	
	if ( !m_zipp ) {
		XX_ERR("Cannot open file writer, %s", *m_path);
		return false;
	}
	return true;
}

/**
 * @func close
 */
bool ZipWriter::close() {
	if ( m_zipp ) {
		close_current_file();
		if ( zipClose((zipFile*)m_zipp, NULL) == ZIP_OK ) {
			m_zipp = nullptr;
		} else {
			XX_ERR("Cannot close zip writer, %s", *m_path);
		}
	}
	return !m_zipp;
}

bool ZipWriter::add_file(cString& path) {
	if ( close_current_file() ) {
		zip_fileinfo zipfi;
		
		int i = zipOpenNewFileInZip3((zipFile*)m_zipp,
																 *path,
																 &zipfi,
																 NULL,
																 0,
																 NULL,
																 0,
																 NULL,
																 Z_DEFLATED,
																 m_level,
																 0,
																 -MAX_WBITS,
																 DEF_MEM_LEVEL,
																 Z_DEFAULT_STRATEGY,
																 m_passwd == "" ? NULL: *m_passwd,
																 0);
		if ( i == ZIP_OK ) {
			return true;
		} else {
			XX_ERR("add zip file error, `%s, %s`", *m_path, *path);
		}
	}
	return false;
}

bool ZipWriter::write(const void* buffer, uint size) {
	return zipWriteInFileInZip((zipFile*)m_zipp, buffer, size) == 0;
}

bool ZipWriter::write(cBuffer& data) {
	return write(*data, data.length());
}

/**
 * Write data to file
 */
bool ZipWriter::write(cString& str) {
	return write(*str, str.length());
}

bool ZipWriter::close_current_file() {
	if ( ! m_new_name.is_empty() ) { // 当前有打开的新文件
		int code = zipCloseFileInZip((zipFile*)m_zipp);
		if ( code != ZIP_OK ) {
			XX_ERR("Cannot close file writer internal documents, %s, %s", *m_path, *m_new_name);
			return false;
		}
		m_new_name = String();
	}
	return true;
}


XX_END
