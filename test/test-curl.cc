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

#include <curl/curl.h>
#include <qgr/utils/util.h>
#include <qgr/utils/http.h>
#include <qgr/utils/buffer.h>

using namespace qgr;

static cString  url     = "http://www.qq.com/";
static cString  PROXY   = "";
unsigned int    TIMEOUT = 2000;

size_t curl_writer(void* buffer, size_t size, size_t count, String* stream) {
	uint len = (uint)(size * count);
	stream->push((char*)buffer, len);
	return len;
};

/**
 * 生成一个easy curl对象，进行一些简单的设置操作
 */
CURL* curl_easy_handler(cString& sUrl,
												cString& sProxy, String& sRsp, unsigned int uiTimeout) {
	CURL* curl = curl_easy_init();
	
	curl_easy_setopt(curl, CURLOPT_URL, *sUrl);
	curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
	
	if ( uiTimeout > 0 ) {
		curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, uiTimeout);
	}
	if ( ! sProxy.is_empty() ) {
		curl_easy_setopt(curl, CURLOPT_PROXY, *sProxy);
	}
	
	// write function //
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_writer);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &sRsp);
	
	return curl;
}

/**
 * 使用select函数监听multi curl文件描述符的状态
 * 监听成功返回0，监听失败返回-1
 */
int curl_multi_select(CURLM* curl_m) {
	int ret = 0;
	
	struct timeval timeout_tv;
	fd_set  fd_read;
	fd_set  fd_write;
	fd_set  fd_except;
	int     max_fd = -1;
	
	// 注意这里一定要清空fdset,curl_multi_fdset不会执行fdset的清空操作  //
	FD_ZERO(&fd_read);
	FD_ZERO(&fd_write);
	FD_ZERO(&fd_except);
	
	// 设置select超时时间  //
	timeout_tv.tv_sec = 1;
	timeout_tv.tv_usec = 0;
	
	// 获取multi curl需要监听的文件描述符集合 fd_set //
	curl_multi_fdset(curl_m, &fd_read, &fd_write, &fd_except, &max_fd);
	
	/**
	 * When max_fd returns with -1,
	 * you need to wait a while and then proceed and call curl_multi_perform anyway.
	 * How long to wait? I would suggest 100 milliseconds at least,
	 * but you may want to test it out in your own particular conditions to find a suitable value.
	 */
	if (-1 == max_fd) {
		return -1;
	}
	
	/**
	 * 执行监听，当文件描述符状态发生改变的时候返回
	 * 返回0，程序调用curl_multi_perform通知curl执行相应操作
	 * 返回-1，表示select错误
	 * 注意：即使select超时也需要返回0，具体可以去官网看文档说明
	 */
	int ret_code = ::select(max_fd + 1, &fd_read, &fd_write, &fd_except, &timeout_tv);
	switch(ret_code) {
		case -1:
		/* select error */
		ret = -1;
		break;
		case 0:
		/* select timeout */
		default:
		/* one or more of curl's file descriptors say there's data to read or write*/
		ret = 0;
		break;
	}
	
	return ret;
}

/**
 * multi curl使用demo
 */
int curl_multi_demo(int num) {
	// 初始化一个multi curl 对象 //
	CURLM* curl_m = curl_multi_init();
	
	Array<String> RspArray(num);
	CURL*         CurlArray[num];
	
	// 设置easy curl对象并添加到multi curl对象中  //
	for (int idx = 0; idx < num; ++idx) {
		CurlArray[idx] = NULL;
		CurlArray[idx] = curl_easy_handler(url, PROXY, RspArray[idx], TIMEOUT);
		if (CurlArray[idx] == NULL) {
			return -1;
		}
		curl_multi_add_handle(curl_m, CurlArray[idx]);
	}
	
	/*
	 * 调用curl_multi_perform函数执行curl请求
	 * url_multi_perform返回CURLM_CALL_MULTI_PERFORM时，表示需要继续调用该函数直到返回值不是CURLM_CALL_MULTI_PERFORM为止
	 * running_handles变量返回正在处理的easy curl数量，running_handles为0表示当前没有正在执行的curl请求
	 */
	int running_handles;
	while (CURLM_CALL_MULTI_PERFORM == curl_multi_perform(curl_m, &running_handles)) {
		LOG(running_handles);
	}
	
	uint count = 0;
	
	/**
	 * 为了避免循环调用curl_multi_perform产生的cpu持续占用的问题，采用select来监听文件描述符
	 */
	while (running_handles) {
		if (-1 == curl_multi_select(curl_m)) {
			XX_ERR("select error");
			break;
		} else {
			// select监听到事件，调用curl_multi_perform通知curl执行相应的操作 //
			while (CURLM_CALL_MULTI_PERFORM == curl_multi_perform(curl_m, &running_handles)) {
				LOG("select: %d", running_handles);
			}
		}
		LOG("select: %d", running_handles);
		count++;
	}
	
	LOG("loop: %d",count);
	
	// 输出执行结果 //
	int       msgs_left;
	CURLMsg*  msg;
	
	while ((msg = curl_multi_info_read(curl_m, &msgs_left))) {
		if (CURLMSG_DONE == msg->msg) {
			int idx;
			for (idx = 0; idx < num; ++idx) {
				if (msg->easy_handle == CurlArray[idx]) break;
			}
			
			if (idx == num) {
				XX_ERR("curl not found");
			} else {
				LOG("curl [%d] completed with status: %d", idx, (uint)msg->data.result);
				LOG("rsp: %s", *RspArray[idx]);
			}
		}
	}
	
	for (int idx = 0; idx < num; ++idx) {
		curl_multi_remove_handle(curl_m, CurlArray[idx]);
		curl_easy_cleanup(CurlArray[idx]);
	}
	
	curl_multi_cleanup(curl_m);
	
}

/**
 * easy curl使用demo
 */
int curl_easy_demo(int num) {
	Array<String> RspArray(num);
	
	for (int idx = 0; idx < num; ++idx) {
		CURL * curl = curl_easy_handler(url, PROXY, RspArray[idx], TIMEOUT);
		CURLcode code = curl_easy_perform(curl);
		LOG("curl [%d] completed with status: %d", idx, (uint)code);
		LOG("rsp: %s", *RspArray[idx]);
		curl_easy_cleanup(curl);
	}
	
	return 0;
}

void test_curl(int argc, char **argv) {
//  test_curl();
	curl_multi_demo(1);
//  curl_easy_demo(1);
}
