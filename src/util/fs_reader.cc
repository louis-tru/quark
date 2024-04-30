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

#include "./fs.h"
#include "./zlib.h"
#include "./handle.h"
#include "./error.h"
#include "./http.h"
#include "./uv.h"
#include "./dict.h"

namespace qk {

	enum IOProtocol {
		Unknown,
		FILE,
		ZIP,
		HTTP,
		HTTPS,
		FTP,
		FTPS,
	};

	IOProtocol fs_get_protocol_from_str(cString& path) {
		if ( fs_is_local_file( path ) ) {
			return FILE;
		}
		if ( fs_is_local_zip( path ) ) {
			return ZIP;
		}
		if ((path[0] == 'h' || path[0] == 'H') &&
				(path[1] == 't' || path[1] == 'T') &&
				(path[2] == 't' || path[2] == 'T') &&
				(path[3] == 'p' || path[3] == 'P')) {
			if (path[4] == ':' &&
					path[5] == '/' &&
					path[6] == '/') {
				return HTTP;
			}
			if ((path[4] == 's' || path[4] == 'S') &&
					path[5] == ':' &&
					path[6] == '/' &&
					path[7] == '/') {
				return HTTPS;
			}
		}
		if ((path[0] == 'f' || path[0] == 'F') &&
				(path[1] == 't' || path[1] == 'T') &&
				(path[2] == 'p' || path[2] == 'P')) {
			if (path[3] == ':' &&
					path[4] == '/' &&
					path[5] == '/') {
				return FTP;
			}
			if ((path[3] == 's' || path[3] == 'S') &&
					path[4] == ':' &&
					path[5] == '/' &&
					path[6] == '/') {
				return FTPS;
			}
		}
		return Unknown;
	}

	String fs_format_part_path(cString& path);
	static const String fs_SEPARATOR("@/", 2);

	class FileReader::Core {
	public:
		~Core() {
			ScopeLock lock(zip_mutex_);
			for (auto i: zips_ ) {
				Release(i.value);
			}
		}

		String zip_path(cString& path) {
			if (path.isEmpty())
				return String();
			int i = path.indexOf(fs_SEPARATOR);
			if (i != -1)
				return path.substr(0, i);
			if (path[path.length() - 1] == fs_SEPARATOR[0])
				return path.substr(0, path.length() - 1);
			return String();
		}

		ZipReader* get_zip_reader(cString& path) throw(Error) {
			ZipReader* reader = zips_[path];
			if (reader) {
				return reader;
			}
			reader = new ZipReader(path);
			if ( !reader->open() ) {
				Release(reader);
				Qk_Throw(ERR_FILE_NOT_EXISTS, "Cannot open zip file, `%s`", *path);
			}
			zips_[path] = reader;
			return reader;
		}

		void read_from_zip(RunLoop* loop, cString& zip, cString& path, bool stream, Cb cb) {
			ScopeLock lock(zip_mutex_);
			Buffer buffer;
			try {
				ZipReader* read = get_zip_reader(zip);
				String inl_path = fs_format_part_path(path.substr(zip.length() + fs_SEPARATOR.length()));
				if ( read->jump(inl_path) ) {
					buffer = read->read();
				} else {
					Error err(ERR_FILE_NO_EXIST_IN_ZIP_PKG, "Zip package internal file does not exist, %s", *path);
					async_reject(cb, std::move(err), loop); return;
				}
			} catch (cError& err) {
				Error e(err);
				async_reject(cb, Error(err), loop); return;
			}
			
			if ( stream ) {
				uint32_t len = buffer.length();
				async_resolve<Object>(cb, StreamResponse(buffer, 1, 0, len, len, nullptr), static_cast<PostMessage*>(loop));
			} else {
				async_resolve<Object>(cb, std::move(buffer), static_cast<PostMessage*>(loop));
			}
		}

