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

#include "../views.h"
#include "../event.h"
#include "../pre_render.h"

namespace qk {
	/**
		* View tree node base type,
		* Provide APIs that do not use security locks on worker and rendering threads,
		* When setting properties on the main thread and calculating view results on the rendering thread, 
		* data that can only be accessed by the rendering thread is usually suffixed with "RT" or mark as @thread Rt
		*
		* @note Release calls can only be made on the main thread
		*
	 * @class View
		*/
	class Qk_EXPORT View: public Notification<UIEvent, UIEventName, Reference> {
		Qk_DISABLE_COPY(View);
		Qk_DEFINE_INLINE_CLASS(InlEvent);
		std::atomic<CStyleSheetsClass*> _cssclass;
	public:

		// Layout mark key values
		enum LayoutMark: uint32_t {
			kLayout_None              = (0),      /* Not have any the mark */
			kLayout_Inner_Width       = (1 << 0), /* The content inside the layout changes that may affect the parent layout */
			kLayout_Inner_Height      = (1 << 1), /* Same as above */
			kLayout_Outside_Width     = (1 << 2), /* The layout frame size border/margin/padding changes, not the content changes */
			kLayout_Outside_Height    = (1 << 3), /* Same as above */
			kLayout_Size_Width        = (kLayout_Inner_Width | kLayout_Outside_Width),
			kLayout_Size_Height       = (kLayout_Inner_Height | kLayout_Outside_Height),
			kLayout_Size_ALL          = (kLayout_Size_Width | kLayout_Size_Height),
			kLayout_Child_Width       = (1 << 4), /* Subview layout size change */
			kLayout_Child_Height      = (1 << 5), /* Same as above */
			kLayout_Child_Size        = (kLayout_Child_Width | kLayout_Child_Height),
			kLayout_Typesetting       = (1 << 6), /* The layout content is offset, and the sub-layout needs to be typesetting */
			kInput_Status             = (1 << 7), /* Input state changes that don't include layout changes */
			kText_Options             = (1 << 8), /* Text configuration changes and may affect subviews */
			kScroll                   = (1 << 9), /* scroll status change */
			kStyle_Class              = (1 << 10), /* View style changes caused by changing class */
			kTransform                = (1 << 30), /* Matrix Transformation, recursive mark */
			kVisible_Region           = (1U << 31), /* Visible range changes */
			kRecursive_Mark           = (kTransform /*| kVisible_Region*/),
		};

		// Child layout change mark key values
		enum ChildLayoutChangeMark : uint32_t {
			kChild_Layout_Size     = (1 << 0),
			kChild_Layout_Visible  = (1 << 1),
			kChild_Layout_Align    = (1 << 2),
			kChild_Layout_Weight   = (1 << 3),
			kChild_Layout_Text     = (1 << 4),
		};

		enum FloatState: uint8_t {
			kNone_FloatState  = (0), // is float wrap content
			kFixed_FloatState = (1 << 0), // fixed size
			kFixedByLock_FloatState = (1 << 1), // by lock size and fixed size
		};

		// container size
		struct Container {
			struct Pre {
				Vec2    value;
				uint8_t state; // At type of FloatState
			};
			Vec2 content; // final content size
			Vec2 content_diff_before_locking; // final content size diff before locking
			Vec2 pre_width; // 0: min, 1:max
			Vec2 pre_height; // The y-axis content size range
			/*
			* The wrap equal false means indicate that the size is unknown,
			* indicates that the size changes with the size of the sub view, and the content is wrapped
			*/
			uint8_t state_x;//!< The x-axis is float wrap content, use internal extrusion size
			uint8_t state_y; //!< The y-axis is float wrap content, use internal extrusion size
			bool    locked_x, locked_y; //!< locked state

			Range to_range() const;
			float clamp_width(float value) const;
			float clamp_height(float value) const;
			bool  set_pre_width(Pre pre);
			bool  set_pre_height(Pre pre);
			Vec2  layout_size_before_locking(Vec2 layout_size) const;
			inline bool float_x() const { return state_x == kNone_FloatState; }
			inline bool float_y() const { return state_y == kNone_FloatState; }
		};

		/**
		 * @prop style sheets class object
		*/
		Qk_DEFINE_ACCE_GET(CStyleSheetsClass*, cssclass);

		/**
		 * @prop parent view
		*/
		Qk_DEFINE_PROP_GET_Atomic(View*, parent);
		Qk_DEFINE_PROP_GET_Atomic(View*, prev);
		Qk_DEFINE_PROP_GET_Atomic(View*, next);
		Qk_DEFINE_PROP_GET_Atomic(View*, first);
		Qk_DEFINE_PROP_GET_Atomic(View*, last);

