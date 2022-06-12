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

#ifndef __noug__layout__layout__
#define __noug__layout__layout__

#include "../util/object.h"
#include "../value.h"

namespace noug {

	class PreRender;
	class TextLines;
	class TextConfig;

	/**
		*
		* Layout and typesetting protocol
		*
		* @class Layout
		*/
	class Layout: public Reference {
	public:

		// Layout mark key values
		enum LayoutMark: uint32_t {
			kLayout_None              = (0),      /* 没有任何标记 */
			kLayout_Size_Width        = (1 << 0), /* 布局尺寸改变, 尺寸改变可能影响父布局 */
			kLayout_Size_Height       = (1 << 1),
			kLayout_Typesetting       = (1 << 2), /* 布局内容偏移, 需要重新对子布局排版 */
			kLayout_Text              = (1 << 3), /* 文本布局改变 */
			// RECURSIVE MARKS
			kRecursive_Transform      = (1 << 30), /* 矩阵变换 recursive mark */
			kRecursive_Visible_Region = (1U << 31), /* 可见范围 */
		};

		// child layout change mark key values
		enum ChildLayoutChangeMark : uint32_t {
			kChild_Layout_Size     = (1 << 0),
			kChild_Layout_Visible  = (1 << 1),
			kChild_Layout_Align    = (1 << 2),
			kChild_Layout_Weight   = (1 << 3),
			kChild_Layout_Text     = (1 << 4),
		};

		// layout size
		struct Size {
			Vec2 layout_size, content_size;
			// Vec2 layout_min_size, layout_max_size; ??
			bool wrap_x, wrap_y;
		};

		Layout();
		Layout(PreRender* pre);

		virtual ~Layout();

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
		virtual Align layout_align();

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
			* Returns the layout content size of object view, 
			* Returns false to indicate that the size is unknown,
			* indicates that the size changes with the size of the subview, and the content is wrapped
			*
			* @func layout_size()
			*/
		virtual Size layout_size();

		/**
			*
			* Returns the and compute layout size of object view
			*
			* @func layout_raw_size(parent_content_size)
			*/
		virtual Size layout_raw_size(Size parent_content_size);

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
			* whether the child layout has been locked
			*
			* @func is_lock_child_layout_size()
			*/
		virtual bool is_lock_child_layout_size();

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
			* @func set_layout_offset_lazy(size)
			*/
		virtual void set_layout_offset_lazy(Vec2 size);

		/**
			* 锁定布局的尺寸。在特定的布局类型中自身无法直接确定其自身尺寸，一般由父布局调用如：flex布局类型
			*
			* 这个方法应该在`layout_forward()`正向迭代中由父布局调用,因为尺寸的调整一般在正向迭代中
			* 
			* 返回锁定后的最终尺寸，调用后视返回后的尺寸为最终尺寸
			* 
			* @func layout_lock(layout_size)
			*/
		virtual Vec2 layout_lock(Vec2 layout_size);

		/**
			*
			* (计算布局自身的尺寸)
			*
			* 从外向内正向迭代布局，比如一些布局方法是先从外部到内部先确定盒子的明确尺寸
			* 
			* 这个方法被调用时父视图尺寸一定是有效的，在调用`content_size`时有两种情况，
			* 返回`false`表示父视图尺寸是wrap的，返回`true`时表示父视图有明确的尺寸
			* 
			* @func layout_forward(mark)
			*/
		virtual bool layout_forward(uint32_t/*LayoutMark*/ mark) = 0;

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
		virtual bool layout_reverse(uint32_t/*LayoutMark*/ mark) = 0;

		/**
		 * 
		 * solve text layout
		 * 
		 * @func layout_text(lines)
		 */
		virtual void layout_text(TextLines *lines, TextConfig* textSet) = 0;

		/**
			* 
			* This method of the parent view is called when the layout content of the child view changes
			*
			* This is not necessarily called by the child layout
			*
			* @func onChildLayoutChange(child, mark)
			*/
		virtual void onChildLayoutChange(Layout* child, uint32_t/*ChildLayoutChangeMark*/ mark) = 0;

		/**
			* 
			* This method of the child view is called when the layout size of the parent view changes
			* 
			* @func onParentLayoutContentSizeChange(parent, mark)
			*/
		virtual void onParentLayoutContentSizeChange(Layout* parent, uint32_t/*LayoutMark*/ mark) = 0;

	protected:
		/**
			* 
			* set layout depth for the cureent view object
			*
			* @func set_layout_depth(newDepth)
			*/	
		void set_layout_depth(uint32_t newDepth);

		/**
			* @func mark(mark)
			*/
		void mark(uint32_t mark);

		/**
			* @func mark_none(mark)
			*/
		void mark_none(uint32_t mark = kLayout_None);

		/**
			* @func unmark(mark)
			*/
		inline void unmark(uint32_t mark = (~kLayout_None/*default unmark all*/)) {
			_layout_mark &= (~mark);
		}

	private:
		/* 下一个预处理视图标记
		*  在绘图前需要调用`layout_forward`与`layout_reverse`处理这些被标记过的视图。
		*  同一时间不会所有视图都会发生改变,如果视图树很庞大的时候,
		*  如果涉及到布局时为了跟踪其中一个视图的变化就需要遍历整颗视图树,为了避免这种情况
		*  把标记的视图独立到视图外部按视图等级进行分类以双向环形链表形式存储(PreRender)
		*  这样可以避免访问那些没有发生改变的视图并可以根据视图等级顺序访问.
		*/
		int32_t _mark_index;

	public:
		/* 
		* @field layout_mark
		*
		* 这些标记后的视图会在开始帧绘制前进行更新.
		*  需要这些标记的原因主要是为了最大程度的节省性能开销,因为程序在运行过程中可能会频繁的更新视图局部属性也可能视图很少发生改变.
		*  1.如果对每次更新如果都更新GPU中的数据那么对性能消耗那将是场灾难,那么记录视图所有的局部变化,待到需要帧渲染时统一进行更新.
		*/
		N_DEFINE_PROP_READ(uint32_t, layout_mark);
		/*
		* @field layout_mark
		*
		* 布局在UI树中所处的深度，0表示还没有加入到UI视图树中
		* 这个值受`View::_visible`影响, View::_visible=false时_depth=0
		*/
		N_DEFINE_PROP_READ(uint32_t, layout_depth);

		/*
		* @field pre_render
		*/
		N_DEFINE_PROP_READ(PreRender*, pre_render);
		
		friend class PreRender;
	};

}
#endif
