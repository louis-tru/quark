/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright (c) 2015, xuewen.chu
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of xuewen.chu nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL xuewen.chu BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * ***** END LICENSE BLOCK ***** */

#ifndef __noug__render__source__
#define __noug__render__source__

#include "../util/util.h"
#include "../util/string.h"
#include "../util/array.h"
#include "../util/event.h"
#include "../util/loop.h"
#include "./pixel.h"

namespace noug {

	class Application;

	/**
	* @class ImageSource
	*/
	class N_EXPORT ImageSource: public Reference {
		N_HIDDEN_ALL_COPY(ImageSource);
	public:

		enum State: int {
			STATE_NONE = 0,
			STATE_LOADING = (1 << 0),
			STATE_LOAD_ERROR = (1 << 1),
			STATE_LOAD_COMPLETE = (1 << 2),
			STATE_DECODEING = (1 << 3),
			STATE_DECODE_ERROR = (1 << 4),
			STATE_DECODE_COMPLETE = (1 << 5),
		};
		
		/**
		 * @event onState
		 */
		N_Event(State, Event<ImageSource, State>);
		
		// Defines props
		N_DEFINE_PROP_READ(String, uri);
		N_DEFINE_PROP_READ(State, state);
		N_DEFINE_PROP_READ(int, width);
		N_DEFINE_PROP_READ(int, height);
		N_DEFINE_PROP_READ(ColorType, type);

		// @constructor
		// <FlowLayout>
		// 	<Image src={app.imagePool.get('http://noug.cc/res/test.jpeg')} />
		// 	<Image src={new ImageSource('http://noug.cc/res/test2.jpeg')} />
		// 	<Image
		// 		src='http://noug.cc/res/test.jpeg'
		// 		width={100} 
		// 		height={100} margin={100} padding={100} 
		// 		fill="#f00,rgba(0,0,0,1)" class="img1" style={{width:100, height: 100}}
		// 	/>
		// </FlowLayout>
		ImageSource(cString& uri);
		ImageSource(Pixel pixel);

		/**
			* @destructor
			*/
		virtual ~ImageSource();

		/**
		 * @func load() async load image source
		 */
		bool load();

		/**
		 * @func ready() ready decode
		 */
		bool ready();

		/**
		 * @func unload() delete load and decode ready
		 */
		void unload();
		
		/**
		 *
		 * mark as gpu texture
		 *
		 * @func mark_as_texture()
		 */
		bool mark_as_texture();

		/**
		 * @func is_ready() is ready draw image
		 */
		inline bool is_ready() const { return _state & STATE_DECODE_COMPLETE; }

		/**
		 * @func size() Use memory size
		 */
		inline uint32_t size() const { return _size; };

	private:
		void _Decode();
		Pixel _memPixel;
		Buffer   _loaded;
		uint32_t _load_id, _size, _used;
		void *_inl;
		N_DEFINE_INLINE_CLASS(Inl);
	};

	/**
	* @class SourceHold
	*/
	class N_EXPORT SourceHold {
	public:
		~SourceHold();
		void set_src(cString& src);
		void set_source(ImageSource* source);
		String src() const;
		ImageSource* source();
	private:
		void handleSourceState(Event<ImageSource, ImageSource::State>& evt);
		virtual void onSourceState(Event<ImageSource, ImageSource::State>& evt);
		Handle<ImageSource> _source;
	};

	/**
	* @class ImagePool
	*/
	class N_EXPORT ImagePool: public Object {
		N_HIDDEN_ALL_COPY(ImagePool);
	public:
		
		/**
		 * @constructor
		 */
		ImagePool(Application* host);
		
		/**
		 * @destructor
		 */
		virtual ~ImagePool();

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
		N_DEFINE_INLINE_CLASS(Inl);
	};

}
#endif
