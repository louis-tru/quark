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

#ifndef __ftr__util__fs__
#define __ftr__util__fs__

#include <ftr/util/cb.h>
#include <ftr/util/handle.h>
#include <ftr/util/loop.h>
#include <vector>

namespace ftr {

	/**
	* @enum FileOpenFlag # File open flag
	*/
	enum FileOpenFlag {
		FOPEN_ACCMODE = 03,
		FOPEN_RDONLY = 00,
		FOPEN_WRONLY = 01,
		FOPEN_RDWR = 02,
		FOPEN_CREAT = 0100,
		FOPEN_EXCL = 0200,
		FOPEN_NOCTTY = 0400,
		FOPEN_TRUNC = 01000,
		FOPEN_APPEND = 02000,
		FOPEN_NONBLOCK = 04000,
		// r 打开只读文件，该文件必须存在。
		FOPEN_R = FOPEN_RDONLY,
		// w 打开只写文件，若文件存在则文件长度清为零，即该文件内容会消失，若文件不存在则建立该文件。
		FOPEN_W = FOPEN_WRONLY | FOPEN_CREAT | FOPEN_TRUNC,
		// a 以附加的方式打开只写文件。若文件不存在，则会建立该文件，如果文件存在，
		//   写入的数据会被加到文件尾，即文件原先的内容会被保留。
		FOPEN_A = FOPEN_WRONLY | FOPEN_CREAT | FOPEN_APPEND,
		// r+ 打开可读写文件，该文件必须存在。
		FOPEN_RP = FOPEN_RDWR,
		// w+ 打开可读写文件，若文件存在则文件长度清为零，即该文件内容会消失。
		//    若文件不存在则建立该文件。
		FOPEN_WP = FOPEN_RDWR | FOPEN_CREAT | FOPEN_TRUNC,
		// a+	以附加方式打开可读写的文件。若文件不存在，则会建立该文件，如果文件存在，
		//    写入的数据会被加到文件尾后，即文件原先的内容会被保留。
		FOPEN_AP = FOPEN_RDWR | FOPEN_CREAT | FOPEN_APPEND,
	};

	enum FileType {
		FTYPE_UNKNOWN,
		FTYPE_FILE,
		FTYPE_DIR,
		FTYPE_LINK,
		FTYPE_FIFO,
		FTYPE_SOCKET,
		FTYPE_CHAR,
		FTYPE_BLOCK
	};

	class FX_EXPORT Dirent: public Object {
		public:
		inline Dirent() {}
		inline Dirent(const String& n, const String& p, FileType t)
			: name(n), pathname(p), type(t) {
		}
		String name;
		String pathname;
		FileType type;
	};

	/**
	* @class FileStat
	*/
	class FX_EXPORT FileStat: public Object {
			FX_HIDDEN_ALL_COPY(FileStat);
		public:
			FileStat();
			FileStat(const String& path);
			FileStat(FileStat&& stat);
			FileStat& operator=(FileStat&& stat);
			virtual ~FileStat();
			bool is_valid() const;
			bool is_file() const;
			bool is_dir() const;
			bool is_link() const;
			bool is_sock() const;
			uint64_t mode() const;
			FileType type() const;
			uint64_t group() const;
			uint64_t owner() const;
			uint64_t nlink() const;
			uint64_t ino() const;
			uint64_t blksize() const;
			uint64_t blocks() const;
			uint64_t flags() const;
			uint64_t gen() const;
			uint64_t dev() const;
			uint64_t rdev() const;
			uint64_t size() const;
			uint64_t atime() const;
			uint64_t mtime() const;
			uint64_t ctime() const;
			uint64_t birthtime() const;
		private:
			FX_DEFINE_INLINE_CLASS(Inl);
			void* _stat;
	};

	/**
	* @class File
	*/
	class FX_EXPORT File: public Object {
			FX_HIDDEN_ALL_COPY(File);
		public:
			typedef ObjectTraits Traits;
			inline  File(const String& path): _path(path), _fp(0) { }
			virtual ~File();
			inline  String path() const { return _path; }
			virtual bool is_open();
			virtual int open(int flag = FOPEN_R);
			virtual int close();
			virtual int read(void* buffer, int64_t size, int64_t offset = -1);
			virtual int write(const void* buffer, int64_t size, int64_t offset = -1);
		private:
			String _path;
			int    _fp;
	};

