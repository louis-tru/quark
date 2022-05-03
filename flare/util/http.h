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

#ifndef __flare__util__http__
#define __flare__util__http__

#include "./handle.h"
#include "./event.h"
#include "./error.h"
#include "./loop.h"
#include "./stream.h"
#include "./dict.h"

namespace flare {

	/**
	* @enum URIType
	*/
	enum URIType {
		URI_UNKNOWN,
		URI_FILE,
		URI_ZIP,
		URI_HTTP,
		URI_HTTPS,
		URI_WS,
		URI_WSS,
		URI_FTP,
		URI_FTPS,
		URI_SFTP,
	};

	/**
	* @enum HttpMethod
	*/
	enum HttpMethod {
		HTTP_METHOD_GET = 0,
		HTTP_METHOD_POST = 1,
		HTTP_METHOD_HEAD = 2,
		HTTP_METHOD_DELETE = 3,
		HTTP_METHOD_PUT = 4,
	};

	/**
	* @enum HttpReadyState
	*/
	enum HttpReadyState {
		HTTP_READY_STATE_INITIAL = 0,        //（初始） 还没有调用send发送请求
		HTTP_READY_STATE_READY   = 1,        // (准备) 已经调用send进入就绪状态等待打开连接或缓存文件
		HTTP_READY_STATE_SENDING = 2,        //（发送） http socket连接打开,开始发送请求
		HTTP_READY_STATE_RESPONSE = 3,       //（接收） 数据发送完成，正在接收数据
		HTTP_READY_STATE_COMPLETED = 4       //（完成） 响应完成,请求结束
	};

	struct ResponseData;
	typedef Callback<ResponseData> HttpCb;
	typedef Dict<String, String> DictSS;

	struct RequestOptions {
		String     url;
		HttpMethod method;
		DictSS     headers;    /* setting custom request headers */
		Buffer     post_data;  /* Non post requests ignore this option */
		String     save;       /* save body content to local disk */
		String     upload;     /* upload loacl file */
		uint64_t   timeout;    /* request timeout time, default no timeout "0" */
		bool       disable_ssl_verify;
		bool       disable_cache;
		bool       disable_cookie;
	};

	struct ResponseData {
		Buffer  data;
		String  http_version;
		int     status_code;
		DictSS  response_headers;
	};

	/**
	* @class URI
	*/
	class F_EXPORT URI {
	public:
		URI();
		URI(cString& src);
		inline bool operator==(const URI& uri) const { return _href == uri.href(); }
		inline bool operator!=(const URI& uri) const { return _href != uri.href(); }
		inline bool is_null() const { return _href.is_empty(); }
		inline URIType type() const { return _uritype; }
		inline String href() const { return _href; }
		inline String host() const { return _host; }
		inline uint16_t port() const { return _port; }
		inline String hostname() const { return _hostname; }
		inline String domain() const { return _domain; }
		inline String origin() const { return _origin; }
		inline String pathname() const { return _pathname; }
		inline String dir() const { return _dir; }
		inline String basename() const { return _basename; }
		inline String extname() const { return _extname; }
		inline String search() const { return _search; }
		static String encode(cString& url);
		static String decode(cString& url);
	private:
		URIType _uritype;
		uint16_t  _port;
		String _href, _host, _hostname;
		String _domain, _origin, _pathname;
		String _dir, _basename, _extname, _search;
	};

	/**
	* @class HttpClientRequest
	*/
	class F_EXPORT HttpClientRequest: public Object, public Stream {
		F_HIDDEN_ALL_COPY(HttpClientRequest);
	public:
		class Delegate {
		public:
			virtual void trigger_http_error(HttpClientRequest* req, cError& error) = 0;
			virtual void trigger_http_write(HttpClientRequest* req) = 0;
			virtual void trigger_http_header(HttpClientRequest* req) = 0;
			virtual void trigger_http_data(HttpClientRequest* req, Buffer buffer) = 0;
			virtual void trigger_http_end(HttpClientRequest* req) = 0;
			virtual void trigger_http_readystate_change(HttpClientRequest* req) = 0;
			virtual void trigger_http_timeout(HttpClientRequest* req) = 0;
			virtual void trigger_http_abort(HttpClientRequest* req) = 0;
		};

