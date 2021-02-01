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

#ifndef __ftr__util__zlib__
#define __ftr__util__zlib__

#include "./fs.h"
#include "./error.h"
#include <map>
#include <vector>

namespace ftr {

	/**
	*
	* 基本压缩解压数据API
	*
	* @class ZLib
	*/
	class FX_EXPORT ZLib {
		public:
			
			/**
			* @func compress
			*/
			static Buffer compress(WeakBuffer buff, int level = -1);
			
			/**
			* @func uncompress
			*/
			static Buffer uncompress(WeakBuffer buff);
			
	};

	/**
	* 提供单个gzip压缩文件的读取与写入
	*
	* @class GZip
	*/
	class FX_EXPORT GZip: public Object {
			FX_HIDDEN_ALL_COPY(GZip);
		public:

			typedef ObjectTraits Traits;
			
			GZip(cString& path): _path(path), _gzfp(nullptr) { }
			
			/**
			* @destructor
			*/
			virtual ~GZip();
			
			/**
			* @ret {String}
			*/
			inline String path() const { return _path; }
			
			/**
			* @override
			*/
			virtual bool is_open();
			
			/**
			* can't read and write at the same time
			* BOTH and BOTH_NEW and BOTH_END_NEW cannot use
			* @override
			*/
			virtual int open(int flag = FOPEN_R);
			
			/**
			* @override
			*/
			virtual int close();
			virtual int read(void* buffer, int64_t size, int64_t offset = -1);
			virtual int write(const void* buffer, int64_t size, int64_t offset = -1);
			
		private:
			SString _path;
			void*  _gzfp;
	};

	/**
	* Note:无法打开空的zip包,zip包必需有内容
	*
	* @class ZipReader
	*/
	class FX_EXPORT ZipReader: public Object {
			FX_HIDDEN_ALL_COPY(ZipReader);
		public:
			
			ZipReader(cString& path, cString& passwd = String());
			
			/**
			* @destructor
			*/
			virtual ~ZipReader();
			
			/**
			* @func open
			*/
			bool open();
			
			/**
			* @func close
			*/
			bool close();
			
			/**
			* @func exists(path) 查找是否存在指定路径的包内文件或目录
			*/
			bool exists(cString& path) const;
			
			/**
			* @func is_file(path) 测试是否为一个包内文件
			*/
			bool is_file(cString& path) const;
			
			/**
			* @func is_directory(path) 测试是否为一个包内目录
			*/
			bool is_directory(cString& path) const;
			
			/**
			* @func readdir(path)
			*/
			std::vector<Dirent> readdir(cString& path) const;
			
			/**
			* @func jump 读取器定位到指定路径的包内文件
			*/
			bool jump(cString& path);
			
			/**
			* @func first 定位包内的第一个文件
			*/
			bool first();
			
			/**
			* @func next 定位包内的当前文件的下一个文件
			*/
			bool next();
			
			/**
			* @func read 从文件流中读数据
			*/
			int read(void* buffer, int size);
			
			/**
			* @func read Read the all file data and return Data
			*/
			Buffer read();
			
			/**
			* @func read Reads the specified size data
			*/
			Buffer read(uint32_t size);
			
			/**
			* @func current 当前文件名称
			*/
			inline String current() const {
				return _cur_it->second.pathname;
			}
			
			/**
			* @func compressed_size 当前文件的压缩大小
			*/
			inline uint32_t compressed_size() const {
				return _cur_it->second.compressed_size;
			}
			
			/**
			* @func uncompressed_size 当前文件的压缩前大小
			*/
			inline uint32_t uncompressed_size() const {
				return _cur_it->second.uncompressed_size;
			}
			
			/**
			* @func path()
			*/
			inline String path() const { return _path; }

			/**
			* @func compatible_path()
			*/
			inline String compatible_path() const { return _compatible_path; }
			
			/**
			* @func passwd
			*/
			inline String passwd() const { return _passwd; }
		
