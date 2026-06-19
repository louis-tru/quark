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

#include "../util/util.h"

#ifndef __quark_render_mem_allocator__
#define __quark_render_mem_allocator__

namespace qk {
	uint32_t alignUp(uint32_t ptr, uint32_t alignment = alignof(void*));
	uint32_t upPow2(uint32_t size);

	constexpr bool isAlignUp(uint32_t ptr) {
		constexpr auto alignment = alignof(void*);
		return ((ptr + (alignment - 1)) & ~(alignment - 1)) == ptr;
	}

	/**
	 * @class MemBlockAllocator - A simple memory allocator for GPU buffers, managing memory in blocks.
	 * Allocates memory in fixed-size blocks and provides an interface for allocating memory within those blocks.
	*/
	template<class T> struct MemBlockAllocator {
		struct MemBlock {
			MemBlock(T val, uint32_t capacity)
				: val(val), capacity(capacity), _next(nullptr) {}
			T val;
			uint32_t begin = 0, end = 0, capacity;
		private:
			MemBlock *_next;
			friend class MemBlockAllocator;
		};
		MemBlockAllocator(uint32_t initialSize = 4096): _blocks(1), _capacity(initialSize) {
			Qk_ASSERT(isAlignUp(initialSize), "Block capacity must be aligned.");
			_head = createBlock(initialSize);
			_current = _head;
		}
		~MemBlockAllocator() {
			auto block = _head;
			do {
				block = deleteBlock_(block);
			} while(block);
		}
		void reset() {
			_current = _head;
			_current->begin = _current->end = 0;
		}
		// release all blocks except the first one, and reset the first block for reuse.
		void clear() {
			auto block = _head->_next;
			while (block)
				block = deleteBlock_(block);
			_current = _head;
			_current->_next = nullptr;
			_current->begin = _current->end = 0;
		}
		const MemBlock& alloc(uint32_t size, uint32_t reserve = 0, uint32_t alignment = 16) {
			if (reserve == 0)
				reserve = size;
			Qk_ASSERT(reserve >= size, "Reserve size must be greater than requested size.");
			size = alignUp(size, alignment);
			auto block = _current;
			auto end = alignUp(block->end, alignment), newEnd = end + size;
			if (end + reserve > block->capacity) {
				block = block->_next;
				while (block) {
					if (block->capacity >= reserve)
						break;
					// delete blocks that are too small to satisfy the reserve
					block = deleteBlock_(block);
				}
				if (!block) {
					uint32_t capacity = std::max(_current->capacity << 1, reserve);
					capacity = upPow2(capacity); // ensure new block can fit reserve
					block = createBlock(capacity);
					_blocks++;
					_capacity += block->capacity;
				}
				_current->_next = block;
				_current = block;
				end = 0; // reset new block
				newEnd = size;
			}
			block->begin = end;
			block->end = newEnd;
			return *block;
		}
		Qk_DISABLE_COPY(MemBlockAllocator);
		MemBlock* createBlock(uint32_t capacity);
		void deleteBlock(MemBlock *block);
		// delete block return next block, or nullptr if no next block
		MemBlock* deleteBlock_(MemBlock *block) {
			_blocks--;
			_capacity -= block->capacity;
			auto next = block->_next;
			return deleteBlock(block), next;
		}
		MemBlock *_head;
		MemBlock *_current; // current block for alloc
		uint32_t _blocks;  ///< Total number of blocks allocated.
		uint32_t _capacity; ///< Total capacity across all blocks in bytes.
	};
}
#endif
