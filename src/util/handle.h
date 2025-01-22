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

#ifndef __quark__util__handle__
#define __quark__util__handle__

#include "./object.h"
#include <functional>

namespace qk {

	/**
	 * @class Handle
	*/
	template<class T, typename Traits = object_traits<T>> class Handle {
		//! Copy constructor is not permitted.
		Qk_HIDDEN_ALL_COPY(Handle);
		T* _data;

		inline T* move() {
			return Traits::isRef ? (Traits::Retain(_data), _data) : collapse();
		}
	public:
		typedef T                Type;

		inline Handle(): _data(nullptr) {}
		inline Handle(T* data): _data(data) { Traits::Retain(data); }
		inline Handle(Handle& handle) { _data = handle.move(); }
		inline Handle(Handle&& handle) { _data = handle.move(); }

		/*
		* Directly assigning values without increasing the number of references
		*/
		inline static Handle without(T* data) {
			Handle h;
			h._data = data;
			Qk_ReturnLocal(h);
		}

		inline ~Handle() { release(); }

		inline Handle& operator=(Handle& handle) {
			Traits::Release(_data);
			_data = handle.move();
			return *this;
		}

		inline Handle& operator=(Handle&& handle) {
			Traits::Release(_data);
			_data = handle.move();
			return *this;
		}

		inline Handle& operator=(T *data) {
			Traits::Retain(data);
			Traits::Release(_data);
			_data = data;
			return *this;
		}

		inline bool operator==(const Handle& h) {
			return _data == h._data;
		}

		inline bool operator==(const T* data) {
			return _data == data;
		}

		inline operator bool() const { return _data != nullptr; }
		inline T* operator->() { return _data; }
		inline T* operator*() { return _data; }
		inline T* value() { return _data; }
		// const
		inline const T* operator->() const { return _data; }
		inline const T* operator*() const { return _data; }
		inline const T* value() const { return _data; }

		/**
		 * @method is_null() Is null data available ?
		 */
		inline bool is_null() const {
			return _data == nullptr;
		}

		/**
		 * @method collapse() Unbinding data, loss of data management, and data removal
		 */
		inline T* collapse() {
			T* data = _data;
			_data = nullptr;
			return data;
		}

		inline void release() {
			Traits::Release(_data); _data = nullptr;
		}
	};

	/**
	 * Shared pointer
	 */
	template <class T> using Sp = Handle<T>;

	/**
	 * @class CPointerHold C Pointer handle
	*/
	template<typename T> class CPointerHold {
	public:
		typedef std::function<void(T*)> Done;
		CPointerHold(T* ptr, Done done): _ptr(ptr), _done(done) {}
		~CPointerHold() { _done(_ptr); }
		inline void collapse() { _done = [](T*p){}; }
		inline operator bool() const { return _ptr != nullptr; }
		inline T* operator->() { return _ptr; }
		inline T* operator*() { return _ptr; }
	private:
		T  *_ptr;
		Done _done;
	};

}
#endif
