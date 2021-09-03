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

#ifndef __flare__layout__view__
#define __flare__layout__view__

#include "./layout.h"

namespace flare {

	# define FX_Views(F) \
		F(View)           F(Box)             F(Image) \
		F(Flow)           F(Flex ) \
		F(Input)          F(Label)           F(Root) \
		F(Scroll)         F(Text)            F(Video) \

	# define FX_View_Class(N) class N;
		FX_Views(FX_View_Class);
	# undef  FX_View_Class

	# define FX_Define_View(N) \
		public: \
		friend class GLRender; \
		friend class SkiaRender; \
		virtual void accept(Visitor *visitor); \

	class Action; // class Action
	class GLRender; // class GLRender
	class SkiaRender; // class SkiaRender

	/**
		* The basic elements of GUI tree
		*
		* @class View
		*/
	class FX_EXPORT View: public Layout {
		FX_HIDDEN_ALL_COPY(View);
		public:

		friend class GLRender;
		friend class SkiaRender;

		/**
			* @constructors
			*/
		View();

		/**
			* @destructor
			*/
		virtual ~View();

		// *******************************************************************
		/**
			* parent view
			*
			* @func parent()
			*/
		inline View* parent() {
			return _parent;
		}

		/**
			* first subview
			*
			* @func first()
			*/
		inline View* first() {
			return _first;
		}

		/**
			* last subview
			*
			* @func last()
			*/
		inline View* last() {
			return _last;
		}

