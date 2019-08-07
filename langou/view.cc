/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright © 2015-2016, xuewen.chu
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

#include "view.h"
#include "label.h"
#include "draw.h"
#include "app.h"
#include "pre-render.h"
#include "root.h"
#include "css.h"
#include "panel.h"

XX_NS(langou)

#define is_mark_pre m_prev_pre_mark
#define revoke_mark_value(mark_value, mark) mark_value &= ~(mark)

/**
 * @class View::Inl
 */
XX_DEFINE_INLINE_MEMBERS(View, Inl) {
 public:
	#define _inl(self) static_cast<View::Inl*>(self)
	
	/**
	 * @func delete_mark 从原来的位置删除
	 */
	void delete_mark() {
		m_prev_pre_mark->m_next_pre_mark = m_next_pre_mark;
		m_next_pre_mark->m_prev_pre_mark = m_prev_pre_mark;
		m_prev_pre_mark = nullptr;
		m_next_pre_mark = nullptr;
	}
	
	/**
	 * @func safe_delete_mark 从原来的位置删除
	 */
	void safe_delete_mark() {
		if ( m_prev_pre_mark ) {
			m_prev_pre_mark->m_next_pre_mark = m_next_pre_mark;
			m_next_pre_mark->m_prev_pre_mark = m_prev_pre_mark;
			m_prev_pre_mark = nullptr;
			m_next_pre_mark = nullptr;
		}
	}
	
	/**
	 * @func full_delete_mark # 删除标记
	 */
	void full_delete_mark() {
		delete_mark();
		mark_value = M_NONE;
	}
	
	/**
	 * @func inl_remove_all_child
	 */
	void inl_remove_all_child() {
		while (m_first) {
			m_first->remove();
		}
	}
	
	/**
	 * @func clear_parent # 清理关联视图信息
	 */
	void clear_parent() {
		if (m_parent) {

			/* 当前为第一个子视图 */
			if (m_parent->m_first == this) {
				m_parent->m_first = m_next;
			}
			else {
				m_prev->m_next = m_next;
			}
			/* 当前为最后一个子视图 */
			if (m_parent->m_last == this) {
				m_parent->m_last = m_prev;
			}
			else {
				m_next->m_prev = m_prev;
			}
		}
	}

	/**
	 * @func set_level_and_visible # settings level and final visible
	 * @arg {int} level
	 */
	void set_level_and_visible(int level, bool visible) {
		m_level = level;
		m_final_visible = visible = visible && m_visible;

		if ( !visible ) {
			blur();
		}
		
		// 在标记库中存在
		if ( is_mark_pre ) {
			delete_mark();
			pre_render()->mark_pre(this);
		} else if ( mark_value ) {
			pre_render()->mark_pre(this);
		}
		
		level++;
		View* v = m_first;
		
		while ( v ) {
			_inl(v)->set_level_and_visible(level, visible);
			v = v->m_next;
		}
	}
	
	void set_level0_and_visible_false() {
		if ( m_level ) {
			m_level = 0;
			m_final_visible = false;
			blur();
			
			if ( is_mark_pre ) {
				delete_mark();
			}
			
			View* v = m_first;
			
			while ( v ) {
				_inl(v)->set_level0_and_visible_false();
				v = v->m_next;
			}
		}
	}
	
	/**
	 * @func set_final_visible_true
	 */
	void set_final_visible_true() {
		if ( m_visible && ! m_final_visible ) {
			m_final_visible = true;
			
			Layout* layout = as_layout();
			if ( layout && layout->mark_value ) {
				pre_render()->mark_pre(layout);
			}
			
			View* view = m_first;
			
			while (view) {
				_inl(view)->set_final_visible_true();
				view = view->m_next;
			}
		}
	}
	
	/**
	 * @func set_final_visible_false
	 */
	void set_final_visible_false() {
		
		if ( m_final_visible ) {
			m_final_visible = false;
			blur();
			
			View* view = m_first;
			
			while (view) {
				_inl(view)->set_final_visible_false();
				view = view->m_next;
			}
		}
	}
	
	/**
	 * @func compute_basic_transform_matrix
	 */
	XX_INLINE void compute_basic_transform_matrix() {
		// xy 布局偏移
		Vec2 offset = layout_offset();
		Vec2 layout_in = m_parent->layout_in_offset();
		offset.x( offset.x() + m_origin.x() + m_translate.x() - layout_in.x() );
		offset.y( offset.y() + m_origin.y() + m_translate.y() - layout_in.y() );
		// 更新基础矩阵
		m_matrix = Mat(offset, m_scale, -m_rotate_z, m_skew);
	}
	
	/**
	 * @func compute_final_matrix
	 */
	bool compute_final_matrix() {
		if ( m_parent ) {
			if ( _inl(m_parent)->compute_final_matrix() || (mark_value & M_TRANSFORM) ) {
				m_parent->m_final_matrix.multiplication(matrix(), m_final_matrix);
				revoke_mark_value(mark_value, M_TRANSFORM); //  Delete M_TRANSFORM
				return true;
			}
		} else {
			if ( mark_value & M_TRANSFORM ) {
				m_final_matrix = matrix();
				revoke_mark_value(mark_value, M_TRANSFORM); //  Delete M_TRANSFORM
				return true;
			}
		}
		return false;
	}
	
	/**
	 * @func compute_final_opacity
	 */
	bool compute_final_opacity() {
		if ( m_parent ) {
			if ( _inl(m_parent)->compute_final_opacity() || (mark_value & M_OPACITY) ) {
				m_final_opacity = m_parent->m_final_opacity * m_opacity;
				revoke_mark_value(mark_value, M_OPACITY); //  Delete M_OPACITY
				return true;
			}
		} else {
			if ( mark_value & M_OPACITY ) {
				m_final_opacity = m_opacity;
				revoke_mark_value(mark_value, M_OPACITY); //  Delete M_OPACITY
				return true;
			}
		}
		return false;
	}
	
	/**
	 * @func layout_offset_from
	 */
	Vec2 layout_offset_from(View* parents) {
		
		Vec2 offset = layout_offset();
		
		if ( m_parent && m_parent != parents ) {
			Vec2 parent_offset = _inl(m_parent)->layout_offset_from(parents);
			Vec2 parent_in_offset = _inl(m_parent)->layout_in_offset();
			
			return Vec2(offset.x() + parent_offset.x() - parent_in_offset.x() + m_origin.x(),
									offset.y() + parent_offset.y() - parent_in_offset.x() + m_origin.y() );
		}
		
		return offset;
	}
	
	void full_refresh_styles(StyleSheetsScope* scope) {

		if ( m_classs ) {
			m_classs->apply(scope);
		}
		
		View* v = m_first;
		if ( v ) {
			scope->push_scope(this);
			while (v) {
				_inl(v)->full_refresh_styles( scope );
				v = v->m_next;
			}
			scope->pop_scope();
		}

		revoke_mark_value(mark_value, M_STYLE);
	}
	
	void local_refresh_styles(StyleSheetsScope* scope) {
		if ( m_classs ) {
			bool effect_child = false;
			m_classs->apply(scope, &effect_child);
			
			if ( effect_child ) { // effect child
				
				View* v = m_first;
				if ( v ) {
					scope->push_scope(this);
					while (v) {
						_inl(v)->full_refresh_styles( scope );
						v = v->m_next;
					}
					scope->pop_scope();
				}
			}
		}
		revoke_mark_value(mark_value, M_STYLE);
	}
	
};

