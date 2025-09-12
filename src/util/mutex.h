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

#ifndef __quark__util__mutex__
#define __quark__util__mutex__

#include "./log.h"

#include <algorithm>
#include <atomic>
#include <mutex>
#include <thread>

// The bulk of this code is cribbed from:
// http://clang.llvm.org/docs/ThreadSafetyAnalysis.html

#if defined(__clang__) && (!defined(SWIG))
#define Qk_THREAD_ANNOTATION_ATTRIBUTE(x)   __attribute__((x))
#else
#define Qk_THREAD_ANNOTATION_ATTRIBUTE(x)   // no-op
#endif

#define Qk_CAPABILITY(x) \
	Qk_THREAD_ANNOTATION_ATTRIBUTE(capability(x))

#define Qk_SCOPED_CAPABILITY \
	Qk_THREAD_ANNOTATION_ATTRIBUTE(scoped_lockable)

#define Qk_GUARDED_BY(x) \
	Qk_THREAD_ANNOTATION_ATTRIBUTE(guarded_by(x))

#define Qk_PT_GUARDED_BY(x) \
	Qk_THREAD_ANNOTATION_ATTRIBUTE(pt_guarded_by(x))

#define Qk_ACQUIRED_BEFORE(...) \
	Qk_THREAD_ANNOTATION_ATTRIBUTE(acquired_before(__VA_ARGS__))

#define Qk_ACQUIRED_AFTER(...) \
	Qk_THREAD_ANNOTATION_ATTRIBUTE(acquired_after(__VA_ARGS__))

#define Qk_REQUIRES(...) \
	Qk_THREAD_ANNOTATION_ATTRIBUTE(requires_capability(__VA_ARGS__))

#define Qk_REQUIRES_SHARED(...) \
	Qk_THREAD_ANNOTATION_ATTRIBUTE(requires_shared_capability(__VA_ARGS__))

#define Qk_ACQUIRE(...) \
	Qk_THREAD_ANNOTATION_ATTRIBUTE(acquire_capability(__VA_ARGS__))

#define Qk_ACQUIRE_SHARED(...) \
	Qk_THREAD_ANNOTATION_ATTRIBUTE(acquire_shared_capability(__VA_ARGS__))

// Would be Qk_RELEASE, but that is already in use as DEBUG vs. RELEASE.
#define Qk_RELEASE_CAPABILITY(...) \
	Qk_THREAD_ANNOTATION_ATTRIBUTE(release_capability(__VA_ARGS__))

// For symmetry with Qk_RELEASE_CAPABILITY.
#define Qk_RELEASE_SHARED_CAPABILITY(...) \
	Qk_THREAD_ANNOTATION_ATTRIBUTE(release_shared_capability(__VA_ARGS__))

#define Qk_TRY_ACQUIRE(...) \
	Qk_THREAD_ANNOTATION_ATTRIBUTE(try_acquire_capability(__VA_ARGS__))

#define Qk_TRY_ACQUIRE_SHARED(...) \
	Qk_THREAD_ANNOTATION_ATTRIBUTE(try_acquire_shared_capability(__VA_ARGS__))

#define Qk_EXCLUDES(...) \
	Qk_THREAD_ANNOTATION_ATTRIBUTE(locks_excluded(__VA_ARGS__))

#define Qk_ASSERT_CAPABILITY(x) \
	Qk_THREAD_ANNOTATION_ATTRIBUTE(assert_capability(x))

#define Qk_ASSERT_SHARED_CAPABILITY(x) \
	Qk_THREAD_ANNOTATION_ATTRIBUTE(assert_shared_capability(x))

#define Qk_RETURN_CAPABILITY(x) \
	Qk_THREAD_ANNOTATION_ATTRIBUTE(lock_returned(x))

#define Qk_NO_THREAD_SAFETY_ANALYSIS \
	Qk_THREAD_ANNOTATION_ATTRIBUTE(no_thread_safety_analysis)

#if defined(Qk_BUILD_FOR_GOOGLE3) && !defined(Qk_BUILD_FOR_WASM_IN_GOOGLE3)
	extern "C" {
		void __google_potentially_blocking_region_begin(void);
		void __google_potentially_blocking_region_end  (void);
	}
	#define Qk_POTENTIALLY_BLOCKING_REGION_BEGIN __google_potentially_blocking_region_begin()
	#define Qk_POTENTIALLY_BLOCKING_REGION_END   __google_potentially_blocking_region_end()
#else
	#define Qk_POTENTIALLY_BLOCKING_REGION_BEGIN
	#define Qk_POTENTIALLY_BLOCKING_REGION_END
#endif

namespace qk {

	class Qk_EXPORT Semaphore {
	public:
		Semaphore(int count = 0);

		// Cleanup the underlying OS semaphore.
		~Semaphore();

		// Increment the counter n times.
		// Generally it's better to call signal(n) instead of signal() n times.
		void signal(int n = 1);

		// Decrement the counter by 1,
		// then if the counter is < 0, sleep this thread until the counter is >= 0.
		void wait();

		// If the counter is positive, decrement it by 1 and return true, otherwise return false.
		bool try_wait();

	private:
		// This implementation follows the general strategy of
		//     'A Lightweight Semaphore with Partial Spinning'
		// found here
		//     http://preshing.com/20150316/semaphores-are-surprisingly-versatile/
		// That article (and entire blog) are very much worth reading.
		//
		// We wrap an OS-provided semaphore with a user-space atomic counter that
		// lets us avoid interacting with the OS semaphore unless strictly required:
		// moving the count from >=0 to <0 or vice-versa, i.e. sleeping or waking threads.
		struct OSSemaphore;

		void osSignal(int n);
		void osWait();

		std::atomic<int> fCount;
		OSSemaphore*     fOSSemaphore;
	};

