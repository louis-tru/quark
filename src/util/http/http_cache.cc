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

#include "./http.inl"

namespace qk {

	class FileCacheReader: public File, public File::Delegate, public Reader {
	public:
		FileCacheReader(Client* client, int64_t size, RunLoop* loop)
			: File(client->_cache_path, loop)
			, _read_count(0)
			, _client(client)
			, _parse_header(true), _offset(0), _size(size)
		{
			Qk_ASSERT_EQ(_client->_cache_reader, nullptr);
			_client->_cache_reader = this;
			set_delegate(this);
			open();
		}

		~FileCacheReader() {
			_client->_cache_reader = nullptr; // clear host reader ptr
		}
		
		void continue_send_and_release() {
			set_delegate(nullptr);
			_client->_cache_reader = nullptr;
			_client->send_http();
			release();
		}
		
		void trigger_file_open(File* file) override {
			read(Buffer(511));
		}

		void trigger_file_close(File* file) override {
			if ( _parse_header ) { // unexpected shutdown
				continue_send_and_release();
			} else {
				// throw error to http client host
				_client->report_error_and_abort(Error(ERR_FILE_UNEXPECTED_SHUTDOWN, "File unexpected shutdown"));
			}
		}

		void trigger_file_error(File* file, cError& error) override {
			if ( _parse_header ) {
				continue_send_and_release();
			} else {
				// throw error to http client host
				_client->report_error_and_abort(error);
			}
		}

		void trigger_file_read(File* file, Buffer& buffer, int flag) override {
			if ( _parse_header ) { // parse cache header
				if ( buffer.length() ) {
					
					String str(buffer.val(), buffer.length()), s("\r\n"), s2(':');
					/*
						Expires: Thu, 27 Apr 2017 11:57:20 GMT
						Last-Modified: Fri, 18 Nov 2016 12:08:17 GMT
						
						... Body ...
						*/

					for ( int i = 0; ; ) {
						int j = str.indexOf(s, i);
						if ( j != -1 && j != 0 ) {
							if ( j == i ) { // parse header end
								_parse_header = false;
								_offset += (j + 2); // save read offset
								_header["expires"] = _header["expires"].trim();

								int64_t expires = parse_time(_header["expires"]);
								if ( expires > time_micro() ) {
									_client->trigger_http_readystate_change(HTTP_READY_STATE_RESPONSE);
									_client->_download_total = Int64::max(_size - _offset, 0);
									_client->trigger_http_header(200, std::move(_header), true);
									read_advance();
								} else {
									// Qk_Log("Read -- %ld, %ld, %s", expires, time(), *_header.get("expires"));
									if (parse_time(_header["last-modified"]) > 0 ||
											!_header["etag"].isEmpty()
									) {
										_client->send_http();
									} else {
										continue_send_and_release(); // invalid cache
									}
								}
								// parse header end
								break;
							} else {
								int k = str.indexOf(s2, i);
								if ( k != -1 && k - i > 1 && j - k > 2 ) {
									// Qk_DLog("%s: %s", *str.substring(i, k), *str.substring(k + 2, j));
									_header[str.substring(i, k).lowerCase()] = str.substring(k + 2, j);
								}
							}
						} else {
							if ( i == 0 ) { // invalid cache
								continue_send_and_release();
							} else { // read next
								_offset += i;
								buffer.reset(511);
								read(buffer, _offset);
							}
							break;
						}
						i = j + 2;
					}
					
				} else { // no cache
					continue_send_and_release();
				}
			} else {
				// read cache
				_read_count--;
				Qk_ASSERT_EQ(_read_count, 0);
				
				if ( buffer.length() ) {
					_offset += buffer.length();
					_client->_download_size += buffer.length();
					_client->trigger_http_data(buffer);
				} else { // end
					_client->http_response_complete(true);
				}
			}
		}

		void trigger_file_write(File* file, Buffer& buffer, int flag) override {
		}

		DictSS& header() {
			return _header;
		}

		void read_advance() override /*Reader*/ {
			if ( !_parse_header ) {
				if ( _read_count == 0 ) {
					_read_count++;
					read(Buffer(BUFFER_SIZE), _offset);
				}
			}
		}

		void read_pause() override /*Reader*/ {}

		bool is_cache() override /*Reader*/ {
			return true;
		}

	private:
		int _read_count;
		Client* _client;
		DictSS _header;
		bool _parse_header;
		uint32_t  _offset;
		int64_t _size;
	};

	class FileWriter: public Object, public File::Delegate {
	public:
		FileWriter(Client* client, cString& path, WriteCacheFlag flag, RunLoop* loop)
			: _client(client)
			, _file(nullptr)
			, _write_flag(flag)
			, _write_count(0), _offset(0)
			, _completed_end(false)
		{
			Qk_ASSERT_NE(flag, kNone_WriteCacheFlag);
			// type:
			// type = 0 only write body
			// type = 1 only write header
			// type = 2 write header and body
			// type = 3 save body

			Qk_ASSERT_EQ(_client->_file_writer, nullptr);
			_client->_file_writer = this;

			// Qk_Log("FileWriter _write_flag -- %i, %s", _write_flag, *path);

			if ( _write_flag == kBody_WriteCacheFlag ) { // only write body
				_file = new File(path, loop); // TODO: Do you want to resume from a breakpoint
			}
			else { // verification cache is valid
				auto headers = _client->response_header();
				Qk_ASSERT(headers.length());

				if ( headers.has("cache-control") ) {
					auto expires = to_expires_from_cache_content(headers["cache-control"]);
					// Qk_Log("FileWriter -- %s", *expires);
					if ( !expires.isEmpty() ) {
						headers["expires"] = expires;
					}
				}

				if ( headers.has("expires") ) {
					int64_t expires = parse_time(headers["expires"]);
					int64_t now = time_micro();
					if ( expires > now ) {
						_file = new File(path, loop);
					}
				} else if ( headers.has("last-modified") || headers.has("etag") ) {
					_file = new File(path, loop);
				}
			}

			if ( _file ) {
				_file->set_delegate(this);
				if (_write_flag == kHeader_WriteCacheFlag) { // only write header
					_file->open(FOPEN_WRONLY | FOPEN_CREAT); // keep old content
				} else {
					_file->open(FOPEN_W); // clear old content
				}
			}
		}