void _view_inl__safe_delete_mark(View* view) {
	_inl(view)->safe_delete_mark();
}

View::View()
: m_parent(nullptr)
, m_prev(nullptr)
, m_next(nullptr)
, m_first(nullptr)
, m_last(nullptr)
, m_translate()
, m_scale(1)
, m_skew(0)
, m_rotate_z(0)
, m_opacity(1)
, m_prev_pre_mark(nullptr)
, m_next_pre_mark(nullptr)
, m_classs(nullptr)
, m_level(0)
, m_matrix()
, m_origin()
, m_final_matrix()
, m_final_opacity(1)
, mark_value(0)
, m_visible(true)
, m_final_visible(false)
, m_draw_visible(false)
, m_need_draw(true)
, m_child_change_flag(false)
, m_receive(false)
, m_ctx_data(nullptr)
, m_action(nullptr)
{	
}

/**
 * @destructor
 */
View::~View() {

	XX_ASSERT(m_parent == nullptr); // 被父视图所保持的对像不应该被析构,这里parent必须为空
	
	blur();
	
	action(nullptr); // del action

	_inl(this)->inl_remove_all_child(); // 删除子视图
	
	if ( is_mark_pre ) { // 删除标记
		_inl(this)->delete_mark();
	}
	
	if ((size_t)m_ctx_data > 0x1) {
		delete m_ctx_data; m_ctx_data = nullptr;
	}
	Release(m_classs); m_classs = nullptr;
}

