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

#include "./layout/view.h"

namespace flare {

	class Box;
	class BoxFill;
	class FillColor;
	class FillImage;
	class FillGradient;
	class FillShadow;
	class FillBorder; // top,right,bottom,left
	class FillBorderRadius; // left-top,right-top,right-bottom,left-bottom
	class Texture;

	typedef BoxFill* Fill;

	/**
	* @class BoxFill
	*/
	class F_EXPORT BoxFill: public Reference {
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
		
		BoxFill();
		
		/**
		* @destructor
		*/
		virtual ~BoxFill();
		
		/**
		* @func next()
		*/
		inline BoxFill* next() { return _next; }
		
		/**
		* @func set_next(value)
		*/
		void set_next(BoxFill* value);
		
		/**
		* @func type()
		*/
		virtual Type type() const { return M_INVALID; }
		
		/**
		* @func as_image()
		*/
		virtual FillImage* as_image() { return nullptr; }
		
		/**
		* @func as_gradient()
		*/
		virtual FillGradient* as_gradient() { return nullptr; }
		
		/**
		* @func assign(left, right)
		* @ret return left value
		*/
		static BoxFill* assign(BoxFill* left, BoxFill* right);
		
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
		virtual Fill copy(Fill to) = 0;
		
		Fill        _next;
		Box*        _host;
		HolderMode  _holder_mode;
		F_DEFINE_INLINE_CLASS(Inl);
	};

	/**
	 * @class FillColor
	 */
	class F_EXPORT FillColor: public BoxFill {
	public:
		// TODO ...
		protected:
		virtual Fill copy(Fill to);
	};

	/**
	* @class FillImage
	*/
	class F_EXPORT FillImage: public BoxFill {
	public:
		FillImage();
		virtual ~FillImage();
		virtual Type type() const { return M_IMAGE; }
		virtual FillImage* as_image() { return this; }
		inline Texture* texture() { return _texture; }
		inline Repeat repeat() const { return _repeat; }
		inline FillPosition position_x() const { return _position_x; }
		inline FillPosition position_y() const { return _position_y; }
		inline FillSize size_x() const { return _size_x; }
		inline FillSize size_y() const { return _size_y; }
		inline bool has_base64() const { return _has_base64_src; }
		String src() const;
		void set_src(cString& value);
		void set_src_base64(cString& data);
		void set_texture(Texture* value);
		void set_repeat(Repeat value);
		void set_position_x(FillPosition value);
		void set_position_y(FillPosition value);
		void set_size_x(FillSize value);
		void set_size_y(FillSize value);
		bool get_background_image_data(Box* host, Vec2& size_out, Vec2& position_out, int& level_out);
	protected:
		virtual BoxFill* copy(BoxFill* to);
	private:
		String    _src;
		bool      _has_base64_src;
		Texture*  _texture;
		Repeat    _repeat;
		FillPosition _position_x, _position_y;
		FillSize _size_x, _size_y;
		int _attributes_flags;
		F_DEFINE_INLINE_CLASS(Inl);
	};

	/**
	* @class FillGradient
	*/
	class F_EXPORT FillGradient: public BoxFill {
	public:
		FillGradient();
		virtual Type type() const { return M_GRADIENT; }
		virtual FillGradient* as_gradient() { return nullptr; }
		protected:
		virtual Fill copy(Fill to);
	};

	/**
	 * @class FillShadow
	 */
	class F_EXPORT FillShadow: public BoxFill {
	public:
		// TODO ...
		protected:
		virtual Fill copy(Fill to);
	};

	/**
	 * @class FillBorder
	 */
	class F_EXPORT FillBorder: public BoxFill {
	public:
		// TODO ...
	protected:
		virtual Fill copy(Fill to);
		float _width;
		Color _color;
	};

	/**
	 * @class FillBorderRadius
	 */
	class F_EXPORT FillBorderRadius: public BoxFill {
	public:
		// TODO ...
	protected:
		virtual Fill copy(Fill to);
	};

}
#endif