		~FileWriter() {
			Releasep(_file);
			_client->_file_writer = nullptr; // clear host writer ptr
		}

		void trigger_file_open(File* file) override {
			if ( _write_flag <= kAll_WriteCacheFlag) { // write header
				String header_str;
				auto& header = _client->response_header();

				for ( auto& i : header ) {
					if (!i.second.isEmpty() && i.first != "cache-control") { // ignore cache-control
						header_str += i.first;
						header_str += string_colon;
						if (i.first == "expires") {
							// 写入一个固定长度的时间字符串,方便以后重写这个值
							auto val = i.second;
							while (val.length() < 36)
								val.append(' ');
							header_str += val;
						} else {
							header_str += i.second;
						}
						header_str += string_header_end;
					}
				}
				header_str += string_header_end;
				_write_count++;
				_offset = header_str.length();
				_file->write(header_str.collapse(), 0, 1); // header write
			}
			for (auto &i: _buffer) {
				auto off = _offset;
				_write_count++;
				_offset += i.length();
				_file->write(i, off);
			}
			_buffer.clear();
		}

		void trigger_file_write(File* file, Buffer& buffer, int extra) override {
			_write_count--; Qk_ASSERT(_write_count >= 0);
			if (extra == 0) {
				_client->trigger_http_data2(buffer);
			}
			if ( _write_count == 0 ) { // all write complete
				if ( _completed_end ) { // is end
					if (_file) {
						//Qk_DLog("------- Write cache %d: file %s, %s", _offset,
						//	_file->path().c_str(),
						//	_client->_uri.href().c_str()
						//);
					}
					_client->trigger_http_end();
				} else {
					_client->read_advance(); // continue read http data
				}
			}
		}

		void write(Buffer& buffer) {
			Qk_ASSERT_EQ(_completed_end, false);
			if ( _file && _write_flag != kHeader_WriteCacheFlag ) { // has body write task
				if ( _file->is_open() ) {
					if ( ++_write_count > 32 )
						_client->read_pause(); // too many write task, pause read http data
					auto off = _offset;
					_offset += buffer.length();
					_file->write(buffer, off); // write body data to file
				} else {
					_buffer.pushBack(std::move(buffer));
					_client->read_pause(); // file not open, pause read http data
				}
			} else { // no body write task
				_client->trigger_http_data2(buffer);
				_client->read_advance();
			}
		}

		void end() {
			_completed_end = true;
			if ( _write_count == 0 && _buffer.length() == 0 ) { // file is write complete
				_client->trigger_http_end();
			}
		}

		void trigger_file_close(File* file) override {
			_client->report_error_and_abort(Error(ERR_FILE_UNEXPECTED_SHUTDOWN, "File unexpected shutdown"));
		}

		void trigger_file_error(File* file, cError& error) override {
			_client->report_error_and_abort(error);
		}

		void trigger_file_read(File* file, Buffer& buffer, int flag) override {}

	private:
		Client* _client;
		File*  _file;
		List<Buffer> _buffer;
		WriteCacheFlag _write_flag;
		int _write_count, _offset;
		bool _completed_end;
	};

	FileCacheReader* FileCacheReader_new(Client* client, int64_t size, RunLoop* loop) {
		return new FileCacheReader(client, size, loop);
	}

	FileWriter* FileWriter_new(Client* client, cString& path, WriteCacheFlag flag, RunLoop* loop) {
		return new FileWriter(client, path, flag, loop);
	}

	void FileWriter_write(FileWriter* self, Buffer& buffer) {
		self->write(buffer);
	}

	void FileWriter_end(FileWriter* self) {
		self->end();
	}

	void FileCacheReader_read_advance(FileCacheReader* self) {
		self->read_advance();
	}

	DictSS& FileCacheReader_header(FileCacheReader* self) {
		return self->header();
	}

	Reader* FileCacheReader_reader(FileCacheReader* self) {
		return self;
	}

	void FileCacheReader_Releasep(FileCacheReader* &p) {
		Releasep(p);
	}

	void FileWriter_Releasep(FileWriter* &p) {
		Releasep(p);
	}

	// cache-control: max-age=100000
	// return: expires str, Sat Aug 10 2024 13:26:02 GMT+0800
	String to_expires_from_cache_content(cString& cache_control) {
		if ( !cache_control.isEmpty() ) {
			int i = cache_control.indexOf(string_max_age);
			if ( i != -1 && i + string_max_age.length() < cache_control.length() ) {
				int j = cache_control.indexOf(',', i);
				String max_age = j != -1
				? cache_control.substring(i + string_max_age.length(), j)
				: cache_control.substring(i + string_max_age.length());
				
				int64_t num = max_age.trim().toNumber<int64_t>();
				if ( num > 0 ) {
					return gmt_time_string( time_second() + num ); // Thu, 30 Mar 2017 06:16:55 GMT
				}
			}
		}
		return String();
	}
}