/**
 * @func set_parent # 设置父视图
 */
void View::set_parent(View* parent) throw(Error) {
	// clear parent
	if (parent != m_parent) {
		_inl(this)->clear_parent();
		
		if ( !m_parent ) {
			retain(); // link to parent and retain ref
		}
		m_parent = parent;
		
		// 设置level
		uint level = parent->m_level;
		if (level) {
			if ( level + 1 != m_level ) {
				_inl(this)->set_level_and_visible(level + 1, parent->m_final_visible);
			} else {
				if ( m_final_visible != parent->m_final_visible ) {
					if ( m_final_visible ) {
						_inl(this)->set_final_visible_false();
					} else {
						_inl(this)->set_final_visible_true();
					}
				}
			}
		} else {
			_inl(this)->set_level0_and_visible_false();
		}
		// 这些标记是必需的
		mark_pre( M_MATRIX | M_SHAPE | M_OPACITY | M_STYLE_FULL );
	}
}

/**
 * #func remove # 删除当前视图,并不从内存清空视图数据
 */
void View::remove() {		
	if (m_parent) {

		blur(); // 辞去焦点
		
		action(nullptr); // del action

		_inl(this)->inl_remove_all_child(); // 删除子视图
		
		if ( is_mark_pre ) { // 删除标记
			_inl(this)->full_delete_mark();
		}
		
		_inl(this)->clear_parent();
					
		remove_event_listener();
		m_level = 0;
		m_parent = m_prev = m_next = nullptr;
		release(); // Disconnect from parent view strong reference
	}
	else {
		// remove_event_listener();
		action(nullptr); // del action
		_inl(this)->inl_remove_all_child(); // 删除子视图
	}
}

/**
 * @func accept_text
 */
void View::accept_text(Ucs2StringBuilder& out) const {
	View* view = m_first;
	while (view) {
		view->accept_text(out);
		view = view->m_next;
	}
}

/**
 & @func inner_text
 */
Ucs2String View::inner_text() const {
	Ucs2StringBuilder str;
	accept_text(str);
	return str.to_basic_string();
}

/**
 * @func append_text
 */
View* View::append_text(cUcs2String& str) throw(Error) {
	Ucs2String str2 = str.trim();
	Label* label = New<Label>();
	append(label);
	label->set_value( str2 );
	return label;
}

/**
 * @func prepend # 前置元素
 * @arg child {View*} # 要前置的元素
 */
void View::prepend(View* child) throw(Error) {
	if (this == child->m_parent)
		_inl(child)->clear_parent();
	else
		child->set_parent(this);
	
	if (m_first) {
		child->m_prev = NULL;
		child->m_next = m_first;
		m_first->m_prev = child;
		m_first = child;
	}
	else { // 当前还没有子视图
		child->m_prev = NULL;
		child->m_next = NULL;
		m_first = child;
		m_last = child;
	}
}

/**
 * @func append # 追加元素至结尾
 * @arg child {View*} # 要追加的元素
 */
void View::append(View* child) throw(Error) {
	if (this == child->m_parent)
		_inl(child)->clear_parent();
	else
		child->set_parent(this);
	
	if (m_last) {
		child->m_prev = m_last;
		child->m_next = NULL;
		m_last->m_next = child;
		m_last = child;
	}
	else { // 当前还没有子视图
		child->m_prev = NULL;
		child->m_next = NULL;
		m_first = child;
		m_last = child;
	}
}

/**
 * @func before # 插入前
 * @arg view {View*} # 要插入的元素
 */
void View::before(View* view) throw(Error) {
	if (m_parent) {
		if (view == this) return;
		if (view->m_parent == m_parent)
			_inl(view)->clear_parent();  // 清除关联
		else
			view->set_parent(m_parent);
		
		if (!m_prev) { // 上面没有兄弟
			m_parent->m_first = view;
		}
		view->m_prev = m_prev;
		view->m_next = this;
		m_prev = view;
	}
	
}

/**
 * @func after # 插入后
 * @arg view {View*} # 要插入的元素
 */
void View::after(View* view) throw(Error) {
	if (m_parent) {
		if (view == this) return;
		if (view->m_parent == m_parent)
			_inl(view)->clear_parent(); // 清除关联
		else
			view->set_parent(m_parent);
		
		if (!m_next) { // 下面没有兄弟
			m_parent->m_last = view;
		}
		view->m_prev = this;
		view->m_next = m_next;
		m_next = view;
	}
}

