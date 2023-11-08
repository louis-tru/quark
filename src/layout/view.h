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

#ifndef __quark__layout__view__
#define __quark__layout__view__

#include "./layout.h"
#include "../event.h"

namespace qk {

	#define Qk_Each_View(F) \
		F(View)  F(Box) \
		F(Image) F(Video) F(Scroll) F(Button) F(FloatLayout) F(Textarea) \
		F(Label) F(Input) F(Root) F(TextLayout) F(FlexLayout) F(FlowLayout)

	#define Qk_Define_View(N) \
	public: \
		friend class ViewRender; \
		virtual void accept(ViewVisitor *visitor) override { visitor->visit##N(this); } \

	class Action;
	class ViewRender;
	class TextInput;

	Qk_DEFINE_VISITOR(View, Qk_Each_View);

	/**
		* The basic elements of UI tree
		*
	 * @class View
		*/
	class Qk_EXPORT View: public Notification<UIEvent, UIEventName, Layout> {
		Qk_HIDDEN_ALL_COPY(View);
	protected:
		struct Transform {
			Vec2 translate, scale, skew;
			float rotate; // 平移向量, 缩放向量, 倾斜向量, z轴旋转弧度
		};
		struct Transform *_transform; // 矩阵变换
		Mat              _matrix; // 父视图矩阵乘以布局矩阵等于最终变换矩阵 (parent.matrix * layout_matrix)
	public:
		typedef ViewVisitor Visitor;
		// @props
		Qk_DEFINE_PROP_ACC(Vec2, translate); // matrix displacement for the view
		Qk_DEFINE_PROP_ACC(Vec2, scale); // Matrix scaling
		Qk_DEFINE_PROP_ACC(Vec2, skew); // Matrix skew, (radian)
		Qk_DEFINE_PROP_ACC(float, rotate); // z-axis rotation of the matrix
		Qk_DEFINE_PROP_ACC(float, x); // x-axis matrix displacement for the view
		Qk_DEFINE_PROP_ACC(float, y); // y-axis matrix displacement for the view
		Qk_DEFINE_PROP_ACC(float, scale_x); // x-axis matrix scaling for the view
		Qk_DEFINE_PROP_ACC(float, scale_y); // y-axis matrix scaling for the view
		Qk_DEFINE_PROP_ACC(float, skew_x); // x-axis matrix skew for the view
		Qk_DEFINE_PROP_ACC(float, skew_y); // y-axis matrix skew for the view
		Qk_DEFINE_PROP_ACC(bool,  is_focus); // keyboard focus view
		// the objects that automatically adjust view properties
		Qk_DEFINE_PROP(Action*, action); // 在指定的时间内根据动作设定运行连续一系列的动作命令，达到类似影片播放效果
		Qk_DEFINE_PROP_GET(View*, parent);
		Qk_DEFINE_PROP_GET(View*, prev);
		Qk_DEFINE_PROP_GET(View*, next);
		Qk_DEFINE_PROP_GET(View*, first);
		Qk_DEFINE_PROP_GET(View*, last);
		// can affect the transparency of subviews
		Qk_DEFINE_PROP(float, opacity); // 可影响子视图的透明度值
		// 视图是否需要接收或处理系统的事件抛出，大部情况下这些事件都是不需要处理的，这样可以提高整体事件处理效率
		// @prop Does the view need to receive or handle event thlines from the system
		Qk_DEFINE_PROP(bool, receive);
		// 设置视图的可见性，这个值设置为`false`时视图为不可见且不占用任何布局空间
		Qk_DEFINE_PROP_GET(bool, visible);
		// 这个值与`visible`完全无关，这个代表视图在当前显示区域是否可见，这个显示区域大多数情况下就是屏幕
		Qk_DEFINE_PROP_GET(bool, visible_region);

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
		 * Returns is can allow append child view
		 * 
		 * @method is_allow_append_child()
		*/
		virtual bool is_allow_append_child();

		/**
		 *
		 * Remove destroy self and child views from parent view
		 *
		 * @method remove()
		 */
		virtual void remove();

		/**
		 *
		 * remove all subview
		 *
		 * @method remove_all_child()
		 */
		virtual void remove_all_child();

		/**
		 *
		 * Setting the visibility properties the view object
		 *
		 * @method set_visible(val)
		 */
		virtual void set_visible(bool val);

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
		virtual void trigger_listener_change(uint32_t name, int count, int change) override;

		/**
		 * @method has_child(child)
		 */
		bool has_child(View* child);

		/**
		 *
		 * Returns layout transformation matrix of the object view
		 *
		 * Mat(layout_offset + transform_origin? + translate + parent->layout_offset_inside, scale, rotate, skew)
		 *
		 * @method layout_matrix()
		 */
		virtual Mat layout_matrix();

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

		/**
		 * @method solve_marks(mark)
		*/
		virtual void solve_marks(uint32_t mark);

		/**
		 * 
		 * returns view position in the screen
		 * 
		 * @method screen_position()
		*/
		virtual Vec2 position();

		/**
			* @method solve_visible_region()
			*/
		virtual bool solve_visible_region();

		/**
		 * Overlap test, test whether the point on the screen overlaps with the view
		 * @method overlap_test
		*/
		virtual bool overlap_test(Vec2 point);
		
		/**
		 * define access receiver
		 * @method accept()
		 */
		virtual void accept(ViewVisitor *visitor);

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
		* @method overlap_test_from_convex_quadrilateral
		*/
		static bool overlap_test_from_convex_quadrilateral(Vec2 quadrilateral_vertex[4], Vec2 point);
		
		/**
		 * @method screen_rect_from_convex_quadrilateral
		*/
		static Rect screen_rect_from_convex_quadrilateral(Vec2 quadrilateral_vertex[4]);
		
		/**
		 * @method screen_region_from_convex_quadrilateral
		*/
		static Region screen_region_from_convex_quadrilateral(Vec2 quadrilateral_vertex[4]);

		/**
			* @overwrite
			*/
		virtual bool layout_forward(uint32_t mark) override;
		virtual bool layout_reverse(uint32_t mark) override;
		virtual void layout_text(TextLines *lines, TextConfig* textSet) override;
		virtual void onChildLayoutChange(Layout* child, uint32_t mark) override;
		virtual void onParentLayoutContentSizeChange(Layout* parent, uint32_t mark) override;

	protected:
		/**
		 * @method set_parent(parent) setting parent view
		 */
		virtual void set_parent(View* parent);

		friend class ViewRender;
		Qk_DEFINE_INLINE_CLASS(Inl);
		Qk_DEFINE_INLINE_CLASS(InlEvent);
	};

}
#endif
