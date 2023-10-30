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

namespace qk {
	class Application;
	class RenderBackend;

	/**
	 * @class ImageSource
	 */
	class Qk_EXPORT ImageSource: public Reference {
		Qk_HIDDEN_ALL_COPY(ImageSource);
	public:
		enum State: int {
			kSTATE_NONE = 0,
			kSTATE_LOADING = (1 << 0),
			kSTATE_LOAD_COMPLETE = (1 << 1),
			kSTATE_LOAD_ERROR = (1 << 2),
			kSTATE_DECODE_ERROR = (1 << 3),
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
		ImageSource(cString& uri = String());
		ImageSource(Array<Pixel>&& pixels);
		ImageSource(cPixelInfo &info, RenderBackend *render = nullptr);

		/**
		 * @destructor
		 */
		virtual ~ImageSource();

		/**
		 * @method load() async load source and decode
		 */
		bool load();

		/**
		 * @method unload() delete load and decode ready
		 */
		void unload();

		/**
		 * @method reload()
		 * 
		 * @param pixels bitmap pixels
		*/
		void reload(Array<Pixel>&& pixels);

		/**
		 *
		 * mark as gpu texture and return success or failure
		 *
		 * @method markAsTexture()
		 */
		bool markAsTexture(RenderBackend *render);

		/**
		 * @method isLoaded() is ready draw image
		 */
		inline bool isLoaded() const { return _state & kSTATE_LOAD_COMPLETE; }

		/**
		 * @method info() Returns pixel info
		*/
		inline cPixelInfo& info() const { return _info; }

		/**
		 * @method info() Returns pixel type
		*/
		inline ColorType type() const { return _info.type(); }

		/**
		 * @method info() Returns pixel bitmap width
		*/
		inline int width() const { return _info.width(); }

		/**
		 * @method info() Returns pixel bitmap height
		*/
		inline int height() const { return _info.height(); }

		/**
		 * @method pixel() Returns pixel data and info
		*/
		inline const Array<Pixel>& pixels() const { return _pixels; }

		/**
		 * @method texture() get the first image texture
		*/
		inline uint32_t texture() const { return _pixels.length() ? _pixels[0]._texture: 0; }

		/**
		 * @method isMipmap() Whether generate mipmap texture
		*/
		inline bool isMipmap() const { return _isMipmap; }

		/**
		 * @method render() as texture for render backend
		*/
		inline RenderBackend* render() const { return _render; }

	protected:
		void _SetTex(const PixelInfo &info, uint32_t texture, bool isMipmap);
	private:
		void _Decode(Buffer& data);
		void _Unload(bool isDestroy);
		PixelInfo    _info;
		Array<Pixel> _pixels;
		uint32_t     _loadId;
		RenderBackend *_render; // weak ref, texture mark
		RunLoop       *_loop;
		bool          _isMipmap; // Whether generate mipmap texture
		friend class ImageSourcePool;
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
		 * @method total_data_size() returns the data memory size total
		 */
		uint64_t total_data_size() const { return _total_data_size; }

		/**
		 * @method get(uri) get image source by uri
		 */
		ImageSource* get(cString& uri);

		/**
		 * @method load(uri) load and return image source by uri 
		 */
		ImageSource* load(cString& uri);

		/**
		 * @method remove(id) remove image source member
		 */
		void remove(cString& uri);

		/**
			* @method clear(all?: bool) clean memory
			*/
		void clear(bool all = false);

	private:
		void handleSourceState(Event<ImageSource, ImageSource::State>& evt);

		struct Member {
			Sp<ImageSource> source;
			uint32_t bytes; // image size
			int64_t time; // load time
		};
		Dict<uint64_t, Member> _sources;
		uint64_t _total_data_size; /* 当前数据占用memory总容量 */
		Mutex _Mutex;
		Application* _host;
	};

	typedef ImageSourcePool ImagePool;


	/**
	* @class ImageSourceHolder
	*/
	class Qk_EXPORT ImageSourceHolder {
	public:
		~ImageSourceHolder();
		Qk_DEFINE_PROP_ACC(String, src);
		Qk_DEFINE_PROP_ACC(ImageSource*, source, NoConst);
	private:
		void handleSourceState(Event<ImageSource, ImageSource::State>& evt);
		virtual void onSourceState(Event<ImageSource, ImageSource::State>& evt);
		Sp<ImageSource> _imageSource;
	};

}
#endif