		HttpClientRequest(RunLoop* loop = RunLoop::current());
		virtual ~HttpClientRequest();
		void set_delegate(Delegate* delegate) throw(Error);
		void set_method(HttpMethod method) throw(Error);
		void set_url(cString& path) throw(Error);
		void set_save_path(cString& path) throw(Error);
		void set_username(cString& username) throw(Error);
		void set_password(cString& password) throw(Error);
		void disable_cache(bool disable) throw(Error);
		void disable_cookie(bool disable) throw(Error);
		void disable_send_cookie(bool disable) throw(Error);
		void disable_ssl_verify(bool disable) throw(Error);
		void set_keep_alive(bool keep_alive) throw(Error);
		void set_timeout(uint64_t timeout_us) throw(Error);
		void set_request_header(cString& name, cString& value) throw(Error);
		void set_form(cString& form_name, cString& value) throw(Error);
		void set_upload_file(cString& form_name, cString& path) throw(Error);
		void clear_request_header() throw(Error);
		void clear_form_data() throw(Error);
		String get_response_header(cString& name);
		const DictSS& get_all_response_headers() const;
		int64_t upload_total() const;
		int64_t upload_size() const;
		int64_t download_total() const;
		int64_t download_size() const;
		String http_response_version() const;
		HttpReadyState ready_state() const;
		int status_code() const;
		String url() const;
		
		/**
		* @func send() 发送请求,如果设置data参数会覆盖之前设置的表单数据
		*/
		void send(Buffer data = Buffer()) throw(Error);
		void send(cString& data) throw(Error);
		
		/**
		* @overwrite
		*/
		virtual void pause();
		
		/**
		* @overwrite
		*/
		virtual void resume();
		
		/**
		* @func abort current sending request
		*/
		void abort();
	
	private:

		F_DEFINE_INLINE_CLASS(Inl);
		Inl* _inl;
	};

	/**
	* @class HttpError
	*/
	class F_EXPORT HttpError: public Error {
	public:
		HttpError(int rc, cString& msg, uint32_t status, cString& url);
		HttpError(const Error& err);
		F_DEFINE_PROP_READ(uint32_t, status);
		F_DEFINE_PROP_READ(String, url);
	};

	F_EXPORT uint32_t http_request(RequestOptions& options, HttpCb cb = 0) throw(HttpError);
	F_EXPORT uint32_t http_request_stream(RequestOptions& options, Callback<StreamResponse> cb = 0) throw(HttpError);
	F_EXPORT uint32_t http_download(cString& url, cString& save, HttpCb cb = 0) throw(HttpError);
	F_EXPORT uint32_t http_upload(cString& url, cString& file, HttpCb cb = 0) throw(HttpError);
	F_EXPORT uint32_t http_get(cString& url, HttpCb cb = 0, bool no_cache = false) throw(HttpError);
	F_EXPORT uint32_t http_get_stream(cString& url, Callback<StreamResponse> cb = 0, bool no_cache = false) throw(HttpError);
	F_EXPORT uint32_t http_post(cString& url, Buffer data, HttpCb cb = 0) throw(HttpError);
	F_EXPORT Buffer http_request_sync(RequestOptions& options) throw(HttpError);
	F_EXPORT void   http_download_sync(cString& url, cString& save) throw(HttpError);
	F_EXPORT Buffer http_upload_sync(cString& url, cString& file) throw(HttpError);
	F_EXPORT Buffer http_get_sync(cString& url, bool no_cache = false) throw(HttpError);
	F_EXPORT Buffer http_post_sync(cString& url, Buffer data) throw(HttpError);
	F_EXPORT void   http_abort(uint32_t id);
	F_EXPORT String http_user_agent();
	F_EXPORT void   http_set_user_agent(cString& user_agent);
	F_EXPORT String http_cache_path();
	F_EXPORT void   http_set_cache_path(cString& path);
	F_EXPORT void   http_clear_cache();
	// http cookie
	F_EXPORT String http_get_cookie(cString& domain, cString& name, cString& path = String(), bool ssl = 0);
	F_EXPORT String http_get_all_cookie_string(cString& domain, cString& path = String(), bool ssl = 0);
	F_EXPORT DictSS http_get_all_cookie(cString& domain, cString& path = String(), bool ssl = 0);
	F_EXPORT void http_set_cookie_with_expression(cString& domain, cString& expression);
	F_EXPORT void http_set_cookie(cString& domain, cString& name, cString& value, 
																int64_t expires = -1, cString& path = String(), bool ssl = 0);
	F_EXPORT void http_delete_cookie(cString& domain, cString& name, cString& path = String(), bool ssl = 0);
	F_EXPORT void http_delete_all_cookie(cString& domain, bool ssl = 0);
	F_EXPORT void http_clear_cookie();

}
#endif
