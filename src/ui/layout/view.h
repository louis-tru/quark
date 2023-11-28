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

#ifndef __quark__ui__view__
#define __quark__ui__view__

#include "../event.h"
#include "./layout.h"

namespace qk {

	#define Qk_Each_View(F) \
		F(View)  F(Box) F(Transform) \
		F(Image) F(Video) F(Scroll) F(Button) F(FloatLayout) F(Textarea) \
		F(Label) F(Input) F(Root) F(TextLayout) F(FlexLayout) F(FlowLayout)

	#define Qk_Define_View(N) \
	public: \
		friend class ViewRender; \
		virtual void accept(ViewVisitor *visitor) override { visitor->visit##N(this); } \

	class Action;
	class ViewRender;
	class TextInput;
	class EventDispatch;

	Qk_DEFINE_VISITOR(View, Qk_Each_View);

	/**
		* The basic elements of UI tree
		*
	 * @class View
		*/
	class Qk_EXPORT View: public Notification<UIEvent, UIEventName, Layout> {
		Qk_HIDDEN_ALL_COPY(View);
	protected:
		Mat _matrix; // 父视图矩阵乘以布局矩阵等于最终变换矩阵 (parent.matrix * layout_matrix)
	public:
		typedef ViewVisitor Visitor;
		/* 
		* 标记后的视图会在开始帧绘制前进行更新.
		* 运行过程中可能会频繁的更新视图局部属性也可能视图很少发生改变.
		*/
		Qk_DEFINE_PROP_GET(uint32_t, mark);

		/**
		 * the objects that automatically adjust view properties
		*/
		Qk_DEFINE_PROP(Action*, action);
		Qk_DEFINE_PROP_GET(View*, parent);
		Qk_DEFINE_PROP_GET(View*, prev);
		Qk_DEFINE_PROP_GET(View*, next);
		Qk_DEFINE_PROP_GET(View*, first);
		Qk_DEFINE_PROP_GET(View*, last);
		/**
		 *  can affect the transparency of subviews
		 */
		Qk_DEFINE_PROP(float, opacity);
		/**
		 * Do views need to receive or handle system event throws? In most cases, 
		 * these events do not need to be handled, which can improve overall event processing efficiency
		*/
		Qk_DEFINE_PROP(bool, receive);
		/** 
		 * Set the visibility of the view. When this value is set to 'false',
		 * the view is invisible and does not occupy any layout space
		*/
		Qk_DEFINE_PROP(bool, visible);
		/**
		 *  这个值与`visible`完全无关，这个代表视图在当前显示区域是否可见，这个显示区域大多数情况下就是屏幕
		*/
		Qk_DEFINE_PROP_GET(bool, visible_region);
		Qk_DEFINE_PROP_ACC(bool, is_focus); // keyboard focus view

		/**
		 * @constructor
		*/
		View();

		/**
		 * @destructor
		*/
		virtual ~View();

		template<class T = View> inline T* prepend_new() {
			return New<T>()->template prepend_to<T>(this);
		}

		template<class T = View> inline T* append_new() {
			return New<T>()->template append_to<T>(this);
		}

		/**
		 * Prepend subview to parent
		 *
		 * @method prepend_to(parent)
		 */
		template<class T = View> inline T* prepend_to(View* parent) {
			parent->prepend(this); return static_cast<T*>(this);
		}

		/**
		 * Append subview to parent
		 *
		 * @method append_to(parent)
		 */
		template<class T = View> inline T* append_to(View* parent) {
			parent->append(this); return static_cast<T*>(this);
		}

		/**
		 * @method is_self_child(child)
		 */
		bool is_self_child(View *child);

		/**
			*
			* Add a sibling view to the front
			*
			* @method before(view)
			*/
		void before(View* view);

		/**
			*
			* Add a sibling view to the back
			*
			* @method after(view)
			*/
		void after(View* view);

		/**
			* 
			* Append subview to front
			* 
			* @method prepend(child)
			*/
		void prepend(View* child);

		/**
			*
			* Append subview to end
			*
			* @method append(child)
			*/
		void append(View* child);

		/**
		 *
		 * Remove self from parent view
		 *
		 * @method remove()
		 */
		void remove();

		/**
		 *
		 * remove all subview
		 *
		 * @method remove_all_child()
		 */
		void remove_all_child();

		/**
		 *
		 * focus keyboard
		 *
		 * @method focus()
		 */
		bool focus();

		/**
		 *
		 * Unfocus keyboard
		 *
		 * @method blur()
		 */
		bool blur();

		/**
		 * 
		 * Returns text input object
		 * 
		 * @method as_text_input()
		*/
		virtual TextInput* as_text_input();

		/**
		 * 
		 * Returns button object
		 * 
		 * @method as_buttn()
		*/
		virtual Button* as_button();

		/**
		 * define access receiver
		 * @method accept()
		 */
		virtual void accept(ViewVisitor *visitor);

		/**
		 *
		 * Can it be the focus
		 *
		 * @method can_become_focus()
		 */
		virtual bool can_become_focus();

		/**
		 *
		 * is clip render the view
		 *
		 * @method clip()
		 */
		virtual bool clip();

		/**
			* @overwrite
			*/
		virtual bool layout_forward(uint32_t mark) override;
		virtual bool layout_reverse(uint32_t mark) override;
		virtual void layout_text(TextLines *lines, TextConfig* textSet) override;
		virtual void onChildLayoutChange(Layout* child, uint32_t mark) override;
		virtual void onParentLayoutContentSizeChange(Layout* parent, uint32_t mark) override;

		// @thread unsafe --------------------------------------------------------------------------
		/**
		 *
		 * Returns final transformation matrix of the view layout
		 *
		 * parent.matrix * layout_matrix
		 *
		 * @method matrix()
		 */
		inline const Mat& matrix() const {
			return _matrix;
		}

	protected:
		/**

		 * Returns layout transformation matrix of the object view
		 *
		 * Mat(layout_offset + transform_origin? + translate + parent->layout_offset_inside, scale, rotate, skew)
		 *
		 * @method layout_matrix()
		 * @thread unsafe 
		 */
		virtual Mat layout_matrix();

		/**
		 * 
		 * returns view position in the screen
		 * 
		 * @method screen_position()
		*/
		virtual Vec2 position();

		/**
		 * Overlap test, test whether the point on the screen overlaps with the view
		 * @method overlap_test
		*/
		virtual bool overlap_test(Vec2 point);

		/**
		 * @method solve_marks(mark)
		*/
		virtual void solve_marks(uint32_t mark);

		/**
			* @method solve_visible_region()
			*/
		virtual bool solve_visible_region();

		/**
		 * notice update for set parent or level
		 * 
		 * @method onActivate()
		*/
		virtual void onActivate();

	private:
		void clear_link(); // Cleaning up associated view information
		void clear_level(Window *win); //  clear layout depth
		void set_level_(uint32_t level, Window *win); // settings depth
		void set_visible_(bool visible, uint32_t level);
		bool is_root();
		void remove_all_child_();
		/**
		 * @method set_parent(parent) setting parent view
		 */
		void set_parent(View* parent);

		Qk_DEFINE_INLINE_CLASS(InlEvent);
		friend class ViewRender;
		friend class EventDispatch;
	};

}
#endif
