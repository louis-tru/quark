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

#ifndef __ftr__view__view__
#define __ftr__view__view__

#include "../util/object.h"
#include "../value.h"

namespace ftr {

	class Action;

	/**
	 * The basic elements of GUI tree
	 *
	 * @class View
	 */
	class FX_EXPORT View: public Reference {
		FX_HIDDEN_ALL_COPY(View);
		public:

		/**
		 * @constructors
		 */
		View();

		/**
		 * @destructor
		 */
		virtual ~View();

		/**
		 * parent view
		 *
		 * @func parent()
		 */
		inline View* parent() const {
			return _parent;
		}

		/**
		 * first subview
		 *
		 * @func first()
		 */
		inline View* first() const {
			return _first;
		}

		/**
		 * last subview
		 *
		 * @func last()
		 */
		inline View* last() const {
			return _last;
		}

		/**
		 * Previous sibling view
		 *
		 * @func prev()
		 */
		inline View* prev() const {
			return _prev;
		}

		/**
		 * Next sibling view
		 *
		 * @func nect()
		 */
		inline View* next() {
			return _next;
		}

		/**
		 * Returns the objects that automatically adjust view properties
		 *
		 * @func action()
		 */
		inline Action* action() {
			return _action;
		}

		/**
		 * Returns the Matrix displacement
		 *
		 * @func translate
		 */
		inline Vec2 translate() const {
			return _translate;
		}

		/**
		 * Returns the Matrix scaling
		 *
		 * @func scale()
		 */
		inline Vec2 scale() const {
			return _scale;
		}

		/**
		 * Returns the Matrix skew
		 *
		 * @func skew()
		 */
		inline Vec2 skew() const {
			return _skew;
		}

		/**
		 * Returns the z-axis rotation of the matrix
		 *
		 * @func skew()
		 */
		inline float rotate() const {
			return _rotate;
		}

		/**
		 * Set the `action` properties of the view object
		 *
		 * @func set_action()
		 */
		void set_action(Action* val);

		/**
		 * Set the matrix `translate` properties of th view object
		 *
		 * @func set_translate()
		 */
		void set_translate(Vec2 val);

		/**
		 * Set the matrix `scale` properties of th view object
		 *
		 * @func set_scale()
		 */
		void set_scale(Vec2 val);

		/**
		 * Set the matrix `skew` properties of th view object
		 *
		 * @func set_skew()
		 */
		void set_skew(Vec2 val);

		/**
		 * Set the z-axis  matrix `rotate` properties of th view object
		 *
		 * @func set_rotate()
		 */
		void set_rotate(float val);

		/**
		 *
		 * Returns the can affect the transparency of subviews
		 *
		 * @func opacity()
		 */
		inline float opacity() {
			return _opacity;
		}

		/**
		 * Set the `opacity` properties of th view object
		 *
		 * @func set_opacity()
		 */
		void set_opacity(float val);

		/**
		 *
		 * 从外向内正向迭代布局，比如一些布局方法是先从外部到内部先确定盒子的明确尺寸
		 * 
		 * @func layout_forward()
		 */
		virtual void layout_forward();

		/**
		 * 
		 * 从内向外反向迭代布局，比如有些视图外部并没有明确的尺寸，
		 * 尺寸是由内部视图挤压外部视图造成的，所以只能先明确内部视图的尺寸
		 * 
		 * @func layout_reverse()
		 */
		virtual void layout_reverse();

		/**
		 * 当一个父布局视图对其中所有的子视图进行布局时，为了调整各个子视图合适位置与尺寸，会调用这个函数对子视图做尺寸上的限制
		 * 这个函数被调用后，其它调用尺寸更改的方法都应该失效，但应该记录被设置的数值一旦解除锁定后才能生效
		 * 
		 * 调用`layout_size_lock(false)`解除锁定
		 * 
		 * 子类实现这个方法
		 * 
		 * @func layout_size_lock()
		 */
		virtual void layout_size_lock(bool lock, Vec2 size = Vec2());

		/**
		 * 
		 * 相对父视图（layout_offset）开始的偏移量
		 * 
		 * @func layout_offset()
		 */
		inline Vec2 layout_offset() const {
			return _layout_offset;
		}

		/**
		 * 
		 * 相对父视图（layout_offset）开始的偏移量
		 * 
		 * @func layout_offset_end()
		 */
		inline Vec2 layout_offset_end() const {
			return _layout_offset_end;
		}

		/**
		 *
		 * 返回布局尺寸(如果为box,size=margin+border+padding+content)
		 *
		 * @func layout_size()
		 */
		inline Vec2 layout_size() const {
			return _layout_offset_end - layout_offset;
		}

		/**
		 * 
		 * 布局内偏移补偿，影响子视图偏移位置
		 *
		 * @func layout_offset_inside()
		 */
		virtual Vec2 layout_offset_inside() const;

		/**
		 * 重新绘制视图以及内部子视图
		 * @func draw()
		 */
		virtual void draw();

		/**
		 * 
		 * 视图基础变换矩阵
		 * Mat(layout_offset + final_origin + translate - parent->layout_inside_offset, scale, rotate, skew)
		 * 
		 * @func matrix()
		 */
		Mat matrix();

		/**
		 * Start the matrix transformation from this origin point
		 *
		 * @func final_origin()
		 */
		inline Vec2 final_origin() const {
			return _final_origin;
		}

		/**
		 * 
		 * 视图最终变换矩阵, parent.final_matrix * matrix
		 * 
		 * @func final_matrix()
		 */
		const Mat& final_matrix();

		/**
		 *
		 * 布局权重（比如在flex布局中代表布局的尺寸）
		 *
		 * @func layout_weight()
		 */
		inline float layout_weight() {
			return _layout_weight;
		}

		private:
		View *_parent;
		View *_first, *_last, *_prev, *_next;
		// View *_next_pre_mark; // 下一个标记预处理视图
		Action *_action; //
		// uint32_t _level; // 在视图树中所处的层级
		Vec2  _translate, _scale, _skew; // 平移向量, 缩放向量, 倾斜向量
		float _rotate;     // z轴旋转角度值
		float _opacity;    // 可影响子视图的透明度值
		Vec2  _final_origin;  // 最终以该点 位移,缩放,旋转,歪斜
		Mat   _final_matrix;  // 父视图矩阵乘以基础矩阵等于最终变换矩阵 (parent.final_matrix * matrix)
		Vec2  _layout_offset; // 相对父视图的开始偏移位置（box包含margin值）
		Vec2  _layout_offset_end; // 相对父视图的结束偏移位置（end=start+margin+border+padding+content）
		float _layout_weight; // layout weight
	};

}

#endif