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

#include "./cb.h"
#include "./string.h"
#include "./loop.h"

namespace quark {

	class DefaultCallbackCore: public CallbackCore<Object, Error> {
	public:
		virtual bool retain() { return 1; }
		virtual void release() {}
		virtual void call(Cb::Data& event) const {}
	};

	static DefaultCallbackCore* default_callback_ = nullptr;
	static Mutex mutex;

	static inline DefaultCallbackCore* default_callback() {
		if ( !default_callback ) {
			ScopeLock scope(mutex);
			if (!default_callback) {
				default_callback_ = NewRetain<DefaultCallbackCore>();
			}
		}
		return default_callback_;
	}

	template<> void* Callback<Object>::DefaultCore() {
		if ( !default_callback ) {
			ScopeLock scope(mutex);
			default_callback_ = NewRetain<DefaultCallbackCore>();
		}
		return default_callback_;
	}

	class WrapCallback: public CallbackCore<Object, Error> {
	public:
		inline WrapCallback(Cb cb, Error* err, Object* data)
		: _cb(cb), _err(err), _data(data) {
		}
		virtual ~WrapCallback() {
			Release(_err);
			Release(_data);
		}
		virtual void call(Cb::Data& evt) const {
			evt.error = _err;
			evt.data = _data;
			_cb->call(evt);
		}
	private:
		Cb        _cb;
		Error*    _err;
		Object*   _data;
	};

	void _async_callback_and_dealloc(Cb cb, Error* e, Object* d, PostMessage* loop) {
		loop->post_message( Cb(new WrapCallback(cb, e, d)) );
	}

}