		/**
		* @prop window
		*/
		Qk_DEFINE_PROP_GET(Window*, window);

		/**
		 * the objects that automatically adjust view properties
		*/
		Qk_DEFINE_PROPERTY(Action*, action) throw(Error);

		/**
		 * @prop props accessor
		*/
		Qk_DEFINE_PROP_GET(CssPropAccessor*, accessor);

		/**
		 * @prop matrix
		*/
		Qk_DEFINE_ACCE_GET(MorphView*, morph_view);

		/**
		* @prop mark_value
		* @thread Rt
		*
		* The marked view will be updated before starting frame drawing.
		* During operation, view local attributes may be updated frequently or the view may rarely change.
		*/
		Qk_DEFINE_PROP_GET(uint32_t, mark_value, Const);

		/**
		* Next preprocessing view tag
		* You need to call `layout_forward` and `layout_reverse` to process these marked views before drawing.
		* Not all views will change at the same time. If the view tree is very large,
		* If it comes to view, in order to track changes in one of the view nodes, it is necessary to traverse the entire view tree. In order to avoid this situation
		* Separate the marked views outside the view, classify them according to the view level, and store them in the form of a two-way circular linked list (PreRender)
		* This avoids accessing views that have not changed and allows them to be accessed sequentially according to the view hierarchy.
		* 
		* @prop mark_index
		* @thread Rt
		*/
		Qk_DEFINE_PROP_GET(uint32_t, mark_index, Const);

		/**
		* @prop level
		* @thread Rt
		*
		* The depth of the layout in the UI tree, 0 indicates that it has not yet been added to the UI view tree.
		* 
		* This value is affected by `View::_visible`, when `View::_visible=false`, _level=0
		*/
		Qk_DEFINE_PROP_GET(uint32_t, level, Const);

		/**
		 *
		 * View at the final position on the screen (parent.matrix * (offset + offset_inside))
		 *
		 * @prop position
		 * @thread Rt
		 */
		Qk_DEFINE_PROP_GET(Vec2, position, ProtectedConst);

		/**
		 * color.a alias opacity = color.a / 255.0
		*/
		Qk_DEFINE_VIEW_ACCESSOR(float, opacity, Const);

		/**
		 * Can affect the blend color of subviews
		*/
		Qk_DEFINE_VIEW_PROPERTY(Color, color, Const);

		/**
		 * Is inherit color from parent view,
		 * if true the final color = parent.final color * self.color.
		 * ** default is CascadeColor::Alpha
		*/
		Qk_DEFINE_VIEW_PROPERTY(CascadeColor, cascade_color, Const);

		/**
		 * @prop Cursor style
		*/
		Qk_DEFINE_VIEW_PROPERTY(CursorStyle, cursor, Const);

		/**
		 * Set the visibility of the view. When this value is set to 'false',
		 * the view is invisible and does not occupy any view space
		*/
		Qk_DEFINE_VIEW_PROPERTY(bool, visible, Const);

		/**
		 * 这个值与`visible`不相关，这个代表视图在当前显示区域是否可见，这个显示区域大多数情况下就是屏幕
		 * This value represents whether the view is visible in the current display area, which is mostly the screen
		*/
		Qk_DEFINE_PROP_GET(bool, visible_area, ProtectedConst);

		/**
		 * Do views need to receive or handle system event throws? In most cases,
		 * these events do not need to be handled, which can improve overall event processing efficiency
		*/
		Qk_DEFINE_PROPERTY(bool, receive, Const);

		/**
		 * Anti-aliasing rendering, default is true
		 */
		Qk_DEFINE_PROPERTY(bool, aa, Const);

		/**
		 * keyboard focus view
		*/
		Qk_DEFINE_ACCESSOR(bool, is_focus, Const);

		/**
		 * @method Make(Window*, ...)
		*/
		template<class T = View, typename... Args>
		static inline T* Make(Window* window, Args... args) {
			return static_cast<T*>(static_cast<View*>(new T(args...))->init(window));
		}

		template<class T = View, typename... Args>
		inline T* prepend_new(Args... args) {
			return Make<T>(_window, args...)->template prepend_to<T>(this);
		}

		template<class T = View, typename... Args>
		inline T* append_new(Args... args) {
			return Make<T>(_window, args...)->template append_to<T>(this);
		}

