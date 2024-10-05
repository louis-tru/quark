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

#ifndef __quark__util__fs__
#define __quark__util__fs__

#include "./cb.h"
#include "./handle.h"
#include "./loop.h"
#include "./stream.h"

namespace qk {

	/**
	 * @field fs_default_mode
	*/
	Qk_Export extern const uint32_t fs_default_mode;

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
		FOPEN_TRUNC = 01000, // 如果文件存在截断为0
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

	struct Dirent {
		String   name;
		String   pathname;
		FileType type;
	};

	class Qk_Export FileSync: public Object {
		Qk_HIDDEN_ALL_COPY(FileSync);
	public:
		FileSync(cString& path);
		virtual ~FileSync();
		bool is_open();
		int  open(int flag = FOPEN_R, uint32_t mode = fs_default_mode);
		int  close();
		int  read(void* buffer, int64_t size, int64_t fdOffset = -1);
		int  write(const void* buffer, int64_t size, int64_t fdOffset = -1);
		// props
		Qk_DEFINE_PGET(String, path, Const);
	private:
		int  _fd;
	};

	class Qk_Export File: public Object {
		Qk_HIDDEN_ALL_COPY(File);
	public:
		class Qk_Export Delegate {
		public:
			virtual void trigger_file_open (File* file) = 0;
			virtual void trigger_file_close(File* file) = 0;
			virtual void trigger_file_error(File* file, cError& error) = 0;
			virtual void trigger_file_read (File* file, Buffer& buffer, int flag) = 0;
			virtual void trigger_file_write(File* file, Buffer& buffer, int flag) = 0;
		};
		File(cString& path, RunLoop* loop = RunLoop::current());
		virtual ~File();
		String path() const;
		void set_delegate(Delegate* delegate);
		bool is_open();
		void open(int flag = FOPEN_R, uint32_t mode = fs_default_mode);
		void close();
		void read (Buffer buffer, int64_t fdOffset = -1, int flag = 0);
		void write(Buffer buffer, int64_t fdOffset = -1, int flag = 0);
	private:
		Qk_DEFINE_INLINE_CLASS(Inl);
		Inl* _inl;
	};

	class Qk_Export FileStat: public Object {
	public:
		FileStat();
		FileStat(cString& path);
		FileStat(const FileStat& stat);
		FileStat& operator=(const FileStat& stat);
		~FileStat();
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
		void* _stat;
	};

	class Qk_Export FileReader: public Object {
		Qk_HIDDEN_ALL_COPY(FileReader);
	public:
		FileReader();
		FileReader(FileReader&& reader);
		virtual ~FileReader();
		virtual uint32_t read_file(cString& path, Callback<Buffer> cb = 0);
		virtual uint32_t read_stream(cString& path, Callback<StreamResponse> cb = 0);
		virtual Buffer read_file_sync(cString& path) throw(Error);
		virtual void abort(uint32_t id);
		virtual bool exists_sync(cString& path);
		virtual bool is_file_sync(cString& path);
		virtual bool is_directory_sync(cString& path);
		virtual Array<Dirent> readdir_sync(cString& path);
		virtual bool is_absolute(cString& path);
		virtual String format(cString& path);
		virtual void clear();
		static void set_shared(FileReader* reader);
		static FileReader* shared();
	private:
		class Core;
		Core* _core;
	};

	/**
	 * @method fs_reader() get shared reader
	*/
	Qk_Export FileReader* fs_reader();

	/**
	* @method each_sync 递归遍历子文件与子目录, 遍历回调回返0停止遍历
	*/
	Qk_Export bool fs_each_sync(cString& path, Callback<Dirent> cb, bool internal = false) throw(Error);
	
