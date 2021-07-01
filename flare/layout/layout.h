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

#ifndef __flare__layout__layout__
#define __flare__layout__layout__

#include "../util/object.h"
#include "../value.h"

namespace flare {

	class PreRender;

	/**
	 *
	 * Layout and typesetting protocol
	 *
	 * @class Layout
	 */
	class Layout: public Reference {
		public:

		// Layout mark value
		enum : uint32_t {
			M_NONE                    = (0),        /* 没有任何标记 */
			M_TRANSFORM               = (1 << 0), /* 矩阵变换 recursive mark */
			M_TRANSFORM_ORIGIN        = (1 << 1), /* 矩阵变换 origin mark */
			M_LAYOUT_SIZE_WIDTH       = (1 << 2), /* 布局尺寸改变, 尺寸改变可能影响父布局 */
			M_LAYOUT_SIZE_HEIGHT      = (1 << 3),
			M_LAYOUT_TYPESETTING      = (1 << 4), /* 布局内容偏移, 需要重新对子布局排版 */
			//**
			M_RECURSIVE               = (M_TRANSFORM | M_TRANSFORM_ORIGIN), /* 需要被递归的标记 */
		};

		// layout align
		enum LayoutAlign: unit8_t {
			// flow/flex
			AUTO = value::AUTO,
			START = value::START,
			CENTER = value::CENTER,
			END = value::END,
			STRETCH = value::STRETCH,
			// BASELINE = value::BASELINE,
			// default
			LEFT_TOP = value::LEFT_TOP,
			CENTER_TOP = value::CENTER_TOP,
			RIGHT_TOP = value::RIGHT_TOP,
			LEFT_CENTER = value::LEFT_CENTER,
			CENTER_CENTER = value::CENTER_CENTER,
			RIGHT_CENTER = value::RIGHT_CENTER,
			LEFT_BOTTOM = value::LEFT_BOTTOM,
			CENTER_BOTTOM = value::CENTER_BOTTOM,
			RIGHT_BOTTOM = value::RIGHT_BOTTOM,
		};

		struct Rect {
			Vec2 origin;
			Vec2 size;
		};

		struct Region {
			float x1, y1;
			float x2, y2;
			float w, h;
		};

		/**
		 * @constructors
		 */
		Layout();

		/**
		 * @destructor
		 */
		virtual ~Layout();

		/**
		 *
		 * 布局在GUI树中所处的深度，0表示还没有加入到GUI视图树中
		 *
		 * @func layout_depth()
		 */
		inline uint32_t layout_depth() const {
			return _depth;
		}

		/**
		 *
		 * 布局权重（比如在flex布局中代表布局的尺寸）
		 *
		 * @func layout_weight()
		 */
		virtual float layout_weight();

		/**
		 *
		 * 布局的对齐方式（九宫格）
		 *
		 * @func layout_align()
		 */
		virtual LayoutAlign layout_align();

		/**
		 * 
		 * Relative to the parent view (layout_offset) to start offset
		 * 
		 * @func layout_offset()
		 */
		virtual Vec2 layout_offset();

		/**
		 *
		 * Returns the layout size of view object (if is box view the: size=margin+border+padding+content)
		 *
		 * @func layout_size()
		 */
		virtual Vec2 layout_size();

		/**
		 * 
		 * Relative to the parent view (layout_offset) to start offset，end position
		 * 
		 * @func layout_offset_end()
		 */
		inline Vec2 layout_offset_end() {
			return layout_offset() + layout_size();
		}

		/**
		 * Returns internal layout offset compensation of the view, which affects the sub view offset position
		 * 
		 * For example: when a view needs to set the scrolling property scroll of a subview, you can set this property
		 *
		 * @func layout_offset_inside()
		 */
		virtual Vec2 layout_offset_inside();

		/**
		 *
		 * Returns the layout content size of object view, 
		 * Returns false to indicate that the size is unknown,
		 * indicates that the size changes with the size of the subview, and the content is wrapped
		 *
		 * @func layout_content_size(is_explicit_width, explicit_height)
		 */
		virtual Vec2 layout_content_size(bool& is_explicit_width, bool& is_explicit_height);

		/**
		 * 
		 * Setting the layout offset of the view object in the parent view
		 *
		 * @func set_layout_offset(val)
		 */
		virtual void set_layout_offset(Vec2 val);

		/**
		 * 
		 * Setting layout offset values lazily mode for the view object
		 *
		 * @func set_layout_offset_lazy()
		 */
		virtual void set_layout_offset_lazy(Rect rect);

