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

#include "./allocator.h"
#include "./util.h"

namespace qk {
	typedef Allocator::Ptr<void> VoidPtr;

	constexpr uint32_t kSmallTypeSize = 8;
	constexpr uint32_t kMinSmallCapacity = 4;

	/// Round up to the next power of 2
	/// @param size Input value
	/// @return The smallest power of 2 >= size
	uint32_t upPow2(uint32_t size) {
		if (size <= 1)
			return 1;
		// Qk_ASSERT_GT(size, 1, "size must be greater than 1");
		// e.g., 5 -> 8, 16 -> 16, 17 -> 32
		// return powf(2, ceilf(log2f(size)));
		// return 1u << (64 - __builtin_clzll(size - 1)); 
		return 1u << (32 - __builtin_clz(size - 1));
	}

	/**
	 * @brief Aligns a value upwards to the nearest multiple of alignment.
	 * @param ptr Original value
	 * @param alignment Alignment (power of two)
	 * @return Aligned value
	 */
	uint32_t alignUp(uint32_t ptr, uint32_t alignment) {
		// constexpr auto a = alignof(size_t) * 8 - 1;
		return (ptr + (alignment - 1)) & ~(alignment - 1);
	}

	/**
	 * @class AllocatorInl
	 * @brief Default allocator implementation using system malloc/free/realloc.
	 */
	struct AllocatorInl: public Allocator {
		void* _malloc(uint32_t size) {
			return ::malloc(size);
		}
		void* _mrealloc(void* ptr, uint32_t size) {
			return ::realloc(ptr, size);
		}
		void _free(void* ptr) {
			::free(ptr);
		}
	};

	/**
	 * @brief Constructs a default Allocator using system allocation functions.
	 */
	Allocator::Allocator()
		: Allocator((void*(Allocator::*)(uint32_t))&AllocatorInl::_malloc,
				(void* (Allocator::*)(void*, uint32_t))&AllocatorInl::_mrealloc,
				(void (Allocator::*)(void*))&AllocatorInl::_free) {}

	/**
	 * @brief Constructs an Allocator with custom allocation function pointers.
	 * @param malloc Pointer to malloc-like function
	 * @param realloc Pointer to realloc-like function
	 * @param free Pointer to free-like function
	 */
	Allocator::Allocator(void*(Allocator::*malloc)(uint32_t size),
			void* (Allocator::*mrealloc)(void* ptr, uint32_t size),
			void  (Allocator::*free)(void *ptr))
		: _malloc(malloc), _mrealloc(mrealloc), _free(free) {}

	/**
	 * @brief Extend allocated memory if size exceeds current capacity.
	 * @param self Pointer to the allocator
	 * @param ptr Pointer wrapper holding memory and capacity
	 * @param size Requested element count
	 * @param sizeOf Size of each element
	 */
	static void Allocator_extend(Allocator* self, VoidPtr* ptr, uint32_t size, uint32_t sizeOf) {
		// Capacity growth policy:
		// - For small element types (<= 8 bytes), enforce minimum capacity of 4
		//   to reduce realloc churn.
		// - For large element types, do NOT enforce minimum capacity.
		//   Typical size is often 1; extra capacity would be pure waste.
		if (sizeOf <= kSmallTypeSize && size < kMinSmallCapacity)
			size = kMinSmallCapacity; // minimum capacity for small types
		size = upPow2(size);
		ptr->val = self->mrealloc(ptr->val, sizeOf * size);
		ptr->capacity = size;
		Qk_ASSERT(ptr->val);
	}

	/**
	 * @brief Shrinks allocated memory if usage is much smaller than capacity.
	 * @param ptr Pointer wrapper holding memory and capacity
	 * @param size Requested element count
	 * @param sizeOf Size of each element
	 */
	void Allocator::shrink(VoidPtr* ptr, uint32_t size, uint32_t sizeOf) {
		uint32_t capacity = ptr->capacity;
		// Shrink if usage is less than 1/4 of current capacity
		if (size < (capacity >> 2)) {
			capacity >>= 1;
			size = upPow2(size);
			size <<= 1;
			size = Qk_Min(size, capacity);
			ptr->val = size ? mrealloc(ptr->val, sizeOf * size): (free(ptr->val), nullptr);
			ptr->capacity = size;
			Qk_ASSERT(ptr->val);
		}
	}

	/**
	 * @brief Ensures capacity >= size by extending if needed.
	 * @param ptr Pointer wrapper holding memory and capacity
	 * @param size Requested element count
	 * @param sizeOf Size of each element
	 */
	void Allocator::extend(VoidPtr* ptr, uint32_t size, uint32_t sizeOf) {
		if (size > ptr->capacity) {
			Allocator_extend(this, ptr, size, sizeOf);
		}
	}

