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

#ifndef __ngui__utils__fs__
#define __ngui__utils__fs__

#include "cb.h"
#include "string.h"
#include "handle.h"
#include "list.h"
#include "loop.h"

/**
 * @ns ngui
 */

XX_NS(ngui)

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
	//
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
	FILE_UNKNOWN,
	FILE_FILE,
	FILE_DIR,
	FILE_LINK,
	FILE_FIFO,
	FILE_SOCKET,
	FILE_CHAR,
	FILE_BLOCK
};

class XX_EXPORT Dirent: public Object {
 public:
	inline Dirent() { }
	inline Dirent(cString& n, cString& p, FileType t)
		: name(n), pathname(p), type(t) {
	}
	String name;
	String pathname;
	FileType type;
};

/**
 * @class FileStat
 */
class XX_EXPORT FileStat: public Object {
	XX_HIDDEN_ALL_COPY(FileStat);
 public:
	FileStat();
	FileStat(cString& path);
	FileStat(FileStat&& stat);
	FileStat& operator=(FileStat&& stat);
	virtual ~FileStat();
	bool is_valid() const;
	bool is_file() const;
	bool is_dir() const;
	bool is_link() const;
	bool is_sock() const;
	uint64 mode() const;
	FileType type() const;
	uint64 group() const;
	uint64 owner() const;
	uint64 nlink() const;
	uint64 ino() const;
	uint64 blksize() const;
	uint64 blocks() const;
	uint64 flags() const;
	uint64 gen() const;
	uint64 dev() const;
	uint64 rdev() const;
	uint64 size() const;
	uint64 atime() const;
	uint64 mtime() const;
	uint64 ctime() const;
	uint64 birthtime() const;
 private:
	XX_DEFINE_INLINE_CLASS(Inl);
	void* m_stat;
};

/**
 * @class IFile
 */
class XX_EXPORT IFile {
 public:
	typedef ProtocolTraits Traits;
	virtual bool is_open() = 0;
	virtual bool open(int flag = FOPEN_R) = 0;
	virtual bool close() = 0;
	virtual int read(void* buffer, int64 size, int64 offset = -1) = 0;
	virtual int write(const void* buffer, int64 size, int64 offset = -1) = 0;
};

/**
 * @class File
 */
class XX_EXPORT File: public Object, public IFile {
	XX_HIDDEN_ALL_COPY(File);
 public:
	typedef DefaultTraits Traits;
	inline File(cString& path): m_path(path), m_fp(0) { }
	virtual ~File();
	inline  String path() const { return m_path; }
	virtual bool is_open();
	virtual bool open(int flag = FOPEN_R);
	virtual bool close();
	virtual int read(void* buffer, int64 size, int64 offset = -1);
	virtual int write(const void* buffer, int64 size, int64 offset = -1);
 private:
	String m_path;
	int m_fp;
};

/**
 * @class AsyncFile
 */
class XX_EXPORT AsyncFile: public Object {
	XX_HIDDEN_ALL_COPY(AsyncFile);
 public:
	typedef DefaultTraits Traits;
	class XX_EXPORT Delegate {
	public:
		virtual void trigger_async_file_open(AsyncFile* file) = 0;
		virtual void trigger_async_file_close(AsyncFile* file) = 0;
		virtual void trigger_async_file_error(AsyncFile* file, cError& error) = 0;
		virtual void trigger_async_file_read(AsyncFile* file, Buffer buffer, int mark) = 0;
		virtual void trigger_async_file_write(AsyncFile* file, Buffer buffer, int mark) = 0;
	};
	AsyncFile(cString& path, RunLoop* loop = RunLoop::current());
	virtual ~AsyncFile();
	String path() const;
	void set_delegate(Delegate* delegate);
	bool is_open();
	void open(int flag = FOPEN_R);
	void close();
	void read(Buffer buffer, int64 offset = -1, int mark = 0);
	void write(Buffer buffer, int64 offset = -1, int mark = 0);
 private:
	XX_DEFINE_INLINE_CLASS(Inl);
	Inl* m_inl;
};

/**
 * @class FileHelper
 * @static
 */
class XX_EXPORT FileHelper {
 public:
	static const uint default_mode;
	
	/**
	 * @func each_sync 递归遍历子子文件与子目录, 遍历回调回返0停止遍历
	 */
	static bool each_sync(cString& path, cCb& cb, bool internal = false);
	
	/**
	 * @func chmod_sync
	 */
	static bool chmod_sync(cString& path, uint mode = default_mode);
	
