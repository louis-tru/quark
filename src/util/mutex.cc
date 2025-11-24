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

#include "./mutex.h"
#include <vector>
#include <cinttypes>

#if !defined(__has_feature)
	#define __has_feature(x) 0
#endif

#if __has_feature(thread_sanitizer)

	/* Report that a lock has been created at address "lock". */
	#define ANNOTATE_RWLOCK_CREATE(lock) \
		AnnotateRWLockCreate(__FILE__, __LINE__, lock)

	/* Report that the lock at address "lock" is about to be destroyed. */
	#define ANNOTATE_RWLOCK_DESTROY(lock) \
		AnnotateRWLockDestroy(__FILE__, __LINE__, lock)

	/* Report that the lock at address "lock" has been acquired.
	   is_w=1 for writer lock, is_w=0 for reader lock. */
	#define ANNOTATE_RWLOCK_ACQUIRED(lock, is_w) \
		AnnotateRWLockAcquired(__FILE__, __LINE__, lock, is_w)

	/* Report that the lock at address "lock" is about to be released. */
	#define ANNOTATE_RWLOCK_RELEASED(lock, is_w) \
	  AnnotateRWLockReleased(__FILE__, __LINE__, lock, is_w)

	#if defined(DYNAMIC_ANNOTATIONS_WANT_ATTRIBUTE_WEAK)
		#if defined(__GNUC__)
			#define DYNAMIC_ANNOTATIONS_ATTRIBUTE_WEAK __attribute__((weak))
		#else
			/* TODO(glider): for Windows support we may want to change this macro in order
			   to prepend __declspec(selectany) to the annotations' declarations. */
			#error weak annotations are not supported for your compiler
		#endif
	#else
		#define DYNAMIC_ANNOTATIONS_ATTRIBUTE_WEAK
	#endif

	extern "C" {
	void AnnotateRWLockCreate(
		const char *file, int line,
		const volatile void *lock) DYNAMIC_ANNOTATIONS_ATTRIBUTE_WEAK;
	void AnnotateRWLockDestroy(
		const char *file, int line,
		const volatile void *lock) DYNAMIC_ANNOTATIONS_ATTRIBUTE_WEAK;
	void AnnotateRWLockAcquired(
		const char *file, int line,
		const volatile void *lock, long is_w) DYNAMIC_ANNOTATIONS_ATTRIBUTE_WEAK;
	void AnnotateRWLockReleased(
		const char *file, int line,
		const volatile void *lock, long is_w) DYNAMIC_ANNOTATIONS_ATTRIBUTE_WEAK;
	}

#else

	#define ANNOTATE_RWLOCK_CREATE(lock)
	#define ANNOTATE_RWLOCK_DESTROY(lock)
	#define ANNOTATE_RWLOCK_ACQUIRED(lock, is_w)
	#define ANNOTATE_RWLOCK_RELEASED(lock, is_w)

#endif

namespace qk {
#ifdef DEBUG

	class SharedMutex::ThreadIDSet {
	public:
		// Returns true if threadID is in the set.
		bool find(std::thread::id threadID) const {
			for (auto& t : fThreadIDs) {
				if (t == threadID) return true;
			}
			return false;
		}

		// Returns true if did not already exist.
		bool tryAdd(std::thread::id threadID) {
			for (auto& t : fThreadIDs) {
				if (t == threadID) return false;
			}
			fThreadIDs.push_back(threadID);
			return true;
		}
		// Returns true if already exists in Set.
		bool tryRemove(std::thread::id threadID) {
			for (int i = 0; i < fThreadIDs.size(); ++i) {
				if (fThreadIDs[i] == threadID) {
					fThreadIDs.erase(fThreadIDs.begin() + i);
					return true;
				}
			}
			return false;
		}

		void swap(ThreadIDSet& other) {
			fThreadIDs.swap(other.fThreadIDs);
		}

		int count() const {
			return fThreadIDs.size();
		}

	private:
		std::vector<std::thread::id> fThreadIDs;
	};

	SharedMutex::SharedMutex()
		: fCurrentShared(new ThreadIDSet)
		, fWaitingExclusive(new ThreadIDSet)
		, fWaitingShared(new ThreadIDSet){
		ANNOTATE_RWLOCK_CREATE(this);
	}

	SharedMutex::~SharedMutex() {  ANNOTATE_RWLOCK_DESTROY(this); }

	void SharedMutex::lock() {
		std::thread::id threadID(thread_self_id());
		int currentSharedCount;
		int waitingExclusiveCount;
		{
			AutoMutexExclusive l(fMu);

			Qk_ASSERT_EQ(fCurrentShared->find(threadID), false,
					  "Thread %" PRIx64 " already has an shared lock\n", threadID);

			Qk_ASSERT_EQ(true, fWaitingExclusive->tryAdd(threadID),
				"Thread %" PRIx64 " already has an exclusive lock\n", threadID);

			currentSharedCount = fCurrentShared->count();
			waitingExclusiveCount = fWaitingExclusive->count();
		}

		if (currentSharedCount > 0 || waitingExclusiveCount > 1) {
			fExclusiveQueue.wait();
		}

		ANNOTATE_RWLOCK_ACQUIRED(this, 1);
	}

