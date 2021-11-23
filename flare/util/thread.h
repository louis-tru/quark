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

#ifndef __flare__util__thread__
#define __flare__util__thread__

#include <thread>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include "./string.h"
#include "./event.h"

namespace flare {

	typedef std::thread::id         ThreadID;
	typedef std::mutex              Mutex;
	typedef std::recursive_mutex    RecursiveMutex;
	typedef std::lock_guard<Mutex>  ScopeLock;
	typedef std::unique_lock<Mutex> Lock;
	typedef std::condition_variable Condition;

	template<> F_EXPORT uint64_t Compare<ThreadID>::hash_code(const ThreadID& key);

	class RunLoop;

	/**
	* @class Thread
	*/
	class F_EXPORT Thread {
		F_HIDDEN_ALL_COPY(Thread);
	 public:
		typedef ThreadID ID;
		typedef NonObjectTraits Traits;
		typedef std::function<int(Thread&)> Exec;
		inline bool is_abort() const { return _abort; }
		inline ID id() const { return _id; }
		inline String name() const { return _name; }
		inline RunLoop* loop() const { return _loop; }
		static ID spawn(Exec exec, cString& name);
		static ID current_id();
		static Thread* current();
		static void sleep(int64_t timeoutUs = 0 /*小于1永久等待*/);
		static void join(ID id, int64_t timeoutUs = 0 /*小于1永久等待*/);
		static void awaken(ID id);
		static void abort(ID id);
		static EventNoticer<>& onProcessSafeExit();
	 private:
		Thread();
		~Thread();
		F_DEFINE_INLINE_CLASS(Inl);
		bool  _abort;
		Mutex _mutex;
		Condition _cond;
		ID    _id;
		String  _name;
		void* _data[256];
		RunLoop* _loop;
		friend class RunLoop;
	};

}
#endif
