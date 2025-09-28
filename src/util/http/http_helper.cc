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

#include "../../version.h"
#include "../http.h"
#include "../fs.h"
#include "../uv.h"
#include "../../os/os.h"
#include "../thread.h"

namespace qk {
	typedef Dict<String, String> Map;

	static uint32_t http_max_connect_pool_size_(5);
	static String   http_cache_path_;
	static String   http_user_agent_;

	HttpError::HttpError(int rc, cString& msg, uint32_t status, cString& url)
		: Error(rc, msg), _status(status), _url(url)
	{}
	HttpError::HttpError(const Error& err): Error(err), _status(0), _url()
	{}

	typedef Callback<StreamResponse> SCb;

	static uint32_t http_request(RequestOptions& options, HttpCb cb, SCb scb, bool isStream) throw(HttpError) {
		
		class Task: public AsyncIOTask, public HttpClientRequest::Delegate, public Stream {
		public:
			HttpCb cb;
			SCb scb;
			bool response_data, isStream;
			Array<String> data;
			HttpClientRequest* client;

			Task() {
				client = new HttpClientRequest(loop());
				client->set_delegate(this);
				response_data = 1;
			}

			~Task() {
				Release(client);
			}

			virtual void trigger_http_error(HttpClientRequest* req, cError& error) {
				HttpError e(error.code(),
										error.message() + ", " + req->url(), req->status_code(), req->url());
				isStream ? scb->reject(&e): cb->reject(&e);
				abort(); // abort and release
			}

			virtual void trigger_http_timeout(HttpClientRequest* req) {
				HttpError e(ERR_HTTP_REQUEST_TIMEOUT,
										String("http request timeout") + ", " + req->url(), 0, req->url());
				isStream ? scb->reject(&e): cb->reject(&e);
				abort(); // abort and release
			}

			virtual void trigger_http_data(HttpClientRequest* req, Buffer &buffer) {
				if ( isStream ) {
					StreamResponse data({client->download_size(),
															client->download_total(), buffer, this, id(), false});
					scb->resolve(&data);
				} else {
					if ( response_data ) {
						data.push(buffer.collapseString());
					}
				}
			}

			virtual void trigger_http_end(HttpClientRequest* req) {
				/*
				100-199 用于指定客户端应相应的某些动作。
				200-299 用于表示请求成功。
				300-399 用于已经移动的文件并且常被包含在定位头信息中指定新的地址信息。
				400-499 用于指出客户端的错误。
				500-599 用于支持服务器错误。
				*/
				if ( client->status_code() > 399 || client->status_code() < 100 ) {
					HttpError e(ERR_HTTP_STATUS_ERROR,
											String::format("Http status error, status code:%d, %s",
																		req->status_code(), req->url().c_str()),
											req->status_code(), req->url());
					isStream ? scb->reject(&e): cb->reject(&e);
				} else if ( isStream ) {
					Buffer buffer;
					StreamResponse data({client->download_size(),
															client->download_total(), buffer, this, id(), true});
					scb->resolve(&data);
				} else {
					ResponseData rdata;
					rdata.data = data.join(String()).collapse();
					rdata.http_version = client->http_response_version();
					rdata.status_code = client->status_code();
					rdata.response_headers = std::move( client->get_all_response_headers() );
					cb->resolve(&rdata);
				}
				abort(); // abort and release
			}

			virtual void trigger_http_abort(HttpClientRequest* req) {
				Qk_DLog("request async abort");
			}

			virtual void trigger_http_write(HttpClientRequest* req) {}
			virtual void trigger_http_header(HttpClientRequest* req) {}
			virtual void trigger_http_readystate_change(HttpClientRequest* req) {}

			virtual void abort() {
				Releasep(client);
				AsyncIOTask::abort();
			}

			virtual void pause() {
				if ( client )
					client->pause();
			}

			virtual void resume() {
				if ( client )
					client->resume();
			}
		};

		Handle<Task> task(new Task());

		HttpClientRequest* req = task->client;

		try {
			req->set_url(options.url);
			req->set_method(options.method);
			req->set_timeout(options.timeout);
			req->disable_cache(options.disable_cache);
			req->disable_ssl_verify(options.disable_ssl_verify);
			req->disable_cookie(options.disable_cookie);

			task->cb = cb;
			task->scb = scb;
			task->isStream = isStream;

			if ( !options.upload.isEmpty() ) { // 需要上传文件
				req->set_upload_file("file", options.upload);
			}

			if ( !options.save.isEmpty() ) {
				task->response_data = 0;
				req->set_save_path(options.save);
			}

			for ( auto& i : options.headers ) {
				req->set_request_header(i.first, i.second);
			}

			req->send(options.post_data);
		} catch (cError& e) {
			throw HttpError(e);
		}

		return task.collapse()->id();
	}

	/**
	* @func request
	*/
	uint32_t http_request(RequestOptions& options, HttpCb cb) throw(HttpError) {
		return http_request(options, cb, 0, 0);
	}

	uint32_t http_request_stream(RequestOptions& options, Callback<StreamResponse> cb) throw(HttpError) {
		return http_request(options, 0, cb, 1);
	}

