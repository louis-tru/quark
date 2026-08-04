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

#include "./vk_mem_allocator.h"
#include "../../os/os.h"

namespace qk {

	static uint32_t vk_memory_size_class(uint32_t size, uint32_t *index) {
		uint32_t result = 1;
		*index = 0;
		while (result < size) {
			Qk_ASSERT(result <= U32::limit_max >> 1, "Vulkan memory allocation is too large");
			result <<= 1;
			(*index)++;
		}
		return result;
	}

	static uint64_t vk_memory_pool_key(uint32_t memoryTypeIndex,
		VkMemoryPropertyFlags flags, uint32_t sizeClassIndex) {
		// 24 bits are enough for Vulkan memory-property flags, and a size class
		// needs at most 32 values for a uint32_t allocation size.
		return uint64_t(memoryTypeIndex) << 40 | uint64_t(flags) << 8 | sizeClassIndex;
	}

	VkMemoryAllocator::VkMemoryAllocator(VkPhysicalDevice physicalDevice, VkDevice device,
		uint32_t memoryLimit, uint32_t maxPooledSize)
		: _device(device)
		, _memoryLimit(memoryLimit)
		, _maxPooledSize(maxPooledSize) {
		vkGetPhysicalDeviceMemoryProperties(physicalDevice, &_memoryProperties);
		if (!_memoryLimit) {
			constexpr uint64_t MB = 1024 * 1024;
			auto size = os_memory() >> 5; // 2GB:64MB, 4GB:128MB, 8GB:256MB
			_memoryLimit = uint32_t(U64::clamp(size, 64 * MB, 512 * MB));
		}
	}

	VkMemoryAllocator::~VkMemoryAllocator() {
		for (auto &entry: _pools) {
			auto pool = entry.second;
			while (!pool->allocations.isNull()) {
				auto allocation = pool->allocations.back();
				pool->allocations.popBack();
				destroy(allocation);
			}
			delete pool;
		}
		while (!_directAllocations.isNull()) {
			auto allocation = _directAllocations.back();
			_directAllocations.popBack();
			destroy(allocation);
		}
	}

	void VkMemoryAllocator::destroy(VkMemory *allocation) {
		if (allocation->mapped)
			vkUnmapMemory(_device, allocation->handle);
		if (allocation->handle)
			vkFreeMemory(_device, allocation->handle, nullptr);
		delete allocation;
	}

	cVkMemory* VkMemoryAllocator::alloc(uint32_t size, uint32_t memoryTypeIndex,
		VkMemoryPropertyFlags flags, bool pooled) {
		Qk_ASSERT(memoryTypeIndex < _memoryProperties.memoryTypeCount,
			"Invalid Vulkan memory type index");
		if (!size || memoryTypeIndex >= _memoryProperties.memoryTypeCount)
			return nullptr;
		ScopeLock lock(_mutex);

		uint32_t sizeClassIndex = 0;
		auto sizeClass = size;
		VkMemoryPool *pool = nullptr;
		if (pooled && size <= _maxPooledSize) {
			sizeClass = vk_memory_size_class(size, &sizeClassIndex);
			auto key = vk_memory_pool_key(memoryTypeIndex, flags, sizeClassIndex);
			if (!_pools.get(key, pool)) {
				pool = new VkMemoryPool(memoryTypeIndex, flags, sizeClass);
				_pools.set(key, pool);
			}
			if (!pool->allocations.isNull()) {
				// The last item is idle whenever this pool has reusable memory.
				auto it = --pool->allocations.end();
				auto allocation = *it;
				if (!allocation->_used) {
					pool->allocations.splice(
						pool->allocations.begin(), pool->allocations, it, it.next());
					allocation->_used = true;
					pool->usedCount++;
					pool->usedSize += allocation->size;
					_stats.usedCount++;
					_stats.usedSize += allocation->size;
					return allocation;
				}
			}
		}

		auto allocation = new VkMemory();
		allocation->size = pool ? sizeClass: size;
		allocation->memoryTypeIndex = memoryTypeIndex;
		allocation->flags = flags;
		allocation->_pool = pool;
		allocation->_used = true;

		VkMemoryAllocateInfo info{VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
		info.allocationSize = allocation->size;
		info.memoryTypeIndex = memoryTypeIndex;
		auto result = vkAllocateMemory(_device, &info, nullptr, &allocation->handle);
		if (result != VK_SUCCESS && _stats.totalSize > _stats.usedSize) {
			// Do not let cached idle allocations turn a recoverable allocation into OOM.
			cleanupIdle();
			result = vkAllocateMemory(_device, &info, nullptr, &allocation->handle);
		}
		if (result != VK_SUCCESS) {
			delete allocation;
			return nullptr;
		}
		if (flags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) {
			if (vkMapMemory(_device, allocation->handle, 0,
				allocation->size, 0, &allocation->mapped) != VK_SUCCESS) {
				vkFreeMemory(_device, allocation->handle, nullptr);
				delete allocation;
				return nullptr;
			}
		}

		if (pool) {
			auto it = pool->allocations.pushFront(allocation);
			allocation->_iterator = it;
			pool->totalCount++;
			pool->usedCount++;
			pool->totalSize += allocation->size;
			pool->usedSize += allocation->size;
			_stats.totalCount++;
			_stats.usedCount++;
			_stats.totalSize += allocation->size;
			_stats.usedSize += allocation->size;
			if (_stats.totalSize > _memoryLimit)
				_cleanupRequired.store(true, std::memory_order_relaxed);
		} else {
			auto it = _directAllocations.pushFront(allocation);
			allocation->_iterator = it;
			_stats.directCount++;
			_stats.directSize += allocation->size;
		}
		return allocation;
	}

	void VkMemoryAllocator::release(cVkMemory *memory) {
		if (!memory)
			return;
		ScopeLock lock(_mutex);
		auto allocation = const_cast<VkMemory*>(memory);
		Qk_ASSERT(allocation->_used, "Vulkan memory allocation is already idle");
		if (!allocation->_used)
			return;
		allocation->_used = false;

		if (allocation->_pool) {
			auto pool = allocation->_pool;
			pool->usedCount--;
			pool->usedSize -= allocation->size;
			_stats.usedCount--;
			_stats.usedSize -= allocation->size;
			// Return to the idle tail. The next allocation of this class reuses it.
			pool->allocations.splice(
				pool->allocations.end(), pool->allocations,
				allocation->_iterator, allocation->_iterator.next());
			if (_stats.totalSize > _memoryLimit)
				_cleanupRequired.store(true, std::memory_order_relaxed);
			return;
		}

		auto size = allocation->size;
		auto it = allocation->_iterator;
		destroy(allocation);
		_directAllocations.erase(it);
		_stats.directCount--;
		_stats.directSize -= size;
	}

	void VkMemoryAllocator::cleanupIdle() {
		for (auto &entry: _pools) {
			auto pool = entry.second;
			while (!pool->allocations.isNull()) {
				auto allocation = pool->allocations.back();
				if (allocation->_used)
					break;
				pool->allocations.popBack();
				pool->totalCount--;
				pool->totalSize -= allocation->size;
				_stats.totalCount--;
				_stats.totalSize -= allocation->size;
				destroy(allocation);
			}
		}
		_cleanupRequired.store(false, std::memory_order_relaxed);
	}

	void VkMemoryAllocator::cleanup() {
		if (!_cleanupRequired.load(std::memory_order_relaxed))
			return;
		ScopeLock lock(_mutex);
		// The limit is a high-water mark. Once crossed, release every idle
		// allocation from every pool; active resources are never affected.
		cleanupIdle();
	}

} // namespace qk
