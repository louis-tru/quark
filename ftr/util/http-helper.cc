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

#include "../version.h"
#include "./http.h"
#include "./fs.h"
#include "./_uv.h"
#include <vector>

namespace ftr {

	typedef std::unordered_map<String, String> Map;

	static String http_cache_path = String();
	static String http_user_agent = "Mozilla/5.0 ftr/util/" FTR_VERSION " (KHTML, like Gecko)";

	String inl__get_http_user_agent() {
		return http_user_agent;
	}

	String inl__get_http_cache_path() {
		return http_cache_path;
	}

	typedef HttpHelper::RequestOptions RequestOptions;
	typedef HttpHelper::ResponseData ResponseData;
	typedef HttpHelper::Cb HCb;
	typedef Callback<StreamResponse> SCb;

	static uint32_t http_request(RequestOptions& options, HCb cb, SCb scb, bool stream) throw(HttpError) {
		
		class Task: public AsyncIOTask, public HttpClientRequest::Delegate, public Stream {
		public:
			HCb cb;
			SCb scb;
			bool stream, full_data;
			ArrayBuffer<String> data;
			HttpClientRequest* client;
			
			Task() {
				client = new HttpClientRequest(loop());
				client->set_delegate(this);
				full_data = 1;
			}
			
			~Task() {
				Release(client);
			}
			
			virtual void trigger_http_error(HttpClientRequest* req, cError& error) {
				HttpError e(error.code(),
										error.message() + ", " + req->url(), req->status_code(), req->url());
				cb->reject(&e);
				abort(); // abort and release
			}

			virtual void trigger_http_timeout(HttpClientRequest* req) {
				HttpError e(ERR_HTTP_REQUEST_TIMEOUT,
										String("http request timeout") + ", " + req->url(), 0, req->url());
				cb->reject(&e);
				abort(); // abort and release
			}

			virtual void trigger_http_data(HttpClientRequest* req, Buffer buffer) {
				if ( stream ) {
					// TODO 现在还不支持暂停与恢复功能
					StreamResponse data(buffer, 0, id(),
														client->download_size(),
														client->download_total(), nullptr /*, this*/);
					scb->resolve(&data);
				} else {
					if ( full_data ) {
						data.push(buffer);
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
																		req->status_code(), req->url().str_c()),
											req->status_code(), req->url());
					cb->reject(&e);
				} else {
					if ( stream ) {
						// TODO 现在还不支持暂停与恢复功能
						StreamResponse data(Buffer(), 1, id(),
															client->download_size(),
															client->download_total(), nullptr /*, this*/);
						scb->resolve(&data);
					} else {
						ResponseData rdata;
						rdata.data = data.join("").collapse();
						rdata.http_version = client->http_response_version();
						rdata.status_code = client->status_code();
						rdata.response_headers = std::move( client->get_all_response_headers() );
						cb->resolve(&rdata);
					}
				}
				abort(); // abort and release
			}
			
			virtual void trigger_http_abort(HttpClientRequest* req) {
				FX_DEBUG("request async abort");
			}
			
			virtual void trigger_http_write(HttpClientRequest* req) {}
			virtual void trigger_http_header(HttpClientRequest* req) {}
			virtual void trigger_http_readystate_change(HttpClientRequest* req) {}
			
			virtual void abort() {
				Release(client); client = nullptr;
				AsyncIOTask::abort();
			}
			
			virtual void pause() {
				if ( client ) client->pause();
			}
			
			virtual void resume() {
				if ( client ) client->resume();
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
			task->stream = stream;
			
			if ( !options.upload.is_empty() ) { // 需要上传文件
				req->set_upload_file("file", options.upload);
			}

			if ( !options.save.is_empty() ) {
				task->full_data = 0;
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
	uint32_t HttpHelper::request(RequestOptions& options, Cb cb) throw(HttpError) {
		return http_request(options, cb, 0, false);
	}

	uint32_t HttpHelper::request_stream(RequestOptions& options, Callback<StreamResponse> cb) throw(HttpError) {
		return http_request(options, 0, cb, true);
	}

	extern RunLoop* get_private_loop();
	extern bool has_private_loop_thread();

	/**
	* @func request_sync
	*/
	Buffer HttpHelper::request_sync(RequestOptions& options) throw(HttpError) {
		if (has_private_loop_thread()) {
			throw HttpError(ERR_CANNOT_RUN_SYNC_IO,
											String::format("cannot send sync http request, %s"
																		, options.url.str_c()), 0, options.url);
		}
		FX_DEBUG("request_sync %s", options.url.str_c());
		typedef Callback<RunLoop::PostSyncData> Cb_;
		bool ok = false;
		HttpError err = Error();
		ResponseData data;

		get_private_loop()->post_sync(Cb_([&](Cb_::Data& d) {
			auto dd = d.data;
			try {
				request(options, HCb([&,dd](HCb::Data& ev) {
					if (ev.error) {
						*const_cast<HttpError*>(&err) = std::move(*static_cast<const HttpError*>(ev.error));
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
	uint32_t HttpHelper::download(cString& url, cString& save, Cb cb) throw(HttpError) {
		RequestOptions options = default_request_options(url);
		options.save = save;
		return http_request(options, cb, 0, false);
	}

	/**
	* @func download_sync
	*/
	void HttpHelper::download_sync(cString& url, cString& save) throw(HttpError) {
		RequestOptions options = default_request_options(url);
		options.save = save;
		request_sync(options);
	}

	/**
	* @func upload
	*/
	uint32_t HttpHelper::upload(cString& url, cString& file, Cb cb) throw(HttpError) {
		RequestOptions options = default_request_options(url);
		options.upload = file;
		options.method = HTTP_METHOD_POST;
		options.disable_cache = true;
		return http_request(options, cb, 0, false);
	}

	/**
	* @func upload
	*/
	Buffer HttpHelper::upload_sync(cString& url, cString& file) throw(HttpError) {
		RequestOptions options = default_request_options(url);
		options.upload = file;
		options.method = HTTP_METHOD_POST;
		options.disable_cache = true;
		return request_sync(options);
	}

	/**
	* @func get
	*/
	uint32_t HttpHelper::get(cString& url, Cb cb, bool no_cache) throw(HttpError) {
		RequestOptions options = default_request_options(url);
		options.disable_cache = no_cache;
		return http_request(options, cb, 0, false);
	}

	/**
	* @func get_stream
	*/
	uint32_t HttpHelper::get_stream(cString& url, SCb cb, bool no_cache) throw(HttpError) {
		RequestOptions options = default_request_options(url);
		options.disable_cache = no_cache;
		return http_request(options, 0, cb, true);
	}

	/**
	* @func post
	*/
	uint32_t HttpHelper::post(cString& url, Buffer data, Cb cb) throw(HttpError) {
		RequestOptions options = default_request_options(url);
		options.method = HTTP_METHOD_POST;
		options.post_data = data;
		return http_request(options, cb, 0, false);
	}

	/**
	* @func get_sync
	*/
	Buffer HttpHelper::get_sync(cString& url, bool no_cache) throw(HttpError) {
		RequestOptions options = default_request_options(url);
		options.disable_cache = no_cache;
		return request_sync(options);
	}

	/**
	* @func post_sync
	*/
	Buffer HttpHelper::post_sync(cString& url, Buffer data) throw(HttpError) {
		RequestOptions options = default_request_options(url);
		options.method = HTTP_METHOD_POST;
		options.post_data = data;
		return request_sync(options);
	}

	/**
	* @func abort
	*/
	void HttpHelper::abort(uint32_t id) {
		AsyncIOTask::safe_abort(id);
	}

	/**
	* @func initialize
	*/
	void HttpHelper::initialize() {
		static int http_initialized = 0;
		if ( ! http_initialized++ ) {
			http_user_agent = String::format("Mozilla/5.0 (%s/%s) ftr/util/"
													FTR_VERSION " (KHTML, like Gecko)", *os::name(), *sys::version());
			set_cache_path(Path::temp("http_cache"));
		}
	}

	/**
	* @func user_agent
	*/
	String HttpHelper::user_agent() {
		return http_user_agent;
	}

	/**
	* @func set_user_agent
	*/
	void HttpHelper::set_user_agent(cString& user_agent) {
		http_user_agent = user_agent;
	}

	/**
	* @func cache_path
	*/
	String HttpHelper::cache_path() {
		return http_cache_path;
	}

	/**
	* @func set_cache_path 设置缓存文件路径
	*/
	void HttpHelper::set_cache_path(cString& path) {
		try {
			FileHelper::mkdir_p_sync(path);
			http_cache_path = path;
		} catch(cError& err) {
			FX_ERR(err.to_string());
		}
	}

	/**
	* @func clean_cache 清理web缓存
	*/
	void HttpHelper::clear_cache() {
		// delete cache files
		if ( ! http_cache_path.is_empty() ) {
			FileHelper::remove_r_sync(http_cache_path);
			set_cache_path(http_cache_path);
		}
	}

}
