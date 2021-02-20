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

#ifndef __ftr__sprite__
#define __ftr__sprite__

#include "./view.h"
#include "../value.h"

/**
 * @ns ftr
 */

namespace ftr {

	using value::Repeat;

	/**
	* @class Sprite
	*/
	class FX_EXPORT Sprite: public View {
		public:
		FX_DEFINE_GUI_VIEW(SPRITE, Sprite, sprite);
		
		Sprite(Vec2 size = Vec2());

		virtual ~Sprite();
		
		/**
		* @func create # 创建一个新精灵对像
		* @arg [size = Size()] {Size}
		*/
		static inline Sprite* create(Vec2 size = Vec2()) {
			return New<Sprite>(size);
		}
		
		/**
		* @func create # 通过图像路径创建一个新精灵对像
		* @arg src {cString&}
		* @arg [size = Size()] {Size}
		*/
		static Sprite* create(cString& src, Vec2 size = Vec2());
		
		/**
		* @func create # 通过纹理对像创建一个精灵对像
		* @arg texture {Texture*}
		* @arg [size = Size()] {Size}
		*/
		static Sprite* create(Texture* texture, Vec2 size = Vec2());
		
		/**
		* @func src 路径,精灵要显示的图像路径
		*/
		String src() const;
		
		/**
		* @func texture 精灵的纹理
		*/
		inline Texture* texture() const { return _texture; }
		
		/**
		* @func start_x 精灵在图像中的开始x轴座标
		*/
		inline float start_x() const { return _start.x(); }
		
		/**
		* @func start_y 精灵在图像中的开始y轴座标
		*/
		inline float start_y() const { return _start.y(); }
		
		/**
		* @func width 精灵宽度
		*/
		inline float width() const { return _size.width(); }
		
		/**
		* @func height 精灵高度
		*/
		inline float height() const { return _size.height(); }
		
		/**
		* @func width_1()
		*/
		inline Value width_1() const { return Value(_size.width()); }
		
		/**
		* @func height_1()
		*/
		inline Value height_1() const { return Value(_size.height()); }
		
		/**
		* @func overlap_test 重叠测试,测试屏幕上的点是否与视图重叠
		*/
		virtual bool overlap_test(Vec2 point);
		
		/**
		* @func set_src
		*/
		void set_src(cString& value);
		
		/**
		* @func set_texture
		*/
		void set_texture(Texture* value);
		
		/**
		* @func set_start_x
		*/
		void set_start_x(float value);
		
		/**
		* @func set_start_y
		*/
		void set_start_y(float value);
		
		/**
		* @func set_width
		*/
		void set_width(float value);
		
		/**
		* @func set_height
		*/
		void set_height(float value);
		
		/**
		* @func set_width_1
		*/
		inline void set_width_1(Value value) { set_width(value.value); }
		
		/**
		* @func set_height_1
		*/
		inline void set_height_1(Value value) { set_height(value.value); }
		
		/**
		* @func start 精灵图片的开始位置
		*/
		inline Vec2 start() const {
			return _start;
		}
		
		/**
		* @func start 精灵图片的开始位置
		*/
		inline void start(Vec2 value) {
			set_start_x(value.x()); set_start_y(value.y());
		}
		
		/**
		* @func size 精灵视图的尺寸
		*/
		inline Vec2 size() const { return _size; }
		
		/**
		* @func size 精灵视图的尺寸
		*/
		inline void set_size(Vec2 value) {
			set_width(value.width()); set_height(value.height());
		}
		
		/**
		* @func ratio_x get
		*/
		inline float ratio_x() const { return _ratio.x(); }
		
		/**
		* @func ratio_y get
		*/
		inline float ratio_y() const { return _ratio.y(); }
		
		/**
		* @func ratio get 图像比例尺
		*/
		inline Vec2 ratio() const { return _ratio; }
		
		/**
		* @func set_ratio_x set
		*/
		void set_ratio_x(float value);
		
		/**
		* @func set_ratio_y set
		*/
		void set_ratio_y(float value);
		
		/**
		* @func ratio set
		*/
		void set_ratio(Vec2 value) {
			set_ratio_x(value.x()); set_ratio_y(value.y());
		}
		
		/**
		* @func repeat get
		*/
		inline Repeat repeat() const { return _repeat; }
		
		/**
		* @func repeat set
		*/
		void set_repeat(Repeat value);
		
		/**
		* @overwrite
		*/
		virtual CGRect screen_rect();
		
		protected:
		/**
		* @overwrite
		*/
		virtual void draw(Draw* draw);
		virtual void set_parent(View* parent) throw(Error);
		virtual void set_draw_visible();
		
		/**
		* @func compute_box_vertex
		*/
		void compute_box_vertex(Vec2 vertex[4]);

		private:
		Vec2      _start;
		Vec2      _size;
		Vec2      _ratio;
		Texture*  _texture;
		int       _tex_level;
		Repeat    _repeat;
		Vec2      _final_vertex[4];      //  最终在屏幕上显示的真实顶点位置，左上/右上/右下/左下
		
		FX_DEFINE_INLINE_CLASS(Inl);
	};

}
#endif