		uint32_t read(cString& path, Cb cb, bool stream) {
			uint32_t id = 0;

			switch (fs_get_protocol_from_str(path)) {
				default:
				case FILE:
					if ( stream ) {
						id = fs_read_stream(path, *reinterpret_cast<Callback<StreamResponse>*>(&cb));
					} else {
						fs_read_file(path, *reinterpret_cast<Callback<Buffer>*>(&cb));
					}
					break;
				case ZIP: {
					String zip = zip_path(path);
					if ( zip.isEmpty() ) {
						async_reject(cb, Error(ERR_INVALID_FILE_PATH, "Invalid file path, \"%s\"", *path), RunLoop::current());
					} else {
						RunLoop* loop = RunLoop::current();
						loop->work(Cb([this, loop, zip, path, stream, cb](Cb::Data& evt) {
							read_from_zip(loop, zip, path, stream, cb);
						}));
					}
					break;
				}
				case FTP:
				case FTPS:
					async_reject(cb, Error(ERR_NOT_SUPPORTED_FILE_PROTOCOL, "This file protocol is not supported"), RunLoop::current());
					break;
				case HTTP:
				case HTTPS:
					try {
						if ( stream ) {
							id = http_get_stream(path, *(Callback<StreamResponse>*)(&cb));
						} else {
							id = http_get(path, HttpCb([cb](HttpCb::Data& e) {
								ResponseData* data = static_cast<ResponseData*>(e.data);
								if (e.error) {
									cb->reject(e.error);
								} else {
									cb->resolve(&data->data);
								}
							}));
						}
					} catch(Error& err) {
						async_reject(cb, Error(err), RunLoop::current());
					}
					break;
			}
			return id;
		}

		Buffer read_sync(cString& path) throw(Error) {
			Buffer rv;

			switch ( fs_get_protocol_from_str(path) ) {
				default:
				case FILE:
					Qk_Check(fs_exists_sync(path),
										ERR_FILE_NOT_EXISTS, "Unable to read file contents, \"%s\"", *path);
					rv = fs_read_file_sync(path);
					break;
				case ZIP: {
					String zip = zip_path(path);
					Qk_Check(!zip.isEmpty(), ERR_FILE_NOT_EXISTS, "Invalid file path, \"%s\"", *path);
					
					ScopeLock lock(zip_mutex_);
					
					ZipReader* read = get_zip_reader(zip);
					String inl_path = fs_format_part_path( path.substr(zip.length() + fs_SEPARATOR.length()) );
					
					if ( read->jump(inl_path) ) {
						rv = read->read();
					} else {
						Qk_Throw(ERR_ZIP_IN_FILE_NOT_EXISTS,
							"Zip package internal file does not exist, %s", *path);
					}
					break;
				}
				case FTP:
				case FTPS:
					Qk_Throw(ERR_NOT_SUPPORTED_FILE_PROTOCOL, "This file protocol is not supported");
					break;
				case HTTP:
				case HTTPS: rv = http_get_sync(path); break;
			}
			return rv;
		}

		void abort(uint32_t id) {
			AsyncIOTask::safe_abort(id);
		}

		bool exists_sync(cString& path, bool file, bool dir) {
			switch ( fs_get_protocol_from_str(path) ) {
				default:
				case FILE:
					if ( file && fs_is_file_sync(path) )
						return true;
					if ( dir  && fs_is_directory_sync(path) )
						return true;
					return false;
				case ZIP: {
					String zip = zip_path(path);
					if ( !zip.isEmpty() ) {
						try {
							ScopeLock lock(zip_mutex_);
							ZipReader* read = get_zip_reader(zip);
							String inl_path = fs_format_part_path( path.substr(zip.length() + fs_SEPARATOR.length()) );
							if ( file && read->is_file( inl_path ) )
								return true;
							if ( dir && read->is_directory( inl_path ) )
								return true;
						} catch(cError &e) {
							Qk_WARN("Warn, %s", e.message().c_str());
						}
					}
					return false;
				}
			}
			return false;
		}

