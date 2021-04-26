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

	# define FX_Views(F) \
		F(View)       F(Box) \
		F(FlexLayout) F(FlowLayout) \
		F(GridLayout) F(Image) \
		F(Input)      F(Label) \
		F(Root)       F(Scroll) \
		F(Text)       F(Video) \

	# define FX_View_Class(E, N) class N;
		FX_Views(FX_View_Class);
	# undef  FX_View_Class

	class Action; // class Action
	class GLRender; // class GLRender
	class SkiaRender; // class SkiaRender

	# define FX_Define_View(N) \
		public: \
		friend class GLRender; \
		friend class SkiaRender; \
		virtual void accept(Visitor *visitor); \

	/**
	 * The basic elements of GUI tree
	 *
	 * @class View
	 */
	class FX_EXPORT View: public Reference {
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
		void prepend(View* child);

		/**
		 *
		 * Append subview to end
		 *
		 * @func append(child)
		 */
		void append(View* child);

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
		protected: virtual void set_parent(View* parent);

		/**
		 * @class Visitor
		 */
		public: class Visitor {
			public:
			# define FX_Visitor(E, N) virtual void visit##N(N *v);
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
		 * Returns final visibility for view
		 *
		 * @func visibility()
		 */
		inline bool visibility() const {
			return _visibility;
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
		 * Returns matrix displacement for the view
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
		 * @func rotate()
		 */
		inline float rotate() const {
			return _rotate;
		}

		/**
		 * 
		 * Returns x-axis matrix displacement for the view
		 *
		 * @func x()
		 */
		inline float x() const {
			return _translate.x();
		}

		/**
		 * 
		 * Returns y-axis matrix displacement for the view
		 *
		 * @func y()
		 */
		inline float y() const {
			return _translate.y();
		}

		/**
		 * 
		 * Returns x-axis matrix scaling for the view
		 *
		 * @func scale_x()
		 */
		inline float scale_x() const {
			return _scale.x();
		}

		/**
		 * 
		 * Returns y-axis matrix scaling for the view
		 *
		 * @func scale_y()
		 */
		inline float scale_y() const {
			return _scale.y();
		}

		/**
		 * 
		 * Returns x-axis matrix skew for the view
		 *
		 * @func skew_x()
		 */
		inline float skew_x() const {
			return _skew.x();
		}

		/**
		 * 
		 * Returns y-axis matrix skew for the view
		 *
		 * @func skew_y()
		 */
		inline float skew_y() const {
			return _skew.y();
		}

		/**
		 *
		 * Returns the can affect the transparency of subviews
		 *
		 * @func opacity()
		 */
		inline float opacity() {
			return _opacity;
		}

		// *******************************************************************
		/**
		 * Set the `action` properties of the view object
		 *
		 * @func set_action()
		 */
		void set_action(Action* val);

		/**
		 * Set the matrix `translate` properties of the view object
		 *
		 * @func set_translate()
		 */
		void set_translate(Vec2 val);

		/**
		 * Set the matrix `scale` properties of the view object
		 *
		 * @func set_scale()
		 */
		void set_scale(Vec2 val);

		/**
		 * Set the matrix `skew` properties of the view object
		 *
		 * @func set_skew()
		 */
		void set_skew(Vec2 val);

		/**
		 * Set the z-axis  matrix `rotate` properties the view object
		 *
		 * @func set_rotate()
		 */
		void set_rotate(float val);

		/**
		 * 
		 * Setting x-axis matrix displacement for the view
		 *
		 * @func x()
		 */
		void set_x(float val);

		/**
		 * 
		 * Setting y-axis matrix displacement for the view
		 *
		 * @func y()
		 */
		void set_y(float val);

		/**
		 * 
		 * Returns x-axis matrix scaling for the view
		 *
		 * @func scale_x()
		 */
		void scale_x(float val);

		/**
		 * 
		 * Returns y-axis matrix scaling for the view
		 *
		 * @func scale_y()
		 */
		void scale_y(float val);

		/**
		 * 
		 * Returns x-axis matrix skew for the view
		 *
		 * @func skew_x()
		 */
		void skew_x(float val);

		/**
		 * 
		 * Returns y-axis matrix skew for the view
		 *
		 * @func skew_y()
		 */
		void skew_y(float val);

		/**
		 * Set the `opacity` properties the view object
		 *
		 * @func set_opacity()
		 */
		void set_opacity(float val);

		/**
		 * 
		 * setting the layout weight of the view object
		 * 
		 * @func set_layout_weight(val)
		 */
		void set_layout_weight(float val);

		// *******************************************************************
		/**
		 *
		 * 从外向内正向迭代布局，比如一些布局方法是先从外部到内部先确定盒子的明确尺寸
		 * 
		 * 这个方法被调用时父视图尺寸一定是有效的，在调用`layout_content_size`时有两种情况，
		 * 返回`false`表示父视图尺寸是wrap的，返回`true`时表示父视图有明确的尺寸
		 * 
		 * @func layout_forward()
		 */
		virtual void layout_forward();

		/**
		 * 
		 * 从内向外反向迭代布局，比如有些视图外部并没有明确的尺寸，
		 * 尺寸是由内部视图挤压外部视图造成的，所以只能先明确内部视图的尺寸。
		 *
		 * 这个方法被调用时子视图尺寸一定是明确的有效的，调用`layout_size()`返回子视图外框尺寸。
		 * 
		 * @func layout_reverse()
		 */
		virtual void layout_reverse();

		/**
		 * 
		 * Setting the layout offset of the view object in the parent view
		 *
		 * @func set_layout_offset(val)
		 */
		void set_layout_offset(Vec2 val);

		/**
		 * 当一个父布局视图对其中所拥有的子视图进行布局时，为了调整各个子视图合适位置与尺寸，如有必要可以调用这个函数对子视图做尺寸限制
		 * 这个函数被调用后，子视图上任何调用尺寸更改的方法都应该失效，但应该记录更改的数值一旦解除锁定后之前更改尺寸属性才可生效
		 * 
		 * 调用`layout_size_lock(false)`解除锁定
		 * 
		 * 子类实现这个方法
		 * 
		 * @func layout_size_lock()
		 */
		virtual void layout_size_lock(bool lock, Vec2 layout_size = Vec2());

		/**
		 * 
		 * This method of the parent view is called when the layout weight of the child view changes
		 *
		 * @func layout_weight_change_notice_from_child(child)
		 */
		virtual void layout_weight_change_notice_from_child(View* child);

		/**
		 *
		 * This method of the parent view is called when the layout size of the child view changes
		 * 
		 * @func layout_size_change_notice_from_child()
		 */
		virtual void layout_size_change_notice_from_child(View* child);

		/**
		 * 
		 * This method of the child view is called when the layout size of the parent view changes
		 * 
		 * @func layout_size_change_notice_from_parent(parent)
		 */
		virtual void layout_size_change_notice_from_parent(View* parent);

		// *******************************************************************
		/**
		 * 
		 * Relative to the parent view (layout_offset) to start offset
		 * 
		 * @func layout_offset()
		 */
		inline Vec2 layout_offset() const {
			return _layout_offset;
		}

		/**
		 * 
		 * Relative to the parent view (layout_offset) to start offset，end position
		 * 
		 * @func layout_offset_end()
		 */
		inline Vec2 layout_offset_end() const {
			return _layout_offset + _layout_size;
		}

		/**
		 *
		 * Returns the layout size of view object (if is box view the: size=margin+border+padding+content)
		 *
		 * @func layout_size()
		 */
		Vec2 layout_size() const {
			return _layout_size;
		}

		/**
		 *
		 * Returns the layout content size of object view, 
		 * Returns false to indicate that the size is unknown,
		 * indicates that the size changes with the size of the subview, and the content is wrapped
		 *
		 * @func layout_content_size(size)
		 */
		virtual bool layout_content_size(Vec2& size);

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
		 * Returns layout transformation matrix of the object view
		 * 
		 * Mat(layout_offset + layout_origin + translate - parent->layout_offset_inside, scale, rotate, skew)
		 * 
		 * @func layout_matrix()
		 */
		Mat layout_matrix() const;

		/**
		 * Start the matrix transformation from this origin point
		 *
		 * @func layout_origin()
		 */
		inline Vec2 layout_origin() const {
			return _layout_origin;
		}

		/**
		 *
		 * 布局权重（比如在flex布局中代表布局的尺寸）
		 *
		 * @func layout_weight()
		 */
		inline float layout_weight() {
			return _layout_weight;
		}

		/**
		 * 
		 * Returns final transformation matrix of the view layout
		 *
		 * parent.transform_matrix * layout_matrix
		 * 
		 * @func transform_matrix()
		 */
		const Mat& transform_matrix();

		/**
		* @enum LayoutMark
		*/
		enum : uint32_t {
			L_NONE                  = 0,          /* 没有任何标记 */
			L_TRANSFORM             = (1 << 0),   /* 矩阵变换 */
			L_OPACITY               = (1 << 3),   /* 透明度 */
			L_VISIBLE               = (1 << 4),   /* 显示与隐藏 */
			L_LAYOUT                = (1 << 5),   //
		};

		// *******************************************************************
		// action:
		private: Action *_action; // 在指定的时间内根据动作设定运行连续一系列的动作命令，达到类似影片播放效果
		// node tree:
		private: View *_parent;
		private: View *_prev, *_next;
		private: View *_first, *_last;
		// layout mark change:
		private: View *_prev_mark, *_next_mark;
		/* 这些标记后的视图会在开始帧绘制前进行更新.
		*  需要这些标记的原因主要是为了最大程度的节省性能开销,因为程序在运行过程中可能会频繁的更新视图局部属性也可能视图很少发生改变.
		*  1.如果对每次更新如果都更新GPU中的数据那么对性能消耗那将是场灾难,那么记录视图所有的局部变化,待到到需要真正帧渲染时统一进行更新.
		*/
		private: uint32_t _layout_mark; /* 布局变化标记 */
		/* 视图在整个视图树中所处的层级，0表示还没有加入到应用程序唯一的视图树中,根视图为1 */
		private: uint32_t _level;
		// layout:
		/* 下一个预处理视图标记
		*  在绘图前需要调用`layout_forward`与`layout_reverse`处理这些被标记过的视图。
		*  同一时间不会所有视图都会发生改变,如果视图树很庞大的时候,
		*  如果涉及到布局时为了跟踪其中一个视图的变化就需要遍历整颗视图树,为了避免这种情况
		*  把标记的视图独立到视图外部按视图等级进行分类以双向环形链表形式存储(PreRender)
		*  这样可以避免访问那些没有发生改变的视图并可以根据视图等级顺序访问.
		*/
		protected: Vec2  _layout_origin; // 最终以该点 位移,缩放,旋转,歪斜
		private:   Vec2  _layout_offset; // 相对父视图的开始偏移位置（box包含margin值）
		protected: Vec2  _layout_size;   // 在布局中所占用的尺寸（margin+border+padding+content）
		private:  float  _layout_weight; // layout weight
		// transform:
		private: Vec2  _translate, _scale, _skew; // 平移向量, 缩放向量, 倾斜向量
		private: float _rotate;     // z轴旋转角度值
		private: float _opacity;    // 可影响子视图的透明度值
		private:  Mat  _transform_matrix; // 父视图矩阵乘以布局矩阵等于最终变换矩阵 (parent.transform_matrix * layout_matrix)
		// layout visible:
		private: bool _visible; // 设置视图的可见性，这个值设置为`false`时视图为不可见且不占用任何布局空间
		private: bool _visibility; // 视图的可见性，受`visible`影响
		private: bool _region_visible; // 这个值与`visible`完全无关，这个代表视图在当前显示区域是否可见，这个显示区域大多数情况下就是屏幕
		private: bool _receive; // 视图是否需要接收或处理系统的事件抛出，大部情况下这些事件都是不需要处理的，这样可以提高整体事件处理效率

		FX_DEFINE_INLINE_CLASS(Inl);
	};

}

#endif