/**
 * @func remove_all_child # 删除所有子视图
 */
void View::remove_all_child() {
	_inl(this)->inl_remove_all_child();
}

void View::set_x(float value) {
	m_translate.x(value);
	mark(M_MATRIX); // 标记基础变换
}

void View::set_y(float value) {
	m_translate.y(value);
	mark(M_MATRIX); // 标记基础变换
}

void View::set_scale_x(float value) {
	m_scale.x(value);
	mark(M_MATRIX); // 标记更新
}

void View::set_scale_y(float value) {
	m_scale.y(value);
	mark(M_MATRIX); // 标记更新
}

void View::set_rotate_z(float value) {
	m_rotate_z = value;
	mark(M_MATRIX); // 标记更新
}

void View::set_skew_x(float value) {
	m_skew.x(value);
	mark(M_MATRIX); // 标记更新
}

void View::set_skew_y(float value) {
	m_skew.y(value);
	mark(M_MATRIX); // 标记更新
}

void View::set_opacity(float value) {
	m_opacity = value;
	mark(M_OPACITY);
}

void View::set_visible(bool value) {
	if (m_visible != value) {
		m_visible = value;
		
		if (m_visible) {
			if ( m_parent && m_parent->m_final_visible ) { // 父视图的显示状态必须要为true才能生效
				_inl(this)->set_final_visible_true();
			}
		} else {
			_inl(this)->set_final_visible_false();
		}
		mark(M_VISIBLE);
	}
}

void View::set_translate(Vec2 value) {
	m_translate = value;
	mark(M_MATRIX); // 标记更新
}

void View::set_scale(Vec2 value) {
	m_scale = value;
	mark(M_MATRIX); // 标记更新
}

void View::set_skew(Vec2 skew) {
	m_skew = skew;
	mark(M_MATRIX); // 标记更新
}

void View::transform(Vec2 translate,
										 Vec2 scale,
										 float rotate_z, Vec2 skew) {
	m_translate = translate;
	m_scale = scale;
	m_rotate_z = rotate_z;
	m_skew = skew;
	mark(M_MATRIX); // 标记更新
}

void View::set_origin_x(float value) {
	if (m_origin.x() != value) {
		m_origin.x(value);
		View* view = m_first;
		while ( view ) {
			view->mark(M_MATRIX);
			view = view->m_next;
		}
		mark(M_MATRIX | M_SHAPE);
	}
}

void View::set_origin_y(float value) {
	if (m_origin.y() != value) {
		m_origin.y(value);
		// 变化这个属性,需要更新变换矩阵、顶点数据、还有子视图的矩阵变换
		View* view = m_first;
		while ( view ) {
			view->mark(M_MATRIX);
			view = view->m_next;
		}
		mark(M_MATRIX | M_SHAPE);
	}
}

/**
 * @func origin
 */
void View::set_origin(Vec2 value) {
	if (m_origin.y() != value.x() || m_origin.y() != value.y()) {
		m_origin = value;
		View* view = m_first;
		while ( view ) {
			view->mark(M_MATRIX);
			view = view->m_next;
		}
		mark(M_MATRIX | M_SHAPE);
	}
}

/**
 * @func force_draw_child set
 */
void View::set_need_draw(bool value) {
	if ( value != m_need_draw ) {
		m_need_draw = value;
		mark(M_MATRIX);
	}
}

/**
 * @func visit child draw
 */
void View::visit(Draw* draw, uint inherit_mark, bool need_draw) {
	View* view = m_first;
	
	if ( m_draw_visible || need_draw ) {
		m_child_change_flag = false;
		while (view) {
			view->mark_value |= inherit_mark;
			view->draw(draw);
			view = view->m_next;
		}
	} else {
		if ( inherit_mark ) {
			while (view) {
				view->mark_value |= inherit_mark;
				view = view->m_next;
			}
		}
	}
}

/**
 * 标记该视图已经发生改变
 */
void View::mark(uint value) {
	mark_value |= value;
	View* parent = m_parent;
	while ( parent && !parent->m_child_change_flag ) {
		parent->m_child_change_flag = true;
		parent = parent->m_parent;
	}
}

/**
 * @func mark_pre
 */
void View::mark_pre(uint value) {
	View::mark(value);
	pre_render()->mark_pre(this);
}

/**
 * @func overlap_test 重叠测试,测试屏幕上的点是否与视图重叠
 */