		/**
			* 当一个父布局视图对其中所拥有的子视图进行布局时，为了调整各个子视图合适位置与尺寸，如有必要可以调用这个函数对子视图做尺寸限制
			* 这个函数被调用后，子视图上任何调用尺寸更改的方法都应该失效，但应该记录更改的数值一旦解除锁定后之前更改尺寸属性才可生效
			* 
			* 调用`lock_layout_size(Vec2(-1,-1))`解除锁定
			* 
			* 子类实现这个方法
			* 
			* 返回锁定后的最终尺寸
			* 
			* @func lock_layout_size(layout_size)
			*/
		virtual Vec2 lock_layout_size(Vec2 layout_size);

		/**
		 *
		 * (计算布局自身的尺寸)
		 *
		 * 从外向内正向迭代布局，比如一些布局方法是先从外部到内部先确定盒子的明确尺寸
		 * 
		 * 这个方法被调用时父视图尺寸一定是有效的，在调用`layout_content_size`时有两种情况，
		 * 返回`false`表示父视图尺寸是wrap的，返回`true`时表示父视图有明确的尺寸
		 * 
		 * @func layout_forward(mark)
		 */
		virtual bool layout_forward(uint32_t mark) = 0;

		/**
		 * 
		 * (计算子布局的偏移位置，以及确定在`layout_forward()`函数没有能确定的尺寸)
		 * 
		 * 从内向外反向迭代布局，比如有些视图外部并没有明确的尺寸，
		 * 尺寸是由内部视图挤压外部视图造成的，所以只能先明确内部视图的尺寸。
		 *
		 * 这个方法被调用时子视图尺寸一定是明确的有效的，调用`layout_size()`返回子视图外框尺寸。
		 * 
		 * @func layout_reverse(mark)
		 */
		virtual bool layout_reverse(uint32_t mark) = 0;

		/**
		 *
		 * 从外向内正向迭代布局
		 * 
		 * @func layout_recursive(mark)
		 */
		virtual void layout_recursive(uint32_t mark) = 0;

		/**
		 * 
		 * This method of the parent view is called when the layout content of the child view changes
		 *
		 * This is not necessarily called by the child layout
		 *
		 * @func layout_typesetting_change(Layout* child)
		 */
		virtual void layout_typesetting_change(Layout* child = 0);

		/**
		 * 
		 * This method of the parent view is called when the layout weight of the child view changes
		 * 
		 * @func layout_typesetting_change_from_child_weight(child, weight)
		 */
		virtual void layout_typesetting_change_from_child_weight(Layout* child, float weight);

		/**
		 * 
		 * This method of the child view is called when the layout size of the parent view changes
		 * 
		 * @func layout_content_size_change_from_parent(parent, mark)
		 */
		virtual void layout_content_size_change_from_parent(Layout* parent, uint32_t mark);

		/**
		 * 
		 * set layout depth for the cureent view object
		 *
		 * @func set_layout_depth(newDepth)
		 */
		protected: void set_layout_depth(uint32_t newDepth);

		/**
		 * @func mark(mark)
		 */
		void mark(uint32_t mark);

		/**
		 * @func mark_recursive(mark)
		 */
		void mark_recursive(uint32_t mark);

		/**
		 * @func mark_none()
		 */
		void mark_none();

		/**
		 * @func unmark(mark)
		 */
		inline void unmark(uint32_t mark = (~M_NONE/*default unmark all*/)) {
			_layout_mark &= (~mark);
		}

		/**
		 * @func layout_mark()
		 */
		public: inline void layout_mark() const {
			return _layout_mark;
		}

		// layout:
		/* 下一个预处理视图标记
		*  在绘图前需要调用`layout_forward`与`layout_reverse`处理这些被标记过的视图。
		*  同一时间不会所有视图都会发生改变,如果视图树很庞大的时候,
		*  如果涉及到布局时为了跟踪其中一个视图的变化就需要遍历整颗视图树,为了避免这种情况
		*  把标记的视图独立到视图外部按视图等级进行分类以双向环形链表形式存储(PreRender)
		*  这样可以避免访问那些没有发生改变的视图并可以根据视图等级顺序访问.
		*/
		private: int32_t _mark_index, _recursive_mark_index;
		/* 这些标记后的视图会在开始帧绘制前进行更新.
		*  需要这些标记的原因主要是为了最大程度的节省性能开销,因为程序在运行过程中可能会频繁的更新视图局部属性也可能视图很少发生改变.
		*  1.如果对每次更新如果都更新GPU中的数据那么对性能消耗那将是场灾难,那么记录视图所有的局部变化,待到需要帧渲染时统一进行更新.
		*/
		private: uint32_t _layout_mark; /* 标记 */
		private: uint32_t _depth; // 这个值受`View::_visible`影响, View::_visible=false时_depth=0
		
		friend class PreRender;
	};

}

#endif