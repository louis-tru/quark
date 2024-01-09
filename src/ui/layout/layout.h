/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright (c) 2015, blue.chu
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of blue.chu nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL blue.chu BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * ***** END LICENSE BLOCK ***** */

#ifndef __quark__ui__layout__
#define __quark__ui__layout__

#include "../types.h"
#include "../view.h"

namespace qk {
	class TextInput;
	class TextLines;
	class TextConfig;
	class TextOptions;
	class UIRender;
	class Window;
	class TransformLayout;
	class ScrollLayoutBase;

	/**
		* Layout tree nodes that can only be called in rendering threads.
		* 
		* The local tree receives command messages sent by the view tree and executes them in the rendering thread.
		*
	 * @class Layout
		*/
	class Qk_EXPORT Layout {
		Qk_HIDDEN_ALL_COPY(Layout);
	public:
		typedef NonObjectTraits Traits;

		// Layout mark key values
		enum LayoutMark: uint32_t {
			kLayout_None              = (0),      /* 没有任何标记 */
			kLayout_Size_Width        = (1 << 0), /* 布局尺寸改变, 尺寸改变可能影响父布局 */
			kLayout_Size_Height       = (1 << 1),
			kLayout_Typesetting       = (1 << 2), /* 布局内容偏移, 需要重新对子布局排版 */
			kTransform_Origin         = (1 << 3),
			kInput_Status             = (1 << 4), /* 输入状态这不包含布局的改变 */
			kScroll                   = (1 << 5), /* scroll status change */
			// RECURSIVE MARKS
			kRecursive_Transform      = (1 << 30), /* 矩阵变换 recursive mark */
			kRecursive_Visible_Region = (1U << 31), /* 可见范围 */
			kRecursive_Mark           = (kRecursive_Transform | kRecursive_Visible_Region),
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
			bool wrap_x, wrap_y;
		};

		/* 
		* @prop mark_value
		*
		* The marked layout will be updated before starting frame drawing.
		* During operation, layout local attributes may be updated frequently or the layout may rarely change.
		*/
		Qk_DEFINE_PROP_GET(uint32_t, mark_value);

		/*
		* Next preprocessing layout tag
		* You need to call `layout_forward` and `layout_reverse` to process these marked layouts before drawing.
		* Not all layouts will change at the same time. If the layout tree is very large,
		* If it comes to layout, in order to track changes in one of the layout nodes, it is necessary to traverse the entire layout tree. In order to avoid this situation
		* Separate the marked views outside the layout, classify them according to the layout level, and store them in the form of a two-way circular linked list (PreRender)
		* This avoids accessing views that have not changed and allows them to be accessed sequentially according to the layout hierarchy.
		* 
		* @prop mark_index
		*/
		Qk_DEFINE_PROP_GET(int32_t, mark_index);

		/*
		* @prop level
		*
		* 布局在UI树中所处的深度，0表示还没有加入到UI视图树中
		* 这个值受`Layout::_visible`影响, Layout::_visible=false时_level=0
		*/
		Qk_DEFINE_PROP_GET(uint32_t, level);

		/**
		 *
		 * View at the final position on the screen (parent.matrix * (offset + offset_inside))
		 *
		 * @prop position()
		 */
		Qk_DEFINE_PROP_GET(Vec2, position, Protected);

		/*
		* @prop window
		*/
		Qk_DEFINE_PROP_GET(Window*, window);

		/*
		* @prop view
		*/
		Qk_DEFINE_PROP_GET(View*, view);

		/**
		 * 
		 * return safe view and retain view
		 * 
		 * @prop safe_view()
		*/
		Qk_DEFINE_PROP_ACC_GET(Sp<View>, safe_view, NoConst);

		/**
		 * parent layout view
		*/
		Qk_DEFINE_PROP_GET(Layout*, parent);
		Qk_DEFINE_PROP_GET(Layout*, prev);
		Qk_DEFINE_PROP_GET(Layout*, next);
		Qk_DEFINE_PROP_GET(Layout*, first);
		Qk_DEFINE_PROP_GET(Layout*, last);

		/**
		 * @prop props accessor
		*/
		Qk_DEFINE_PROP_GET(PropAccessor*, accessor);

		/**
		 *  can affect the transparency of subviews
		 */
		Qk_DEFINE_PROP(float, opacity);