	// sync
	/**
	* @method fs_chmod_sync
	*/
	Qk_Export void fs_chmod_sync(cString& path, uint32_t mode = fs_default_mode) throw(Error);
	Qk_Export void fs_chown_sync(cString& path, uint32_t owner, uint32_t group) throw(Error);
	Qk_Export void fs_mkdir_sync(cString& path, uint32_t mode = fs_default_mode) throw(Error);
	Qk_Export void fs_mkdirs_sync(cString& path, uint32_t mode = fs_default_mode) throw(Error);
	Qk_Export void fs_rename_sync(cString& name, cString& new_name) throw(Error);
	Qk_Export void fs_link_sync(cString& path, cString& newPath) throw(Error);
	Qk_Export void fs_unlink_sync(cString& path) throw(Error);
	Qk_Export void fs_rmdir_sync(cString& path) throw(Error);
	Qk_Export Array<Dirent> fs_readdir_sync(cString& path) throw(Error);
	Qk_Export FileStat fs_stat_sync(cString& path) throw(Error);
	Qk_Export bool fs_exists_sync(cString& path);
	Qk_Export bool fs_is_file_sync(cString& path);
	Qk_Export bool fs_is_directory_sync(cString& path);
	Qk_Export bool fs_readable_sync(cString& path);
	Qk_Export bool fs_writable_sync(cString& path);
	Qk_Export bool fs_executable_sync(cString& path);
	// recursion
	/**
	* @method fs_chmod_recursion_sync  递归设置
	* # 多线程中,设置stop_signal值为true来终止操作
	*/
	Qk_Export bool fs_chmod_recursion_sync(cString& path, uint32_t mode = fs_default_mode, bool* stop_signal = nullptr) throw(Error);
	Qk_Export bool fs_chown_recursion_sync(cString& path, uint32_t owner, uint32_t group, bool* stop_signal = nullptr) throw(Error);
	Qk_Export bool fs_remove_recursion_sync(cString& path, bool* stop_signal = nullptr) throw(Error);
	Qk_Export bool fs_copy_recursion_sync(cString& source, cString& target, bool* stop_signal = nullptr) throw(Error);
	Qk_Export bool fs_copy_sync(cString& source, cString& target, bool* stop_signal = nullptr) throw(Error);
	// async
	Qk_Export void fs_chmod(cString& path, uint32_t mode = fs_default_mode, Cb cb = 0);
	Qk_Export void fs_chown(cString& path, uint32_t owner, uint32_t group, Cb cb = 0);
	Qk_Export void fs_mkdir(cString& path, uint32_t mode = fs_default_mode, Cb cb = 0);
	Qk_Export void fs_mkdirs(cString& path, uint32_t mode = fs_default_mode, Cb cb = 0);
	Qk_Export void fs_rename(cString& name, cString& new_name, Cb cb = 0);
	Qk_Export void fs_link(cString& path, cString& newPath, Cb cb = 0);
	Qk_Export void fs_unlink(cString& path, Cb cb = 0);
	Qk_Export void fs_rmdir(cString& path, Cb cb = 0);
	Qk_Export void fs_readdir(cString& path, Callback<Array<Dirent>> cb = 0);
	Qk_Export void fs_stat(cString& path, Callback<FileStat> cb = 0);
	Qk_Export void fs_exists(cString& path, Callback<Bool> cb = 0);
	Qk_Export void fs_is_file(cString& path, Callback<Bool> cb = 0);
	Qk_Export void fs_is_directory(cString& path, Callback<Bool> cb = 0);
	Qk_Export void fs_readable(cString& path, Callback<Bool> cb = 0);
	Qk_Export void fs_writable(cString& path, Callback<Bool> cb = 0);
	Qk_Export void fs_executable(cString& path, Callback<Bool> cb = 0);
	// recursion
	Qk_Export uint32_t fs_chmod_recursion(cString& path, uint32_t mode = fs_default_mode, Cb cb = 0);
	Qk_Export uint32_t fs_chown_recursion(cString& path, uint32_t owner, uint32_t group, Cb cb = 0);
	Qk_Export uint32_t fs_remove_recursion(cString& path, Cb cb = 0);
	Qk_Export uint32_t fs_copy_recursion(cString& source, cString& target, Cb cb = 0);
	Qk_Export uint32_t fs_copy(cString& source, cString& target, Cb cb = 0);
	Qk_Export void     fs_abort(uint32_t id);
		// read stream
	Qk_Export uint32_t fs_read_stream(cString& path, Callback<StreamResponse> cb = 0);
		// read file
	Qk_Export Buffer fs_read_file_sync(cString& path, int64_t size = -1) throw(Error);
	Qk_Export void fs_read_file(cString& path, Callback<Buffer> cb = 0, int64_t size = -1);
		// write file
	Qk_Export int  fs_write_file_sync(cString& path, cString& str) throw(Error);
	Qk_Export int  fs_write_file_sync(cString& path, const void* data, int64_t size) throw(Error);
	Qk_Export void fs_write_file(cString& path, cString& str, Callback<Buffer> cb = 0);
	Qk_Export void fs_write_file(cString& path, Buffer buffer, Callback<Buffer> cb = 0);
		// open file fd
	Qk_Export int  fs_open_sync(cString& path, int flag = FOPEN_R) throw(Error);
	Qk_Export void fs_open(cString& path, int flag = FOPEN_R, Callback<Int32> cb = 0);
	Qk_Export void fs_open(cString& path, Callback<Int32> cb = 0);
	Qk_Export void fs_close_sync(int fd) throw(Error);
	Qk_Export void fs_close(int fd, Cb cb = 0);
		// read with fd
	Qk_Export int  fs_read_sync(int fd, void* data, int64_t size, int64_t fdOffset = -1) throw(Error);
	Qk_Export int  fs_write_sync(int fd, const void* data, int64_t size, int64_t fdOffset = -1) throw(Error);
	Qk_Export void fs_read(int fd, Buffer buffer, Callback<Buffer> cb);
	Qk_Export void fs_read(int fd, Buffer buffer, int64_t fdOffset = -1, Callback<Buffer> cb = 0);
	Qk_Export void fs_write(int fd, Buffer buffer, Callback<Buffer> cb);
	Qk_Export void fs_write(int fd, Buffer buffer, int64_t fdOffset = -1, Callback<Buffer> cb = 0);

