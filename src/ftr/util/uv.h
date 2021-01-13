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

#ifndef __ftr__util__uv_1__
#define __ftr__util__uv_1__

#include <uv.h>
#include "ftr/util/util.h"

FX_NS(ftr)

/**
 * @class UVRequestWrap
 */
template<class uv_req, class Context, class Data = Object>
class UVRequestWrap: public Object {
 public:
	inline UVRequestWrap(Context* ctx, cCb& cb = 0, Data data = Data())
	: _ctx(ctx), _cb(cb), _data(std::move(data)) {
		_req.data = this;
		if (Context::Traits::is_reference) Retain(_ctx);
	}
	virtual ~UVRequestWrap() {
		if (Context::Traits::is_reference) Release(_ctx);
	}
	static inline UVRequestWrap* cast(uv_req* req) {
		return (UVRequestWrap*)req->data;
	}
	inline Context* ctx() { return _ctx; }
	inline Callback<>& cb() { return _cb; }
	inline uv_req* req() { return &_req; }
	inline Data& data() { return _data; }
 private:
	uv_req    _req;
	Context*  _ctx;
	Callback<>  _cb;
	Data      _data;
};

/**
 * @class AsyncIOTask
 */
class FX_EXPORT AsyncIOTask: public Reference {
	FX_HIDDEN_ALL_COPY(AsyncIOTask);
 public:
	AsyncIOTask(RunLoop* loop = RunLoop::current());
	virtual ~AsyncIOTask();
	static void safe_abort(uint32_t id);
	inline bool is_abort() const { return _abort; }
	inline uint32_t id() const { return _id; }
	inline RunLoop* loop() { return _loop; }
	virtual void abort();
 private:
	uint32_t _id;
	bool _abort;
	RunLoop* _loop;
};

FX_END
#endif
