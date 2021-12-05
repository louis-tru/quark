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

#ifndef __flare__fill__
#define __flare__fill__

#include "./value.h"

namespace flare {

	class Box;
	class FillBox;
	class FillColor;
	class FillImage;
	class FillGradient;
	class FillShadow;
	class FillBorder; // top,right,bottom,left
	class FillBorderRadius; // left-top,right-top,right-bottom,left-bottom
	class Texture;
	class Canvas;

	typedef FillBox* Fill;

	/**
	* @class FillBox, Single linked list struct
	*/
	class F_EXPORT FillBox: public Reference {
	 public:
		
		enum Type {
			M_INVALID,
			M_COLOR,
			M_IMAGE,
			M_GRADIENT,
			M_SHADOW,
			M_BORDER,
			M_BORDER_RADIUS,
		};

		enum HolderMode {
			M_INDEPENDENT,
			M_SHARED,
			M_DISABLE,
		};
		
		/**
		 * @constructor
		 */
		FillBox();
		
		/**
		* @destructor
		*/
		virtual ~FillBox();
		
		/**
		* @func next()
		*/
		inline Fill next() { return _next; }
		
		/**
		* @func set_next(value)
		*/
		Fill set_next(Fill value);
		
		/**
		* @func type()
		*/
		virtual Type type() const;
		
		/**
		* @func hold(left, right)
		* @ret return left value
		*/
		static Fill assign(Fill left, Fill right);
		
		/**
		* @func allow_multi_holder()
		*/
		inline HolderMode holder_mode() const { return _holder_mode; }
		
		/**
		* @func set_holder_mode(value)
		*/
		Fill set_holder_mode(HolderMode mode);
		
		/**
		 * @override
		 */
		virtual bool retain() override;

		/**
		* @func copy(to)
		*/
		virtual Fill copy(Fill to) = 0;

		/**
		 * 
		 * draw fill background
		 *
		 * @func draw(host, canvas)
		 */
		virtual void draw(Box* host, Canvas* canvas, uint8_t alpha, FillBorderRadius* radius = nullptr) = 0;
		
	 protected:
		/**
		* @func mark()
		*/
		void mark();

		Fill        _next;
		HolderMode  _holder_mode;

		F_DEFINE_INLINE_CLASS(Inl);
	};

	/**
	 * @class FillColor
	 */
	class F_EXPORT FillColor: public FillBox {
	 public:
		FillColor(Color color = Color());
		F_DEFINE_PROP(Color, color);
		virtual Type type() const override;
		virtual Fill copy(Fill to) override;
		virtual void draw(Box* host, Canvas* canvas, uint8_t alpha, FillBorderRadius* radius) override;
		// @consts
		static Fill WHITE;
		static Fill BLACK;
		static Fill BLUE;
	};

	/**
	* @class FillImage
	*/
	class F_EXPORT FillImage: public FillBox {
	 public:
		FillImage(cString& src = String());
		virtual ~FillImage();
		virtual Type type() const override;
		F_DEFINE_PROP(String, src);
		F_DEFINE_PROP(Texture*, texture);
		F_DEFINE_PROP(Repeat, repeat);
		F_DEFINE_PROP(FillPosition, position_x);
		F_DEFINE_PROP(FillPosition, position_y);
		F_DEFINE_PROP(FillSize, size_x);
		F_DEFINE_PROP(FillSize, size_y);
		F_DEFINE_PROP_READ(bool, has_base64);
		void set_src_base64(cString& data);
		bool get_image_data(Box* host, Vec2& size_out, Vec2& position_out, int& level_out);
		virtual Fill copy(Fill to) override;
		virtual void draw(Box* host, Canvas* canvas, uint8_t alpha, FillBorderRadius* radius) override;
	 private:
		int _attributes_flags;
		F_DEFINE_INLINE_CLASS(Inl);
	};

	/**
	* @class FillGradient
	*/
	class F_EXPORT FillGradient: public FillBox {
	 public:
		FillGradient();
		virtual Type type() const override;
		virtual Fill copy(Fill to) override;
		virtual void draw(Box* host, Canvas* canvas, uint8_t alpha, FillBorderRadius* radius) override;
	 private:
	};

	/**
	 * @class FillShadow
	 */
	class F_EXPORT FillShadow: public FillBox {
	 public:
		virtual Type type() const override;
		virtual Fill copy(Fill to) override;
		virtual void draw(Box* host, Canvas* canvas, uint8_t alpha, FillBorderRadius* radius) override;
	 private:
	};

	/**
	 * @class FillBorder
	 */
	class F_EXPORT FillBorder: public FillBox {
	 public:
		enum Style {
			dashed,
			dotted,
			Double,
			groove,
			hidden,
			inherit,
			initial,
			inset,
			outset,
			none,
			revert,
			ridge,
			solid,
			unset,
		};
		virtual Type type() const override;
		virtual Fill copy(Fill to) override;
		virtual void draw(Box* host, Canvas* canvas, uint8_t alpha, FillBorderRadius* radius) override;
	 protected:
		Color _color[4];
		float _width[4];
		Style _style[4];
	};

	/**
	 * @class FillBorderRadius
	 */
	class F_EXPORT FillBorderRadius: public FillBox {
	 public:
		F_DEFINE_PROP(float, left_top);
		F_DEFINE_PROP(float, right_top);
		F_DEFINE_PROP(float, right_bottom);
		F_DEFINE_PROP(float, left_bottom);
		virtual Type type() const override;
		virtual Fill copy(Fill to) override;
		virtual void draw(Box* host, Canvas* canvas, uint8_t alpha, FillBorderRadius* radius) override;
	};

}
#endif
