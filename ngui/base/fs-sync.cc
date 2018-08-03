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

#include "error.h"
#include "fs.h"
#include <uv.h>

#if XX_WIN
#include <io.h>
#include <direct.h>
#else
#include <unistd.h>
#endif

XX_NS(ngui)

extern void inl__set_file_stat(FileStat* stat, uv_stat_t* uv_stat);
extern const int inl__file_mode_mask[];

// ---------------------------------------FileHelper------------------------------------------

static bool each_sync(Array<Dirent>& ls, cCb& cb, bool internal) {
	for ( auto& i : ls ) {
		Dirent& dirent = i.value();
		if ( !internal ) { // 外部优先
			if ( !sync_callback(cb, nullptr, &dirent) ) { // 停止遍历
				return false;
			}
		}
		if ( dirent.type == FILE_DIR ) { // 目录
			auto ls = FileHelper::ls_sync(dirent.pathname);
			if ( !each_sync(ls, cb, internal) ) { // 停止遍历
				return false;
			}
		}
		if ( internal ) { // 内部优先
			if ( !sync_callback(cb, nullptr, &dirent) ) { // 停止遍历
				return false;
			}
		}
	}
	return true;
}

bool FileHelper::each_sync(cString& path, cCb& cb, bool internal) {
	FileStat stat = stat_sync(path);
	if ( !stat.is_valid() ) {
		return false;
	}
	Array<Dirent> ls;
	ls.push(Dirent(Path::basename(path), Path::format("%s", *path), stat.type()));
	
	return ngui::each_sync(ls, cb, internal);
}

bool FileHelper::mkdir_p_sync(cString& path, uint mode) {
	
	cchar* path2 = Path::fallback_c(path);
	
	uv_fs_t req;
	
	if ( uv_fs_access(uv_default_loop(), &req, path2, F_OK, nullptr) == 0 ) {
		return true;
	}
	
	uint len = (uint)strlen(path2);
	char c = path2[len - 1];
	if (c == '/' || c == '\\') {
		len--;
	}
	char p[len + 1];
	memcpy(p, path2, len);
	p[len] = '\0';
	
	int i = len;
	
	for ( ; i > 0; i-- ) {
		
		c = p[i];
		
		if (c == '/' || c == '\\') {
			p[i] = '\0';
			
			if ( uv_fs_access(uv_default_loop(), &req, p, F_OK, nullptr) == 0 ) { // ok
				p[i] = '/';
				break;
			}
			
			while ( i > 1 ) {
				c = p[i - 1];
				if ( c == '/' || c == '\\') {
					i--;
				}
				else {
					break;
				}
			}
		}
	}
	
	i++;
	len++;
	
	for ( ; i < len; i++ ) {
		if (p[i] == '\0') {
			if ( uv_fs_mkdir(uv_default_loop(), &req, p, mode, nullptr) == 0 ) {
				if (i + 1 == len) {
					return true;
				}
				else {
					p[i] = '/';
				}
			}
			else {
				return false;
			}
		}
	}
	return false;
}

bool FileHelper::chmod_sync(cString& path, uint mode) {
	uv_fs_t req;
	return uv_fs_chmod(uv_default_loop(), &req, Path::fallback_c(path), mode, nullptr) == 0;
}

bool FileHelper::chown_sync(cString& path, uint owner, uint group) {
	uv_fs_t req;
	return uv_fs_chown(uv_default_loop(), &req, Path::fallback_c(path), owner, group, nullptr) == 0;
}

static bool default_stop_signal = false;

bool FileHelper::chmod_r_sync(cString& path, uint mode, bool* stop_signal) {
	
	if ( stop_signal == nullptr ) {
		stop_signal = &default_stop_signal;
	}
	
	uv_fs_t req;
	
	return each_sync(path, Cb([&](Se& d) {
		if ( *stop_signal ) { // 停止信号
			d.return_value = false;
		} else {
			Dirent* dirent = static_cast<Dirent*>(d.data);
			int r = uv_fs_chmod(uv_default_loop(), &req,
													Path::fallback_c(dirent->pathname), mode, nullptr);
			d.return_value = (r == 0);
		}
	}));
}

bool FileHelper::chown_r_sync(cString& path, uint owner, uint group, bool* stop_signal) {
	
	if (stop_signal == nullptr) {
		stop_signal = &default_stop_signal;
	}
	
	uv_fs_t req;
	
	return each_sync(path, Cb([&](Se& d) {
		if (*stop_signal) { // 停止信号
			d.return_value = false;
		} else {
			Dirent* dirent = static_cast<Dirent*>(d.data);
			int r = uv_fs_chown(uv_default_loop(), &req,
													Path::fallback_c(dirent->pathname), owner, group, nullptr);
			d.return_value = (r == 0);
		}
	}));
}

