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
 * DISCLAIMED. IN NO EVENT SHALL Louis.chu BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES.
 *
 * ***** END LICENSE BLOCK ***** */

#ifndef __quark_render_vulkan_vk_mem_allocator__
#define __quark_render_vulkan_vk_mem_allocator__

#include <vulkan/vulkan.h>
#include "../../util/dict.h"
#include "../../util/list.h"
#include "../../util/thread.h"
#include <atomic>

namespace qk {
	struct VkMemoryPool;
	struct VkMemory;
	typedef List<VkMemory*> VkMemoryList;

	struct VkMemory {
		VkDeviceMemory handle = VK_NULL_HANDLE;
		uint32_t size = 0;
		void *mapped = nullptr;
		uint32_t memoryTypeIndex = 0;
		VkMemoryPropertyFlags flags = 0;
		VkMemory() = default;
		VkMemory(VkMemory&&) = default;
		VkMemory(const VkMemory&) = delete;
	private:
		bool _used = false;
		VkMemoryPool *_pool = nullptr;
		VkMemoryList::Iterator _iterator;
		friend class VkMemoryAllocator;
	};
	typedef const VkMemory cVkMemory;

	struct VkMemoryPool {
		uint32_t memoryTypeIndex;
		VkMemoryPropertyFlags flags;
		uint32_t size;
		uint32_t totalCount = 0;
		uint32_t usedCount = 0;
		uint32_t totalSize = 0;
		uint32_t usedSize = 0;
		// Used allocations stay at the front, idle allocations stay at the back.
		VkMemoryList allocations;

		VkMemoryPool(uint32_t memoryTypeIndex, VkMemoryPropertyFlags flags, uint32_t size)
			: memoryTypeIndex(memoryTypeIndex), flags(flags), size(size) {}
	};

	class VkMemoryAllocator {
	public:
		struct Stats {
			uint32_t totalSize = 0;
			uint32_t usedSize = 0;
			uint32_t totalCount = 0;
			uint32_t usedCount = 0;
			uint32_t directSize = 0;
			uint32_t directCount = 0;
		};
		VkMemoryAllocator(VkPhysicalDevice physicalDevice, VkDevice device,
			uint32_t memoryLimit = 0,
			uint32_t maxPooledSize = 4 * 1024 * 1024);
		~VkMemoryAllocator();
		cVkMemory* alloc(uint32_t size, uint32_t memoryTypeIndex,
			VkMemoryPropertyFlags flags, bool pooled = true);
		void release(cVkMemory *memory);
		void cleanup();
		inline bool cleanupRequired() const {
			return _cleanupRequired.load(std::memory_order_relaxed);
		}
		inline Stats stats() const {
			ScopeLock lock(_mutex);
			return _stats;
		}
	private:
		void destroy(VkMemory *memory);
		void cleanupIdle();
		VkDevice _device;
		VkPhysicalDeviceMemoryProperties _memoryProperties{};
		uint32_t _memoryLimit;
		uint32_t _maxPooledSize;
		mutable Mutex _mutex;
		std::atomic_bool _cleanupRequired{false};
		Stats _stats;
		Dict<uint64_t, VkMemoryPool*> _pools;
		VkMemoryList _directAllocations;
	};

} // namespace qk

#endif