	// Implementation Detail:
	// The shared threads need two separate queues to keep the threads that were added after the
	// exclusive lock separate from the threads added before.
	void SharedMutex::unlock() {
		ANNOTATE_RWLOCK_RELEASED(this, 1);
		std::thread::id threadID(thread_self_id());
		int sharedWaitingCount;
		int exclusiveWaitingCount;
		int sharedQueueSelect;
		{
			AutoMutexExclusive l(fMu);
			Qk_ASSERT_EQ(fCurrentShared->count(), 0);
			Qk_ASSERT_EQ(true, fWaitingExclusive->tryRemove(threadID),
				"Thread %" PRIx64 " did not have the lock held.\n", threadID);
			exclusiveWaitingCount = fWaitingExclusive->count();
			sharedWaitingCount = fWaitingShared->count();
			fWaitingShared.swap(fCurrentShared);
			sharedQueueSelect = fSharedQueueSelect;
			if (sharedWaitingCount > 0) {
				fSharedQueueSelect = 1 - fSharedQueueSelect;
			}
		}

		if (sharedWaitingCount > 0) {
			fSharedQueue[sharedQueueSelect].signal(sharedWaitingCount);
		} else if (exclusiveWaitingCount > 0) {
			fExclusiveQueue.signal();
		}
	}

	void SharedMutex::assertHeld() const {
		std::thread::id threadID(thread_self_id());
		AutoMutexExclusive l(fMu);
		Qk_ASSERT_EQ(fCurrentShared->count(), 0);
		Qk_ASSERT_EQ(fWaitingExclusive->find(threadID), true);
	}

	void SharedMutex::lockShared() {
		std::thread::id threadID(thread_self_id());
		int exclusiveWaitingCount;
		int sharedQueueSelect;
		{
			AutoMutexExclusive l(fMu);
			exclusiveWaitingCount = fWaitingExclusive->count();
			if (exclusiveWaitingCount > 0) {
				Qk_ASSERT_EQ(true, fWaitingShared->tryAdd(threadID),
					"Thread %" PRIx64 " was already waiting!\n", threadID);
			} else {
				Qk_ASSERT_EQ(true, fCurrentShared->tryAdd(threadID),
					"Thread %" PRIx64 " already holds a shared lock!\n", threadID);
			}
			sharedQueueSelect = fSharedQueueSelect;
		}

		if (exclusiveWaitingCount > 0) {
			fSharedQueue[sharedQueueSelect].wait();
		}

		ANNOTATE_RWLOCK_ACQUIRED(this, 0);
	}

	void SharedMutex::unlockShared() {
		ANNOTATE_RWLOCK_RELEASED(this, 0);
		std::thread::id threadID(thread_self_id());

		int currentSharedCount;
		int waitingExclusiveCount;
		{
			AutoMutexExclusive l(fMu);
			Qk_ASSERT_EQ(true, fCurrentShared->tryRemove(threadID),
				"Thread %" PRIx64 " does not hold a shared lock.\n", threadID);
			currentSharedCount = fCurrentShared->count();
			waitingExclusiveCount = fWaitingExclusive->count();
		}

		if (0 == currentSharedCount && waitingExclusiveCount > 0) {
			fExclusiveQueue.signal();
		}
	}

	void SharedMutex::assertHeldShared() const {
		std::thread::id threadID(thread_self_id());
		AutoMutexExclusive l(fMu);
		Qk_ASSERT(fCurrentShared->find(threadID));
	}

#else

	// The fQueueCounts fields holds many counts in an int32_t in order to make managing them atomic.
	// These three counts must be the same size, so each gets 10 bits. The 10 bits represent
	// the log of the count which is 1024.
	//
	// The three counts held in fQueueCounts are:
	// * Shared - the number of shared lock holders currently running.
	// * WaitingExclusive - the number of threads waiting for an exclusive lock.
	// * WaitingShared - the number of threads waiting to run while waiting for an exclusive thread
	//   to finish.
	static const int kLogThreadCount = 10;

	enum {
		kSharedOffset          = (0 * kLogThreadCount),
		kWaitingExlusiveOffset = (1 * kLogThreadCount),
		kWaitingSharedOffset   = (2 * kLogThreadCount),
		kSharedMask            = ((1 << kLogThreadCount) - 1) << kSharedOffset,
		kWaitingExclusiveMask  = ((1 << kLogThreadCount) - 1) << kWaitingExlusiveOffset,
		kWaitingSharedMask     = ((1 << kLogThreadCount) - 1) << kWaitingSharedOffset,
	};

