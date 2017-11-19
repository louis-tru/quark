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

#ifndef __ngui__http__
#define __ngui__http__

#include "handle.h"
#include "string.h"
#include "map.h"
#include "event.h"
#include "error.h"
#include "loop.h"
#include "fs.h"

XX_NS(ngui)

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

/**
 * @class URI
 */
class XX_EXPORT URI: public Object {
 public:
  URI();
  URI(cString& src);
  inline bool operator==(const URI& uri) const { return _href == uri.href(); }
  inline bool operator!=(const URI& uri) const { return _href != uri.href(); }
  inline bool is_null() const { return _href.is_empty(); }
  inline URIType type() const { return _uritype; }
  inline cString& href() const { return _href; }
  inline cString& host() const { return _host; }
  inline uint     port() const { return _port; }
  inline cString& hostname() const { return _hostname; }
  inline cString& domain() const { return _domain; }
  inline cString& origin() const { return _origin; }
  inline cString& pathname() const { return _pathname; }
  inline cString& dir() const { return _dir; }
  inline cString& basename() const { return _basename; }
  inline cString& extname() const { return _extname; }
  inline cString& search() const { return _search; }
  static String encode(cString& url);
  static String decode(cString& url);
 private:
  URIType _uritype;
  uint16 _port;
  String _href, _host, _hostname;
  String _domain, _origin, _pathname;
  String _dir, _basename, _extname, _search;
};

/**
 * @class HttpClientRequest 需注意线程安全问题
 */
class XX_EXPORT HttpClientRequest: public Object, public SimpleStream {
  XX_HIDDEN_ALL_COPY(HttpClientRequest);
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
  void set_timeout(uint64 timeout_us) throw(Error);
  void set_request_header(cString& name, cString& value) throw(Error);
  void set_form(cString& form_name, cString& value) throw(Error);
  void set_upload_file(cString& form_name, cString& path) throw(Error);
  void clear_request_header() throw(Error);
  void clear_form_data() throw(Error);
  String get_response_header(cString& name);
  const Map<String, String>& get_all_response_headers();
  int64 upload_total() const;
  int64 upload_size() const;
  int64 download_total() const;
  int64 download_size() const;
  HttpReadyState ready_state() const;
  uint status_code() const;
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
  XX_DEFINE_INLINE_CLASS(Inl);
  Inl* m_inl;
};

/**
 * @class HttpError
 */
class XX_EXPORT HttpError: public Error {
 public:
  inline HttpError(int rc, cString& msg, uint status, cString& url)
  : Error(rc, msg), m_status(status), m_url(url) {
  }
  inline HttpError(const Error& err): Error(err), m_status(0), m_url() { }
  inline uint status() const { return m_status; }
  inline String url() const { return m_url; }
 private:
  uint   m_status;
  String m_url;
};

/**
 * @class HttpHelper
 */
class XX_EXPORT HttpHelper {
 public:
  struct RequestOptions {
    String              url;
    HttpMethod          method;
    Map<String, String> headers;    /* setting custom request headers */
    Buffer              post_data;  /* Non post requests ignore this option */
    String              save;       /* save body content to local disk */
    String              upload;     /* upload loacl file */
    bool                disable_ssl_verify;
    bool                disable_cache;
    bool                disable_cookie;
  };
  static uint request(RequestOptions& options, cCb& cb = 0) throw(HttpError);
  static uint request_stream(RequestOptions& options, cCb& cb = 0) throw(HttpError);
  static Buffer request_sync(RequestOptions& options) throw(HttpError);
  static uint download(cString& url, cString& save, cCb& cb = 0) throw(HttpError);
  static void download_sync(cString& url, cString& save) throw(HttpError);
  static uint upload(cString& url, cString& file, cCb& cb = 0) throw(HttpError);
  static Buffer upload_sync(cString& url, cString& file) throw(HttpError);
  static uint get(cString& url, cCb& cb = 0, bool no_cache = false) throw(HttpError);
  static uint get_stream(cString& url, cCb& cb = 0, bool no_cache = false) throw(HttpError);
  static uint post(cString& url, Buffer data, cCb& cb = 0) throw(HttpError);
  static Buffer get_sync(cString& url, bool no_cache = false) throw(HttpError);
  static Buffer post_sync(cString& url, Buffer data) throw(HttpError);
  static void abort(uint id);
  static void initialize();
  static String user_agent();
  static void set_user_agent(cString& user_agent);
  static String cache_path();
  static void set_cache_path(cString& path);
  static void clear_cache();
  static void clear_cookie();
};

XX_END
#endif
