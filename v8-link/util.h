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

#include <functional>
#include <string>

#ifndef __v8_link_util__
#define __v8_link_util__

#define CHECK(cond, ...)   if(!(cond)) FATAL(__VA_ARGS__)
#if DEBUG
# define DLOG(...)         v8::log(__VA_ARGS__)
# define DCHECK(cond, ...) if(!(cond)) FATAL(__VA_ARGS__)
#else
# define DLOG(...)          ((void)0)
# define DCHECK(cond, ...)	((void)0)
#endif

#define v8_ns(name) namespace name {
#define v8_ns_end		}
#define LOG(...)		  v8::log(__VA_ARGS__)
#define FATAL(...)    v8::fatal(__FILE__, __LINE__, __func__, ##__VA_ARGS__)
#define UNIMPLEMENTED()	FATAL("Unimplemented code")
#define UNREACHABLE()  	FATAL("Unreachable code")
#define MIN(A, B)          ((A) < (B) ? (A) : (B))
#define MAX(A, B)          ((A) > (B) ? (A) : (B))

namespace v8 {

	struct DefaultTraits {
		template<class T> inline static bool Retain(T* obj) { return 0; }
		template<class T> inline static void Release(T* obj) { delete obj; }
		static constexpr bool is_reference = false;
	};
	struct CPointerTraits: DefaultTraits {
		template<class T> inline static void Release(T* obj) { free(obj); }
	};

	template<class T, class T2 = typename T::Traits> class UniquePtr {
		UniquePtr(const UniquePtr& h) = delete;
		UniquePtr& operator=(const UniquePtr& h) = delete;
		inline T* shift() {
			return Traits::Retain(m_data) ? m_data : collapse();
		}
	 public:
		typedef T Type;
		typedef T2 Traits;
		
		inline UniquePtr(): m_data(nullptr) { }
		inline UniquePtr(T* data): m_data(data) { Traits::Retain(data); }
		inline UniquePtr(UniquePtr& handle) { m_data = handle.shift(); }
		inline UniquePtr(UniquePtr&& handle) { m_data = handle.shift(); }
		~UniquePtr() { clear(); }
		inline UniquePtr& operator=(UniquePtr& handle) {
			clear();
			m_data = handle.shift();
			return *this;
		}
		inline UniquePtr& operator=(UniquePtr&& handle) {
			clear();
			m_data = handle.shift();
			return *this;
		}
		inline T* operator->() { return m_data; }
		inline const T* operator->() const { return m_data; }
		inline T* operator*() { return m_data; }
		inline const T* operator*() const { return m_data; }
		inline const T* value() const { return m_data; }
		inline T* value() { return m_data; }
		inline bool is_null() const { return m_data == nullptr; }
		inline T* collapse() {
			T* data = m_data; m_data = nullptr;
			return data;
		}
		inline void clear() {
			Traits::Release(m_data); m_data = nullptr;
		}
	 private:
		T* m_data;
	};

	/**
	 * @class ScopeClear
	 */
	class ScopeClear {
	 public:
		typedef std::function<void()> Clear;
		ScopeClear(Clear clear): m_clear(clear) { }
		~ScopeClear() { m_clear(); }
		inline void cancel() { m_clear = [](){ }; }
	 private:
		Clear m_clear;
	};
	
	void log(const std::string& msg);
	void log(const char* msg, ...);
	void fatal(const char* msg = 0, ...);
	void fatal(const char* file, int line, const char* func, const char* msg = 0, ...);
}

#endif