	SharedMutex::SharedMutex() : fQueueCounts(0) { ANNOTATE_RWLOCK_CREATE(this); }
	SharedMutex::~SharedMutex() {  ANNOTATE_RWLOCK_DESTROY(this); }

	void SharedMutex::lock() {
		// Increment the count of exclusive queue waiters.
		int32_t oldQueueCounts = fQueueCounts.fetch_add(1 << kWaitingExlusiveOffset,
														std::memory_order_acquire);

		// If there are no other exclusive waiters and no shared threads are running then run
		// else wait.
		if ((oldQueueCounts & kWaitingExclusiveMask) > 0 || (oldQueueCounts & kSharedMask) > 0) {
			fExclusiveQueue.wait();
		}
		ANNOTATE_RWLOCK_ACQUIRED(this, 1);
	}

	void SharedMutex::unlock() {
		ANNOTATE_RWLOCK_RELEASED(this, 1);

		int32_t oldQueueCounts = fQueueCounts.load(std::memory_order_relaxed);
		int32_t waitingShared;
		int32_t newQueueCounts;
		do {
			newQueueCounts = oldQueueCounts;

			// Decrement exclusive waiters.
			newQueueCounts -= 1 << kWaitingExlusiveOffset;

			// The number of threads waiting to acquire a shared lock.
			waitingShared = (oldQueueCounts & kWaitingSharedMask) >> kWaitingSharedOffset;

			// If there are any move the counts of all the shared waiters to actual shared. They are
			// going to run next.
			if (waitingShared > 0) {

				// Set waiting shared to zero.
				newQueueCounts &= ~kWaitingSharedMask;

				// Because this is the exclusive release, then there are zero readers. So, the bits
				// for shared locks should be zero. Since those bits are zero, we can just |= in the
				// waitingShared count instead of clearing with an &= and then |= the count.
				newQueueCounts |= waitingShared << kSharedOffset;
			}

		} while (!fQueueCounts.compare_exchange_strong(oldQueueCounts, newQueueCounts,
													   std::memory_order_release,
													   std::memory_order_relaxed));

		if (waitingShared > 0) {
			// Run all the shared.
			fSharedQueue.signal(waitingShared);
		} else if ((newQueueCounts & kWaitingExclusiveMask) > 0) {
			// Run a single exclusive waiter.
			fExclusiveQueue.signal();
		}
	}

	void SharedMutex::lockShared() {
		int32_t oldQueueCounts = fQueueCounts.load(std::memory_order_relaxed);
		int32_t newQueueCounts;
		do {
			newQueueCounts = oldQueueCounts;
			// If there are waiting exclusives then this shared lock waits else it runs.
			if ((newQueueCounts & kWaitingExclusiveMask) > 0) {
				newQueueCounts += 1 << kWaitingSharedOffset;
			} else {
				newQueueCounts += 1 << kSharedOffset;
			}
		} while (!fQueueCounts.compare_exchange_strong(oldQueueCounts, newQueueCounts,
														std::memory_order_acquire,
														std::memory_order_relaxed));

		// If there are waiting exclusives, then this shared waits until after it runs.
		if ((newQueueCounts & kWaitingExclusiveMask) > 0) {
			fSharedQueue.wait();
		}
		ANNOTATE_RWLOCK_ACQUIRED(this, 0);
	}

	void SharedMutex::unlockShared() {
		ANNOTATE_RWLOCK_RELEASED(this, 0);

		// Decrement the shared count.
		int32_t oldQueueCounts = fQueueCounts.fetch_sub(1 << kSharedOffset,
														std::memory_order_release);

		// If shared count is going to zero (because the old count == 1) and there are exclusive
		// waiters, then run a single exclusive waiter.
		if (((oldQueueCounts & kSharedMask) >> kSharedOffset) == 1
			&& (oldQueueCounts & kWaitingExclusiveMask) > 0) {
			fExclusiveQueue.signal();
		}
	}
#endif
} // namespace qk

extern "C" {

	using qk::SharedMutex;

	qk_rwlock_t qk_rwlock_create() {
		return reinterpret_cast<qk_rwlock_t>(new SharedMutex());
	}

	void qk_rwlock_destroy(qk_rwlock_t lock) {
		delete reinterpret_cast<SharedMutex*>(lock);
	}

	void qk_rwlock_rdlock(qk_rwlock_t lock) {
		reinterpret_cast<SharedMutex*>(lock)->lockShared();
	}

	void qk_rwlock_wrlock(qk_rwlock_t lock) {
		reinterpret_cast<SharedMutex*>(lock)->lock();
	}

	void qk_rwlock_unlock_rd(qk_rwlock_t lock) {
		reinterpret_cast<SharedMutex*>(lock)->unlockShared();
	}

	void qk_rwlock_unlock_wr(qk_rwlock_t lock) {
		reinterpret_cast<SharedMutex*>(lock)->unlock();
	}
}