	/**
	 * @func chmod_p  # 递归设置
	 *                # 多线程中,设置stop_signal值为true来终止操作
	 */
	static bool chmod_r_sync(cString& path, uint mode = default_mode, bool* stop_signal = nullptr);
	static bool chown_sync(cString& path, uint owner, uint group);
	static bool chown_r_sync(cString& path, uint owner, uint group, bool* stop_signal = nullptr);
	static bool mkdir_sync(cString& path, uint mode = default_mode);
	static bool mkdir_p_sync(cString& path, uint mode = default_mode);
	static bool rename_sync(cString& name, cString& new_name);
	static bool mv_sync(cString& name, cString& new_name);
	static bool unlink_sync(cString& path);
	static bool rmdir_sync(cString& path);
	static bool rm_r_sync(cString& path, bool* stop_signal = nullptr);
	static bool cp_sync(cString& source, cString& target, bool* stop_signal = nullptr);
	static bool cp_r_sync(cString& source, cString& target, bool* stop_signal = nullptr);
	static Array<Dirent> readdir_sync(cString& path);
	static Array<Dirent> ls_sync(cString& path);
	static FileStat stat_sync(cString& path);
	static bool exists_sync(cString& path);
	static bool is_file_sync(cString& path);
	static bool is_directory_sync(cString& path);
	static bool readable_sync(cString& path);
	static bool writable_sync(cString& path);
	static bool executable_sync(cString& path);
	static void abort(uint id);
	static void chmod(cString& path, uint mode = default_mode, cCb& cb = 0);
	static void chown(cString& path, uint owner, uint group, cCb& cb = 0);
	static void mkdir(cString& path, uint mode = default_mode, cCb& cb = 0);
	static void mkdir_p(cString& path, uint mode = default_mode, cCb& cb = 0);
	static void rename(cString& name, cString& new_name, cCb& cb = 0);
	static void mv(cString& name, cString& new_name, cCb& cb = 0);
	static void unlink(cString& path, cCb& cb = 0);
	static void rmdir(cString& path, cCb& cb = 0);
	static void readdir(cString& path, cCb& cb = 0);
	static void ls(cString& path, cCb& cb = 0);
	static void stat(cString& path, cCb& cb = 0);
	static void exists(cString& path, cCb& cb = 0);
	static void is_file(cString& path, cCb& cb = 0);
	static void is_directory(cString& path, cCb& cb = 0);
	static void readable(cString& path, cCb& cb = 0);
	static void writable(cString& path, cCb& cb = 0);
	static void executable(cString& path, cCb& cb = 0);
	static uint chmod_r(cString& path, uint mode = default_mode, cCb& cb = 0);
	static uint chown_r(cString& path, uint owner, uint group, cCb& cb = 0);
	static uint rm_r(cString& path, cCb& cb = 0);
	static uint cp(cString& source, cString& target, cCb& cb = 0);
	static uint cp_r(cString& source, cString& target, cCb& cb = 0);
	// read stream
	static uint read_stream(cString& path, cCb& cb = 0);
	// read file
	static Buffer read_file_sync(cString& path, int* err = nullptr);
	static void read_file(cString& path, cCb& cb = 0);
	// write file
	static int  write_file_sync(cString& path, cString& str);
	static int  write_file_sync(cString& path, const void* data, int64 size);
	static void write_file(cString& path, cString& str, cCb& cb = 0);
	static void write_file(cString& path, Buffer buffer, cCb& cb = 0);
	// open file fd
	static int  open_sync(cString& path, int flag = FOPEN_R);
	static void open(cString& path, int flag = FOPEN_R, cCb& cb = 0);
	static void open(cString& path, cCb& cb = 0);
	static int  close_sync(int fd);
	static void close(int fd, cCb& cb = 0);
	// read with fd
	static int  read_sync(int fd, void* data, int64 size, int64 offset = -1);
	static int  write_sync(int fd, const void* data, int64 size, int64 offset = -1);
	static void read(int fd, Buffer buffer, cCb& cb);
	static void read(int fd, Buffer buffer, int64 offset = -1, cCb& cb = 0);
	static void write(int fd, Buffer buffer, cCb& cb);
	static void write(int fd, Buffer buffer, int64 offset = -1, cCb& cb = 0);
};

/**
 * @class Path
 * @static
 */
class XX_EXPORT Path {
 public:
	/**
	 * @func extname {String} # Get the path basename
	 * @ret {String}
	 */
	static String basename(cString& path);
	
	/**
	 * @func extname {String} # Get the path dirname
	 * @arg path {cString&}
	 * @ret {String}
	 */
	static String dirname(cString& path);
	
