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

#ifndef __flare__image_source__
#define __flare__image_source__

#include "./util/util.h"
#include "./util/string.h"
#include "./util/array.h"
#include "./util/event.h"

namespace flare {

	class         Application;
	class         PixelData;
	typedef const PixelData cPixelData;

	/** @enum ColorType
			Describes how pixel bits encode color. A pixel may be an alpha mask, a grayscale, RGB, or ARGB.
	*/
	enum ColorType: int {
		COLOR_TYPE_INVALID = 0,
		COLOR_TYPE_ALPHA_8,      //!< pixel with alpha in 8-bit byte
		COLOR_TYPE_RGB_565,      //!< pixel with 5 bits red, 6 bits green, 5 bits blue, in 16-bit word
		COLOR_TYPE_ARGB_4444,    //!< pixel with 4 bits for alpha, red, green, blue; in 16-bit word
		COLOR_TYPE_RGBA_8888,    //!< pixel with 8 bits for red, green, blue, alpha; in 32-bit word
		COLOR_TYPE_RGB_888X,     //!< pixel with 8 bits each for red, green, blue; in 32-bit word
		COLOR_TYPE_BGRA_8888,    //!< pixel with 8 bits for blue, green, red, alpha; in 32-bit word
		COLOR_TYPE_RGBA_1010102, //!< 10 bits for red, green, blue; 2 bits for alpha; in 32-bit word
		COLOR_TYPE_BGRA_1010102, //!< 10 bits for blue, green, red; 2 bits for alpha; in 32-bit word
		COLOR_TYPE_RGB_101010X,  //!< pixel with 10 bits each for red, green, blue; in 32-bit word
		COLOR_TYPE_BGR_101010X,  //!< pixel with 10 bits each for blue, green, red; in 32-bit word
		COLOR_TYPE_GRAY_8,       //!< pixel with grayscale level in 8-bit byte
	};
	
	/**
	* @class PixelData
	*/
	class F_EXPORT PixelData: public Object {
	 public:

		/**
		* @func pixel_bit_size()
		*/
		static uint32_t bytes_per_pixel(ColorType type);

		/**
		 *
		 * decode jpg/png/gif... image format data
		 *
		 * @func decode()
		 */
		static PixelData decode(cBuffer& raw);

		PixelData();
		PixelData(cPixelData& data);
		PixelData(PixelData&& data);
		PixelData(ColorType type);
		PixelData(Buffer body, int width, int height, ColorType type);
		PixelData(WeakBuffer body, int width, int height, ColorType type);
		PixelData(const Array<WeakBuffer>& body, int width, int height, ColorType type);
		
		/**
		* @func body 图像数据主体
		*/
		inline cWeakBuffer& body(uint32_t index = 0) const { return _body[index]; }
		
		/**
		* @func body_count
		* */
		inline uint32_t body_count() const { return _body.length(); }
		
		F_DEFINE_PROP_READ(int, width); // width 图像宽度
		F_DEFINE_PROP_READ(int, height); // height 图像高度
		F_DEFINE_PROP_READ(ColorType, type); // format 图像像素的排列格式
		
	 private:
		Buffer      _data;
		Array<WeakBuffer> _body;
	};

	/**
	* @class ImageSource
	*/
	class F_EXPORT ImageSource: public Reference {
		F_HIDDEN_ALL_COPY(ImageSource);
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
		F_Event(State, Event<ImageSource, State>);
		
		// Defines props
		F_DEFINE_PROP_READ(String, id);
		F_DEFINE_PROP_READ(State, state);
		F_DEFINE_PROP_READ(int, width);
		F_DEFINE_PROP_READ(int, height);
		F_DEFINE_PROP_READ(ColorType, type);

		// @constructor
		// <FlowLayout>
		// 	<Image src={app.imagePool.get('http://flare.cool/res/test.jpeg')} />
		// 	<Image src={new ImageSource('http://flare.cool/res/test2.jpeg')} />
		// 	<Image
		// 		src='http://flare.cool/res/test.jpeg'
		// 		width={100} 
		// 		height={100} margin={100} padding={100} 
		// 		fill="#f00,rgba(0,0,0,1)" class="img1" style={{width:100, height: 100}}
		// 	/>
		// </FlowLayout>
		ImageSource(cString& uri);
		ImageSource(PixelData pixel);

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

	 private:
		void _Decode();
		PixelData _memPixel;
		Buffer   _loaded;
		uint32_t _load_id;
		void *_inl;
		F_DEFINE_INLINE_CLASS(Inl);
	};

	class F_EXPORT ImagePool: public Object {
		F_HIDDEN_ALL_COPY(ImagePool);
	 public:

		uint64_t total_data_size() const { return _total_data_size; }

		/**
			* @func clear
			*/
		void clear(bool full = false);
		
	 private:
		uint64_t _total_data_size; /* 当前数据占刚在总容量 */
	};

}

#endif
