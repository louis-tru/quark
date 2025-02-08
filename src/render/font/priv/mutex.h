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

//@private head

#ifndef __quark__font__priv__mutex__
#define __quark__font__priv__mutex__

#include "../../../util/loop.h"
#include "./semaphore.h"

namespace qk {

	class Qk_CAPABILITY("mutex") Mutex0 {
	public:

		void acquire() Qk_ACQUIRE() {
			fSemaphore.wait();
			Qk_DEBUGCODE(fOwner = thread_self_id();)
		}

		void release() Qk_RELEASE_CAPABILITY() {
			this->assertHeld();
			Qk_DEBUGCODE(fOwner = ThreadID();)
			fSemaphore.signal();
		}

		void assertHeld() Qk_ASSERT_CAPABILITY(this) {
			Qk_DEBUGCODE(
				Qk_ASSERT(fOwner == thread_self_id())
			);
		}

	private:
		Semaphore fSemaphore{1};
		Qk_DEBUGCODE(ThreadID fOwner;)
	};

	class Qk_SCOPED_CAPABILITY AutoMutexExclusive {
	public:
		AutoMutexExclusive(Mutex0& mutex) Qk_ACQUIRE(mutex) : fMutex(mutex) { fMutex.acquire(); }
		~AutoMutexExclusive() Qk_RELEASE_CAPABILITY() { fMutex.release(); }

		AutoMutexExclusive(const AutoMutexExclusive&) = delete;
		AutoMutexExclusive(AutoMutexExclusive&&) = delete;

		AutoMutexExclusive& operator=(const AutoMutexExclusive&) = delete;
		AutoMutexExclusive& operator=(AutoMutexExclusive&&) = delete;

	private:
		Mutex0& fMutex;
	};

	// There are two shared lock implementations one debug the other is high performance. They implement
	// an interface similar to pthread's rwlocks.
	// This is a shared lock implementation similar to pthreads rwlocks. The high performance
	// implementation is cribbed from Preshing's article:
	// http://preshing.com/20150316/semaphores-are-surprisingly-versatile/
	//
	// This lock does not obey strict queue ordering. It will always alternate between readers and
	// a single writer.
	class Qk_CAPABILITY("mutex") SharedMutex {
	public:
		SharedMutex();
		~SharedMutex();
		// Acquire lock for exclusive use.
		void acquire() Qk_ACQUIRE();

		// Release lock for exclusive use.
		void release() Qk_RELEASE_CAPABILITY();

		// Fail if exclusive is not held.
		void assertHeld() const Qk_ASSERT_CAPABILITY(this);

		// Acquire lock for shared use.
		void acquireShared() Qk_ACQUIRE_SHARED();

		// Release lock for shared use.
		void releaseShared() Qk_RELEASE_SHARED_CAPABILITY();

		// Fail if shared lock not held.
		void assertHeldShared() const Qk_ASSERT_SHARED_CAPABILITY(this);

	private:
#if Qk_DEBUG
		class ThreadIDSet;
		std::unique_ptr<ThreadIDSet> fCurrentShared;
		std::unique_ptr<ThreadIDSet> fWaitingExclusive;
		std::unique_ptr<ThreadIDSet> fWaitingShared;
		int fSharedQueueSelect{0};
		mutable Mutex0 fMu;
		Semaphore fSharedQueue[2];
		Semaphore fExclusiveQueue;
#else
		std::atomic<int32_t> fQueueCounts;
		Semaphore            fSharedQueue;
		Semaphore            fExclusiveQueue;
#endif  // Qk_DEBUG
	};

#if !Qk_DEBUG
	inline void SharedMutex::assertHeld() const {};
	inline void SharedMutex::assertHeldShared() const {};
#endif  // Qk_DEBUG

	class Qk_SCOPED_CAPABILITY AutoSharedMutexExclusive {
	public:
		explicit AutoSharedMutexExclusive(SharedMutex& lock) Qk_ACQUIRE(lock)
				: fLock(lock) {
			lock.acquire();
		}
		~AutoSharedMutexExclusive() Qk_RELEASE_CAPABILITY() { fLock.release(); }

	private:
		SharedMutex& fLock;
	};

	class Qk_SCOPED_CAPABILITY AutoSharedMutexShared {
	public:
		explicit AutoSharedMutexShared(SharedMutex& lock) Qk_ACQUIRE_SHARED(lock)
				: fLock(lock)  {
			lock.acquireShared();
		}

		// You would think this should be Qk_RELEASE_SHARED_CAPABILITY, but Qk_SCOPED_CAPABILITY
		// doesn't fully understand the difference between shared and exclusive.
		// Please review https://reviews.llvm.org/D52578 for more information.
		~AutoSharedMutexShared() Qk_RELEASE_CAPABILITY() { fLock.releaseShared(); }

	private:
		SharedMutex& fLock;
	};

} // namespace qk

typedef qk::Mutex0 QkMutex;
typedef qk::AutoMutexExclusive QkAutoMutexExclusive;
typedef qk::SharedMutex QkSharedMutex;
typedef qk::AutoSharedMutexExclusive QkAutoSharedMutexExclusive;
typedef qk::AutoSharedMutexShared QkAutoSharedMutexShared;

#endif
