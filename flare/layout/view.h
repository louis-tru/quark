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
#include "../event.h"

F_NAMESPACE_START

#define F_Each_View(F) \
	F(View)  F(Box) \
	F(Image) F(Video) F(Scroll) \
	F(Input) F(Text) F(Label) F(Root) F(FlowLayout) F(FlexLayout)

#define F_Define_View(N) \
public: \
	virtual void accept(ViewVisitor *visitor) override { visitor->visit##N(this); } \

#define F_View(N) class N;
	F_Each_View(F_View);
#undef F_View

class ViewVisitor {
public:
#define F_Visitor(N) virtual void visit##N(N *v) = 0;
	F_Each_View(F_Visitor);
#undef F_Visitor
};

class Render;
class Action;

/**
	* The basic elements of UI tree
	*
	* @class View
	*/
class F_EXPORT View: public Notification<UIEvent, UIEventName, Layout> {
	F_HIDDEN_ALL_COPY(View);
public:

	/**
	 * @constructor
	 */
	View();

	/**
	 * @destructor
	 */
	virtual ~View();

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
	 * Append subview to parent
	 *
	 * @func append_to(parent)
	 */
	View* append_to(View* parent);

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
		* Accepting visitors
		* 
		* @func accept(visitor)
		*/
	virtual void accept(ViewVisitor *visitor);

	/**
	 * @func draw(canvas, alpha)
	 */
	// virtual void draw(Render* render);
	
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
		*
		* get is keyboard focus 
		*
		* @func is_focus()
		*/
	bool is_focus() const;

	/**
		*
		* setting focus value
		*
		* @func set_is_focus(value)
		*/
	void set_is_focus(bool value);
	
	/**
		*
		* Can it be the focus
		* 
		* @func can_become_focus()
		*/
	virtual bool can_become_focus();

	/**
	 * @overwrite
	 */
	virtual void trigger_listener_change(const NameType& name, int count, int change);

	/**
	 * @func has_child(child)
	 */
	bool has_child(View* child);
	
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
	inline float x() const { return translate()[0]; }

	/**
		* 
		* Returns y-axis matrix displacement for the view
		*
		* @func y()
		*/
	inline float y() const { return translate()[1]; }

	/**
		* 
		* Returns x-axis matrix scaling for the view
		*
		* @func scale_x()
		*/
	inline float scale_x() const { return scale()[0]; }

	/**
		* 
		* Returns y-axis matrix scaling for the view
		*
		* @func scale_y()
		*/
	inline float scale_y() const { return scale()[1]; }

	/**
		* 
		* Returns x-axis matrix skew for the view
		*
		* @func skew_x()
		*/
	inline float skew_x() const { return skew()[0]; }

	/**
		* 
		* Returns y-axis matrix skew for the view
		*
		* @func skew_y()
		*/
	inline float skew_y() const { return skew()[1]; }

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
		* 
		* Returns layout transformation matrix of the object view
		* 
		* Mat(layout_offset + transform_origin? + translate - parent->layout_offset_inside, scale, rotate, skew)
		* 
		* @func layout_matrix()
		*/
	virtual Mat layout_matrix();

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
		* @func solve_visible_region()
		*/
	virtual bool solve_visible_region();

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
	virtual void layout_typesetting_change(Layout* child, TypesettingChangeMark mark = T_NONE);

	/**
		*
		* Setting parent parent view
		*
		* @func set_parent(parent)
		*/
protected:
	virtual void set_parent(View* parent);

	// *******************************************************************

	// the objects that automatically adjust view properties
	F_DEFINE_PROP(Action*, action); // 在指定的时间内根据动作设定运行连续一系列的动作命令，达到类似影片播放效果
	F_DEFINE_PROP_READ(View*, parent);
	F_DEFINE_PROP_READ(View*, prev);
	F_DEFINE_PROP_READ(View*, next);
	F_DEFINE_PROP_READ(View*, first);
	F_DEFINE_PROP_READ(View*, last);

private:
	struct Transform {
		Vec2 translate, scale, skew; // 平移向量, 缩放向量, 倾斜向量
		float rotate; // z轴旋转角度值
	};
	Transform *_transform; // 矩阵变换
	Mat        _matrix; // 父视图矩阵乘以布局矩阵等于最终变换矩阵 (parent.matrix * layout_matrix)
	// can affect the transparency of subviews
	F_DEFINE_PROP(uint8_t, opacity); // 可影响子视图的透明度值
	// 视图是否需要接收或处理系统的事件抛出，大部情况下这些事件都是不需要处理的，这样可以提高整体事件处理效率
	// @prop Does the view need to receive or handle event throws from the system
	F_DEFINE_PROP(bool, receive);
	// 设置视图的可见性，这个值设置为`false`时视图为不可见且不占用任何布局空间
	F_DEFINE_PROP_READ(bool, visible);
	// 这个值与`visible`完全无关，这个代表视图在当前显示区域是否可见，这个显示区域大多数情况下就是屏幕
	F_DEFINE_PROP_READ(bool, visible_region);

	friend class Box;
	friend class SkiaRender;

	F_DEFINE_INLINE_CLASS(Inl);
	F_DEFINE_INLINE_CLASS(InlEvent);
};

F_NAMESPACE_END
#endif
