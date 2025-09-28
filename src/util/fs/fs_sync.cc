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

#include "../error.h"
#include "../fs.h"
#include <uv.h>
#if Qk_WIN
#include <io.h>
#include <direct.h>
#else
#include <unistd.h>
#endif

#define UV_LOOP RunLoop::current()->uv_loop()

namespace qk {

	void file_stat_copy(uv_stat_t* uv_stat, FileStat* stat);
	int  file_flag_mask(int flag);

	static bool default_stop_signal = false;

	static void uv_error(int err, cChar* msg = nullptr) throw(Error) {
		throw Error((int)err, "%s, %s, %s",
								uv_err_name((int)errno), uv_strerror((int)err), msg ? msg: "");
	}

	static bool each_sync_inl(Array<Dirent>& ls, Callback<Dirent>& cb, bool internal) throw(Error) {
		for ( auto& dirent : ls ) {
			if ( !internal ) { // external priority
				if ( cb->resolve(&dirent) ) { // stop each
					return false;
				}
			}
			if ( dirent.type == FTYPE_DIR ) { // directory
				auto ls = fs_readdir_sync(dirent.pathname);
				if ( !each_sync_inl(ls, cb, internal) ) { // stop each
					return false;
				}
			}
			if ( internal ) { // internal priority
				if ( cb->resolve(&dirent) ) { // stop each
					return false;
				}
			}
		}
		return true;
	}

	bool fs_each_sync(cString& path, Callback<Dirent> cb, bool internal) throw(Error) {
		FileStat stat = fs_stat_sync(path);
		Array<Dirent> ls = {
			Dirent{fs_basename(path), fs_format("%s", *path), stat.type()}
		};
		return each_sync_inl(ls, cb, internal);
	}

	void fs_chmod_sync(cString& path, uint32_t mode) throw(Error) {
		uv_fs_t req;
		int r = uv_fs_chmod(UV_LOOP, &req, fs_fallback_c(path), mode, nullptr);
		if (r != 0) {
			uv_error(r, *path);
		}
	}

	void fs_chown_sync(cString& path, uint32_t owner, uint32_t group) throw(Error) {
		uv_fs_t req;
		int r = uv_fs_chown(UV_LOOP, &req, fs_fallback_c(path), owner, group, nullptr);
		if (r != 0) {
			uv_error(r, *path);
		}
	}

	void fs_mkdir_sync(cString& path, uint32_t mode) throw(Error) {
		uv_fs_t req;
		int r = uv_fs_mkdir(UV_LOOP, &req, fs_fallback_c(path), mode, nullptr);
		if (r != 0) {
			uv_error(r, *path);
		}
	}