	/**
	* @class AsyncFile
	*/
	class FX_EXPORT AsyncFile: public Object {
			FX_HIDDEN_ALL_COPY(AsyncFile);
		public:
			typedef ObjectTraits Traits;
			class FX_EXPORT Delegate {
				public:
					virtual void trigger_async_file_open(AsyncFile* file) = 0;
					virtual void trigger_async_file_close(AsyncFile* file) = 0;
					virtual void trigger_async_file_error(AsyncFile* file, cError& error) = 0;
					virtual void trigger_async_file_read(AsyncFile* file, Buffer buffer, int mark) = 0;
					virtual void trigger_async_file_write(AsyncFile* file, Buffer buffer, int mark) = 0;
			};
			AsyncFile(const String& path, RunLoop* loop = RunLoop::current());
			virtual ~AsyncFile();
			String path() const;
			void set_delegate(Delegate* delegate);
			bool is_open();
			void open(int flag = FOPEN_R);
			void close();
			void read(Buffer buffer, int64_t offset = -1, int mark = 0);
			void write(Buffer buffer, int64_t offset = -1, int mark = 0);
		private:
			FX_DEFINE_INLINE_CLASS(Inl);
			Inl* _inl;
	};

	/**
	* @class FileHelper
	* @static
	*/
	class FX_EXPORT FileHelper {
		public:
			static const uint32_t default_mode;
			
			/**
			* @func each_sync 递归遍历子文件与子目录, 遍历回调回返0停止遍历
			*/
			static bool each_sync(const String& path, cCb& cb, bool internal = false) throw(Error);
			
			// sync
			/**
			* @func chmod_sync
			*/
			static void chmod_sync(const String& path, uint32_t mode = default_mode) throw(Error);
			
			/**
			* @func chmod_p  # 递归设置
			*                # 多线程中,设置stop_signal值为true来终止操作
			*/
			static void chown_sync(const String& path, uint32_t owner, uint32_t group) throw(Error);
			static void mkdir_sync(const String& path, uint32_t mode = default_mode) throw(Error);
			static void rename_sync(const String& name, const String& new_name) throw(Error);
			static void link_sync(const String& path, const String& newPath) throw(Error);
			static void unlink_sync(const String& path) throw(Error);
			static void rmdir_sync(const String& path) throw(Error);
			static std::vector<Dirent> readdir_sync(const String& path) throw(Error);
			static FileStat stat_sync(const String& path) throw(Error);
			static bool exists_sync(const String& path);
			static bool is_file_sync(const String& path);
			static bool is_directory_sync(const String& path);
			static bool readable_sync(const String& path);
			static bool writable_sync(const String& path);
			static bool executable_sync(const String& path);
			// recursion
			static bool chmod_r_sync(const String& path, uint32_t mode = default_mode, bool* stop_signal = nullptr) throw(Error);
			static bool chown_r_sync(const String& path, uint32_t owner, uint32_t group, bool* stop_signal = nullptr) throw(Error);
			static void mkdir_p_sync(const String& path, uint32_t mode = default_mode) throw(Error);
			static bool remove_r_sync(const String& path, bool* stop_signal = nullptr) throw(Error);
			static bool copy_sync(const String& source, const String& target, bool* stop_signal = nullptr) throw(Error);
			static bool copy_r_sync(const String& source, const String& target, bool* stop_signal = nullptr) throw(Error);
			// async
			static void chmod(const String& path, uint32_t mode = default_mode, cCb& cb = 0);
			static void chown(const String& path, uint32_t owner, uint32_t group, cCb& cb = 0);
			static void mkdir(const String& path, uint32_t mode = default_mode, cCb& cb = 0);
			static void rename(const String& name, const String& new_name, cCb& cb = 0);
			static void link(const String& path, const String& newPath, cCb& cb = 0);
			static void unlink(const String& path, cCb& cb = 0);
			static void rmdir(const String& path, cCb& cb = 0);
			static void readdir(const String& path, cCb& cb = 0);
			static void stat(const String& path, cCb& cb = 0);
			static void exists(const String& path, cCb& cb = 0);
			static void is_file(const String& path, cCb& cb = 0);
			static void is_directory(const String& path, cCb& cb = 0);
			static void readable(const String& path, cCb& cb = 0);
			static void writable(const String& path, cCb& cb = 0);
			static void executable(const String& path, cCb& cb = 0);
			// recursion
			static void mkdir_p(const String& path, uint32_t mode = default_mode, cCb& cb = 0);
			static uint32_t chmod_r(const String& path, uint32_t mode = default_mode, cCb& cb = 0);
			static uint32_t chown_r(const String& path, uint32_t owner, uint32_t group, cCb& cb = 0);
			static uint32_t remove_r(const String& path, cCb& cb = 0);
			static uint32_t copy(const String& source, const String& target, cCb& cb = 0);
			static uint32_t copy_r(const String& source, const String& target, cCb& cb = 0);
			static void abort(uint32_t id);
			// read stream
			static uint32_t read_stream(const String& path, cCb& cb = 0);
			// read file
			static Buffer read_file_sync(const String& path, int64_t size = -1) throw(Error);
			static void read_file(const String& path, cCb& cb = 0, int64_t size = -1);
			// write file
			static int  write_file_sync(const String& path, const String& str) throw(Error);
			static int  write_file_sync(const String& path, const void* data, int64_t size) throw(Error);
			static void write_file(const String& path, const String& str, cCb& cb = 0);
			static void write_file(const String& path, Buffer buffer, cCb& cb = 0);
			// open file fd
			static int  open_sync(const String& path, int flag = FOPEN_R) throw(Error);
			static void open(const String& path, int flag = FOPEN_R, cCb& cb = 0);
			static void open(const String& path, cCb& cb = 0);
			static void close_sync(int fd) throw(Error);
			static void close(int fd, cCb& cb = 0);
			// read with fd
			static int  read_sync(int fd, void* data, int64_t size, int64_t offset = -1) throw(Error);
			static int  write_sync(int fd, const void* data, int64_t size, int64_t offset = -1) throw(Error);
			static void read(int fd, Buffer buffer, cCb& cb);
			static void read(int fd, Buffer buffer, int64_t offset = -1, cCb& cb = 0);
			static void write(int fd, Buffer buffer, cCb& cb);
			static void write(int fd, Buffer buffer, int64_t offset = -1, cCb& cb = 0);
	};

