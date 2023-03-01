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

#ifndef __quark_render_source__
#define __quark_render_source__

#include "../util/util.h"
#include "../util/string.h"
#include "../util/event.h"
#include "./pixel.h"
#include "../util/loop.h"
#include "./canvas.h"

namespace qk {
	class Application;

	/**
	* @class ImageSource
	*/
	class Qk_EXPORT ImageSource: public Reference {
		Qk_HIDDEN_ALL_COPY(ImageSource);
	public:
		enum State: int {
			kSTATE_NONE = 0,
			kSTATE_LOADING = (1 << 0),
			kSTATE_LOAD_ERROR = (1 << 1),
			kSTATE_LOAD_COMPLETE = (1 << 2),
			kSTATE_DECODEING = (1 << 3), //!< ready decodeing
			kSTATE_DECODE_ERROR = (1 << 4),
			kSTATE_DECODE_COMPLETE = (1 << 5),
		};

		/**
		 * @event onState
		 */
		Qk_Event(State, Event<ImageSource, State>);

		// Defines props
		Qk_DEFINE_PROP_GET(String, uri);
		Qk_DEFINE_PROP_GET(State, state);

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
		ImageSource(Array<Pixel>&& pixels);

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
		Sp<ImageSource> mark_as_texture();

		/**
		 * @func is_ready() is ready draw image
		 */
		inline bool is_ready() const { return _state & kSTATE_DECODE_COMPLETE; }

		/**
		 * @func pixel() Returns pixel info
		*/
		inline cPixelInfo& info() const { return _info; }

		/**
		 * @func pixel() Returns pixel data and info
		*/
		inline const Array<Pixel>& pixel() const { return _pixels; }

	private:
		void _Decode();
		void _Load();
		PixelInfo    _info;
		Array<Pixel> _pixels;
		Buffer   _loaded;
		uint32_t _load_id;
	};


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
		void handleSourceState(Event<ImageSource, ImageSource::State>& evt);

		struct Member {
			uint32_t            size;
			Handle<ImageSource> source;
		};
		Dict<uint64_t, Member> _sources;
		uint64_t _total_data_size; /* 当前数据占用memory总容量 */
		Mutex _Mutex;
		Application* _host;
	};

	typedef ImageSourcePool ImagePool;


	/**
	* @class SourceHold
	*/
	class Qk_EXPORT ImageSourceHold {
	public:
		~ImageSourceHold();
		Qk_DEFINE_PROP_ACC(String, src);
		Qk_DEFINE_PROP_ACC(ImageSource*, source, NoConst);
	private:
		void handleSourceState(Event<ImageSource, ImageSource::State>& evt);
		virtual void onSourceState(Event<ImageSource, ImageSource::State>& evt);
		Handle<ImageSource> _imageSource;
	};

}
#endif
