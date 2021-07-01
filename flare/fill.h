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

#ifndef __flare__core__fill__
#define __flare__core__fill__

#include "../layout/view.h"

namespace flare {

	class Box;
	class BoxFill;
	class FillColor;
	class FillImage;
	class FillGradient;
	class FillShadow;
	class FillBorder; // top,right,bottom,left
	class FillBorderRadius; // left-top,right-top,right-bottom,left-bottom

	typedef BoxFill* FillPtr;

	/**
	* @class BoxFill
	*/
	class FX_EXPORT BoxFill: public Reference {
		public:
		
		enum Type {
			M_INVALID,
			M_COLOR,
			M_IMAGE,
			M_GRADIENT,
			M_SHADOW,
		};

		enum HolderMode {
			M_INDEPENDENT,
			M_MULTIPLE,
			M_DISABLE,
		};
		
		Background();
		
		/**
		* @destructor
		*/
		virtual ~Background();
		
		/**
		* @func next()
		*/
		inline Background* next() { return _next; }
		
		/**
		* @func set_next(value)
		*/
		void set_next(Background* value);
		
		/**
		* @func type()
		*/
		virtual Type type() const { return M_INVALID; }
		
		/**
		* @func as_image()
		*/
		virtual BackgroundImage* as_image() { return nullptr; }
		
		/**
		* @func as_gradient()
		*/
		virtual BackgroundGradient* as_gradient() { return nullptr; }
		
		/**
		* @func assign(left, right)
		* @ret return left value
		*/
		static Background* assign(Background* left, Background* right);
		
		/**
		* @func allow_multi_holder()
		*/
		inline HolderMode holder_mode() const { return _holder_mode; }
		
		/**
		* @func set_holder_mode(value)
		*/
		void set_holder_mode(HolderMode mode);
		
		/**
		* @overwrite
		*/
		virtual bool retain();
		virtual void release();
		
		protected:
		
		/**
		* @func mark_value()
		*/
		void mark(uint32_t mark_value);
		
		/**
		* @func set_host(host)
		*/
		void set_host(Box* host);
		
		/**
		* @func copy(to)
		*/
		virtual BoxFill* copy(BoxFill* to) = 0;
		
		BoxFill* _next;
		Box*        _host;
		HolderMode  _holder_mode;
		FX_DEFINE_INLINE_CLASS(Inl);
	};

	/**
	 * @class FillColor
	 */
	class FX_EXPORT FillColor: public BoxFill {
		public:
		// TODO ...
		protected:
		virtual BoxFill* copy(BoxFill* to);
	};

	/**
	* @class FillImage
	*/
	class FX_EXPORT FillImage: public BoxFill {
		public:

		/**
		* @enum Repeat 纹理重复方式
		*/
		enum Repeat: uint8_t {
			NONE = value::NONE,
			REPEAT = value::REPEAT,
			REPEAT_X = value::REPEAT_X,
			REPEAT_Y = value::REPEAT_Y,
			MIRRORED_REPEAT = value::MIRRORED_REPEAT,
			MIRRORED_REPEAT_X = value::MIRRORED_REPEAT_X,
			MIRRORED_REPEAT_Y = value::MIRRORED_REPEAT_Y,
		};

		/**
		* @enum BackgroundPositionType
		*/
		enum BackgroundPositionType: uint8_t {
			PIXEL = value::PIXEL,     /* 像素值  px */
			PERCENT = value::PERCENT,   /* 百分比  % */
			LEFT = value::LEFT,      /* 居左 */
			RIGHT = value::RIGHT,     /* 居右  % */
			CENTER = value::CENTER,    /* 居中 */
			TOP = value::TOP,       /* 居上 */
			BOTTOM = value::BOTTOM,    /* 居下 */
		};
		
		/**
		* @enum BackgroundSizeType
		*/
		enum BackgroundSizeType: uint8_t {
			AUTO = value::AUTO,      /* 自动值  auto */
			PIXEL = value::PIXEL,     /* 像素值  px */
			PERCENT = value::PERCENT,   /* 百分比  % */
		};

		typedef TemplateValue<BackgroundSizeType, BackgroundSizeType::AUTO, float> BackgroundSize;
		typedef TemplateValue<BackgroundPositionType, BackgroundPositionType::PIXEL, float> BackgroundPosition;
		
		struct BackgroundSizeCollection {
			BackgroundSize x, y;
		};

		struct BackgroundPositionCollection {
			BackgroundPosition x, y;
		};

		BackgroundImage();
		virtual ~BackgroundImage();
		virtual Type type() const { return M_IMAGE; }
		virtual BackgroundImage* as_image() { return this; }
		inline Texture* texture() { return _texture; }
		inline Repeat repeat() const { return _repeat; }
		inline BackgroundPosition position_x() const { return _position_x; }
		inline BackgroundPosition position_y() const { return _position_y; }
		inline BackgroundSize size_x() const { return _size_x; }
		inline BackgroundSize size_y() const { return _size_y; }
		inline bool has_base64() const { return _has_base64_src; }
		String src() const;
		void set_src(cString& value);
		void set_src_base64(cString& data);
		void set_texture(Texture* value);
		void set_repeat(Repeat value);
		void set_position_x(BackgroundPosition value);
		void set_position_y(BackgroundPosition value);
		void set_size_x(BackgroundSize value);
		void set_size_y(BackgroundSize value);
		bool get_background_image_data(Box* host, Vec2& size_out, Vec2& position_out, int& level_out);

		protected:
		virtual Background* copy(Background* to);
		private:

		String    _src;
		bool      _has_base64_src;
		Texture*  _texture;
		Repeat    _repeat;
		BackgroundPosition _position_x, _position_y;
		BackgroundSize _size_x, _size_y;
		int _attributes_flags;
		FX_DEFINE_INLINE_CLASS(Inl);
	};

	/**
	* @class FillGradient
	*/
	class FX_EXPORT FillGradient: public BoxFill {
		public:
		FillGradient();
		virtual Type type() const { return M_GRADIENT; }
		virtual FillGradient* as_gradient() { return nullptr; }
		protected:
		virtual BoxFill* copy(BoxFill* to);
	};

	/**
	 * @class FillShadow
	 */
	class FX_EXPORT FillShadow: public BoxFill {
		public:
		// TODO ...
		protected:
		virtual BoxFill* copy(BoxFill* to);
	};

	/**
	 * @class FillBorder
	 */
	class FX_EXPORT FillBorder: public BoxFill {
		public:
		// TODO ...
		protected:
		virtual BoxFill* copy(BoxFill* to);
		float _width;
		Color _color;
	};

	/**
	 * @class FillBorderRadius
	 */
	class FX_EXPORT FillBorderRadius: public BoxFill {
		public:
		// TODO ...
		protected:
		virtual BoxFill* copy(BoxFill* to);
	};

}
#endif
