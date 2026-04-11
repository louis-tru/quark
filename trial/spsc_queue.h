/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright (c) 2015, Louis.chu
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of Louis.chu nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL Louis.chu BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * ***** END LICENSE BLOCK ***** */

// @private head

#ifndef __quark__spsc_queue__
#define __quark__spsc_queue__

#include "../src/util/util.h"

namespace qk {

	uint32_t upPow2(uint32_t size);

	/**
	 * @class SPSCQueue
	 * @brief A single-producer, single-consumer lock-free queue.
	 *
	 * This is a bounded queue implemented as a circular buffer.
	 * It supports one producer thread and one consumer thread without locks.
	 *
	 * Template parameter:
	 *   - T: The type of elements stored in the queue. Must be copyable/movable.
	 */
	template<typename T>
	class SPSCQueue {
		Qk_DISABLE_COPY(SPSCQueue);
	public:
		explicit SPSCQueue(uint32_t capacity = 128)
			: _head(0), _tail(0)
			, _capacity(upPow2(Qk_Min(2, capacity)))
			, _mask(_capacity - 1)
			, _val(new T[_capacity]) {
		}

		~SPSCQueue() {
			delete[] _val;
		}

		uint32_t capacity() const { return _capacity; }

		bool empty() const {
			return _head.load(std::memory_order_acquire) ==
						 _tail.load(std::memory_order_acquire);
		}

		uint32_t size() const {
			uint32_t head = _head.load(std::memory_order_acquire);
			uint32_t tail = _tail.load(std::memory_order_acquire);
			return (tail - head) & _mask;
		}

		/**
		 * Push one item.
		 * Return false if queue is full.
		 */
		bool push(const T& value) {
			uint32_t tail = _tail.load(std::memory_order_relaxed);
			uint32_t next = (tail + 1) & _mask;

			if (next == _head.load(std::memory_order_acquire))
				return false; // full

			_val[tail] = value;
			_tail.store(next, std::memory_order_release);
			return true;
		}

		/**
		 * Pop one item.
		 * Return false if queue is empty.
		 */
		bool pop(T& out) {
			uint32_t head = _head.load(std::memory_order_relaxed);

			if (head == _tail.load(std::memory_order_acquire))
				return false; // empty

			out = std::move(_val[head]);
			_head.store((head + 1) & _mask, std::memory_order_release);
			return true;
		}

		/**
		 * Extend queue capacity.
		 * This must be called only when producer/consumer are externally synchronized.
		 *
		 * Typical usage:
		 *   if (!queue.push(v)) {
		 *       lock rt / stop producer-consumer briefly;
		 *       queue.extend(queue.capacity() * 2);
		 *       bool ok = queue.push(v);
		 *       ...
		 *   }
		 */
		void extend(uint32_t capacity) {
			capacity = upPow2(capacity);
			if (capacity <= _capacity)
				return;

			uint32_t head = _head.load(std::memory_order_acquire);
			uint32_t tail = _tail.load(std::memory_order_acquire);
			uint32_t len  = (tail - head) & _mask;

			T* newVal = new T[capacity];
			uint32_t newMask = capacity - 1;

			// linearize old ring into new buffer [0..len)
			for (uint32_t i = 0; i < len; i++) {
				newVal[i] = std::move(_val[(head + i) & _mask]);
			}

			delete[] _val;
			_val = newVal;
			_capacity = capacity;
			_mask = newMask;

			_head.store(0, std::memory_order_release);
			_tail.store(len, std::memory_order_release);
		}

	private:
		alignas(64) std::atomic<uint32_t> _head; // consumer
		alignas(64) std::atomic<uint32_t> _tail; // producer
		uint32_t _capacity;
		uint32_t _mask;
		T* _val;
	};

}

#endif /* defined(__quark__spsc_queue__) */