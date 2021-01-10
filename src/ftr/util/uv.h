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
	: m_ctx(ctx), m_cb(cb), m_data(move(data)) {
		m_req.data = this;
		if (Context::Traits::is_reference) Retain(m_ctx);
	}
	virtual ~UVRequestWrap() {
		if (Context::Traits::is_reference) Release(m_ctx);
	}
	static inline UVRequestWrap* cast(uv_req* req) {
		return (UVRequestWrap*)req->data;
	}
	inline Context* ctx() { return m_ctx; }
	inline Callback<>& cb() { return m_cb; }
	inline uv_req* req() { return &m_req; }
	inline Data& data() { return m_data; }
 private:
	uv_req    m_req;
	Context*  m_ctx;
	Callback<>  m_cb;
	Data      m_data;
};

/**
 * @class AsyncIOTask
 */
class FX_EXPORT AsyncIOTask: public Reference {
	FX_HIDDEN_ALL_COPY(AsyncIOTask);
 public:
	AsyncIOTask(RunLoop* loop = RunLoop::current());
	virtual ~AsyncIOTask();
	static void safe_abort(uint id);
	inline bool is_abort() const { return m_abort; }
	inline uint id() const { return m_id; }
	inline RunLoop* loop() { return m_loop; }
	virtual void abort();
 private:
	uint m_id;
	bool m_abort;
	RunLoop* m_loop;
};

FX_END
#endif