	static RequestOptions default_request_options(cString& url) {
		return {
			url,
			HTTP_METHOD_GET,
			Map(),
			Buffer(),
			String(),
			String(),
			0,
			false,
			false,
			false,
		};
	}

	/**
	* @func download
	*/
	uint32_t http_download(cString& url, cString& save, HttpCb cb) throw(HttpError) {
		RequestOptions options = default_request_options(url);
		options.save = save;
		return http_request(options, cb, 0, 0);
	}

	/**
	* @func upload
	*/
	uint32_t http_upload(cString& url, cString& file, HttpCb cb) throw(HttpError) {
		RequestOptions options = default_request_options(url);
		options.upload = file;
		options.method = HTTP_METHOD_POST;
		options.disable_cache = true;
		return http_request(options, cb, 0, 0);
	}

	/**
	* @func get
	*/
	uint32_t http_get(cString& url, HttpCb cb, bool no_cache) throw(HttpError) {
		RequestOptions options = default_request_options(url);
		options.disable_cache = no_cache;
		return http_request(options, cb, 0, 0);
	}

	/**
	* @func get_stream
	*/
	uint32_t http_get_stream(cString& url, SCb cb, bool no_cache) throw(HttpError) {
		RequestOptions options = default_request_options(url);
		options.disable_cache = no_cache;
		return http_request(options, 0, cb, 1);
	}

	/**
	* @func post
	*/
	uint32_t http_post(cString& url, Buffer data, HttpCb cb) throw(HttpError) {
		RequestOptions options = default_request_options(url);
		options.method = HTTP_METHOD_POST;
		options.post_data = data;
		return http_request(options, cb, 0, 0);
	}

	Buffer http_request_sync(RequestOptions& options) throw(HttpError) {
		if (has_backend_thread()) {
			throw HttpError(ERR_CANNOT_RUN_SYNC_IO,
											String::format("cannot send sync http request, %s"
																		, options.url.c_str()), 0, options.url);
		}
		// Qk_DLog("request_sync %s", options.url.c_str());
		typedef Callback<RunLoop::PostSyncData> Cb2;
		bool ok = false;
		HttpError err = Error(0);
		ResponseData data;

		backend_loop()->post_sync(Cb2([&](Cb2::Data& d) {
			auto dd = d.data;
			try {
				http_request(options, HttpCb([&,dd](HttpCb::Data& ev) {
					if (ev.error) {
						*const_cast<HttpError*>(&err) = std::move(*static_cast<HttpError*>(ev.error));
					} else {
						*const_cast<ResponseData*>(&data) = std::move(*ev.data);
						*const_cast<bool*>(&ok) = true;
					}
					dd->complete();
				}));
			} catch(const HttpError& e) {
				*const_cast<HttpError*>(&err) = e;
				dd->complete();
			}
		}));

		if (ok) {
			return data.data;
		} else {
			throw err;
		}
	}

	void http_download_sync(cString& url, cString& save) throw(HttpError) {
		RequestOptions options = default_request_options(url);
		options.save = save;
		http_request_sync(options);
	}

	Buffer http_upload_sync(cString& url, cString& file) throw(HttpError) {
		RequestOptions options = default_request_options(url);
		options.upload = file;
		options.method = HTTP_METHOD_POST;
		options.disable_cache = true;
		return http_request_sync(options);
	}

	Buffer http_get_sync(cString& url, bool no_cache) throw(HttpError) {
		RequestOptions options = default_request_options(url);
		options.disable_cache = no_cache;
		return http_request_sync(options);
	}

	Buffer http_post_sync(cString& url, Buffer data) throw(HttpError) {
		RequestOptions options = default_request_options(url);
		options.method = HTTP_METHOD_POST;
		options.post_data = data;
		return http_request_sync(options);
	}

	void http_abort(uint32_t id) {
		AsyncIOTask::safe_abort(id);
	}

	String http_user_agent() {
		if (http_user_agent_.isEmpty()) {
			http_user_agent_ = String::format(
				"Quark/" Qk_VERSION " %s/%s %s %s (%s)",
				*os_name(), *os_version(), *os_arch(), *os_brand(), *os_model()
			);
		}
		return http_user_agent_;
	}

	void http_set_user_agent(cString& user_agent) {
		http_user_agent_ = user_agent;
	}

	String http_cache_path() {
		if (http_cache_path_.isEmpty()) {
			http_set_cache_path(fs_temp("http_cache"));
		}
		return http_cache_path_;
	}

	void http_set_cache_path(cString& path) {
		try {
			fs_mkdirs_sync(path);
			http_cache_path_ = path;
		} catch(cError& err) {
			Qk_ELog(err);
		}
	}

	void http_clear_cache() {
		// delete cache files
		if ( ! http_cache_path_.isEmpty() ) {
			fs_remove_recursion_sync(http_cache_path_);
			http_set_cache_path(http_cache_path_);
		}
	}
	
	uint32_t http_max_connect_pool_size() {
		return http_max_connect_pool_size_;
	}

	void http_set_max_connect_pool_size(uint32_t size) {
		http_max_connect_pool_size_ = Uint32::clamp(size, 2, 100);
	}

}
