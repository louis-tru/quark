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

	class FileWriter: public Object, public File::Delegate {
	public:
		FileWriter(Host	*host, cString& path, WriteFlag flag, RunLoop* loop)
			: _host(host)
			, _file(nullptr)
			, _write_flag(flag)
			, _write_count(0), _offset(0)
			, _completed_end(false)
		{
			Qk_ASSERT_NE(flag, kNone_WriteFlag);
			// type:
			// type = 0 only write body
			// type = 1 only write header
			// type = 2 write header and body
			// type = 3 only write body

			Qk_ASSERT_EQ(_host->_file_writer, nullptr);
			_host->_file_writer = this;

			// Qk_Log("FileWriter _write_flag -- %i, %s", _write_flag, *path);

			if ( _write_flag == kBody_WriteFlag ) { // only write body
				_file = new File(path, loop); // TODO: Do you want to resume from a breakpoint
			}
			else { // verification cache is valid
				const auto &headers = _host->response_header();
				Qk_ASSERT(headers.length());

				String expires;
				if ( headers.get("cache-control", expires) ) {
					expires = to_expires_from_cache_content(expires);
				}
				if ((!expires.isEmpty() && parse_time(expires) > time_millisecond()) ||
							headers.has("last-modified") || headers.has("etag")
				) { // valid cache
					_file = new File(path, loop);
				} // else Invalid cache, discard
			}

			if ( _file ) {
				_file->set_delegate(this);
				if (_write_flag == kHeader_WriteFlag) { // only write header
					_file->open(FOPEN_WRONLY | FOPEN_CREAT); // keep old content
				} else {
					_file->open(FOPEN_W); // clear old content
				}
			}
		}

		~FileWriter() {
			Releasep(_file);
			_host->_file_writer = nullptr; // clear host writer ptr
		}

		void trigger_file_open(File* file) override {
			if ( _write_flag <= kAll_WriteFlag) { // write header
				String header_str;
				auto& header = _host->response_header();
				if (!header.has("expires")) {
					// write expires header placeholder, avoid later rewrite difficulty.
					// because the header string length have to fixed length.
					header["expires"] = gmt_time_string( time_second() );
				}

				for ( auto& i : header ) {
					if (!i.second.isEmpty() && i.first != "cache-control") { // ignore cache-control
						header_str += i.first;
						header_str += string_colon;
						if (i.first == "expires") {
							// pad expires header to fixed length 36
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
			_write_count--;
			Qk_ASSERT(_write_count >= 0);
			if (extra == 0) {
				_host->trigger_http_data(buffer);
			}
			if ( _write_count == 0 ) { // all write complete
				if ( _completed_end ) { // is end
					if (_file) {
						//Qk_DLog("------- Write cache %d: file %s, %s", _offset,
						//	_file->path().c_str(),
						//	_host->_uri.href().c_str()
						//);
					}
					_host->on_http_end();
				} else {
					_host->read_advance(); // continue read http data
				}
			}
		}

		void write(Buffer& buffer) {
			Qk_ASSERT_EQ(_completed_end, false);
			if ( _file && _write_flag != kHeader_WriteFlag ) { // has body write task
				if ( _file->is_open() ) {
					if ( ++_write_count > 32 )
						_host->read_pause(); // too many write task, pause read http data
					auto off = _offset;
					_offset += buffer.length();
					_file->write(buffer, off); // write body data to file
				} else {
					_buffer.pushBack(std::move(buffer));
					_host->read_pause(); // file not open, pause read http data
				}
			} else { // no body write task
				_host->trigger_http_data(buffer);
				_host->read_advance();
			}
		}

		void end() {
			_completed_end = true;
			if ( _write_count == 0 && _buffer.length() == 0 ) { // file is write complete
				_host->on_http_end();
			}
		}

		void trigger_file_close(File* file) override {
			_host->on_error_and_abort(Error(ERR_FILE_UNEXPECTED_SHUTDOWN, "File unexpected shutdown"));
		}

		void trigger_file_error(File* file, cError& error) override {
			_host->on_error_and_abort(error);
		}

		void trigger_file_read(File* file, Buffer& buffer, int flag) override {}

	private:
		Host* _host;
		File*  _file;
		List<Buffer> _buffer;
		WriteFlag _write_flag;
		int _write_count, _offset;
		bool _completed_end;
	};

	FileWriter* FileWriter_new(Host* host, cString& path, WriteFlag flag, RunLoop* loop) {
		return new FileWriter(host, path, flag, loop);
	}

	void FileWriter_write(FileWriter* self, Buffer& buffer) {
		self->write(buffer);
	}

	void FileWriter_end(FileWriter* self) {
		self->end();
	}

	void FileWriter_Releasep(FileWriter* &p) {
		Releasep(p);
	}

	String get_expires_from_header(const DictSS& header) {
		String expires;
		// priority: cache-control > expires
		if ( header.get("cache-control", expires) && !expires.isEmpty() )
			expires = to_expires_from_cache_content(expires);
		// get expires header if cache-control not exist
		if ( expires.isEmpty() )
			header.get("expires", expires);
		return expires;
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