bool FileHelper::mkdir_sync(cString& path, uint mode) {
	uv_fs_t req;
	return uv_fs_mkdir(uv_default_loop(), &req, Path::fallback_c(path), mode, nullptr) == 0;
}

bool FileHelper::rename_sync(cString& name, cString& new_name) {
	uv_fs_t req;
	return uv_fs_rename(uv_default_loop(), &req,
											Path::fallback_c(name), Path::fallback_c(new_name), nullptr) == 0;
}

bool FileHelper::mv_sync(cString& name, cString& new_name) {
	return rename_sync(name, new_name);
}

bool FileHelper::unlink_sync(cString& path) {
	uv_fs_t req;
	return uv_fs_unlink(uv_default_loop(), &req, Path::fallback_c(path), nullptr) == 0;
}

bool FileHelper::rmdir_sync(cString& path) {
	uv_fs_t req;
	return uv_fs_rmdir(uv_default_loop(), &req, Path::fallback_c(path), nullptr) == 0;
}

bool FileHelper::rm_r_sync(cString& path, bool* stop_signal) {
	
	if ( stop_signal == nullptr ) {
		stop_signal = &default_stop_signal;
	}
	uv_fs_t req;
	
	return each_sync(path, Cb([&](Se& d) {
		if ( *stop_signal ) { // 停止信号
			d.return_value = false;
		} else {
			Dirent* dirent = static_cast<Dirent*>(d.data);
			cchar* p = Path::fallback_c(dirent->pathname);
			if ( dirent->type == FILE_DIR ) {
				d.return_value = uv_fs_rmdir(uv_default_loop(), &req, p, nullptr) == 0;
			} else {
				d.return_value = uv_fs_unlink(uv_default_loop(), &req, p, nullptr) == 0;
			}
		}
	}), true);
}

static bool cp_sync2(cString& source, cString& target, bool* stop_signal) {
	
	File source_file(source);
	File target_file(target);
	
	if ( source_file.open(FOPEN_R) && target_file.open(FOPEN_W) ) {
		
		int size = 1024 * 512; // 512 kb
		Buffer data(size);
		
		int64 len = source_file.read(*data, size);
		
		while ( len > 0 ) {
			if ( target_file.write(*data, len) != len ) { // 写入数据失败
				return false;
			}
			if ( *stop_signal ) { // 停止信号
				return  false;
			}
			len = source_file.read(*data, size);
		}
		
		return true;
	}
	
	return false;
}

bool FileHelper::cp_sync(cString& source, cString& target, bool* stop_signal) {
	return cp_sync2(source, target, stop_signal ? stop_signal : &default_stop_signal);
}

bool FileHelper::cp_r_sync(cString& source, cString& target, bool* stop_signal) {
	
	if ( !is_directory_sync(Path::dirname(target)) ) { // 没有父目录,无法复制
		return false;
	}
	
	if ( stop_signal == nullptr ) {
		stop_signal = &default_stop_signal;
	}
	
	uint s_len = Path::format("%s", *source).length();
	String xx_path = Path::format("%s", *target);
	
	return each_sync(source, Cb([&](Se& d) {
		
		Dirent* dirent = static_cast<Dirent*>(d.data);
		String target = xx_path + dirent->pathname.substr(s_len); // 目标文件
		
		switch (dirent->type) {
			case FILE_DIR:
				d.return_value = mkdir_sync(target); /* create dir */
				break;
			case FILE_FILE:
				d.return_value = cp_sync2(dirent->pathname, target, stop_signal);
				break;
			default: break;
		}
		if ( *stop_signal ) { // 停止信号
			d.return_value = false;
		}
	}));
}

Array<Dirent> FileHelper::readdir_sync(cString& path) {
	return ls_sync(path);
}

Array<Dirent> FileHelper::ls_sync(cString& path) {
	Array<Dirent> ls;
	uv_fs_t req;
	String p = Path::format("%s", *path) + '/';
	if ( uv_fs_scandir(uv_default_loop(), &req, Path::fallback_c(path), 1, nullptr) ) {
		uv_dirent_t ent;
		while ( uv_fs_scandir_next(&req, &ent) == 0 ) {
			ls.push( Dirent(ent.name, p + ent.name, FileType(ent.type)) );
		}
	}
	uv_fs_req_cleanup(&req);
	return ls;
}

FileStat FileHelper::stat_sync(cString& path) {
	FileStat stat;
	uv_fs_t req;
	if (uv_fs_stat(uv_default_loop(), &req, Path::fallback_c(path), nullptr) == 0) {
		inl__set_file_stat(&stat, &req.statbuf);
	}
	return stat;
}

