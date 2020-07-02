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

#include <uv.h>
#include "ftr/util/cb.h"
#include "ftr/util/string.h"
#include "ftr/util/loop.h"
#include "ftr/util/uv-1.h"

FX_NS(ftr)

class DefaultStaticCallback: public CallbackCore<Object> {
 public:
	virtual bool retain() { return 1; }
	virtual void release() { }
	virtual void call(CbD& event) const { }
};

static DefaultStaticCallback* default_callback_p = nullptr;
static Mutex mutex;

static inline DefaultStaticCallback* default_callback() {
	if ( !default_callback_p ) {
		ScopeLock scope(mutex);
		default_callback_p = NewRetain<DefaultStaticCallback>();
	}
	return default_callback_p;
}

template<>
Callback<Object>::Callback(int type): Handle<CallbackCore<Object>>(default_callback()) {
}

class WrapCallback: public CallbackCore<Object> {
 public:
	inline WrapCallback(cCb& cb, Error* err, Object* data)
	: m_inl_cb(cb), m_err(err), m_data(data) {
	}
	virtual ~WrapCallback() {
		Release(m_err);
		Release(m_data);
	}
	virtual void call(CbD& evt) const {
		evt.error = m_err;
		evt.data = m_data;
		m_inl_cb->call(evt);
	}
 private:
	Callback<> m_inl_cb;
	Error* m_err;
	Object* m_data;
};

void async_callback_and_dealloc(cCb& cb, Error* e, Object* d, PostMessage* loop) {
	loop->post_message( Cb(new WrapCallback(cb, e, d)) );
}

/**
 * @func sync_callback
 */
int sync_callback(cCb& cb, cError* err, Object* data) {
	CbD evt = { err, data, 0 };
	cb->call(evt);
	return evt.return_value;
}

/**
 * @func async_callback
 */
void async_callback(cCb& cb, PostMessage* loop) {
	if ( loop ) {
		loop->post_message( cb );
	} else {
		sync_callback(cb);
	}
}

struct TaskList {
	Mutex mutex;
	Map<uint, AsyncIOTask*> values;
};

static TaskList* tasks = new TaskList;

AsyncIOTask::AsyncIOTask(RunLoop* loop)
: m_id(iid32()), m_abort(false), m_loop(loop) {
	FX_CHECK(m_loop);
	ScopeLock scope(tasks->mutex);
	tasks->values.set(m_id, this);
}

AsyncIOTask::~AsyncIOTask() {
	ScopeLock scope(tasks->mutex);
	tasks->values.del(m_id);
}

void AsyncIOTask::abort() {
	if ( !m_abort ) {
		m_abort = true;
		release(); // end
	}
}

void AsyncIOTask::safe_abort(uint id) {
	if (id) {
		ScopeLock scope(tasks->mutex);
		auto i = tasks->values.find(id);
		if (i.is_null()) return;
		
		i.value()->m_loop->post(Cb([id](CbD& e) {
			AsyncIOTask* task = nullptr;
			{ //
				ScopeLock scope(tasks->mutex);
				auto i = tasks->values.find(id);
				if (!i.is_null()) {
					task = i.value();
				}
			}
			if (task) {
				task->abort();
			}
		}));
	}
}

FX_END
