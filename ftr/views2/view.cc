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

#include "./view.h"
#include "./label.h"
#include "../draw.h"
#include "../app.h"
#include "../_pre-render.h"
#include "./root.h"
#include "../css/css.h"
#include "./panel.h"

namespace ftr {

	#define is_mark_pre _prev_pre_mark
	#define revoke_mark_value(mark_value, mark) mark_value &= ~(mark)

	/**
	 * @class View::Inl
	 */
	FX_DEFINE_INLINE_MEMBERS(View, Inl) {
		public:
		#define _inl(self) static_cast<View::Inl*>(self)
		
		/**
		 * @func delete_mark 从原来的位置删除
		 */
		void delete_mark() {
			_prev_pre_mark->_next_pre_mark = _next_pre_mark;
			_next_pre_mark->_prev_pre_mark = _prev_pre_mark;
			_prev_pre_mark = nullptr;
			_next_pre_mark = nullptr;
		}
		
		/**
		 * @func safe_delete_mark 从原来的位置删除
		 */
		void safe_delete_mark() {
			if ( _prev_pre_mark ) {
				_prev_pre_mark->_next_pre_mark = _next_pre_mark;
				_next_pre_mark->_prev_pre_mark = _prev_pre_mark;
				_prev_pre_mark = nullptr;
				_next_pre_mark = nullptr;
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
			while (_first) {
				_first->remove();
			}
		}
		
		/**
		 * @func clear # 清理关联视图信息
		 */
		void clear() {
			if (_parent) {
				/* 当前为第一个子视图 */
				if (_parent->_first == this) {
					_parent->_first = _next;
				} else {
					_prev->_next = _next;
				}
				/* 当前为最后一个子视图 */
				if (_parent->_last == this) {
					_parent->_last = _prev;
				} else {
					_next->_prev = _prev;
				}
			}
		}

		/**
		 * @func set_level_and_visible # settings level and final visible
		 * @arg {int} level
		 */
		void set_level_and_visible(int level, bool visible) {
			_level = level;
			_final_visible = visible = visible && _visible;

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
			View* v = _first;
			
			while ( v ) {
				_inl(v)->set_level_and_visible(level, visible);
				v = v->_next;
			}
		}
		
		void set_level0_and_visible_false() {
			if ( _level ) {
				_level = 0;
				_final_visible = false;
				blur();
				
				if ( is_mark_pre ) {
					delete_mark();
				}
				
				View* v = _first;
				
				while ( v ) {
					_inl(v)->set_level0_and_visible_false();
					v = v->_next;
				}
			}
		}
		
		/**
		 * @func set_final_visible_true
		 */
		void set_final_visible_true() {
			if ( _visible && ! _final_visible ) {
				_final_visible = true;
				
				Layout* layout = as_layout();
				if ( layout && layout->mark_value ) {
					pre_render()->mark_pre(layout);
				}
				
				View* view = _first;
				
				while (view) {
					_inl(view)->set_final_visible_true();
					view = view->_next;
				}
			}
		}
		
		/**
		 * @func set_final_visible_false
		 */
		void set_final_visible_false() {
			
			if ( _final_visible ) {
				_final_visible = false;
				blur();
				
				View* view = _first;
				
				while (view) {
					_inl(view)->set_final_visible_false();
					view = view->_next;
				}
			}
		}
		
		/**
		 * @func compute_basic_transform_matrix
		 */
		FX_INLINE void compute_basic_transform_matrix() {
			// xy 布局偏移
			Vec2 offset = layout_offset();
			Vec2 layout_in = _parent->layout_in_offset();
			offset.x( offset.x() + _origin.x() + _translate.x() - layout_in.x() );
			offset.y( offset.y() + _origin.y() + _translate.y() - layout_in.y() );
			// 更新基础矩阵
			_matrix = Mat(offset, _scale, -_rotate_z, _skew);
		}
		
		/**
		 * @func compute_final_matrix
		 */
		bool compute_final_matrix() {
			if ( _parent ) {
				if ( _inl(_parent)->compute_final_matrix() || (mark_value & M_TRANSFORM) ) {
					_parent->_final_matrix.multiplication(matrix(), _final_matrix);
					revoke_mark_value(mark_value, M_TRANSFORM); //  Delete M_TRANSFORM
					return true;
				}
			} else {
				if ( mark_value & M_TRANSFORM ) {
					_final_matrix = matrix();
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
			if ( _parent ) {
				if ( _inl(_parent)->compute_final_opacity() || (mark_value & M_OPACITY) ) {
					_final_opacity = _parent->_final_opacity * _opacity;
					revoke_mark_value(mark_value, M_OPACITY); //  Delete M_OPACITY
					return true;
				}
			} else {
				if ( mark_value & M_OPACITY ) {
					_final_opacity = _opacity;
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
			
			if ( _parent && _parent != parents ) {
				Vec2 parent_offset = _inl(_parent)->layout_offset_from(parents);
				Vec2 parent_in_offset = _inl(_parent)->layout_in_offset();
				
				return Vec2(offset.x() + parent_offset.x() - parent_in_offset.x() + _origin.x(),
										offset.y() + parent_offset.y() - parent_in_offset.x() + _origin.y() );
			}
			
			return offset;
		}
		
		void full_refresh_styles(StyleSheetsScope* scope) {

			if ( _classs ) {
				_classs->apply(scope);
			}
			
			View* v = _first;
			if ( v ) {
				scope->push_scope(this);
				while (v) {
					_inl(v)->full_refresh_styles( scope );
					v = v->_next;
				}
				scope->pop_scope();
			}

			revoke_mark_value(mark_value, M_STYLE);
		}
		
		void local_refresh_styles(StyleSheetsScope* scope) {
			if ( _classs ) {
				bool effect_child = false;
				_classs->apply(scope, &effect_child);
				
				if ( effect_child ) { // effect child
					
					View* v = _first;
					if ( v ) {
						scope->push_scope(this);
						while (v) {
							_inl(v)->full_refresh_styles( scope );
							v = v->_next;
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
		: _parent(nullptr)
		, _prev(nullptr)
		, _next(nullptr)
		, _first(nullptr)
		, _last(nullptr)
		, _translate()
		, _scale(1)
		, _skew(0)
		, _rotate_z(0)
		, _opacity(1)
		, _prev_pre_mark(nullptr)
		, _next_pre_mark(nullptr)
		, _classs(nullptr)
		, _level(0)
		, _matrix()
		, _origin()
		, _final_matrix()
		, _final_opacity(1)
		, mark_value(0)
		, _visible(true)
		, _final_visible(false)
		, _draw_visible(false)
		, _need_draw(true)
		, _child_change_flag(false)
		, _receive(false)
		, _ctx_data(nullptr)
		, _action(nullptr)
	{}

	/**
	 * @destructor
	 */
	View::~View() {

		ASSERT(_parent == nullptr); // 被父视图所保持的对像不应该被析构,这里parent必须为空
		
		blur();
		
		action(nullptr); // del action

		_inl(this)->inl_remove_all_child(); // 删除子视图
		
		if ( is_mark_pre ) { // 删除标记
			_inl(this)->delete_mark();
		}
		
		if ((size_t)_ctx_data > 0x1) {
			delete _ctx_data; _ctx_data = nullptr;
		}
		Release(_classs); _classs = nullptr;
	}

	/**
	 * @func set_parent # 设置父视图
	 */
	void View::set_parent(View* parent) throw(Error) {
		// clear parent
		if (parent != _parent) {
			_inl(this)->clear();
			
			if ( !_parent ) {
				retain(); // link to parent and retain ref
			}
			_parent = parent;
			
			// 设置level
			uint32_t level = parent->_level;
			if (level) {
				if ( level + 1 != _level ) {
					_inl(this)->set_level_and_visible(level + 1, parent->_final_visible);
				} else {
					if ( _final_visible != parent->_final_visible ) {
						if ( _final_visible ) {
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
		if (_parent) {

			blur(); // 辞去焦点
			
			action(nullptr); // del action

			_inl(this)->inl_remove_all_child(); // 删除子视图
			
			if ( is_mark_pre ) { // 删除标记
				_inl(this)->full_delete_mark();
			}
			
			_inl(this)->clear();
			
			remove_event_listener();
			_level = 0;
			_parent = _prev = _next = nullptr;
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
	void View::accept_text(Array<String16>& out) const {
		View* view = _first;
		while (view) {
			view->accept_text(out);
			view = view->_next;
		}
	}

	/**
	 & @func inner_text
	 */
	String16 View::inner_text() const {
		Array<String16> list;
		accept_text(list);
		
		uint32_t totla = 0;
		
		for (int i = 0; i < list.length(); i++) {
			totla += list[i].length();
		}
		
		ArrayBuffer<uint16_t> text = ArrayBuffer<uint16_t>::alloc(totla);
		
		for (int j = 0; j < list.length(); j++) {
			text.write(list[j].c_str(), text.length(), list[j].length());
		}
		
		return text.collapse_string();
	}

	/**
	 * @func append_text
	 */
	View* View::append_text(cString16& str) throw(Error) {
		String16 str2 = str.trim();
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
		if (this == child->_parent)
			_inl(child)->clear();
		else
			child->set_parent(this);
		
		if (_first) {
			child->_prev = NULL;
			child->_next = _first;
			_first->_prev = child;
			_first = child;
		}
		else { // 当前还没有子视图
			child->_prev = NULL;
			child->_next = NULL;
			_first = child;
			_last = child;
		}
	}

	/**
	 * @func append # 追加元素至结尾
	 * @arg child {View*} # 要追加的元素
	 */
	void View::append(View* child) throw(Error) {
		if (this == child->_parent)
			_inl(child)->clear();
		else
			child->set_parent(this);
		
		if (_last) {
			child->_prev = _last;
			child->_next = NULL;
			_last->_next = child;
			_last = child;
		}
		else { // 当前还没有子视图
			child->_prev = NULL;
			child->_next = NULL;
			_first = child;
			_last = child;
		}
	}

	/**
	 * @func before # 插入前
	 * @arg view {View*} # 要插入的元素
	 */
	void View::before(View* view) throw(Error) {
		if (_parent) {
			if (view == this) return;
			if (view->_parent == _parent)
				_inl(view)->clear();  // 清除关联
			else
				view->set_parent(_parent);
			
			if (_prev) {
				_prev->_next = view;
			} else { // 上面没有兄弟
				_parent->_first = view;
			}
			view->_prev = _prev;
			view->_next = this;
			_prev = view;
		}
		
	}

	/**
	 * @func after # 插入后
	 * @arg view {View*} # 要插入的元素
	 */
	void View::after(View* view) throw(Error) {
		if (_parent) {
			if (view == this) return;
			if (view->_parent == _parent)
				_inl(view)->clear(); // 清除关联
			else
				view->set_parent(_parent);
			
			if (_next) {
				_next->_prev = view;
			} else { // 下面没有兄弟
				_parent->_last = view;
			}
			view->_prev = this;
			view->_next = _next;
			_next = view;
		}
	}

	/**
	 * @func remove_all_child # 删除所有子视图
	 */
	void View::remove_all_child() {
		_inl(this)->inl_remove_all_child();
	}

	void View::set_x(float value) {
		_translate.x(value);
		mark(M_MATRIX); // 标记基础变换
	}

	void View::set_y(float value) {
		_translate.y(value);
		mark(M_MATRIX); // 标记基础变换
	}

	void View::set_scale_x(float value) {
		_scale.x(value);
		mark(M_MATRIX); // 标记更新
	}

	void View::set_scale_y(float value) {
		_scale.y(value);
		mark(M_MATRIX); // 标记更新
	}

	void View::set_rotate_z(float value) {
		_rotate_z = value;
		mark(M_MATRIX); // 标记更新
	}

	void View::set_skew_x(float value) {
		_skew.x(value);
		mark(M_MATRIX); // 标记更新
	}

	void View::set_skew_y(float value) {
		_skew.y(value);
		mark(M_MATRIX); // 标记更新
	}

	void View::set_opacity(float value) {
		_opacity = value;
		mark(M_OPACITY);
	}

	void View::set_visible(bool value) {
		if (_visible != value) {
			_visible = value;
			
			if (_visible) {
				if ( _parent && _parent->_final_visible ) { // 父视图的显示状态必须要为true才能生效
					_inl(this)->set_final_visible_true();
				}
			} else {
				_inl(this)->set_final_visible_false();
			}
			mark(M_VISIBLE);
		}
	}

	void View::set_translate(Vec2 value) {
		_translate = value;
		mark(M_MATRIX); // 标记更新
	}

	void View::set_scale(Vec2 value) {
		_scale = value;
		mark(M_MATRIX); // 标记更新
	}

	void View::set_skew(Vec2 skew) {
		_skew = skew;
		mark(M_MATRIX); // 标记更新
	}

	void View::transform(Vec2 translate,
											 Vec2 scale,
											 float rotate_z, Vec2 skew) {
		_translate = translate;
		_scale = scale;
		_rotate_z = rotate_z;
		_skew = skew;
		mark(M_MATRIX); // 标记更新
	}

	void View::set_origin_x(float value) {
		if (_origin.x() != value) {
			_origin.x(value);
			View* view = _first;
			while ( view ) {
				view->mark(M_MATRIX);
				view = view->_next;
			}
			mark(M_MATRIX | M_SHAPE);
		}
	}

	void View::set_origin_y(float value) {
		if (_origin.y() != value) {
			_origin.y(value);
			// 变化这个属性,需要更新变换矩阵、顶点数据、还有子视图的矩阵变换
			View* view = _first;
			while ( view ) {
				view->mark(M_MATRIX);
				view = view->_next;
			}
			mark(M_MATRIX | M_SHAPE);
		}
	}

	/**
	 * @func origin
	 */
	void View::set_origin(Vec2 value) {
		if (_origin.y() != value.x() || _origin.y() != value.y()) {
			_origin = value;
			View* view = _first;
			while ( view ) {
				view->mark(M_MATRIX);
				view = view->_next;
			}
			mark(M_MATRIX | M_SHAPE);
		}
	}

	/**
	 * @func force_draw_child set
	 */
	void View::set_need_draw(bool value) {
		if ( value != _need_draw ) {
			_need_draw = value;
			mark(M_MATRIX);
		}
	}

	/**
	 * 标记该视图已经发生改变
	 */
	void View::mark(uint32_t value) {
		mark_value |= value;
		View* parent = _parent;
		while ( parent && !parent->_child_change_flag ) {
			parent->_child_change_flag = true;
			parent = parent->_parent;
		}
	}

	/**
	 * @func mark_pre
	 */
	void View::mark_pre(uint32_t value) {
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

	void View::set_draw_visible() {
		// noop
	}

	void View::solve() {
		uint32_t mark_value = this->mark_value;
		
		if ( mark_value & M_BASIC_MATRIX ) {
			_inl(this)->compute_basic_transform_matrix(); // 计算基础矩阵
		}
		
		if ( mark_value & M_TRANSFORM ) {
			_parent->_final_matrix.multiplication(_matrix, _final_matrix);
			_final_opacity = _parent->_final_opacity * _opacity; // 最终的不透明度
			set_draw_visible();
		} else {
			if ( mark_value & M_OPACITY ) {
				_final_opacity = _parent->_final_opacity * _opacity;
			}
			if ( mark_value & M_SHAPE ) {
				set_draw_visible();
			}
		}
	}

	void View::draw(Draw* draw) {
		if ( _visible ) {
			
			if ( mark_value ) {
				if ( mark_value & M_BASIC_MATRIX ) { // 基础变换
					_inl(this)->compute_basic_transform_matrix(); // 计算基础矩阵
				}
				
				if ( mark_value & M_TRANSFORM ) {
					_parent->_final_matrix.multiplication(_matrix, _final_matrix);
				}
				
				if ( mark_value & M_OPACITY ) {
					_final_opacity = _parent->_final_opacity * _opacity;
				}
			}
			
			visit(draw);
			
			mark_value = M_NONE;
		}
	}

	void View::visit(Draw* draw, uint32_t inherit_mark, bool need_draw) {
		View* view = _first;
		
		if ( _draw_visible || need_draw ) {
			_child_change_flag = false;
			while (view) {
				view->mark_value |= inherit_mark;
				view->draw(draw);
				view = view->_next;
			}
		} else {
			if ( inherit_mark ) {
				while (view) {
					view->mark_value |= inherit_mark;
					view = view->_next;
				}
			}
		}
	}

	Vec2 View::layout_offset() {
		return Vec2();
	}

	Vec2 View::layout_in_offset() {
		return _origin;
	}

	Vec2 View::layout_offset_from(View* parents) {
		return _inl(this)->layout_offset_from(parents);
	}

	CGRect View::screen_rect() {
		return { position(), Vec2(0, 0) };
	}

	/**
	 * @func matrix 基础矩阵,通过计算从父视图矩阵开始的位移,缩放,旋转,歪斜得到的矩阵。
	 */
	const Mat& View::matrix() {
		if ( mark_value & M_BASIC_MATRIX ) {
			if ( _parent ) {
				_inl(this)->compute_basic_transform_matrix(); // 计算基础矩阵
			} else {
				Vec2 offset = layout_offset();
				_matrix = Mat(offset, _scale, -_rotate_z, _skew);
			}
			revoke_mark_value(mark_value, M_BASIC_MATRIX); // Delete BASIC_MATRIX
		}
		return _matrix;
	}

	const Mat& View::final_matrix() {
		_inl(this)->compute_final_matrix();
		return _final_matrix;
	}

	float View::final_opacity() {
		_inl(this)->compute_final_opacity();
		return _final_opacity;
	}

	Vec2 View::position() {
		const Mat& mat = final_matrix();
		return Vec2(mat[2], mat[5]);
	}

	/**
	 * @func set_class("cls1 clas2 clas3")
	 */
	void View::set_class(cString& name) {
		set_class(name.split(' '));
	}

	void View::set_class(const Array<String>& name) {
		if ( !_classs ) {
			_classs = new StyleSheetsClass(this);
		}
		_classs->name(name);
	}

	void View::add_class(cString& names) {
		if ( !_classs ) {
			_classs = new StyleSheetsClass(this);
		}
		_classs->add(names);
	}

	void View::remove_class(cString& names) {
		if ( _classs ) {
			_classs->remove(names);
		}
	}

	void View::toggle_class(cString& names) {
		if ( !_classs ) {
			_classs = new StyleSheetsClass(this);
		}
		_classs->toggle(names);
	}

	void View::refresh_styles(StyleSheetsScope* sss) {
		if ( mark_value & M_STYLE_FULL ) { // full
			_inl(this)->full_refresh_styles(sss);
		} else { // local
			_inl(this)->local_refresh_styles(sss);
		}
	}

	bool View::has_child(View* child) {
		if ( child && child->_level < _level ) {
			View* parent = child->_parent;
			while (parent) {
				if ( parent == this ) {
					return true;
				}
				parent = parent->_parent;
			}
		}
		return false;
	}

}
