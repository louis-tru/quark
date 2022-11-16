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

#ifndef __quark__render__source__
#define __quark__render__source__

#include "../util/util.h"
#include "../util/string.h"
#include "../util/event.h"
#include "./pixel.h"
#include "../util/loop.h"

namespace quark {

	class Application;

	/**
	* @class ImageSource
	*/
	class Qk_EXPORT ImageSource: public Reference {
		Qk_HIDDEN_ALL_COPY(ImageSource);
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
		Qk_Event(State, Event<ImageSource, State>);
		
		// Defines props
		Qk_DEFINE_PROP_GET(String, uri);
		Qk_DEFINE_PROP_GET(State, state);
		Qk_DEFINE_PROP_GET(int, width);
		Qk_DEFINE_PROP_GET(int, height);
		Qk_DEFINE_PROP_GET(ColorType, type);

		// @constructor
		// <FlowLayout>
		// 	<Image src={app.imagePool.get('http://quarks.cc/res/test.jpeg')} />
		// 	<Image src={new ImageSource('http://quarks.cc/res/test2.jpeg')} />
		// 	<Image
		// 		src='http://quarks.cc/res/test.jpeg'
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
		Qk_DEFINE_INLINE_CLASS(Inl);
	};

}
#endif
