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

#include "./_render-looper.h"
#include "./util/loop.h"

namespace flare {

RenderLooper::RenderLooper(AppInl* host)
	: _host(host), _id(nullptr) 
{}

RenderLooper::~RenderLooper() {
	stop();
}

struct LooperData: Object {
	int id;
	AppInl* host;
	Cb cb;
};

void looper(CbData& ev, LooperData* data) {
	if ( data->id && !is_exited() ) {
		// 60fsp
		data->host->render_loop()->post(data->cb, 1000.0 / 60.0 * 1000);
		data->host->triggerRender();
		// DLOG("onRender");
	} else {
		Release(data);
	}
}

void RenderLooper::start() {
	typedef Callback<RunLoop::PostSyncData> Cb;
	_host->render_loop()->post_sync(Cb([this](Cb::Data &ev) {
		if (!_id) {
			LooperData* data = new LooperData();
			data->id = getId32();
			data->host = _host;
			data->cb = Cb(&looper, data);
			_id = &data->id;
			Cb::Data d;
			looper(d, data);
		}
		ev.data->complete();
	}));
}

void RenderLooper::stop() {
	typedef Callback<RunLoop::PostSyncData> Cb;
	_host->render_loop()->post_sync(Cb([this](Cb::Data& ev) {
		if (_id) {
			*_id = 0;
			_id = nullptr;
		}
		ev.data->complete();
	}));
}

}