		Array<Dirent> readdir_sync(cString& path) throw(Error) {
			Array<Dirent> rv;
			switch ( fs_get_protocol_from_str(path) ) {
				default:
				case FILE:
					rv = fs_readdir_sync(path);
				case ZIP: {
					String zip = zip_path(path);
					if ( !zip.isEmpty() ) {
						try {
							ScopeLock lock(zip_mutex_);
							ZipReader* read = get_zip_reader(zip);
							String inl_path = fs_format_part_path( path.substr(zip.length() + fs_SEPARATOR.length()) );
							rv = read->readdir(inl_path);
						} catch(cError &e) {
							Qk_WARN("Warn, %s", e.message().c_str());
						}
					}
					break;
				}
			}
			Qk_ReturnLocal(rv);
		}

		String format(cString& path) {
			int index = -1;
			switch ( fs_get_protocol_from_str(path) ) {
				default:
				case ZIP:
				case FILE: return fs_format("%s", *path);
				case HTTP: index = path.indexOf('/', 8); break;
				case HTTPS:index = path.indexOf('/', 9); break;
				case FTP:  index = path.indexOf('/', 7); break;
				case FTPS: index = path.indexOf('/', 8); break;
			}
			if (index == -1) {
				return path;
			}
			String s = fs_format_part_path(path.substr(index));
			if (s.isEmpty()) {
				return path.substr(0, index);
			} else {
				return path.substr(0, index + 1) + s;
			}
		}
		
		bool is_absolute(cString& path) {
			
			if ( fs_is_local_absolute(path) ) {
				return true;
			} else {
				switch ( fs_get_protocol_from_str(path) ) {
					case ZIP:
					case FILE:
					case HTTP:
					case HTTPS:
					case FTP:
					case FTPS: return true;
					default: return false;
				}
			}
		}
		
		void clear() {
			ScopeLock lock(zip_mutex_);
			for ( auto& i: zips_ ) {
				Release(i.value);
			}
			zips_.clear();
		}
		
	private:
		Mutex zip_mutex_;
		Dict<String, ZipReader*> zips_;
	};

	FileReader::FileReader(): _core(new Core()) { }

	FileReader::FileReader(FileReader&& reader): _core(reader._core) {
		reader._core = nullptr;
	}

	FileReader::~FileReader() {
		delete _core;
		_core = nullptr;
	}

	uint32_t FileReader::read_file(cString& path, Callback<Buffer> cb) {
		return _core->read(path, *reinterpret_cast<Cb*>(&cb), false);
	}

	uint32_t FileReader::read_stream(cString& path, Callback<StreamResponse> cb) {
		return _core->read(path, *(Cb*)&cb, true);
	}

	Buffer FileReader::read_file_sync(cString& path) throw(Error) {
		return _core->read_sync(path);
	}

	void FileReader::abort(uint32_t id) {
		_core->abort(id);
	}

	bool FileReader::exists_sync(cString& path) {
		return _core->exists_sync(path, 1, 1);
	}
	
	bool FileReader::is_file_sync(cString& path) {
		return _core->exists_sync(path, 1, 0);
	}

	bool FileReader::is_directory_sync(cString& path) {
		return _core->exists_sync(path, 0, 1);
	}
	
	Array<Dirent> FileReader::readdir_sync(cString& path) {
		try {
			return _core->readdir_sync(path);
		} catch(Error& err) {
			Qk_ERR(err);
		}
		return Array<Dirent>();
	}

	String FileReader::format(cString& path) {
		return _core->format(path);
	}

	bool FileReader::is_absolute(cString& path) {
		return _core->is_absolute(path);
	}

	void FileReader::clear() {
		return _core->clear();
	}

	static FileReader* __shared_instance = nullptr;

	void FileReader::set_shared(FileReader* reader) {
		if (__shared_instance != reader) {
			Release(__shared_instance);
			__shared_instance = reader;
		}
	}

	FileReader* FileReader::shared() {
		if ( !__shared_instance ) {
			__shared_instance = new FileReader();
		}
		return __shared_instance;
	}

	FileReader* fs_reader() {
		return FileReader::shared();
	}

}
