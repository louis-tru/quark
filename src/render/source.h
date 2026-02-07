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
 * DISCLAIMED. IN NO EVENT SHALL Louis.chu BE LIABLE FOR ANY
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
#include "../util/thread.h"
#include "../util/thread/mutex.h"

namespace qk {
	class RenderResource;
	struct TexStat;

	/**
	 * @class ImageSource
	 *
	 * ImageSource represents a complete image resource lifecycle in Qk:
	 *
	 *   URI / Raw Pixels
	 *        ↓
	 *   Decode to pixel buffer
	 *        ↓
	 *   Optional premultiplied alpha conversion
	 *        ↓
	 *   Optional GPU texture upload
	 *        ↓
	 *   Notify all dependent views when ready
	 *
	 * This is NOT a simple image loader.
	 * It is the bridge between:
	 *   - CPU-side pixel data
	 *   - GPU-side texture resources
	 *   - View rendering pipeline
	 *
	 * Responsibilities:
	 *  - Asynchronous image loading and decoding
	 *  - Pixel format and premultiplied alpha handling
	 *  - Optional GPU texture creation and mipmap generation
	 *  - State change notification via event system
	 *  - Safe sharing between multiple views through reference counting
	 *
	 * Threading model:
	 *  - Can be created and assigned from any thread
	 *  - Decoding and texture upload are handled internally
	 *  - State changes are broadcast through Qk_Event(State)
	 *
	 * Typical usage:
	 *  - Image views
	 *  - Box background filters
	 *  - Any renderable element requiring image data
	 *
	 * ImageSource is designed to be pooled, shared, and reused.
	 * ImageSourceHold manages atomic ownership of it inside views.
	 */
	class Qk_EXPORT ImageSource: public Reference {
		Qk_DEFINE_INLINE_CLASS(Inl);
		Qk_DISABLE_COPY(ImageSource);
	public:
		/**
		 * @enum State source load and decode state
		*/
		enum State {
			kSTATE_NONE = 0,
			kSTATE_LOADING = (1 << 0),
			kSTATE_LOAD_COMPLETE = (1 << 1),
			kSTATE_LOAD_ERROR = (1 << 2),
			kSTATE_DECODE_ERROR = (1 << 3),
		};

		/**
		 * @enum PremulFlags premultiplied alpha flags
		*/
		enum PremulFlags: uint8_t {
			kNone_PremulFlags, //!< no convert to premultiplied alpha
			kConvert_PremulFlags, //!< convert to premultiplied alpha
			kOnlyMark_PremulFlags, //!< only mark as premultiplied alpha
		};

		/**
		 * Represents loading and decoding state of the image.
		 * These flags are used to notify dependent views when the image
		 * becomes renderable or encounters errors.
		 * 
		 * @event State Fired when the state of the image source changes.
		 */
		Qk_Event(State, Event<ImageSource, State>, SharedMutex);

		// Defines props
		Qk_DEFINE_PROP_GET(String, uri, Const);
		Qk_DEFINE_PROP_GET(State, state, Const);
		Qk_DEFINE_PROP_GET(bool, premultipliedAlpha, Const); // is premultiplied alpha
		Qk_DEFINE_PROPERTY(PremulFlags, premulFlags, Const); // default as kConvert_PremulFlags

		/**
		 * Create an ImageSource from URI.
		 * Loading and decoding are performed asynchronously.
		 * The returned object may not be immediately renderable.
		 * 
		 * @example
		 * ```tsx
		 * <box>
		 * 	<image src={app.imagePool.get('http://quarks.cc/res/test.jpeg')} />
		 * 	<image src={new ImageSource('http://quarks.cc/res/test2.jpeg')} />
		 * 	<image src='http://quarks.cc/res/test.jpeg' width={100} height={100} />
		 * </box>
		 * ```
		 * 
		 * @param uri {cString&} image source URI
		 * @param loop {RunLoop* = current_loop()} run loop for async load
		 * @return {Sp<ImageSource>} image source smart pointer
		 */
		static Sp<ImageSource> Make(cString& uri, RunLoop *loop = current_loop());
		/**
		 * Create source and mark to gpu texture
		 * @param pixels {Array<Pixel>&&} pixel array move
		 * @param res {RenderResource* = nullptr} mark as texture if not null
		 * @return {Sp<ImageSource>} image source smart pointer
		*/
		static Sp<ImageSource> Make(Array<Pixel>&& pixels, RenderResource *res = nullptr);
		static Sp<ImageSource> Make(Pixel&& pixel, RenderResource *res = nullptr);

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
		 *
		 * Upload pixel data to GPU as texture.
		 * After this call, the image becomes GPU-ready for rendering.
		 * 
		 * @param res {RenderResource* = nullptr} render resource pointer, default use shared render resource
		 * @return {bool} return mark success or failure
		 */
		bool markAsTexture(RenderResource *res = nullptr);

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
		 * @method pixel(index) Returns pixel data and info
		*/
		inline cPixel* pixel(uint32_t index) const {
			return index < _pixels.length() ? &_pixels[index]: nullptr;
		}

		/**
		 * @method texture(index) get  image texture with index
		*/
		inline const TexStat* texture(uint32_t index) const {
			return index < _tex.length() ? _tex[index]: nullptr;
		}

