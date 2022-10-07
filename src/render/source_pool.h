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

#ifndef __quark__render__source_pool__
#define __quark__render__source_pool__

#include "./source.h"
#include "../util/array.h"

namespace quark {

	class Application;

	/**
	* @class ImageSourcePool
	*/
	class Qk_EXPORT ImageSourcePool: public Object {
		Qk_HIDDEN_ALL_COPY(ImageSourcePool);
	public:
		
		/**
		 * @constructor
		 */
		ImageSourcePool(Application* host);
		
		/**
		 * @destructor
		 */
		virtual ~ImageSourcePool();

		/**
		 * @func total_data_size() returns the data memory size total
		 */
		uint64_t total_data_size() const { return _total_data_size; }

		/**
		 * @func get(uri) get image source by uri
		 */
		ImageSource* get(cString& uri);

		/**
		 * @func remove(id) remove image source member
		 */
		void remove(cString& uri);

		/**
			* @func clear(full?: bool) clear memory
			*/
		void clear(bool full = false);
		
	private:
		struct Member {
			uint32_t size;
			Handle<ImageSource> source;
		};
		Dict<uint64_t, Member> _sources;
		uint64_t _total_data_size; /* 当前数据占用memory总容量 */
		Mutex _Mutex;
		Application* _host;
		Qk_DEFINE_INLINE_CLASS(Inl);
	};

	typedef ImageSourcePool ImagePool;

}
#endif
