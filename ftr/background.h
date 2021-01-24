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

#ifndef __ftr__background__
#define __ftr__background__

#include "ftr/box.h"

namespace ftr {

	class BackgroundImage;
	class BackgroundGradient;

	/**
	* @class Background
	*/
	class FX_EXPORT Background: public Reference {
		public:
		
		enum Type {
			M_INVALID,
			M_IMAGE,
			M_GRADIENT,
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
		void mark(uint mark_value);
		
		/**
		* @func set_host(host)
		*/
		void set_host(Box* host);
		
		/**
		* @func copy(to)
		*/
		virtual Background* copy(Background* to) = 0;
		
		Background* _next;
		Box*        _host;
		HolderMode  _holder_mode;
		FX_DEFINE_INLINE_CLASS(Inl);
		friend class Box;
		friend class GLDraw;
	};

	typedef Background* BackgroundPtr;

	/**
	* @class BackgroundImage
	*/
	class FX_EXPORT BackgroundImage: public Background {
		public:
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
		BackgroundPosition  _position_x;
		BackgroundPosition  _position_y;
		BackgroundSize      _size_x;
		BackgroundSize      _size_y;
		int _attributes_flags;
		FX_DEFINE_INLINE_CLASS(Inl);
		friend class Box;
		friend class GLDraw;
	};

	/**
	* @class BackgroundGradient
	*/
	class FX_EXPORT BackgroundGradient: public Background {
		public:
		BackgroundGradient();
		virtual Type type() const { return M_GRADIENT; }
		virtual BackgroundGradient* as_gradient() { return nullptr; }
		protected:
		virtual Background* copy(Background* to);
		friend class GLDraw;
	};

}
#endif
