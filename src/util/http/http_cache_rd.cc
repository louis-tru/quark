/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright (c) 2015, Louis.chu
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of Louis.chu nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL Louis.chu BE LIABLE FOR ANY
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
		FileCacheReader(Host* host, int64_t size, RunLoop* loop)
			: File(host->_cache_path, loop)
			, _read_count(0)
			, _host(host)
			, _parse_header(true), _offset(0), _size(size)
		{
			Qk_ASSERT_EQ(_host->_cache_reader, nullptr);
			_host->_cache_reader = this;
			set_delegate(this);
			open();
		}

		~FileCacheReader() {
			_host->_cache_reader = nullptr; // clear host reader ptr
		}

		// continue send http and release this reader if no cache
		void continue_send_and_release() {
			set_delegate(nullptr);
			_host->_cache_reader = nullptr;
			_host->send_http();
			release();
		}

		void trigger_file_open(File* file) override {
			read(Buffer(511)); // alloc 511+1 byte buffer and read
		}

		void trigger_file_close(File* file) override {
			if ( _parse_header ) { // unexpected shutdown
				continue_send_and_release();
			} else {
				// throw error to http client host
				_host->on_error_and_abort(Error(ERR_FILE_UNEXPECTED_SHUTDOWN, "File unexpected shutdown"));
			}
		}

		void trigger_file_error(File* file, cError& error) override {
			if ( _parse_header ) {
				continue_send_and_release();
			} else {
				// throw error to http client host
				_host->on_error_and_abort(error);
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
								if ( expires > time_millisecond() ) { // Use caching completely if valid cache
									_host->on_http_readystate_change(HTTP_READY_STATE_RESPONSE);
									_host->_download_total = Int64::max(_size - _offset, 0);
									_host->on_http_header(200, std::move(_header), true); // from cache
									read_advance();
								}
								else {
									// validate cache by server
									if ( parse_time(_header["last-modified"]) > 0 || !_header["etag"].isEmpty() ) {
										_host->send_http();
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
					_host->_download_size += buffer.length();
					_host->on_http_data(buffer, true);
				} else { // end
					_host->on_response_complete(true);
				}
			}
		}

		void trigger_file_write(File* file, Buffer& buffer, int flag) override {
		}

		DictSS& header() {
			return _header;
		}

		// impl Reader
		void read_advance() override {
			if ( !_parse_header ) {
				if ( _read_count == 0 ) {
					_read_count++;
					read(Buffer(BUFFER_SIZE), _offset);
				}
			}
		}

		// impl Reader
		void read_pause() override {}

		// impl Reader
		bool is_cache() override {
			return true;
		}

	private:
		int _read_count;
		Host* _host;
		DictSS _header;
		bool _parse_header;
		uint32_t  _offset;
		int64_t _size;
	};

	FileCacheReader* FileCacheReader_new(Host* host, int64_t size, RunLoop* loop) {
		return new FileCacheReader(host, size, loop);
	}

	void FileCacheReader_Releasep(FileCacheReader* &p) {
		Releasep(p);
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

} // namespace qk