		template<class Return = View>
		inline Return* prepend_to(View* parent) {
			return parent->prepend(this), static_cast<Return*>(this);
		}

		template<class Return = View>
		inline Return* append_to(View* parent) {
			return parent->append(this), static_cast<Return*>(this);
		}

		/**
		 * @method destroy() heap memory destructor
		 */
		void destroy() override;

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
		 * @method is_self_child(child)
		 */
		bool is_self_child(View *child);

		/**
			*
			* Add a sibling view to the front
			*
			* @method before(view)
			*/
		void before(View *view);

		/**
			*
			* Add a sibling view to the back
			*
			* @method after(view)
			*/
		void after(View *view);

		/**
			* 
			* Append sub view to front
			* 
			* @method prepend(child)
			*/
		void prepend(View *child);

		/**
			*
			* Append sub view to end
			*
			* @method append(child)
			*/
		void append(View *child);

		/**
		 *
		 * Remove self from parent view
		 *
		 * @method remove()
		 */
		void remove();

		/**
		 *
		 * remove all sub view
		 *
		 * @method remove_all_child()
		 */
		void remove_all_child();

		/**
		 *
		 * Can it be the focus
		 *
		 * @method can_become_focus()
		 */
		virtual bool can_become_focus();

		/**
		 * 
		 * Returns button object
		 * 
		 * @method asButton()
		*/
		virtual Button* asButton();

		/**
		 * 
		 * Returns text input object
		 * 
		 * @method asTextInput()
		*/
		virtual TextInput* asTextInput();

		/**
		 * 
		 * Returns as matrix
		 * 
		 * @method asMorphView()
		*/
		virtual MorphView* asMorphView();

		/**
		 * Returns as entity
		*/
		virtual Entity* asEntity();

		/**
		 * Returns as agent
		*/
		virtual Agent* asAgent();

		/**
		 * 
		 * Returns as textOptions
		 * 
		 * @method asTextOptions()
		*/
		virtual TextOptions* asTextOptions();

		/**
		 * 
		 * Returns as ScrollView
		 * 
		 * @method asScrollView()
		*/
		virtual ScrollView* asScrollView();

		/**
			*
			* Layout weight (such as representing the size grow/shrink of the layout in a flex layout)
			*
			* @method layout_weight()
			*/
		virtual Vec2 layout_weight();

		/**
			*
			* Layout alignment (nine grid)
			*
			* @method layout_align()
			*/
		virtual Align layout_align();

		/**
		 *
		 * is clip render the view
		 *
		 * @method is_clip()
		 */
		virtual bool is_clip();

		/**
		 * @method viewType()
		*/
		virtual ViewType viewType() const;

		/**
			* 
			* Relative to the parent view (layout_offset) to start offset
			* 
			* @method layout_offset()
			* @thread Rt
			*/
		virtual Vec2 layout_offset();

		/**
			*
			* Returns the size of view object (if is box view the: size=margin+border+padding+content)
			*
			* @method layout_size()
			* @thread Rt
			*/
		virtual Vec2 layout_size();

		/**
		 * Returns the container size of view object, Includes size/max/min/wrap at ContainerSize
		 * 
		 * @method layout_container()
		*/
		virtual const Container& layout_container();

		/**
			* Returns internal view offset compensation of the view, which affects the sub view offset position
			* 
			* For example: when a view needs to set the scrolling property scroll of a sub view, you can set this property
			*
			* @method layout_offset_inside()
			* @thread Rt
			*/
		virtual Vec2 layout_offset_inside();

		/**
			* 
			* Setting the view offset of the view object in the parent view
			*
			* @method set_layout_offset(val)
			* @thread Rt
			*/
		virtual void set_layout_offset(Vec2 val);

		/**
			* 
			* Setting view offset values free mode for the view object
			*
			* @method set_layout_offset_free(size)
			* @thread Rt
			*/
		virtual void set_layout_offset_free(Vec2 size);

		/**
			* 强制固定布局的尺寸。在特定的布局类型中自身无法直接确定其自身尺寸，一般由父布局调用如：flex布局类型
			* Forces the layout to have a fixed size. In certain layout types, 
			* the layout itself cannot directly determine its own size, 
			* and is usually called by the parent layout, such as the flex layout type.
			* 
			* 返回固定后的最终尺寸.
			* Returns the final size after pinning.
			* 
			* @method layout_lock_width(size)
			* @thread Rt
			*/
		virtual float layout_lock_width(float size);

		/**
			* @method layout_lock_height(size)
			* @thread Rt
			*/
		virtual float layout_lock_height(float size);