	/**
	 * @brief Adjusts capacity to fit size. 
	 * - If size > capacity: extend
	 * - Else: shrink if much smaller
	 * @param ptr Pointer wrapper holding memory and capacity
	 * @param size Requested element count
	 * @param sizeOf Size of each element
	 */
	void Allocator::resize(VoidPtr* ptr, uint32_t size, uint32_t sizeOf) {
		if (size > ptr->capacity) {
			Allocator_extend(this, ptr, size, sizeOf);
		} else {
			shrink(ptr, size, sizeOf);
		}
	}

	Allocator Allocator::_shared;

	// ============================================================================
	// LinearAllocator Implementation
	// ============================================================================

	/**
	 * @brief Constructs a LinearAllocator with an initial block size.
	 * @param initialSize Size of the first memory block
	 */
	LinearAllocator::LinearAllocator(size_t initialSize)
		: Allocator((void*(Allocator::*)(uint32_t))&LinearAllocator::_malloc,
			(void* (Allocator::*)(void*, uint32_t))&LinearAllocator::_mrealloc,
			(void (Allocator::*)(void*))&LinearAllocator::_free)
		, _head(nullptr), _current(nullptr), _lastPtr(nullptr) {
		newBlock(initialSize);
	}

	constexpr size_t alignment = alignof(void*);

	/**
	 * @brief Destructor. Frees all allocated memory blocks.
	 */
	LinearAllocator::~LinearAllocator() {
		clear();
		::free(_head);
		_head = _current = nullptr;
	}

	/**
	 * @brief Allocate memory from the current block or new block.
	 * @param size Requested size
	 * @return Pointer to allocated memory
	 */
	void* LinearAllocator::_malloc(uint32_t size) {
		size = alignUp(size, alignment);
		size_t allSize = size + alignment; // Reserve alignment space for size header

		// If not enough space, move to or create a new block
		if (_current->offset + allSize > _current->capacity) {
			size_t newSize = _current->capacity * 2;
			if (newSize < allSize) {
				newSize = upPow2((uint32_t)allSize);
			}
			addBlock(newSize);
		}

		char* ptr = _current->data + _current->offset;
		_current->offset += allSize;
		*(size_t*)ptr = size;              // Store allocation size
		_lastPtr = ptr + alignment;        // User pointer after header
		return _lastPtr;
	}

	/**
	 * @brief Reallocate a memory block.
	 * - If ptr is null, allocate new
	 * - If shrinking, reuse
	 * - If last allocation, may grow in place
	 * - Otherwise, alloc new + memcpy
	 *
	 * @param ptr Pointer to old memory
	 * @param size New requested size
	 * @return Pointer to new memory
	 */
	void* LinearAllocator::_mrealloc(void* ptr, uint32_t size) {
		if (!ptr) {
			return _malloc(size);
		}

		size_t oldSize = *((size_t*)ptr - 1);
		if (size <= oldSize) {
			return ptr;
		}
		size = alignUp(size, alignment);

		if (ptr == _lastPtr) {
			size_t diff = size - oldSize;
			// Try in-place growth
			if (_current->offset + diff <= _current->capacity) {
				_current->offset += diff;
				*(size_t*)((char*)ptr - alignment) = size; // Update stored size
				return ptr;
			}
		}

		void* newPtr = malloc(size);
		::memcpy(newPtr, ptr, oldSize);
		return newPtr;
	}

	/**
	 * @brief Free memory (no-op).
	 * @param ptr Unused
	 */
	void LinearAllocator::_free(void* ptr) {
		// Individual free is not supported
	}

	/**
	 * @brief Soft reset allocator.
	 * - Resets first block offset
	 * - Keeps all blocks
	 */
	void LinearAllocator::reset() {
		_head->offset = 0;
		_current = _head;
		_lastPtr = nullptr;
	}

	/**
	 * @brief Clear allocator.
	 * - Frees all blocks except head
	 * - Resets head offset
	 */
	void LinearAllocator::clear() {
		auto block = _head->next;
		while (block) {
			auto next = block->next;
			::free(block);
			block = next;
		}
		_head->next = nullptr;
		_head->offset = 0;
		_current = _head;
		_lastPtr = nullptr;
	}

	/**
	 * @brief Allocate a new block and append to list.
	 * @param blockSize Size of new block
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
	 * @brief Find a block with enough space, or allocate new.
	 * @param blockSize Minimum required size
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
			::free(block);
			block = next;
		}
		newBlock(blockSize);
	}

} // namespace qk