bool FileHelper::exists_sync(cString& path) {
	uv_fs_t req;
	return uv_fs_access(uv_default_loop(), &req, Path::fallback_c(path), F_OK, nullptr) == 0;
}

bool FileHelper::is_file_sync(cString& path) {
	uv_fs_t req;
	if (uv_fs_stat(uv_default_loop(), &req, Path::fallback_c(path), nullptr) == 0) {
		return !S_ISDIR(req.statbuf.st_mode);
	}
	return false;
}

bool FileHelper::is_directory_sync(cString& path) {
	uv_fs_t req;
	if (uv_fs_stat(uv_default_loop(), &req, Path::fallback_c(path), nullptr) == 0) {
		return S_ISDIR(req.statbuf.st_mode);
	}
	return false;
}

bool FileHelper::readable_sync(cString& path) {
	uv_fs_t req;
	return uv_fs_access(uv_default_loop(), &req, Path::fallback_c(path), W_OK, nullptr) == 0;
}

bool FileHelper::writable_sync(cString& path) {
	uv_fs_t req;
	return uv_fs_access(uv_default_loop(), &req, Path::fallback_c(path), W_OK, nullptr) == 0;
}

bool FileHelper::executable_sync(cString& path) {
	uv_fs_t req;
	int r = uv_fs_access(uv_default_loop(), &req, Path::fallback_c(path), X_OK, nullptr);
	return r = 0;
}

// --------------- file read write ---------------

// read file

Buffer FileHelper::read_file_sync(cString& path, int* err) {
	
	int64 size = -1, offset = -1;
	
	Buffer buff;
	uv_fs_t req;
	int fd = uv_fs_open(uv_default_loop(), &req, Path::fallback_c(path), O_RDONLY, 0, nullptr);
	int r = 0;
	
	while (1) {
		if ( fd < 0 ) {
			r = fd;
			break;
		}
		if ( size < 0 ) {
			r = uv_fs_fstat(uv_default_loop(), &req, fd, nullptr);
			if ( r < 0 )
				break;
			size = req.statbuf.st_size;
		}
		char* buffer = (char*)::malloc(size + 1); // 为兼容C字符串多加1位0
		if ( buffer ) {
			uv_buf_t buf;
			buf.base = buffer;
			buf.len = size;
			r = uv_fs_read(uv_default_loop(), &req, fd, &buf, 1, offset, nullptr);
			
			if ( r > 0 ) {
				buffer[r] = '\0';
				buff = Buffer(buffer, r);
			} else {
				free(buffer);
			}
		}
		break;
	}
	
	if ( fd > 0 ) {
		uv_fs_close(uv_default_loop(), &req, fd, nullptr);
	}
	if ( err ) {
		*err = r < 0 ? r : 0;
	}
	return buff;
}

// write file

int FileHelper::write_file_sync(cString& path, cString& str) {
	return write_file_sync(path, *str, str.length() );
}
int FileHelper::write_file_sync(cString& path, const void* buffer, int64 size) {
	uv_fs_t req;
	int fp = uv_fs_open(uv_default_loop(), &req,
											Path::fallback_c(path),
											O_WRONLY | O_CREAT | O_TRUNC, default_mode, nullptr);
	uv_buf_t buf;
	buf.base = (char*)buffer;
	buf.len = size < 0 ? 0 : size;
	int r = uv_fs_write(uv_default_loop(), &req, fp, &buf, 1, -1, nullptr);
	uv_fs_close(uv_default_loop(), &req, fp, nullptr);
	return r;
}
// open/close file fd
int FileHelper::open_sync(cString& path, FileOpenMode mode) {
	uv_fs_t req;
	int fd = uv_fs_open(uv_default_loop(), &req,
											Path::fallback_c(path),
											inl__file_mode_mask[mode], default_mode, nullptr);
	return fd;
}
int FileHelper::close_sync(int fd) {
	uv_fs_t req;
	int r = uv_fs_close(uv_default_loop(), &req, fd, nullptr);
	return r;
}
// read with fd
int FileHelper::read_sync(int fd, void* buffer, int64 size, int64 offset) {
	uv_fs_t req;
	uv_buf_t buf;
	buf.base = (char*)buffer;
	buf.len = size;
	int r = uv_fs_read(uv_default_loop(), &req, fd, &buf, 1, offset, nullptr);
	return r;
}
int FileHelper::write_sync(int fd, const void* buffer, int64 size, int64 offset) {
	uv_fs_t req;
	uv_buf_t buf;
	buf.base = (char*)buffer;
	buf.len = size;
	int r = uv_fs_write(uv_default_loop(), &req, fd, &buf, 1, offset, nullptr);
	return r;
}

XX_END