	/**
	* @class Path
	* @static
	*/
	class FX_EXPORT Path {
		public:
			/**
			* @func extname {String} # Get the path basename
			* @ret {String}
			*/
			static String basename(const String& path);
			
			/**
			* @func extname {String} # Get the path dirname
			* @arg path {const String&}
			* @ret {String}
			*/
			static String dirname(const String& path);
			
			/**
			* @func extname # Get the path extname
			* @arg path {const String&}
			* @ret {String}
			*/
			static String extname(const String& path);
			
			/**
			* @func executable_path # Get the executable path
			* @ret {const String&}
			*/
			static String executable();
			
			/**
			* @func documents_dir # Get the documents dir.
			* @ret {const String&} # The path that can be write/read a file in
			*/
			static String documents(const String& child = String());
			
			/**
			* @func temp_dir # Get the temp dir.
			* @ret {const String&} # The path that can be write/read a file in
			*/
			static String temp(const String& child = String());
			
			/**
			* @func resources_dir # Get the resoures dir
			* @ret {const String&}
			*/
			static String resources(const String& child = String());
			
			/**
			* @func is_absolute # Is absolute path
			* @ret {bool}
			*/
			static bool is_local_absolute(const String& path);
			
			/**
			* @func is_local_zip
			*/
			static bool is_local_zip(const String& path);
			
			/**
			* @func is_local_file
			*/
			static bool is_local_file(const String& path);
			
			/**
			* @func format
			* @arg format {const char*}
			* @arg [...] {const char*}
			* @ret {String}
			*/
			static SString format(const char* path, ...);
			
			/**
			* @func format
			*/
			static SString format(const String& path);
			
			/**
			* @func restore
			*/
			static String fallback(const String& path);
			
			/**
			* @func cwd # Getting current working directory
			* @ret {String}
			* @static
			*/
			static String cwd();
			
			/**
			* @func chdir # Setting current working directory
			* @arg path {const String&}
			* @ret {bool}
			* @static
			*/
			static bool chdir(const String& path);
	};

	/**
	* @class FileReader
	*/
	class FX_EXPORT FileReader: public Object {
			FX_HIDDEN_ALL_COPY(FileReader);
		public:
			FileReader();
			FileReader(FileReader&& reader);
			virtual ~FileReader();
			virtual uint32_t read_file(const String& path, cCb& cb = 0);
			virtual uint32_t read_stream(const String& path, cCb& cb = 0);
			virtual Buffer read_file_sync(const String& path) throw(Error);
			virtual void abort(uint32_t id);
			virtual bool exists_sync(const String& path);
			virtual bool is_file_sync(const String& path);
			virtual bool is_directory_sync(const String& path);
			virtual std::vector<Dirent> readdir_sync(const String& path);
			virtual bool is_absolute(const String& path);
			virtual String format(const String& path);
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