	inline void Semaphore::signal(int n) {
		int prev = fCount.fetch_add(n, std::memory_order_release);

		// We only want to call the OS semaphore when our logical count crosses
		// from <0 to >=0 (when we need to wake sleeping threads).
		//
		// This is easiest to think about with specific examples of prev and n.
		// If n == 5 and prev == -3, there are 3 threads sleeping and we signal
		// std::min(-(-3), 5) == 3 times on the OS semaphore, leaving the count at 2.
		//
		// If prev >= 0, no threads are waiting, std::min(-prev, n) is always <= 0,
		// so we don't call the OS semaphore, leaving the count at (prev + n).
		int toSignal = std::min(-prev, n);
		if (toSignal > 0) {
			this->osSignal(toSignal);
		}
	}

	inline void Semaphore::wait() {
		// Since this fetches the value before the subtract, zero and below means that there are no
		// resources left, so the thread needs to wait.
		if (fCount.fetch_sub(1, std::memory_order_acquire) <= 0) {
			Qk_POTENTIALLY_BLOCKING_REGION_BEGIN;
			this->osWait();
			Qk_POTENTIALLY_BLOCKING_REGION_END;
		}
	}

	class Qk_CAPABILITY("mutex") QkMutex {
	public:
		void lock() Qk_ACQUIRE() {
			fSemaphore.wait();
			Qk_DEBUGCODE(fOwner = std::this_thread::get_id();)
		}

		void unlock() Qk_RELEASE_CAPABILITY() {
			this->assertHeld();
			Qk_DEBUGCODE(fOwner = std::this_thread::get_id();)
			fSemaphore.signal();
		}

		void assertHeld() Qk_ASSERT_CAPABILITY(this) {
			Qk_DEBUGCODE(
				Qk_ASSERT(fOwner == std::this_thread::get_id())
			);
		}

	private:
		Semaphore fSemaphore{1};
		Qk_DEBUGCODE(std::thread::id fOwner;)
	};

	// There are two shared lock implementations one debug the other is high performance. They implement
	// an interface similar to pthread's rwlocks.
	// This is a shared lock implementation similar to pthreads rwlocks. The high performance
	// implementation is cribbed from Preshing's article:
	// http://preshing.com/20150316/semaphores-are-surprisingly-versatile/
	//
	// This lock does not obey strict queue ordering. It will always alternate between readers and
	// a single writer.
	class Qk_EXPORT Qk_CAPABILITY("mutex") SharedMutex {
	public:
		SharedMutex();
		~SharedMutex();
		// Acquire lock for exclusive use.
		void lock() Qk_ACQUIRE();

		// Release lock for exclusive use.
		void unlock() Qk_RELEASE_CAPABILITY();

		// Fail if exclusive is not held.
		void assertHeld() const Qk_ASSERT_CAPABILITY(this);

		// Acquire lock for shared use.
		void lockShared() Qk_ACQUIRE_SHARED();

		// Release lock for shared use.
		void unlockShared() Qk_RELEASE_SHARED_CAPABILITY();

		// Fail if shared lock not held.
		void assertHeldShared() const Qk_ASSERT_SHARED_CAPABILITY(this);

	private:
#ifdef DEBUG
		class ThreadIDSet;
		std::unique_ptr<ThreadIDSet> fCurrentShared;
		std::unique_ptr<ThreadIDSet> fWaitingExclusive;
		std::unique_ptr<ThreadIDSet> fWaitingShared;
		int fSharedQueueSelect{0};
		mutable QkMutex fMu;
		Semaphore fSharedQueue[2];
		Semaphore fExclusiveQueue;
#else
		std::atomic<int32_t> fQueueCounts;
		Semaphore            fSharedQueue;
		Semaphore            fExclusiveQueue;
#endif  // DEBUG
	};

#ifndef DEBUG
	inline void SharedMutex::assertHeld() const {};
	inline void SharedMutex::assertHeldShared() const {};
#endif  // DEBUG

	class Qk_SCOPED_CAPABILITY AutoMutexExclusive {
		Qk_DISABLE_COPY_AND_MOVE(AutoMutexExclusive);
	public:
		AutoMutexExclusive(QkMutex& mutex) Qk_ACQUIRE(mutex) : fMutex(mutex) { fMutex.lock(); }
		~AutoMutexExclusive() Qk_RELEASE_CAPABILITY() { fMutex.unlock(); }
	private:
		QkMutex& fMutex;
	};

	class Qk_SCOPED_CAPABILITY AutoSharedMutexExclusive {
		Qk_DISABLE_COPY_AND_MOVE(AutoSharedMutexExclusive);
	public:
		AutoSharedMutexExclusive(SharedMutex& lock) Qk_ACQUIRE(lock): fLock(lock) { lock.lock(); }
		~AutoSharedMutexExclusive() Qk_RELEASE_CAPABILITY() { fLock.unlock(); }
	private:
		SharedMutex& fLock;
	};

	class Qk_SCOPED_CAPABILITY AutoSharedMutexShared {
		Qk_DISABLE_COPY_AND_MOVE(AutoSharedMutexShared);
	public:
		AutoSharedMutexShared(SharedMutex& lock) Qk_ACQUIRE_SHARED(lock): fLock(lock) { lock.lockShared(); }
		// You would think this should be Qk_RELEASE_SHARED_CAPABILITY, but Qk_SCOPED_CAPABILITY
		// doesn't fully understand the difference between shared and exclusive.
		// Please review https://reviews.llvm.org/D52578 for more information.
		~AutoSharedMutexShared() Qk_RELEASE_CAPABILITY() { fLock.unlockShared(); }
	private:
		SharedMutex& fLock;
	};

} // namespace qk

#endif
