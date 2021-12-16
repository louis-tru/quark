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
#include "../util/array.h"
#include "../util/dict.h"

namespace flare {

	class         Application;
	class         PixelData;
	typedef const PixelData cPixelData;

	/** @enum ColorType
			Describes how pixel bits encode color. A pixel may be an alpha mask, a grayscale, RGB, or ARGB.
	*/
	enum ColorType: int {
		kInvalid_ColorType = 0,
		kAlpha_8_ColorType = 1,  //!< pixel with alpha in 8-bit byte
		kRGB_565_ColorType,      //!< pixel with 5 bits red, 6 bits green, 5 bits blue, in 16-bit word
		kARGB_4444_ColorType,    //!< pixel with 4 bits for alpha, red, green, blue; in 16-bit word
		kRGBA_8888_ColorType,    //!< pixel with 8 bits for red, green, blue, alpha; in 32-bit word
		kRGB_888x_ColorType,     //!< pixel with 8 bits each for red, green, blue; in 32-bit word
		kBGRA_8888_ColorType,    //!< pixel with 8 bits for blue, green, red, alpha; in 32-bit word
		kRGBA_1010102_ColorType, //!< 10 bits for red, green, blue; 2 bits for alpha; in 32-bit word
		kBGRA_1010102_ColorType, //!< 10 bits for blue, green, red; 2 bits for alpha; in 32-bit word
		kRGB_101010x_ColorType,  //!< pixel with 10 bits each for red, green, blue; in 32-bit word
		kBGR_101010x_ColorType,  //!< pixel with 10 bits each for blue, green, red; in 32-bit word
		kGray_8_ColorType,       //!< pixel with grayscale level in 8-bit byte
	};
	
	/**
	* @class PixelData
	*/
	class F_EXPORT PixelData: public Object {
	 public:

		/**
		* @func pixel_bit_size()
		*/
		static uint32_t pixel_bit_size(ColorType type);

		/**
		 *
		 * decode jpg/png/gif... image format data
		 *
		 * @func decode()
		 */
		static PixelData decode(Buffer raw);

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
		
		/**
		* @func width 图像宽度
		*/
		inline int width() const { return _width; }
		
		/**
		* @func height 图像高度
		*/
		inline int height() const { return _height; }
		
		/**
		* @func format 图像像素的排列格式
		*/
		inline ColorType type() const { return _type; }
		
	 private:
		Buffer      _data;
		int _width, _height;
		Array<WeakBuffer> _body;
		ColorType _type;
	};

	/**
	* @class ImageSource
	*/
	class F_EXPORT ImageSource: public Reference {
		F_HIDDEN_ALL_COPY(ImageSource);
	 public:

		enum State {
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
		 * 
		 * mark as gpu texture
		 *
		 * @func mark_as_texture()
		 */
		bool mark_as_texture();

		/**
		 * @func load() async load image source
		 */
		bool load();

		/**
		 * @func load_sync() sync load image source
		 */
		bool load_sync();

		/**
		 * @func ready() async ready
		 */
		bool ready();

		/**
		 * @func ready_sync() sync ready
		 */
		bool ready_sync();

		/**
		 * @func unload() delete load and ready
		 */
		void unload();

		inline String id() const { return _id }
		inline State state() const { return _state; }
		inline bool is_ready() const { return _state & STATE_DECODE_COMPLETE; }

		ColorType type() const;

		int width() const;
		int height() const;

	 private:
		String _id;
		State _state;
		PixelData _pixel;
		void *_inl;
	};

	class F_EXPORT ImagePool: public Object {
		F_HIDDEN_ALL_COPY(ImagePool);
	 public:
	 private:
	};

}

#endif