		/**
			*
			* (计算布局自身的尺寸) 
			* (Calculate the size of the layout itself)
			*
			* 从外向内正向迭代布局，比如一些布局方法是先从外部到内部先确定盒子的明确尺寸
			* Iterate the layout from the outside in. For example,
			* some layout methods first determine the exact size of the box from the outside in.
			* 
			* @method layout_forward(mark)
			* @thread Rt
			*/
		virtual void layout_forward(uint32_t/*LayoutMark*/ mark);

		/**
			* 从内向外反向迭代布局，重新调整子视图偏移位置，并且如果视图为包裹尺寸时会被内部视图所挤压。
			* terate the layout from the inside out, readjust the subview offset position,
			* and if the view is the package size, it will be squeezed by the inner view.
			*
			* 这个方法被调用时子视图尺寸一定是明确的有效的，调用`layout_size()`返回子视图外框尺寸。
			* The subview size must be clear and valid when this method is called, 
			* and calling `layout_size()` returns the subview's outer frame size.
			*
			* @method layout_reverse(mark)
			* @thread Rt
			*/
		virtual void layout_reverse(uint32_t/*LayoutMark*/ mark);

		/**
		 * 
		 * solve text view
		 * 
		 * @method layout_text(lines)
		 * @thread Rt
		 */
		virtual void layout_text(TextLines *lines, TextConfig* cfg);

		/**
			* 
			* This method of the parent layout is called when the layout content of the child layout changes
			*
			* This is not necessarily called by the child layout
			*
			* @method onChildLayoutChange(child, mark)
			* @thread Rt
			*/
		virtual void onChildLayoutChange(View* child, uint32_t/*ChildLayoutChangeMark*/ mark);

		/**
		 * calling after text options change
		 * 
		 * @method text_config(cfg)
		 * @thread Rt
		*/
		virtual void text_config(TextConfig* cfg);

		/**
		 * Overlap test, test whether the point on the screen overlaps with the view
		 * @method overlap_test
		 * @thread Rt
		*/
		virtual bool overlap_test(Vec2 point);

		/**
		 * 
		 * Returns view client rect, for: (box = content + padding + border)
		 * 
		 * @method client_size()
		 * @thread Rt
		*/
		virtual Vec2 client_size();

		/**
		 * Returns the client region of the view, which includes content, padding, and border.
		 * @method client_region()
		 * @thread Rt
		*/
		virtual Region client_region();

		/**
		 * @method solve_marks(mark)
		 * @thread Rt
		*/
		virtual void solve_marks(const Mat &mat, View *parent, uint32_t mark);

		/**
			* @method solve_visible_area()
			* @thread Rt
			*/
		virtual void solve_visible_area(const Mat &mat);

		/**
		 * notice update for set parent or level
		 * 
		 * @method onActivate()
		 * @thread Rt
		*/
		virtual void onActivate();

		/**
		 * @method draw()
		 * @thread Rt
		 */
		virtual void draw(Painter *render);

		/**
			* @method mark(mark)
			*/
		void mark(uint32_t mark, bool isRt);

		/**
			* @method mark_layout(mark)
			*/
		void mark_layout(uint32_t mark, bool isRt);

		/**
			* @method unmark(mark)
			* @thread Rt
			*/
		inline void unmark(uint32_t mark = (~kLayout_None/*default unmark all*/)) {
			_mark_value &= (~mark);
		}

		/**
		 * @method preRender()
		*/
		PreRender& preRender();

		/**
		 * Safely use and hold view objects in rendering thread,
		 * Because view objects may be destroyed at any time on the main thread
		 * @method tryRetain() Returns safe self hold
		*/
		View* tryRetain_rt();

	protected:
		/**
		 * Compute the view is visible in the screen or the clipped region
		*/
		bool compute_visible_area(const Mat &mat, Vec2 bounds[4]);

		View(); // @constructor
		virtual View* init(Window* win);
	private:
		void set_parent(View *parent); // setting parent view
		void clear_link(); // Cleaning up associated view information
		void set_visible_rt(bool visible);
		void set_level_rt(uint32_t level); // settings depth
		void clear_level_rt(); //  clear view depth rt
		void applyClass_rt(CStyleSheetsClass* parentSsc); // apply class for self
		void applyClassAll_rt(CStyleSheetsClass* parentSsc); // apply class for all subviews
		CStyleSheetsClass* parentSsclass_rt();

		friend class Painter;
		friend class PreRender;
		friend class EventDispatch;
		friend class Root;
	};

}
#endif
