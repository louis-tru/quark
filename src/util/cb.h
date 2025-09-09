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

#ifndef __quark__util__cb__
#define __quark__util__cb__

#include "./util.h"
#include "./error.h"
#include "./handle.h"
#include <functional>

namespace qk {

	class PostMessage;

	template<class D, class E = Error>
	struct CallbackData {
		E *error;
		D *data;
		int rc;
		void *extra = nullptr;
	};

	template<class D, class E = Error>
	class CallbackCore: public Reference {
		Qk_DISABLE_COPY(CallbackCore);
	public:
		typedef CallbackData<D, E> Data;
		inline CallbackCore() {}
		inline int call(E* e, D* d) const {
			Data evt = { e,d,0,0 };
			const_cast<CallbackCore*>(this)->call(evt);
			return evt.rc;
		}
		inline int resolve(D* d = nullptr) const {
			return call(nullptr, d);
		}
		inline int reject(E* e) const {
			return call(e, nullptr);
		}
		inline int resolve(D &&d) const {
			return call(nullptr, &d);
		}
		inline int reject(E &&e) const {
			return call(&e, nullptr);
		}
		virtual void call(Data& evt) = 0;
	};

	template<class T, class D = Object, class E = Error>
	class CallbackCoreImpl: public CallbackCore<D, E> {
	public:
		inline CallbackCoreImpl(T* ctx): _ctx(ctx) {
			if ( object_traits<T>::is::ref ) {
				object_traits<T>::Retain(_ctx);
			}
		}
		virtual ~CallbackCoreImpl() {
			if ( object_traits<T>::is::ref ) {
				object_traits<T>::Release(_ctx);
			}
		}
	protected:
		T* _ctx;
	};

	template<class T, class D, class E>
	class StaticCallback: public CallbackCoreImpl<T, D, E> {
	public:
		typedef void (*Func)(CallbackData<D, E>& evt, T* ctx);
		inline StaticCallback(Func func, T* ctx = nullptr)
			: CallbackCoreImpl<T, D, E>(ctx), _func(func) {}
		void call(CallbackData<D, E>& evt) { _func(evt, this->_ctx); }
	private:
		Func _func;
	};

	template<class T, class D, class E>
	class MemberCallback: public CallbackCoreImpl<T, D, E> {
	public:
		typedef void (T::*Func)(CallbackData<D, E>& evt);
		inline MemberCallback(Func func, T* ctx)
			: CallbackCoreImpl<T, D, E>(ctx), _func(func) {}
		void call(CallbackData<D, E>& evt) { (this->_ctx->*_func)(evt); }
	private:
		Func _func;
	};

	template<class T, class D, class E>
	class LambdaCallback: public CallbackCoreImpl<T, D, E> {
	public:
		typedef std::function<void(CallbackData<D, E>& evt)> Func;
		inline LambdaCallback(Func &&func, T* ctx = nullptr)
			: CallbackCoreImpl<T, D, E>(ctx), _func(std::move(func)) {}
		void call(CallbackData<D, E>& evt) { _func(evt); }
	private:
		Func _func;
	};

	template<class D = Object, class E = Error>
	class Callback: public Handle<CallbackCore<D, E>> {
	public:
		typedef CallbackCore<D, E> Core;
		typedef CallbackData<D, E> Data;

		template <class T = Object>
		using Static = typename StaticCallback<T, D, E>::Func;
		template <class T = Object>
		using Member = typename MemberCallback<T, D, E>::Func;
		template <class T = Object>
		using Lambda = typename LambdaCallback<T, D, E>::Func;

		enum { kNoop = 0 };
		inline Callback(int type = kNoop): Handle<Core>(static_cast<Core*>(Callback<>::DefaultCore())){}
		inline Callback(Core* cb): Handle<Core>(cb) {}
		inline Callback(const Callback& cb): Handle<Core>(*const_cast<Callback*>(&cb)) {}
		inline Callback(Callback& cb): Handle<Core>(cb) {}
		inline Callback(Callback&& cb): Handle<Core>(cb) {}

		template<class T = Object>
		inline Callback(Static<T> func, T* ctx = nullptr):
			Handle<Core>(new StaticCallback<T, D, E>(func, ctx)) {}

		template<class T = Object>
		inline Callback(Member<T> func, T* ctx):
			Handle<Core>(new MemberCallback<T, D, E>(func, ctx)) {}

		template<class T = Object>
		inline Callback(Lambda<T> &&func, T* ctx = nullptr):
			Handle<Core>(new LambdaCallback<T, D, E>(std::move(func), ctx)) {}

		inline Callback& operator=(const Callback& cb) {
			Handle<Core>::operator=(*const_cast<Callback*>(&cb));
			return *this;
		}
		inline Callback& operator=(Callback& cb) {
			Handle<Core>::operator=(cb);
			return *this;
		}
		inline Callback& operator=(Callback&& cb) {
			Handle<Core>::operator=(cb);
			return *this;
		}
		inline Core* detach() { return nullptr; }
	private:
		static void* DefaultCore();
		template<class D2, class E2> friend class Callback;
	};

	template<> void* Callback<Object>::DefaultCore();

	typedef Callback<> Cb;
	typedef const Cb cCb;

	Qk_EXPORT void _async_callback_and_dealloc(Cb &cb, Error* e, Object* d, PostMessage* loop);

	/**
	 *  @method _async_callback() async callback and move dealloc data
	 */
	template<class D, class E, class D2, class E2>
	void _async_callback(Callback<D, E> &cb, E2* e, D2* d, PostMessage* loop) {
		if ( loop ) {
			Qk_Type_Check(D, D2);
			Qk_Type_Check(E, E2);
			_async_callback_and_dealloc(*reinterpret_cast<Cb*>(&cb),
				static_cast<Error*>(e ? new E2(std::move(*e)): nullptr),
				static_cast<Object*>(d ? new D2(std::move(*d)): nullptr), loop
			);
		} else {
			cb->call(e, d);
		}
	}

	template<class E, class D, class E2, class D2>
	inline void async_callback(Callback<D, E> cb, E2&& err, D2&& data, PostMessage* loop = nullptr) {
		_async_callback(cb, &err, &data, loop);
	}

	template<class D, class E>
	inline void async_callback(Callback<D, E> cb, PostMessage* loop = nullptr) {
		_async_callback(cb, (E*)nullptr, (D*)nullptr, loop);
	}

	template<class D, class E, class D2>
	inline void async_resolve(Callback<D, E> cb, D2&& data, PostMessage* loop = nullptr) {
		_async_callback(cb, (E*)nullptr, &data, loop);
	}

	template<class D, class E, class E2>
	inline void async_reject(Callback<D, E> cb, E2&& err, PostMessage* loop = nullptr) {
		_async_callback(cb, &err, (D*)nullptr, loop);
	}

}
#endif