		/**
		 * Set the visibility of the view. When this value is set to 'false',
		 * the view is invisible and does not occupy any layout space
		*/
		Qk_DEFINE_PROP(bool, visible);

		/**
		 *  这个值与`visible`不相关，这个代表视图在当前显示区域是否可见，这个显示区域大多数情况下就是屏幕
		*/
		Qk_DEFINE_PROP(bool, visible_region, Protected);

		/**
		 * Do views need to receive or handle system event throws? In most cases,
		 * these events do not need to be handled, which can improve overall event processing efficiency
		*/
		Qk_DEFINE_PROP(bool, receive);

		/**
		 * @prop transform()
		*/
		Qk_DEFINE_PROP_ACC_GET(TransformLayout*, transform, NoConst);

		/**
		 * @constructor
		*/
		Layout(Window *win);

		/**
		 * @destructor
		*/
		virtual ~Layout();

		/**
		 * 
		 * Returns text input object
		 * 
		 * @method asTextInput()
		*/
		virtual TextInput* asTextInput();

		/**
		 * 
		 * Returns as transform
		 * 
		 * @method asTransform()
		*/
		virtual TransformLayout* asTransform();

		/**
		 * 
		 * Returns as textOptions
		 * 
		 * @method asTextOptions()
		*/
		virtual TextOptions* asTextOptions();

		/**
		 * 
		 * Returns as ScrollLayoutBase
		 * 
		 * @method asScrollLayoutBase()
		*/
		virtual ScrollLayoutBase* asScrollLayoutBase();

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
		virtual bool layout_forward(uint32_t/*LayoutMark*/ mark);

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
		virtual bool layout_reverse(uint32_t/*LayoutMark*/ mark);

		/**
		 * 
		 * solve text layout
		 * 
		 * @func layout_text(lines)
		 */
		virtual void layout_text(TextLines *lines, TextConfig* textSet);

		/**
			* 
			* This method of the parent view is called when the layout content of the child view changes
			*
			* This is not necessarily called by the child layout
			*
			* @func onChildLayoutChange(child, mark)
			*/
		virtual void onChildLayoutChange(Layout* child, uint32_t/*ChildLayoutChangeMark*/ mark);

		/**
			* 
			* This method of the child view is called when the layout size of the parent view changes
			* 
			* @func onParentLayoutContentSizeChange(parent, mark)
			*/
		virtual void onParentLayoutContentSizeChange(Layout* parent, uint32_t/*LayoutMark*/ mark);

		/**
		 * Overlap test, test whether the point on the screen overlaps with the view
		 * @method overlap_test
		*/
		virtual bool overlap_test(Vec2 point);

		/**
		 * 
		 * returns view position center in the position
		 * 
		 * @method center()
		*/
		virtual Vec2 center();

		/**
		 * @method solve_marks(mark)
		*/
		virtual void solve_marks(const Mat &mat, uint32_t mark);

		/**
			* @method solve_visible_region()
			*/
		virtual bool solve_visible_region(const Mat &mat);

		/**
		 * notice update for set parent or level
		 * 
		 * @method onActivate()
		*/
		virtual void onActivate();

		/**
		 *
		 * is clip render the view
		 *
		 * @method is_clip()
		 */
		virtual bool is_clip();

		/**
		 * @method draw()
		 */
		virtual void draw(UIRender *render);

		/**
		 * @method viewType()
		*/
		virtual ViewType viewType() const;

		/**
			* @func mark_layout(mark)
			*/
		void mark_layout(uint32_t mark);

		/**
			* @func mark_render(mark)
			*/
		void mark_render(uint32_t mark = kLayout_None);

		/**
			* @func unmark(mark)
			*/
		inline void unmark(uint32_t mark = (~kLayout_None/*default unmark all*/)) {
			_mark_value &= (~mark);
		}

	private:
		void before(Layout* view);
		void after(Layout* view);
		void prepend(Layout* child);
		void append(Layout* child);
		void remove();
		void set_parent(Layout* parent);
		void clear_link(); // Cleaning up associated view information
		void clear_level(); //  clear layout depth
		void set_level_(uint32_t level); // settings depth
		void set_visible_(bool visible, uint32_t level);

		friend class UIRender;
		friend class PreRender;
		friend class EventDispatch;
		friend class View;
		friend class RootLayout;
	};

	template<class _View>
	inline _View* View::newView() {
		return new _View(new _View::Layout(_layout->_window));
	}

}
#endif