bool View::overlap_test(Vec2 point) {
	return false;
}

/**
 * @func overlap_test_from_convex_quadrilateral
 */
bool View::overlap_test_from_convex_quadrilateral(Vec2* quadrilateral_vertex, Vec2 point) {
	/*
	 * 直线方程：(x-x1)(y2-y1)-(y-y1)(x2-x1)=0
	 * 平面座标系中凸四边形内任一点是否存在：
	 * [(x-x1)(y2-y1)-(y-y1)(x2-x1)][(x-x4)(y3-y4)-(y-y4)(x3-x4)] < 0  and
	 * [(x-x2)(y3-y2)-(y-y2)(x3-x2)][(x-x1)(y4-y1)-(y-y1)(x4-x1)] < 0
	 */
	
	float x = point.x();
	float y = point.y();
	
	#define x1 quadrilateral_vertex[0].x()
	#define y1 quadrilateral_vertex[0].y()
	#define x2 quadrilateral_vertex[1].x()
	#define y2 quadrilateral_vertex[1].y()
	#define x3 quadrilateral_vertex[2].x()
	#define y3 quadrilateral_vertex[2].y()
	#define x4 quadrilateral_vertex[3].x()
	#define y4 quadrilateral_vertex[3].y()
	
	if (((x-x1)*(y2-y1)-(y-y1)*(x2-x1))*((x-x4)*(y3-y4)-(y-y4)*(x3-x4)) < 0 &&
			((x-x2)*(y3-y2)-(y-y2)*(x3-x2))*((x-x1)*(y4-y1)-(y-y1)*(x4-x1)) < 0
	) {
		return true;
	}
	
	#undef x1
	#undef y1
	#undef x2
	#undef y2
	#undef x3
	#undef y3
	#undef x4
	#undef y4
	
	return false;
}

/**
 * @func screen_rect_from_convex_quadrilateral
 */
CGRect View::screen_rect_from_convex_quadrilateral(Vec2 quadrilateral_vertex[4]) {
	
	Region re = screen_region_from_convex_quadrilateral(quadrilateral_vertex);
	
	return { Vec2(re.x, re.y), Vec2(re.w, re.h) };
}

/**
 * @func screen_region_from_convex_quadrilateral
 */
Region View::screen_region_from_convex_quadrilateral(Vec2* quadrilateral_vertex) {
	
#define A quadrilateral_vertex[0]
#define B quadrilateral_vertex[1]
#define C quadrilateral_vertex[2]
#define D quadrilateral_vertex[3]
	
	Vec2 min, max, size;
	
	float w1 = fabs(A.x() - C.x());
	float w2 = fabs(B.x() - D.x());
	
	if (w1 > w2) {
		if ( A.x() > C.x() ) {
			max.x( A.x() ); min.x( C.x() );
		} else {
			max.x( C.x() ); min.x( A.x() );
		}
		if ( B.y() > D.y() ) {
			max.y( B.y() ); min.y( D.y() );
		} else {
			max.y( D.y() ); min.y( B.y() );
		}
		size = Vec2(w1, max.y() - min.y());
	} else {
		if ( B.x() > D.x() ) {
			max.x( B.x() ); min.x( D.x() );
		} else {
			max.x( D.x() ); min.x( B.x() );
		}
		if ( A.y() > C.y() ) {
			max.y( A.y() ); min.y( C.y() );
		} else {
			max.y( C.y() ); min.y( A.y() );
		}
		size = Vec2(w2, max.y() - min.y());
	}
	
#undef A
#undef B
#undef C
#undef D
	
	return {
		min.x(), min.y(),
		max.x(), max.y(),
		size.width(), size.height()
	};
}

/**
 * @func set_draw_visible
 */
void View::set_draw_visible() {
	// noop
}

/**
 * @func solve
 */
void View::solve() {
	uint mark_value = this->mark_value;
	
	if ( mark_value & M_BASIC_MATRIX ) {
		_inl(this)->compute_basic_transform_matrix(); // 计算基础矩阵
	}
	
	if ( mark_value & M_TRANSFORM ) {
		m_parent->m_final_matrix.multiplication(m_matrix, m_final_matrix);
		m_final_opacity = m_parent->m_final_opacity * m_opacity; // 最终的不透明度
		set_draw_visible();
	} else {
		if ( mark_value & M_OPACITY ) {
			m_final_opacity = m_parent->m_final_opacity * m_opacity;
		}
		if ( mark_value & M_SHAPE ) {
			set_draw_visible();
		}
	}
}