		private:

			SString _path;   // Zip file path
			SString _compatible_path; // zip:///var/data/test.zip?
			SString _passwd;
			
			struct _unz_file_pos {
				size_t pos_in_zip_directory;   /* offset in zip file directory */
				size_t num_of_file;            /* # of file */
			};
			
			struct unz_entry_info {
				_unz_file_pos  pos;
				SString  pathname;
				uint32_t  compressed_size;
				uint32_t  uncompressed_size;
			};
			
			typedef std::map<SString, unz_entry_info> Info;
			typedef std::map<SString, std::vector<Dirent>> DirInfo;
			typedef Info::iterator iterator;
			
			void*     _unzp;
			bool      _is_open;
			iterator  _cur_it;
			Info      _file_info;
			DirInfo   _dir_info;
		
			FX_DEFINE_INLINE_CLASS(Inl);
	};

	/**
	* zip 包不提供删除修改包内文件功能
	* 只可追加包内新文件
	* 如果要删除与修改,可重新创建新zip包
	*
	* @class ZipWriter
	*/
	class FX_EXPORT ZipWriter: public Object {
			FX_HIDDEN_ALL_COPY(ZipWriter);
		public:
			/**
			*
			*
			Create a zipfile.
			pathname contain on Windows XP a filename like "c:\\zlib\\zlib113.zip" or on
			an Unix computer "zlib/zlib113.zip".
			if the file pathname exist and append==APPEND_STATUS_CREATEAFTER, the zip
			will be created at the end of the file.
			(useful if the file contain a self extractor code)
			if the file pathname exist and append==APPEND_STATUS_ADDINZIP, we will
			add files in existing zip (be sure you don't add file that doesn't exist)
			If the zipfile cannot be opened, the return value is NULL.
			Else, the return value is a zipFile Handle, usable with other function
			of this zip package.
			*
			*
			* @enum OpenMode
			*/
			enum OpenMode {
				OPEN_MODE_CREATE = 0,         // 创建新的zip包,如果如果已存在该文件先删除存在文件
				OPEN_MODE_CREATE_AFTER = 1,   // 在存在文件在文件结尾创建zip包,zip包必需存在
				OPEN_MODE_ADD_IN_ZIP = 2      // 在存在的zip包追加内容,zip包必需存在
			};
			
			ZipWriter(cString& path, cString& passwd = String());

			/**
			* @destructor
			*/
			virtual ~ZipWriter();
			
			/**
			* @func open
			*/
			bool open(OpenMode mode = OPEN_MODE_CREATE);
			
			/**
			* @func close
			*/
			bool close();
			
			/**
			* 获取压缩等级
			* 0 - 9 个压缩等级, 数字越大需要更多处理时间
			* -1自动,0为不压缩,1最佳速度,9最佳压缩
			* 默认为-1
			*
			* @func level
			*/
			inline int level() const { return _level; }
			
			/**
			* @func set_level 设置压缩等级
			*/
			inline void set_level(int value) { _level = value; }
			
			/**
			* @func add_file 在zip包内创建一个新文件
			*/
			bool add_file(cString& path);
			
			/**
			* @func write Writes a block of data to the new file
			*/
			bool write(WeakBuffer data);
			
			/**
			* @func write
			*/
			template<class T> static bool write(const ArrayBuffer<T>& buff) {
				return write(buff.val(), buff.size());
			}
			
			/**
			* @func name 当前新文件名称
			*/
			inline String name() const { return _new_name; }
			
			/**
			* @func path
			*/
			inline String path() const { return _path; }
			
			/**
			* @func passwd
			*/
			inline String passwd() const { return _passwd; }
		
		private:
		
			bool close_current_file();
			
			SString   _path;   // Zip file path
			SString   _passwd;
			OpenMode  _open_mode;
			int       _level; // compression level
			void*     _zipp;
			String    _new_name;
	};

}
#endif
