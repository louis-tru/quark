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

// @private head

#ifndef __quark__util__ssl__
#define __quark__util__ssl__

#include "./util.h"
#include <openssl/ssl.h>

namespace qk {

	class BIOData {
	public:
		BIOData() //: env_(nullptr),
							:	initial_(kInitialBufferLength),
								length_(0),
								read_head_(nullptr),
								write_head_(nullptr) {
		}

		~BIOData();

		static BIO* New();

		// NewFixed takes a copy of `len` bytes from `data` and returns a BIO that,
		// when read from, returns those bytes followed by EOF.
		static BIO* NewFixed(const char* data, size_t len);

		// void AssignEnvironment(Environment* env);

		// Move read head to next buffer if needed
		void TryMoveReadHead();

		// Allocate new buffer for write if needed
		void TryAllocateForWrite(size_t hint);

		// Read `len` bytes maximum into `out`, return actual number of read bytes
		size_t Read(char* out, size_t size);

		// Memory optimization:
		// Deallocate children of write head's child if they're empty
		void FreeEmpty();

		// Return pointer to internal data and amount of
		// contiguous data available to read
		char* Peek(size_t* size);

		// Return pointers and sizes of multiple internal data chunks available for
		// reading
		size_t PeekMultiple(char** out, size_t* size, size_t* count);

		// Find first appearance of `delim` in buffer or `limit` if `delim`
		// wasn't found.
		size_t IndexOf(char delim, size_t limit);

		// Discard all available data
		void Reset();

		// Put `len` bytes from `data` into buffer
		void Write(const char* data, size_t size);

		// Return pointer to internal data and amount of
		// contiguous data available for future writes
		char* PeekWritable(size_t* size);

		// Commit reserved data
		void Commit(size_t size);

		// Return size of buffer in bytes
		inline size_t Length() const {
			return length_;
		}

		inline void set_initial(size_t initial) {
			initial_ = initial;
		}

		static inline BIOData* FromBIO(BIO* bio) {
			Qk_Assert_Ne(bio->ptr, nullptr);
			return static_cast<BIOData*>(bio->ptr);
		}

	private:
		static int New(BIO* bio);
		static int Free(BIO* bio);
		static int Read(BIO* bio, char* out, int len);
		static int Write(BIO* bio, const char* data, int len);
		static int Puts(BIO* bio, const char* str);
		static int Gets(BIO* bio, char* out, int size);
		static long Ctrl(BIO* bio, int cmd, long num,  // NOLINT(runtime/int)
										void* ptr);

		// Enough to handle the most of the client hellos
		static const size_t kInitialBufferLength = 1024;
		static const size_t kThroughputBufferLength = 16384;

		static const BIO_METHOD method;

		class Buffer {
		public:
			Buffer(size_t len) :read_pos_(0),
				write_pos_(0),
				len_(len),
				next_(nullptr) 
			{
				data_ = new char[len];
				// v8->isolate()->AdjustAmountOfExternalAllocatedMemory(len);
			}

			~Buffer() {
				delete[] data_;
				// 	const int64_t len = static_cast<int64_t>(len_);
				// 	v8->isolate()->AdjustAmountOfExternalAllocatedMemory(-len);
			}
			size_t read_pos_;
			size_t write_pos_;
			size_t len_;
			Buffer* next_;
			char* data_;
		};

		size_t initial_;
		size_t length_;
		Buffer* read_head_;
		Buffer* write_head_;
	};

}
#endif
