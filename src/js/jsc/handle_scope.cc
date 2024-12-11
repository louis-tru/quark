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

#include "./jsc.h"

namespace qk { namespace js {

	// --- H a n d l e . S p o c e ---

	class JscHandleScope {
	public:
		JscHandleScope(JscWorker *worker): _worker(worker) {
			DCHECK(worker);
			_prev = _worker->_scope;
			_prev->_scope = this;
		}
		~JscHandleScope() {
			auto ctx = _worker->_ctx;
			for (auto i: _handles) {
				JSValueUnprotect(ctx, i);
			}
			_worker->_scope = _prev;
		}
		void add(JSValueRef handle) {
			JSValueProtect(_worker->_ctx, handle);
			_handles.push(handle);
		}
		void escape(JSValueRef handle) {
			DCHECK(_prev);
			_prev->add(handle);
		}
		static int NumberOfHandles(JscWorker *worker) {
			DCHECK(worker);
			auto handle_scope = worker->_scope;
			DCHECK(handle_scope);
			int r = 0;
			while(handle_scope) {
				r += handle_scope->_handles.length();
				handle_scope = handle_scope->_prev;
			}
			return r;
		}
	private:
		JscWorker *_worker;
		JscHandleScope *_prev;
		Array<JSValueRef> _handles;
	};

	template<>
	JSValue* JscWorker::addToScope(JSValueRef ref) {
		DCHECK(_scope);
		_scope->add(ref);
		return Cast(ref);
	}

	HandleScope::HandleScope(Worker* worker) {
		_val[0] = new JscHandleScope(WORKER(worker));
	}

	HandleScope::~HandleScope() {
		delete reinterpret_cast<JscHandleScope*>(_val[0])->~JscHandleScope();
	}

	EscapableHandleScope::EscapableHandleScope(Worker* worker): HandleScope(worker) {
	}

	template<>
	JSValue* EscapableHandleScope::escape(JSValue* val) {
		reinterpret_cast<JscHandleScope*>(_val[0])->escape(Back(val));
	}

} }