/**
 * @func draw
 */
void View::draw(Draw* draw) {
	if ( m_visible ) {
		
		if ( mark_value ) {
			if ( mark_value & M_BASIC_MATRIX ) { // 基础变换
				_inl(this)->compute_basic_transform_matrix(); // 计算基础矩阵
			}
			
			if ( mark_value & M_TRANSFORM ) {
				m_parent->m_final_matrix.multiplication(m_matrix, m_final_matrix);
			}
			
			if ( mark_value & M_OPACITY ) {
				m_final_opacity = m_parent->m_final_opacity * m_opacity;
			}
		}
		
		visit(draw);
		
		mark_value = M_NONE;
	}
}

/**
 * @func layout_offset 获取布局偏移值
 */
Vec2 View::layout_offset() {
	return Vec2();
}

/**
 * @func layout_in_offset
 */
Vec2 View::layout_in_offset() {
	return m_origin;
}

/**
 * @func layout_offset
 */
Vec2 View::layout_offset_from(View* parents) {
	return _inl(this)->layout_offset_from(parents);
}

/**
 * @func screen_rect
 */
CGRect View::screen_rect() {
	return { position(), Vec2(0, 0) };
}

/**
 * @func matrix 基础矩阵,通过计算从父视图矩阵开始的位移,缩放,旋转,歪斜得到的矩阵。
 */
const Mat& View::matrix() {
	if ( mark_value & M_BASIC_MATRIX ) {
		if ( m_parent ) {
			_inl(this)->compute_basic_transform_matrix(); // 计算基础矩阵
		} else {
			Vec2 offset = layout_offset();
			m_matrix = Mat(offset, m_scale, -m_rotate_z, m_skew);
		}
		revoke_mark_value(mark_value, M_BASIC_MATRIX); // Delete BASIC_MATRIX
	}
	return m_matrix;
}

/**
 * @func final_matrix
 */
const Mat& View::final_matrix() {
	_inl(this)->compute_final_matrix();
	return m_final_matrix;
}

/**
 * @func final_opacity
 */
float View::final_opacity() {
	_inl(this)->compute_final_opacity();
	return m_final_opacity;
}

/**
 * @func position
 */
Vec2 View::position() {
	const Mat& mat = final_matrix();
	return Vec2(mat[2], mat[5]);
}

/**
 * "cls1 clas2 clas3"
 * @func set_class
 */
void View::set_class(cString& name) {
	set_class(name.split(' '));
}

/**
 * @func set_class
 */
void View::set_class(const Array<String>& name) {
	if ( !m_classs ) {
		m_classs = new CSSViewClasss(this);
	}
	m_classs->name(name);
}

/**
 * @func add_class
 */
void View::add_class(cString& names) {
	if ( !m_classs ) {
		m_classs = new CSSViewClasss(this);
	}
	m_classs->add(names);
}

/**
 * @func remove_class
 */
void View::remove_class(cString& names) {
	if ( m_classs ) {
		m_classs->remove(names);
	}
}

/**
 * @func toggle_class
 */
void View::toggle_class(cString& names) {
	if ( !m_classs ) {
		m_classs = new CSSViewClasss(this);
	}
	m_classs->toggle(names);
}


/**
 * @func refresh_styles
 */
void View::refresh_styles(StyleSheetsScope* sss) {
	if ( mark_value & M_STYLE_FULL ) { // full
		_inl(this)->full_refresh_styles(sss);
	} else { // local
		_inl(this)->local_refresh_styles(sss);
	}
}

static Button* first_button_2(View* v) {
	v = v->first();
	
	while (v) {
		if ( v->final_visible() ) {
			auto btn = v->as_button();
			if ( btn ) {
				return btn;
			} else {
				Panel* panel = v->as_panel();
				
				if ( !panel || (panel->enable_select() && panel->allow_entry()) ) {
					Button* btn = first_button_2(v);
					if ( btn ) {
						return btn;
					}
				}
			}
		}
		v = v->next();
	}
	return nullptr;
}

/**
 * @func first_button
 */
Button* View::first_button() {
	return first_button_2(this);
}

/**
 * @func has_child(child)
 */
bool View::has_child(View* child) {
	if ( child && child->m_level < m_level ) {
		View* parent = child->m_parent;
		while (parent) {
			if ( parent == this ) {
				return true;
			}
			parent = parent->m_parent;
		}
	}
	return false;
}

XX_END
