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

// @private head

#include "../http.h"
#include "../net.h"
#include "../fs.h"
#include "../codec.h"
#include "../errno.h"
#include "../../version.h"
#include "../list.h"
#include "../dict.h"
#include <http_parser.h>
#include <zlib.h>

#ifndef __quark__util__http__http_inl__
#define __quark__util__http__http_inl__

#define BUFFER_SIZE (65535)
namespace qk {
	class HttpHandler;
	class FileCacheReader;
	class FileWriter;

	typedef List<HttpHandler*>::Iterator HandlerID;
	typedef HttpClientRequest::Impl Host; // Http Client Host
	typedef HttpClientRequest::Delegate HttpDelegate;

	extern cString string_method[5];
	extern cString string_colon;
	extern cString string_space;
	extern cString string_header_end;
	extern cString string_max_age;
	extern cString content_type_form;
	extern cString content_type_multipart_form;
	extern cString multipart_boundary_start;
	extern cString multipart_boundary_end;

	enum FormType {
		FORM_TYPE_TEXT,
		FORM_TYPE_FILE,
	};

	enum WriteFlag {
		kNone_WriteFlag = 0, // don't save cache
		kHeader_WriteFlag, // save header and body
		kAll_WriteFlag, // save all (header, body, chunked ...)
		kBody_WriteFlag, // only save body, For download file
	};

	struct FormValue {
		FormType type;
		String   data;
		String   name;
	};

	struct MultipartFormValue {
		FormType type;
		String   data;
		String   headers;
	};

	// Reader interface, used for http data read control, http or cache reader
	class Reader {
	public:
		virtual void read_advance() = 0;
		virtual void read_pause() = 0;
		virtual bool is_cache() = 0;
	};

	class HttpHandlerPool {
	public:
		struct connect_req {
			Host* host;
			Cb cb;
			uint32_t wait_id;
			uint16_t port;
		};
		HttpHandlerPool();
		~HttpHandlerPool();
		void request(Host* host, Cb cb);
		void release(HttpHandler* c, bool immediatelyRelease);
	private:
		void call_req_tasks(Lock *lock);
		HttpHandler* get(Host* host, uint16_t port);
		Mutex _mutex;
		List<HttpHandler*> _handlers;
		List<connect_req> _reqs;
	};

	class HttpClientRequest::Impl: public Reference {
	public:
		struct RetainRef {
			RetainRef(Impl* hold): hold(hold) {}
			~RetainRef() { hold->_retain = nullptr; }
			Sp<Impl> hold;
			bool    ending = false;
		};
		Impl(HttpClientRequest* cli, RunLoop* loop);
		~Impl();
		void set_delegate(HttpDelegate* delegate);
		inline RunLoop* loop() { return _loop; }
		inline DictSS& response_header() { return _response_header; }
		void check_is_can_modify() throw(Error);
		void send(Buffer data) throw(Error);
		void abort();
		void pause();
		void resume();
		Reader* reader();
		void read_advance();
		void read_pause();
		bool is_disable_cache();
		void on_http_readystate_change(HttpReadyState ready_state);
		void on_http_header(uint32_t status_code, DictSS&& header, bool fromCache);
		void on_http_write();
		void on_http_data(Buffer& buffer, bool fromCache);
		void trigger_http_data(Buffer& buffer);
		void on_response_complete(bool fromCache);
		void on_error_and_abort(cError& error);
		void on_http_timeout();
		void send_http();
		void cache_file_stat_cb(Callback<FileStat>::Data& evt);
		void on_http_end();
		void end_(bool abort);
	private:
		HttpClientRequest* _cli;
		RunLoop*      _loop;
		HttpDelegate* _delegate;
		int64_t      _upload_total;    /* 需上传到服务器数据总量 */
		int64_t      _upload_size;     /* 已写上传到服务器数据尺寸 */
		int64_t      _download_total;  /* 需下载数据总量 */
		int64_t      _download_size;   /* 已下载数据量 */
		HttpReadyState _ready_state; /* 请求状态 */
		int         _status_code;    /* 服务器响应http状态码 */
		HttpMethod  _method;
		URI         _uri;
		HttpHandler* _handler;
		FileCacheReader* _cache_reader;
		FileWriter* _file_writer;
		DictSS      _request_header, _response_header;
		Dict<String, FormValue> _form_data;
		Buffer      _post_data;
		String      _username, _password;
		String      _save_path, _cache_path, _http_response_version;
		RetainRef*  _retain;
		uint64_t    _timeout;
		uint32_t    _wait_connect_id;
		WriteFlag   _write_flag;
		bool        _disable_cache, _disable_cookie;
		bool        _disable_send_cookie, _disable_ssl_verify;
		bool        _keep_alive, _pause, _url_no_cache_arg, _canSave;
		static HttpHandlerPool* _pool;
		friend class HttpClientRequest;
		friend class HttpHandler;
		friend class FileCacheReader;
		friend class FileWriter;
		friend class HttpHandlerPool;
	};

	String uri_encode(cString& url, bool component = false, bool secondary = false);
	void HttpHandler_bind_host_and_send(HttpHandler *conn, Host* host);
	Reader* HttpHandler_reader(HttpHandler* self);
	String to_expires_from_cache_content(cString& cache_control);
	FileCacheReader* FileCacheReader_new(Host* host, int64_t size, RunLoop* loop);
	FileWriter* FileWriter_new(Host* host, cString& path, WriteFlag flag, RunLoop* loop);
	void FileWriter_write(FileWriter* self, Buffer& buffer);
	void FileWriter_end(FileWriter* self);
	void FileCacheReader_read_advance(FileCacheReader* self);
	DictSS& FileCacheReader_header(FileCacheReader* self);
	Reader* FileCacheReader_reader(FileCacheReader* self);
	void FileCacheReader_Releasep(FileCacheReader* &p);
	void FileWriter_Releasep(FileWriter* &p);
	String get_expires_from_header(const DictSS& header);
}
#endif