	/**
	* @func extname {String} # Get the path basename
	* @ret {String}
	*/
	Qk_Export String fs_basename(cString& path);
	
	/**
	* @func extname {String} # Get the path dirname
	* @arg path {cString&}
	* @ret {String}
	*/
	Qk_Export String fs_dirname(cString& path);
	
	/**
	* @func extname # Get the path extname
	* @arg path {cString&}
	* @ret {String}
	*/
	Qk_Export String fs_extname(cString& path);
	
	/**
	* @func executable_path # Get the executable path
	* @ret {cString&}
	*/
	Qk_Export String fs_executable();
	
	/**
	* @func documents_dir # Get the documents dir.
	* @ret {cString&} # The path that can be write/read a file in
	*/
	Qk_Export String fs_documents(cString& child = String());
	
	/**
	* @func temp_dir # Get the temp dir.
	* @ret {cString&} # The path that can be write/read a file in
	*/
	Qk_Export String fs_temp(cString& child = String());
	
	/**
	* @func resources_dir # Get the resoures dir
	* @ret {cString&}
	*/
	Qk_Export String fs_resources(cString& child = String());
	
	/**
	* @func is_absolute # Is absolute path
	* @ret {bool}
	*/
	Qk_Export bool fs_is_local_absolute(cString& path);
	
	/**
	* @func is_local_zip
	*/
	Qk_Export bool fs_is_local_zip(cString& path);
	
	/**
	* @func is_local_file
	*/
	Qk_Export bool fs_is_local_file(cString& path);
	
	/**
	* @func format
	* @arg format {cChar*}
	* @arg [...] {cChar*}
	* @ret {String}
	*/
	Qk_Export String fs_format(cChar* path, ...);
	
	/**
	* @func format
	*/
	Qk_Export String fs_format(cString& path);
	
	/**
	* @func fallback
	*/
	Qk_Export String fs_fallback(cString& path);

	/**
	* @func fallback_c
	*/
	Qk_Export cChar* fs_fallback_c(cString& path);
	
	/**
	* @func cwd # Getting current working directory
	* @ret {String}
	* @static
	*/
	Qk_Export String fs_cwd();
	
	/**
	* @func chdir # Setting current working directory
	* @arg path {cString&}
	* @ret {bool}
	* @static
	*/
	Qk_Export bool fs_chdir(cString& path);

}
#endif