		/**
		 * @method isMipmap() Whether generate mipmap texture
		*/
		inline bool isMipmap() const { return _isMipmap; }

		/**
		 * @method count() pixels count
		*/
		inline uint32_t count() const { return _pixels.length(); }

		/**
		 * Convert pixel array to premultiplied alpha format in-place.
		 * Required for correct blending in the Qk rendering pipeline.
		 *
		 * @param pixels {Array<Pixel>&} pixel array reference and will be modified
		 */
		static void convertToPremultipliedAlpha(Array<Pixel> &pixels);

	private:
		ImageSource(RenderResource *res, RunLoop *loop);
		void decode(Buffer& data);
		void afterDecode(Array<Pixel>& pixels, bool success);
		void unloadInl(bool destroy);
		void reloadTexture();
		static bool toPremultipliedAlpha(Pixel &pixel);

		PixelInfo    _info;
		Array<Pixel> _pixels;
		Array<const TexStat*> _tex;
		uint32_t     _loadId;
		RenderResource *_res; // weak ref, texture mark
		RunLoop       *_loop;
		bool          _isMipmap; // Whether generate mipmap texture
	};

	/**
	* @class ImageSourcePool
	*/
	class Qk_EXPORT ImageSourcePool: public Object {
		Qk_DISABLE_COPY(ImageSourcePool);
	public:
		Qk_DEFINE_PROP_GET(RunLoop*, loop);
		Qk_DEFINE_PROP_GET(uint32_t, capacity, Const); // Used memory size total

		/**
		 * @constructor
		 */
		ImageSourcePool(RunLoop *loop);
		~ImageSourcePool() override;

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

		/**
		 * @method shared() get shared image source pool
		*/
		static ImageSourcePool* shared();

	private:
		void handleSourceState(Event<ImageSource, ImageSource::State>& evt);

		struct Member {
			Sp<ImageSource> source;
			uint32_t        bytes; // image size
			int64_t         time; // load time
		};
		Dict<uint64_t, Member> _sources;
		QkMutex _Mutex;
	};

	typedef ImageSourcePool ImagePool;

	/**
	 * @class ImageSourceHold
	 *
	 * A thread-safe holder for ImageSource used by Views and renderable objects.
	 *
	 * Responsibilities:
	 *  - Manage the lifetime of ImageSource with atomic exchange
	 *  - Support setting image source from URI or directly from ImageSource
	 *  - Integrate with ImageSourcePool for caching and reuse
	 *  - Listen to ImageSource state changes (loading / ready / error)
	 *  - Provide a stable abstraction for views to consume images safely
	 *
	 * Design notes:
	 *  - ImageSource may be created, loaded, or swapped from any thread
	 *  - All assignments are performed using atomic CAS to avoid locks
	 *  - ImageSourceHold guarantees safe retain/release ordering during swaps
	 *  - Used by ImageView, Box background, and other renderable components
	 *
	 * This class does NOT perform rendering.
	 * It only manages the image source lifecycle and notifications.
	 */
	class Qk_EXPORT ImageSourceHold {
	public:
		ImageSourceHold();
		~ImageSourceHold();

		/**
		 * Get the current image source URI.
		 */
		String src() const;

		/**
		 * Set image source from a URI string.
		 *
		 * Supported forms:
		 *  - http / https / file paths (resolved via ImageSourcePool)
		 *  - data URI (base64-encoded image)
		 *
		 * Data URI handling:
		 *  - Parse mime type to determine image format
		 *  - Slice base64 payload directly from the original buffer (no extra copy)
		 *  - Decode base64 → binary → pixels
		 *  - Convert pixels to premultiplied alpha (required by Qk renderer)
		 *  - Create ImageSource from decoded pixels
		 *
		 * Non-data URIs are delegated to ImageSourcePool for caching and reuse.
		 *
		 * Failure cases:
		 *  - Invalid data URI
		 *  - Unsupported image format
		 *
		 * This method is thread-safe. ImageSource and ImageSourcePool
		 * are designed for concurrent access.
		 * 
		 * @example
		 *  img.src = 'http://quarks.cc/res/test.jpeg';
		 *  img.src = 'file:///path/to/your/image.png';
		 *  img.src = 'data:image/png;base64,iVBORw0KGgo...';
		 *  img.src = 'data:image/jpeg;base64,/9j/4AAQSkZJRgABAQAAAQ...';
		 *
		 * @param val Source URI string
		 * @return true if the source was successfully set, false on failure
		 */
		bool set_src(String val);

		/**
		 * Get the current image source.
		 */
		Sp<ImageSource> source();

		/**
		 * Set image source directly.
		 */
		bool set_source(Sp<ImageSource> val);
	private:
		void handleSourceState(Event<ImageSource, ImageSource::State>& evt);
		virtual void onSourceState(ImageSource::State evt);
		virtual ImagePool* imgPool();

		// Atomic pointer to the active ImageSource
		std::atomic<ImageSource*> _imageSource;
	};

	/**
	 * get shared image source pool
	*/
	inline ImagePool* shared_imgPool() {
		return ImageSourcePool::shared();
	}

}
#endif