	/**
	 * @func extname # Get the path extname
	 * @arg path {cString&}
	 * @ret {String}
	 */
	static String extname(cString& path);
	
	/**
	 * @func executable_path # Get the executable path
	 * @ret {cString&}
	 */
	static String executable();
	
	/**
	 * @func documents_dir # Get the documents dir.
	 * @ret {cString&} # The path that can be write/read a file in
	 */
	static String documents(cString& path = String());
	
	/**
	 * @func temp_dir # Get the temp dir.
	 * @ret {cString&} # The path that can be write/read a file in
	 */
	static String temp(cString& path = String());
	
	/**
	 * @func resources_dir # Get the resoures dir
	 * @ret {cString&}
	 */
	static String resources(cString& path = String());
	
	/**
	 * @func is_absolute # Is absolute path
	 * @ret {bool}
	 */
	static bool is_local_absolute(cString& path);
	
	/**
	 * @func is_local_zip
	 */
	static bool is_local_zip(cString& path);
	
	/**
	 * @func is_local_file
	 */
	static bool is_local_file(cString& path);
	
	/**
	 * @func format
	 * @arg format {cchar*}
	 * @arg [...] {cchar*}
	 * @ret {String}
	 */
	static String format(cchar* path, ...);
	
	/**
	 * @func format
	 */
	static String format(cString& path);
	
	/**
	 * @func restore
	 */
	static String fallback(cString& path);
	
	/**
	 * @func restore_c
	 */
	static cchar* fallback_c(cString& path);
	
	/**
	 * @func cwd # Getting current working directory
	 * @ret {String}
	 * @static
	 */
	static String cwd();
	
	/**
	 * @func chdir # Setting current working directory
	 * @arg path {cString&}
	 * @ret {bool}
	 * @static
	 */
	static bool chdir(cString& path);
	
};

/**
 * @class FileSearch # Resources files search
 */
class XX_EXPORT FileSearch: public Object {
	XX_HIDDEN_ALL_COPY(FileSearch);
 public:
	
	FileSearch();
	
	/**
	 * @destructor
	 */
	virtual ~FileSearch();
	
	/**
	 * 添加一个搜索路径,路径必需存在
	 */
	void add_search_path(cString& path);
	
	/**
	 * @func add_zip_search_path() 添加一个zip包内的搜索路径,只能添加没加密的zip包
	 */
	void add_zip_search_path(cString& zip_path, cString& path);
	
	/**
	 * @func get_search_paths() Gets the array of search paths.
	 */
	Array<String> get_search_paths() const;
	
	/**
	 * @func remove_search_path() remove search path
	 */
	void remove_search_path(cString& path);
	
	/**
	 * @func remove_all_search_path() Removes all search paths.
	 */
	void remove_all_search_path();
	
	/**
	 *
	 * To obtain the absolute path to the file already exists.
	 * If no such file returns the empty string ""
	 * If it is a zip package path, will return with the prefix "zip:///home/xxx/test.apk@/assets/bb.jpg"
	 * @func get_absolute_path
	 */
	String get_absolute_path(cString& path) const;
	
	/**
	 * @func exists # Find the file exists
	 */
	bool exists(cString& path) const;
	
	/**
	 * @func read_file Read the all file data and return Data
	 */
	Buffer read(cString& path) const;
	
	/**
	 * @func share # Gets the instance of FileSearch.
	 */
	static FileSearch* shared();
	
 private:
	class SearchPath;
	class ZipInSearchPath;
	List<SearchPath*> m_search_paths; // Search path list
};

/**
 * @class FileReader
 */
class XX_EXPORT FileReader: public Object {
	XX_HIDDEN_ALL_COPY(FileReader);
 public:
	FileReader();
	FileReader(FileReader&& reader);
	virtual ~FileReader();
	virtual uint read_file(cString& path, cCb& cb = 0);
	virtual uint read_stream(cString& path, cCb& cb = 0);
	virtual Buffer read_file_sync(cString& path) throw(Error);
	virtual void abort(uint id);
	virtual bool exists_sync(cString& path);
	virtual bool is_file_sync(cString& path);
	virtual bool is_directory_sync(cString& path);
	virtual Array<Dirent> readdir_sync(cString& path);
	virtual bool is_absolute(cString& path);
	virtual String format(cString& path);
	virtual void clear();
	static void set_shared_instance(FileReader* reader);
	static FileReader* shared();
 private:
	class Core; 
	Core* m_core;
};

XX_INLINE FileSearch* f_search() {
	return FileSearch::shared();
}

XX_INLINE FileReader* f_reader() {
	return FileReader::shared();
}

XX_END
#endif
