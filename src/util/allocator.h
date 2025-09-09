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

	Qk_EXPORT void Fatal(const char* file, uint32_t line, const char* func, const char* msg = 0, ...);

	/**
	 * @class Allocator
	 * @brief Base class for custom memory allocators.
	 *
	 * Provides an interface similar to standard malloc/realloc/free,
	 * with support for allocator-aware smart pointers (`Ptr`).
	 */
	class Qk_EXPORT Allocator {
		Qk_DISABLE_COPY(Allocator); ///< Disable copy and assignment
	public:
		/**
		 * @struct Ptr
		 * @brief Container pointer managed by an allocator.
		 *
		 * A thin wrapper holding a pointer to memory, its allocator,
		 * and the allocated capacity.
		 *
		 * @tparam T Value type.
		 * @tparam A Allocator type (default: Allocator).
		 */
		template<typename T, typename A = Allocator>
		struct Ptr {
			/// Default constructor using the shared global allocator.
			Ptr(): allocator(Allocator::shared()), val(nullptr), capacity(0) {
				Qk_ASSERT(allocator);
			}

			/// Construct with explicit allocator, pointer, and capacity.
			Ptr(A* a, T* v, uint32_t c): allocator(a), val(v), capacity(c) {
				Qk_ASSERT(allocator);
			}

			/// Free the memory and reset pointer to null.
			void free() { allocator->free(val), val = 0, capacity = 0; }

			/// Resize the memory buffer.
			void resize(uint32_t size) { allocator->resize(this, size); }

			/// Extend the buffer by additional elements.
			void extend(uint32_t size) { allocator->extend(this, size); }

			/// Shrink the buffer capacity.
			void shrink(uint32_t size) { allocator->shrink(this, size); }

			A       *allocator; ///< Allocator instance.
			T       *val;       ///< Allocated pointer.
			uint32_t capacity;  ///< Number of elements allocated.
		};

		/**
		 * @brief Default constructor.
		 *
		 * Initializes allocator with default malloc/realloc/free
		 * function pointers.
		 */
		explicit Allocator();

		/// Allocate raw memory.
		inline void* malloc(uint32_t size) {
			return (this->*_malloc)(size);
		}

		/// Reallocate raw memory.
		inline void* mrealloc(void* ptr, uint32_t size) {
			return (this->*_mrealloc)(ptr, size);
		}

		/// Free raw memory.
		inline void free(void *ptr) {
			(this->*_free)(ptr);
		}

		/// Allocate memory for an array of @p count elements of type @p T.
		template<typename T>
		inline T* alloc(uint32_t count) {
			return (T*)malloc(count * sizeof(T));
		}

		/// Reallocate memory for an array of @p count elements of type @p T.
		template<typename T>
		inline T* realloc(T* ptr, uint32_t count) {
			return (T*)mrealloc(ptr, count * sizeof(T));
		}

		/// Resize a Ptr container to hold @p size elements.
		template<typename T, typename A>
		inline void resize(Ptr<T, A> *ptr, uint32_t size) {
			resize((Ptr<void>*)ptr, size, sizeof(T));
		}

		/// Extend a Ptr container by @p size elements.
		template<typename T, typename A>
		inline void extend(Ptr<T, A> *ptr, uint32_t size) {
			extend((Ptr<void>*)ptr, size, sizeof(T));
		}

		/// Shrink a Ptr container by @p size elements.
		template<typename T, typename A>
		inline void shrink(Ptr<T, A> *ptr, uint32_t size) {
			shrink((Ptr<void>*)ptr, size, sizeof(T));
		}

		/**
		 * @brief Resize a generic Ptr container.
		 * @param ptr Pointer wrapper to resize.
		 * @param size New number of elements.
		 * @param sizeOf Size of each element in bytes.
		 */
		void resize(Ptr<void> *ptr, uint32_t size, uint32_t sizeOf);

		/**
		 * @brief Extend a generic Ptr container.
		 * @param ptr Pointer wrapper to extend.
		 * @param size Number of elements to add.
		 * @param sizeOf Size of each element in bytes.
		 */
		void extend(Ptr<void> *ptr, uint32_t size, uint32_t sizeOf);

		/**
		 * @brief Shrink a generic Ptr container.
		 * @param ptr Pointer wrapper to shrink.
		 * @param size Number of elements to remove.
		 * @param sizeOf Size of each element in bytes.
		 */
		void shrink(Ptr<void> *ptr, uint32_t size, uint32_t sizeOf);

		/**
		 * @brief Access the default shared allocator.
		 * @return Pointer to the global Allocator instance.
		 */
		inline static Allocator* shared() { return &_shared; }

	protected:
		/// Constructor with explicit function pointers.
		Allocator(void*(Allocator::*malloc)(uint32_t size),
				void* (Allocator::*mrealloc)(void* ptr, uint32_t size),
				void  (Allocator::*free)(void *ptr));

	private:
		static Allocator _shared; ///< Default global allocator.

		void* (Allocator::*_malloc)(uint32_t size); ///< malloc function pointer
		void* (Allocator::*_mrealloc)(void* ptr, uint32_t size); ///< realloc function pointer
		void  (Allocator::*_free)(void *ptr); ///< free function pointer
	};

	/**
	 * @class LinearAllocator
	 * @brief A simple linear memory allocator.
	 *
	 * Features:
	 * - Fast sequential allocation with minimal overhead.
	 * - Supports soft reset to reuse allocated memory.
	 * - Supports clear to release extra memory blocks, keeping only the first block.
	 * - Provides limited support for reallocation of the last allocation.
	 * - Suitable for temporary objects, frame allocations, parsing, or arenas.
	 * 
	 * @code
	 * qk::LinearAllocator allocator(1024);
	 *
	 * // Allocate 256 bytes
	 * void* ptr = allocator.malloc(256);
	 *
	 * // Reallocate the same block to 512 bytes
	 * ptr = allocator.mrealloc(ptr, 512);
	 *
	 * // Soft reset (reuse blocks without freeing)
	 * allocator.reset();
	 *
	 * // Clear (free all except first block)
	 * allocator.clear();
	 * @endcode
	 */
	class Qk_EXPORT LinearAllocator: public Allocator {
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
		 * @brief Allocates a block of memory from the allocator.
		 * @param size Number of bytes to allocate.
		 * @return Pointer to the allocated memory.
		 *
		 * @note
		 * - Allocation is sequential within blocks.
		 * - Individual deallocation is not supported.
		 */
		void* _malloc(uint32_t size);

		/**
		 * @brief Reallocates a previously allocated memory block.
		 *
		 * - If @p ptr is the most recent allocation, attempts to expand in place.
		 * - Otherwise, allocates a new block and copies old data.
		 *
		 * @param ptr Pointer to the previously allocated memory.
		 * @param size New size in bytes.
		 * @return Pointer to reallocated memory (may differ from input).
		 */
		void* _mrealloc(void* ptr, uint32_t size);

		/**
		 * @brief Frees a previously allocated memory block.
		 *
		 * @param ptr Pointer to the memory to free.
		 * @note Individual deallocation is not supported;
		 *       this is a no-op unless @p ptr is the last allocated block.
		 */
		void _free(void* ptr);

		/**
		 * @struct Block
		 * @brief Represents a memory block in the allocator.
		 */
		struct Block {
			Block* next;      ///< Pointer to the next block.
			size_t capacity;  ///< Total size of the block in bytes.
			size_t offset;    ///< Current allocation offset in the block.
			char data[1];     ///< Flexible array for actual storage.
		};

		Block* _head;      ///< First block (head of the block list).
		Block* _current;   ///< Current block used for allocation.

		char *_lastPtr;    ///< Pointer to the last allocated memory (for realloc).

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
