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

#include "./allocator.h"
#include "./util.h"

#ifndef Qk_Min_CAPACITY
# define Qk_Min_CAPACITY (4)
#endif

namespace qk {
	typedef Allocator::Ptr<void> VoidPtr;

	uint32_t upPow2(uint32_t size) {
		// Round up to next power of 2
		// return powf(2, ceilf(log2f(size)));
		return 1u << (64 - __builtin_clzll(size-1)); // 向上取最近的2的幂
	}

	static void* Allocator_malloc(uint32_t size) {
		return ::malloc(size);
	}

	static void* Allocator_realloc(void *ptr, uint32_t size) {
		return ::realloc(ptr, size);
	}

	static void Allocator_free(void* ptr) {
		::free(ptr);
	}

	static void Allocator_extend(cAllocator *self, VoidPtr *ptr, uint32_t size, uint32_t sizeOf) {
		size = Qk_Max(Qk_Min_CAPACITY, size);
		size = upPow2(size);
		ptr->val = self->realloc(ptr->val, sizeOf * size);
		ptr->capacity = size;
		Qk_ASSERT(ptr->val);
	}

	void Allocator::shrink(VoidPtr *ptr, uint32_t size, uint32_t sizeOf) const {
		uint32_t capacity = ptr->capacity;
		if ( size > Qk_Min_CAPACITY && size < (capacity >> 2) ) { // if size < 1/4
			capacity >>= 1;
			size = upPow2(size);
			size <<= 1;
			size = Qk_Min(size, capacity);
			ptr->val = realloc(ptr->val, sizeOf * size);
			ptr->capacity = size;
			Qk_ASSERT(ptr->val);
		}
	}

	void Allocator::extend(VoidPtr *ptr, uint32_t size, uint32_t sizeOf) const {
		if (size > ptr->capacity) {
			Allocator_extend(this, ptr, size, sizeOf);
		}
	}

	void Allocator::resize(VoidPtr *ptr, uint32_t size, uint32_t sizeOf) const {
		if (size > ptr->capacity) {
			Allocator_extend(this, ptr, size, sizeOf);
		} else {
			shrink(ptr, size, sizeOf);
		}
	}

	const Allocator Allocator::Default{
		&Allocator_malloc,
		&Allocator_realloc,
		&Allocator_free,
	};

	/**
	 * @brief Constructs a LinearAllocator with an initial block size.
	 * @param initialSize Size of the first memory block.
	 */
	LinearAllocator::LinearAllocator(size_t initialSize)
			: _head(nullptr), _current(nullptr) {
		newBlock(initialSize);
	}

	/**
	 * @brief Destructor. Frees all allocated memory blocks.
	 */
	LinearAllocator::~LinearAllocator() {
		clear();
		::free(_head);
		_head = _current = nullptr;
	}

	/**
	 * @brief Aligns a pointer/address up to the given alignment.
	 * @param ptr The original address/offset.
	 * @param alignment Alignment value (power of two).
	 * @return The aligned address/offset.
	 */
	inline size_t alignUp(size_t ptr, size_t alignment) {
		return (ptr + (alignment - 1)) & ~(alignment - 1);
	}

	/**
	 * @brief Allocate memory from the allocator.
	 * @param size Number of bytes to allocate.
	 * @param alignment Memory alignment (default: max alignment).
	 * @return Pointer to allocated memory.
	 *
	 * Notes:
	 * - Allocation is sequential within blocks.
	 * - If the current block does not have enough space, will search next blocks.
	 * - If no existing block fits, a new block is allocated.
	 */
	void* LinearAllocator::allocate(size_t size, size_t alignment) {
		// Align the current offset within the current block
		size_t alignedOffset = alignUp(_current->offset, alignment);

		// If the current block cannot fit, find or allocate a new block
		if (alignedOffset + size > _current->capacity) {
				// Determine new block size (double the current or at least next power of two)
				size_t newSize = _current->capacity * 2;
				if (newSize < size) {
					// Round up to next power of 2
					newSize = upPow2(size);
				}
				addBlock(newSize);
				alignedOffset = 0; // Reset offset for the new/current block
		}

		void* ptr = _current->data + alignedOffset;
		_current->offset = alignedOffset + size; // Update offset
		return ptr;
	}

	/**
	 * @brief Soft reset the allocator.
	 *
	 * - Resets the first block's offset to zero.
	 * - Keeps all allocated blocks for reuse.
	 * - Subsequent allocations will reuse existing memory.
	 */
	void LinearAllocator::reset() {
		if (_head) {
			_head->offset = 0;
			_current = _head;
		}
	}

	/**
	 * @brief Clear the allocator.
	 *
	 * - Frees all blocks except the head.
	 * - Resets the head block offset.
	 */
	void LinearAllocator::clear() {
		if (!_head) return;

		Block* block = _head->next;
		while (block) {
			Block* next = block->next;
			::free(block);
			block = next;
		}
		_head->next = nullptr;
		_head->offset = 0;
		_current = _head;
	}

	/**
	 * @brief Allocate a new block and append to the linked list.
	 * @param blockSize Size of the new memory block.
	 */
	void LinearAllocator::newBlock(size_t blockSize) {
		size_t totalSize = sizeof(Block) - 1 + blockSize;
		Block* block = (Block*)::malloc(totalSize);
		Qk_ASSERT(block);

		block->next = nullptr;
		block->capacity = blockSize;
		block->offset = 0;

		if (!_head) {
			_head = _current = block;
		} else {
			_current->next = block;
			_current = block;
		}
	}

	/**
	 * @brief Find or allocate a block that can fit the requested size.
	 *
	 * - If there is a next block with enough capacity, reuse it.
	 * - Otherwise, free old insufficient blocks and allocate a new block.
	 *
	 * @param blockSize Requested block size.
	 */
	void LinearAllocator::addBlock(size_t blockSize) {
		auto block = _current->next;
		while (block) {
			if (block->capacity >= blockSize) {
				block->offset = 0;
				_current->next = block;
				_current = block;
				return;
			}
			auto next = block->next;
			::free(block); // Free old block that cannot fit the request
			block = next;
		}
		newBlock(blockSize);
	}

} // namespace qk
