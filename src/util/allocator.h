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

#include "./macros.h"
#include <utility>

#ifndef __quark__util__allocator__
#define __quark__util__allocator__

namespace qk {
	struct Allocator;
	typedef const Allocator cAllocator;

	/**
	 * @class Allocator
	 * custom memory allocator
	 */
	struct Qk_EXPORT Allocator {
		template<typename T, typename A = Allocator>
		struct Ptr { // Container Pointer
			Ptr(): allocator(&A::Default), val(nullptr), capacity(0) {}
			Ptr(const A* a, T* v, uint32_t c): allocator(a), val(v), capacity(c) {}
			void free() { allocator->free(val),val = 0,capacity = 0; }
			void resize(uint32_t size) { allocator->resize(this, size); }
			void extend(uint32_t size) { allocator->extend(this, size); }
			void shrink(uint32_t size) { allocator->shrink(this, size); }
			const A *allocator;
			T       *val;
			uint32_t capacity;
		};

		static cAllocator Default; // default allocator static object
		void* (*malloc)(uint32_t size); // malloc function pointer
		void* (*realloc)(void* ptr, uint32_t size); // realloc function pointer
		void  (*free)(void *ptr); // free function pointer

		template<typename T>
		inline T* alloc(uint32_t count) const {
			return (T*)malloc(count * sizeof(T));
		}
		template<typename T, typename A>
		inline void resize(Ptr<T, A> *ptr, uint32_t size) const {
			resize((Ptr<void>*)ptr, size, sizeof(T));
		}
		template<typename T, typename A>
		inline void extend(Ptr<T, A> *ptr, uint32_t size) const {
			extend((Ptr<void>*)ptr, size, sizeof(T));
		}
		template<typename T, typename A>
		inline void shrink(Ptr<T, A> *ptr, uint32_t size) const {
			shrink((Ptr<void>*)ptr, size, sizeof(T));
		}
		void resize(Ptr<void> *ptr, uint32_t size, uint32_t sizeOf) const;
		void extend(Ptr<void> *ptr, uint32_t size, uint32_t sizeOf) const;
		void shrink(Ptr<void> *ptr, uint32_t size, uint32_t sizeOf) const;
	};

	/**
	 * @class LinearAllocator
	 * @brief A simple linear memory allocator.
	 *
	 * Features:
	 * - Fast sequential allocation with minimal overhead.
	 * - Supports soft reset to reuse allocated memory.
	 * - Supports clear to release extra memory blocks, keeping only the first block.
	 * - Suitable for temporary objects, frame allocations, parsing, or arenas.
	 */
	class Qk_EXPORT LinearAllocator {
		Qk_HIDDEN_ALL_COPY(LinearAllocator); ///< Disable copy and assignment
	public:
		/**
		 * @brief Constructs a LinearAllocator with an initial block size.
		 * @param initialSize The size of the initial memory block (default: 4096 bytes).
		 */
		explicit LinearAllocator(size_t initialSize = 4096);

		/**
		 * @brief Destructor. Frees all allocated memory blocks.
		 */
		~LinearAllocator();

		/**
		 * @brief Allocates a block of memory from the allocator.
		 * @param size Number of bytes to allocate.
		 * @param alignment Memory alignment (default: alignof(std::max_align_t)).
		 * @return Pointer to the allocated memory.
		 *
		 * Notes:
		 * - Allocation is sequential within blocks.
		 * - Individual deallocation is not supported.
		 */
		void* allocate(size_t size, size_t alignment = alignof(std::max_align_t));

		/**
		 * @brief Soft reset the allocator.
		 *
		 * - Resets the allocation offset of the first block to zero.
		 * - Keeps all existing memory blocks for reuse.
		 * - Subsequent allocations will reuse the existing memory.
		 */
		void reset();

		/**
		 * @brief Clear the allocator.
		 *
		 * - Frees all memory blocks except the first one.
		 * - Resets the offset of the first block.
		 * - Useful for completely clearing allocator memory while keeping the first block.
		 */
		void clear();

	private:
		/**
		 * @struct Block
		 * @brief Represents a memory block in the allocator.
		 *
		 * Members:
		 * - next: Pointer to the next block.
		 * - capacity: Total size of the block in bytes.
		 * - offset: Current allocation offset within the block.
		 * - data: Flexible array member storing actual data.
		 */
		struct Block {
			Block* next;      ///< Pointer to the next block
			size_t capacity;  ///< Total size of the block in bytes
			size_t offset;    ///< Current allocation offset
			char data[1];     ///< Flexible array for storing data
		};

		Block* _head;      ///< First block (head of the block list)
		Block* _current;   ///< Current block used for allocation

		/**
		 * @brief Allocate a new block and append it to the block list.
		 * @param blockSize Size of the new memory block.
		 */
		void newBlock(size_t blockSize);

		/**
		 * @brief Find or allocate a block that can fit the requested size.
		 *
		 * - If there is an existing block in the list with enough capacity, reuse it.
		 * - Otherwise, allocate a new block.
		 *
		 * @param blockSize Requested block size.
		 */
		void addBlock(size_t blockSize);
	};
} // namespace qk

#endif // __quark__util__allocator__