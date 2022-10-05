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

#ifndef __quark__util__stream__
#define __quark__util__stream__

#include "./array.h"

namespace quark {

	class Stream {
	public:
		virtual void pause() = 0;
		virtual void resume() = 0;
	};

	 /**
	 * @class StreamResponse
	 */
	class StreamResponse: public Object {
	public:
		inline StreamResponse(Buffer buffer, bool complete = 0
											, uint32_t id = 0, uint64_t size = 0
											, uint64_t total = 0, Stream* stream = nullptr)
		: _buffer(buffer), _complete(complete)
		, _size(size), _total(total), _id(id), _stream(stream) {
		}
		inline bool complete() const { return _complete; }
		inline int64_t size() const { return _size; }
		inline int64_t total() const { return _total; }
		inline Buffer& buffer() { return _buffer; }
		inline cBuffer& buffer() const { return _buffer; }
		inline uint32_t id() const { return _id; }
		inline Stream* stream() const { return _stream; }
		inline void pause() { if ( _stream ) _stream->pause(); }
		inline void resume() { if ( _stream ) _stream->resume(); }
	private:
		Buffer    _buffer;
		bool      _complete;
		int64_t   _size, _total;
		uint32_t  _id;
		Stream*   _stream;
	};

}
#endif