	void fs_mkdirs_sync(cString& path, uint32_t mode) throw(Error) {

		cChar* path2 = fs_fallback_c(path);

		uv_fs_t req;
		
		if ( uv_fs_access(UV_LOOP, &req, path2, F_OK, nullptr) == 0 ) {
			return;
		}
		
		int len = strlen(path2);
		Char c = path2[len - 1];
		if (c == '/' || c == '\\') {
			len--;
		}

		Buffer buf(len);
		char *p = buf.val();
		memcpy(p, path2, len);
		// p[len] = '\0';

		int i = len;
		
		for ( ; i > 0; i-- ) {
			
			c = p[i];
			
			if (c == '/' || c == '\\') {
				p[i] = '\0';
				
				if ( uv_fs_access(UV_LOOP, &req, p, F_OK, nullptr) == 0 ) { // ok
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
				int r = uv_fs_mkdir(UV_LOOP, &req, p, mode, nullptr);
				if ( r == 0 ) {
					if (i + 1 == len) {
						return;
					}
					else {
						p[i] = '/';
					}
				}
				else {
					uv_error(r, *path); // throw error
				}
			}
		}
	}

	void fs_rename_sync(cString& name, cString& new_name) throw(Error) {
		uv_fs_t req;
		int r = uv_fs_rename(UV_LOOP, &req,
												fs_fallback_c(name), fs_fallback_c(new_name), nullptr);
		if (r != 0) {
			uv_error(r, *String::format("rename %s to %s", *name, *new_name));
		}
	}

	void fs_link_sync(cString& path, cString& newPath) throw(Error) {
		uv_fs_t req;
		int r = uv_fs_link(
			UV_LOOP, &req,
			fs_fallback_c(path),
			fs_fallback_c(newPath), nullptr
		);
		if (r != 0) {
			uv_error(r, *String::format("link %s to %s", *path, *newPath));
		}
	}

	void fs_unlink_sync(cString& path) throw(Error) {
		uv_fs_t req;
		int r = uv_fs_unlink(UV_LOOP, &req, fs_fallback_c(path), nullptr);
		if ( r != 0 ) {
			uv_error(r, *path);
		}
	}

	void fs_rmdir_sync(cString& path) throw(Error) {
		uv_fs_t req;
		int r = uv_fs_rmdir(UV_LOOP, &req, fs_fallback_c(path), nullptr);
		if ( r != 0 ) {
			uv_error(r, *path);
		}
	}

	Array<Dirent> fs_readdir_sync(cString& path) throw(Error) {
		Array<Dirent> ls;
		uv_fs_t req;
		String p = fs_format("%s", *path) + '/';
		int r = uv_fs_scandir(UV_LOOP, &req, fs_fallback_c(path), 1, nullptr);
		if ( r > 0 ) {
			uv_dirent_t ent;
			while ( uv_fs_scandir_next(&req, &ent) == 0 ) {
				ls.push( Dirent{ent.name, p + ent.name, FileType(ent.type)} );
			}
		} else if ( r < 0) {
			uv_error(r, *path);
		}
		uv_fs_req_cleanup(&req);
		return ls;
	}

	FileStat fs_stat_sync(cString& path) throw(Error) {
		FileStat stat;
		uv_fs_t req;
		int r = uv_fs_stat(UV_LOOP, &req, fs_fallback_c(path), nullptr);
		if (r == 0) {
			file_stat_copy(&req.statbuf, &stat);
		} else {
			uv_error(r, *path);
		}
		return stat;
	}

	bool fs_exists_sync(cString& path) {
		uv_fs_t req;
		return uv_fs_access(UV_LOOP, &req, fs_fallback_c(path), F_OK, nullptr) == 0;
	}

	bool fs_is_file_sync(cString& path) {
		uv_fs_t req;
		if (uv_fs_stat(UV_LOOP, &req, fs_fallback_c(path), nullptr) == 0) {
			return !S_ISDIR(req.statbuf.st_mode);
		}
		return false;
	}

	bool fs_is_directory_sync(cString& path) {
		uv_fs_t req;
		if (uv_fs_stat(UV_LOOP, &req, fs_fallback_c(path), nullptr) == 0) {
			return S_ISDIR(req.statbuf.st_mode);
		}
		return false;
	}

	bool fs_readable_sync(cString& path) {
		uv_fs_t req;
		return uv_fs_access(UV_LOOP, &req, fs_fallback_c(path), R_OK, nullptr) == 0;
	}

	bool fs_writable_sync(cString& path) {
		uv_fs_t req;
		return uv_fs_access(UV_LOOP, &req, fs_fallback_c(path), W_OK, nullptr) == 0;
	}

	bool fs_executable_sync(cString& path) {
		uv_fs_t req;
		int r = uv_fs_access(UV_LOOP, &req, fs_fallback_c(path), X_OK, nullptr);
		return (r == 0);
	}

	bool fs_chmod_recursion_sync(cString& path, uint32_t mode, bool* stop_signal) throw(Error) {
		if ( stop_signal == nullptr ) {
			stop_signal = &default_stop_signal;
		}

		uv_fs_t req;
		
		return fs_each_sync(path, Callback<Dirent>([&](Callback<Dirent>::Data& d) {
			if ( *stop_signal ) { // 停止信号
				d.rc = 1;
			} else {
				Dirent* dirent = d.data;
				int r = uv_fs_chmod(UV_LOOP, &req,
														fs_fallback_c(dirent->pathname), mode, nullptr);
				if (r != 0) {
					uv_error(r, *dirent->pathname);
				}
			}
		}));
	}

	bool fs_chown_recursion_sync(cString& path, uint32_t owner, uint32_t group, bool* stop_signal) throw(Error) 
	{
		if (stop_signal == nullptr) {
			stop_signal = &default_stop_signal;
		}
		uv_fs_t req;
		
		return fs_each_sync(path, Callback<Dirent>([&](Callback<Dirent>::Data& d) {
			if (*stop_signal) { // 停止信号
				d.rc = 1;
			} else {
				Dirent* dirent = d.data;
				int r = uv_fs_chown(UV_LOOP, &req,
														fs_fallback_c(dirent->pathname), owner, group, nullptr);
				if (r != 0) {
					uv_error(r, *dirent->pathname);
				}
			}
		}));
	}

	bool fs_remove_recursion_sync(cString& path, bool* stop_signal) throw(Error) {
		if (!fs_exists_sync(path)) {
			return true;
		}
		if ( stop_signal == nullptr ) {
			stop_signal = &default_stop_signal;
		}
		uv_fs_t req;

		return fs_each_sync(path, Callback<Dirent>([&](Callback<Dirent>::Data& d) {
			if ( *stop_signal ) { // 停止信号
				d.rc = 0;
			} else {
				Dirent* dirent = d.data;
				cChar* p = fs_fallback_c(dirent->pathname);
				int r;
				if ( dirent->type == FTYPE_DIR ) {
					r = uv_fs_rmdir(UV_LOOP, &req, p, nullptr);
				} else {
					r = uv_fs_unlink(UV_LOOP, &req, p, nullptr);
				}
				if (r != 0) {
					uv_error(r, *dirent->pathname);
				}
			}
		}), true);
	}

	bool fs_copy_recursion_sync(cString& source, cString& target, bool* stop_signal) throw(Error) {

		if ( !fs_is_directory_sync(fs_dirname(target)) ) { // 没有父目录,无法复制
			return false;
		}

		if ( stop_signal == nullptr ) {
			stop_signal = &default_stop_signal;
		}

		uint32_t s_len = fs_format("%s", *source).length();
		String path = fs_format("%s", *target);

		return fs_each_sync(source, Callback<Dirent>([&](Callback<Dirent>::Data& d) {
			if ( *stop_signal ) { // 停止信号
				d.rc = 1;
			} else {
				Dirent* dirent = d.data;
				String target = path + dirent->pathname.substr(s_len); // 目标文件

				switch (dirent->type) {
					case FTYPE_DIR:
						if (!fs_exists_sync(target))
							fs_mkdir_sync(target); /* create dir */
						break;
					case FTYPE_FILE:
						d.rc = fs_copy_sync(dirent->pathname, target, stop_signal);
						break;
					default: break;
				}
			}
		}));
	}

	bool fs_copy_sync(cString& source, cString& target, bool* stop_signal) throw(Error) {
		if (stop_signal == nullptr) {
			stop_signal = &default_stop_signal;
		}

		FileSync source_file(source);
		FileSync target_file(target);
		int r;
		r = source_file.open(FOPEN_R);
		if (r) {
			uv_error(r, *source);
		}
		r = target_file.open(FOPEN_W);
		if (r) {
			uv_error(r, *target);
		}
			
		int size = 1024 * 512; // 512 kb
		auto data = Buffer::alloc(size);

		int64_t len = source_file.read(*data, size);
		
		while ( len > 0 ) {
			r = target_file.write(*data, len);
			if ( r != len ) { // 写入数据失败
				uv_error(r, *target);
			}
			if ( *stop_signal ) { // 停止信号
				return false;
			}
			len = source_file.read(*data, size);
			if (len < 0) {
				uv_error(r, *source);
			}
		}

		return true;
	}

	// --------------- file read write ---------------

	// read file
	Buffer fs_read_file_sync(cString& path, int64_t size) throw(Error) {

		int64_t offset = -1;

		Buffer buff;
		uv_fs_t req;
		int fp = fs_open_sync(path, O_RDONLY);
		int r = 0;

		do {
			if ( size < 0 ) {
				r = uv_fs_fstat(UV_LOOP, &req, fp, nullptr);
				if ( r < 0 )
					break;
				size = req.statbuf.st_size;
			}
			Char* buffer = (Char*)::malloc(size + 1); // 为兼容C字符串多加1位0
			if ( buffer ) {
				uv_buf_t buf;
				buf.base = buffer;
				buf.len = size;
				r = uv_fs_read(UV_LOOP, &req, fp, &buf, 1, offset, nullptr);

				if ( r > 0 ) {
					buffer[r] = '\0';
					buff = Buffer::from(buffer, r);
				} else {
					free(buffer);
				}
			}
			break;
		} while(0);

		if ( fp > 0 ) {
			fs_close_sync(fp);
		}

		if ( r < 0 ) {
			uv_error(r, *path);
		}

		return buff;
	}

	// write file
	int fs_write_file_sync(cString& path, cString& str) throw(Error) {
		return fs_write_file_sync(path, *str, str.length() );
	}
	
	int fs_write_file_sync(cString& path, cVoid* buffer, int64_t size) throw(Error) {
		uv_fs_t req;
		int fp = fs_open_sync(path, O_WRONLY | O_CREAT | O_TRUNC);
		uv_buf_t buf;
		buf.base = (Char*)buffer;
		buf.len = size < 0 ? 0 : size;
		int r = uv_fs_write(UV_LOOP, &req, fp, &buf, 1, -1, nullptr);
		uv_fs_close(UV_LOOP, &req, fp, nullptr);
		return r;
	}

	// open/close file fd
	int fs_open_sync(cString& path, int flag) throw(Error) {
		uv_fs_t req;
		int fp = uv_fs_open(UV_LOOP, &req,
												fs_fallback_c(path),
												file_flag_mask(flag), fs_default_mode, nullptr);
		if ( fp < 0 ) {
			uv_error(fp, *path);
		}
		return fp;
	}

	void fs_close_sync(int fd) throw(Error) {
		uv_fs_t req;
		int r = uv_fs_close(UV_LOOP, &req, fd, nullptr);
		if (r != 0) {
			uv_error(r);
		}
	}

	// read with fd
	int fs_read_sync(int fd, void* buffer, int64_t size, int64_t offset) throw(Error) {
		uv_fs_t req;
		uv_buf_t buf;
		buf.base = (Char*)buffer;
		buf.len = size;
		int r = uv_fs_read(UV_LOOP, &req, fd, &buf, 1, offset, nullptr);
		if (r < 0) {
			uv_error(r);
		}
		return r;
	}

	int fs_write_sync(int fd, cVoid* buffer, int64_t size, int64_t offset) throw(Error) {
		uv_fs_t req;
		uv_buf_t buf;
		buf.base = (Char*)buffer;
		buf.len = size;
		int r = uv_fs_write(UV_LOOP, &req, fd, &buf, 1, offset, nullptr);
		if (r < 0) {
			uv_error(r);
		}
		return r;
	}

}