		/**
			* Previous sibling view
			*
			* @func prev()
			*/
		inline View* prev() {
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
			*
			* Add a sibling view to the front
			*
			* @func before(view)
			*/
		void before(View* view);

		/**
			*
			* Add a sibling view to the back
			*
			* @func after(view)
			*/
		void after(View* view);

		/**
			* 
			* Append subview to front
			* 
			* @func prepend(child)
			*/
		virtual void prepend(View* child);

		/**
			*
			* Append subview to end
			*
			* @func append(child)
			*/
		virtual void append(View* child);

		/**
			*
			* Remove and destroy self
			* 
			* @func remove()
			*/
		virtual void remove();

		/**
			*
			* remove all subview
			*
			* @func remove_all_child()
			*/
		virtual void remove_all_child();

		/**
			*
			* Setting parent parent view
			*
			* @func set_parent(parent)
			*/
		protected:
		virtual void set_parent(View* parent);

		/**
			* @class Visitor
			*/
		public: class Visitor {
			public:
			# define FX_Visitor(N) virtual void visit##N(N *v);
				FX_Views(FX_Visitor);
			# undef  FX_Visitor
		};

		/**
			*
			* Accepting visitors
			* 
			* @func accept(visitor)
			*/
		virtual void accept(Visitor *visitor);

		/**
			* @func visit(visitor)
			*/
		void visit(Visitor *visitor);

		/**
			*
			* Does the view need to receive or handle event throws from the system
			*
			* @func receive()
			*/
		inline bool receive() const {
			return _receive;
		}

		/**
			*
			* Returns visibility for the view
			*
			* @func visible()
			*/
		inline bool visible() const {
			return _visible;
		}

		/**
		* 
		* Returns region visibility for the view
		* 
		* @func region_visible
		*/
		inline bool region_visible() const {
			return _region_visible;
		}
		
		/**
			* 
			* Sets whether the view needs to receive or handle event throws from the system
			*
			* @func set_receive()
			*/
		void set_receive(bool val);

		/**
			* 
			* Setting the visibility properties the view object
			*
			* @func set_visible(val)
			*/
		virtual void set_visible(bool val);

		/**
			* 
			* focus keyboard
			*
			* @func focus()
			*/
		bool focus();
		
		/**
			*
			* Unfocus keyboard
			*
			* @func blur()
			*/
		bool blur();
		
		/**
			* @func is_focus()
			*/
		bool is_focus() const;
		
		/**
			*
			* Can it be the focus
			* 
			* @func can_become_focus()
			*/
		virtual bool can_become_focus();

		// *******************************************************************
		/**
			* Returns the objects that automatically adjust view properties
			*
			* @func action()
			*/
		inline Action* action() {
			return _action;
		}

		/**
			* Set the `action` properties of the view object
			*
			* @func set_action()
			*/
		void set_action(Action* val);

		/**
			* Returns matrix displacement for the view
			*
			* @func translate
			*/
		Vec2 translate() const;

		/**
			* Returns the Matrix scaling
			*
			* @func scale()
			*/
		Vec2 scale() const;

		/**
			* Returns the Matrix skew
			*
			* @func skew()
			*/
		Vec2 skew() const;

		/**
			* Returns the z-axis rotation of the matrix
			*
			* @func rotate()
			*/
		float rotate() const;

		/**
			* 
			* Returns x-axis matrix displacement for the view
			*
			* @func x()
			*/
		float x() const;

		/**
			* 
			* Returns y-axis matrix displacement for the view
			*
			* @func y()
			*/
		float y() const;

		/**
			* 
			* Returns x-axis matrix scaling for the view
			*
			* @func scale_x()
			*/
		inline float scale_x() const {
			return scale().x();
		}

		/**
			* 
			* Returns y-axis matrix scaling for the view
			*
			* @func scale_y()
			*/
		inline float scale_y() const {
			return scale().y();
		}

		/**
			* 
			* Returns x-axis matrix skew for the view
			*
			* @func skew_x()
			*/
		inline float skew_x() const {
			return skew().x();
		}

		/**
			* 
			* Returns y-axis matrix skew for the view
			*
			* @func skew_y()
			*/
		inline float skew_y() const {
			return skew().y();
		}

		/**
			*
			* Returns the can affect the transparency of subviews
			*
			* @func opacity()
			*/
		inline float opacity() const {
			return _opacity;
		}

		/**
			* Set the matrix `translate` properties of the view object
			*
			* @func set_translate(val)
			*/
		void set_translate(Vec2 val);

		/**
			* Set the matrix `scale` properties of the view object
			*
			* @func set_scale(val)
			*/
		void set_scale(Vec2 val);

		/**
			* Set the matrix `skew` properties of the view object
			*
			* @func set_skew(val)
			*/
		void set_skew(Vec2 val);

		/**
			* Set the z-axis  matrix `rotate` properties the view object
			*
			* @func set_rotate(val)
			*/
		void set_rotate(float val);

		/**
			* 
			* Setting x-axis matrix displacement for the view
			*
			* @func set_x(val)
			*/
		void set_x(float val);

		/**
			* 
			* Setting y-axis matrix displacement for the view
			*
			* @func set_y(val)
			*/
		void set_y(float val);

		/**
			* 
			* Returns x-axis matrix scaling for the view
			*
			* @func set_scale_x(val)
			*/
		void set_scale_x(float val);

		/**
			* 
			* Returns y-axis matrix scaling for the view
			*
			* @func set_scale_y(val)
			*/
		void set_scale_y(float val);

		/**
			* 
			* Returns x-axis matrix skew for the view
			*
			* @func set_skew_x(val)
			*/
		void set_skew_x(float val);

		/**
			* 
			* Returns y-axis matrix skew for the view
			*
			* @func set_skew_y(val)
			*/
		void set_skew_y(float val);

		/**
			* Set the `opacity` properties the view object
			*
			* @func set_opacity(val)
			*/
		void set_opacity(float val);

		// *******************************************************************

		/**
			* 
			* Calculate the transform origin value
			* 
			* @func solve_transform_origin()
			*/
		virtual Vec2 solve_transform_origin();

		/**
			* 
			* Returns layout transformation matrix of the object view
			* 
			* Mat(layout_offset + transform_origin + translate - parent->layout_offset_inside, scale, rotate, skew)
			* 
			* @func layout_matrix()
			*/
		Mat layout_matrix();

		/**
			* Start the matrix transformation from this origin point
			*
			* @func transform_origin()
			*/
		inline Vec2 transform_origin() const {
			return _transform_origin;
		}

		/**
			* 
			* Returns final transformation matrix of the view layout
			*
			* parent.matrix * layout_matrix
			* 
			* @func matrix()
			*/
		inline const Mat& matrix() const {
			return _matrix;
		}

		/**
			* @func solve_region_visible()
			*/
		virtual bool solve_region_visible();

		/**
		* @func overlap_test 重叠测试,测试屏幕上的点是否与视图重叠
		*/
		virtual bool overlap_test(Vec2 point);
		
		/**
		* @func overlap_test_from_convex_quadrilateral
		*/
		static bool overlap_test_from_convex_quadrilateral(Vec2 quadrilateral_vertex[4], Vec2 point);
		
		/**
		* @func screen_rect_from_convex_quadrilateral
		*/
		static Rect screen_rect_from_convex_quadrilateral(Vec2 quadrilateral_vertex[4]);
		
		/**
		* @func screen_region_from_convex_quadrilateral
		*/
		static Region screen_region_from_convex_quadrilateral(Vec2 quadrilateral_vertex[4]);

		/**
			* @overwrite
			*/
		virtual bool layout_forward(uint32_t mark);
		virtual bool layout_reverse(uint32_t mark);
		virtual void layout_recursive(uint32_t mark);
		virtual Vec2 layout_offset_inside();
		virtual void layout_typesetting_change(Layout* child, TypesettingChangeMark mark);

		// *******************************************************************

		// transform
		struct Transform {
			Vec2 translate, scale, skew; // 平移向量, 缩放向量, 倾斜向量
			float rotate; // z轴旋转角度值
		};

		private:
		Action *_action; // 在指定的时间内根据动作设定运行连续一系列的动作命令，达到类似影片播放效果
		// node tree:
		View *_parent;
		View *_prev, *_next;
		View *_first, *_last;
		Transform *_transform; // 矩阵变换
		Vec2  _transform_origin; // origin 最终以该点 位移,缩放,旋转,歪
		float _opacity; // 可影响子视图的透明度值
		Mat   _matrix; // 父视图矩阵乘以布局矩阵等于最终变换矩阵 (parent.matrix * layout_matrix)
		// layout visible:
		bool _visible; // 设置视图的可见性，这个值设置为`false`时视图为不可见且不占用任何布局空间
		bool _region_visible; // 这个值与`visible`完全无关，这个代表视图在当前显示区域是否可见，这个显示区域大多数情况下就是屏幕
		bool _receive; // 视图是否需要接收或处理系统的事件抛出，大部情况下这些事件都是不需要处理的，这样可以提高整体事件处理效率

		FX_DEFINE_INLINE_CLASS(Inl);
	};

}

#endif