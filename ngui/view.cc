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

XX_NS(ngui)

#define is_mark_pre m_prev_pre_mark
#define revoke_mark_value(mark_value, mark) mark_value &= ~(mark)

XX_DEFINE_INLINE_MEMBERS(ViewController, Inl) {
public:
	inline void set_view(View* view) { m_view = view; }
};

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
		clear_children();
	}
	
	/**
	 * @func clear_top # 清理top信息
	 */
	void clear_top() { // 清除弱引用
		if (m_top) {
			if ( ! m_id.is_empty() ) {
				m_top->controller()->del_member(m_id);
			}
			if (m_ctr) {
				String id = m_ctr->id();
				if ( ! id.is_empty() ) {
					m_top->controller()->del_member(id);
				}
			}
		}
	}

	/**
	 * @func clear_parent # 清理关联视图信息
	 */
	void clear_parent() {
		if (m_parent) {
			_inl(m_parent)->clear_children();
			
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
	 * @func del_old_top
	 * @arg old_top {View*}
	 */
	void del_old_top(View* old_top) throw(Error) {
		// 普通视图不允许单独离开原控制器,只有ViewController视图才可以.
		XX_ASSERT_ERR(controller(),
								 "Ordinary views are not allowed to leave the original controller alone, "
								 "Only Controller views can.");
		if ( ! m_id.is_empty() ) { // delete view id ref
			old_top->controller()->del_member(m_id);
		}
		String id = m_ctr->id();
		if ( ! id.is_empty() ) { // delete controller id ref
			old_top->controller()->del_member(id);
		}
	}
	
	/**
	 * @func set_top # 设置Top视图
	 */
	void set_top(View* top) throw(Error) {
		View* old_top = m_top;
		
		if (top) {
			if (old_top != top) {
				if (old_top) {
					del_old_top(old_top);
				}
				if ( ! m_id.is_empty()) { // 设置视图在所属控制器中的id引用
					top->controller()->set_member(m_id, this);
				}
				if (m_ctr) { // 设置控制器在所属控制器中的id引用
					String id = m_ctr->id();
					if ( ! id.is_empty() ) {
						top->controller()->set_member(id, m_ctr);
					}
				}
				m_top = top;
			}
		} else if (old_top) {
			del_old_top(old_top);
			m_top = NULL;
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
	
	void set_ctr(ViewController* ctr) throw(Error) {
		/*
		 * 在这里控制器视图必须为空.这可能是由于在事件中循环设置视图控制器造成的
		 */
		XX_ASSERT_ERR(ctr->view() == nullptr, ERR_CONTROLLER_LOOP_SET_VIEW_ERROR,
									"Loop settings view controller error");
		/*
		 * 一个视图最多只能有一个控制器并且不允许更换。但控制器可以更换不同的视图,视图被更换时旧的视图被删除。
		 * 与视图一样视图控制器也由父视图所保持,当所属的视图从父视图删除时,视图控制器也将被调用release()
		 */
		XX_ASSERT_ERR(m_ctr == nullptr, ERR_ONLY_VIEW_CONTROLLER_ERROR,
									"Views allow only a unique controller and are not allowed to change");
		
		m_ctr = ctr;
		Inl_ViewController(m_ctr)->set_view(this);
		
		if ( m_parent ) {
			ctr->retain(); // 被父视图保持
		}
	}
	
	void del_ctr() {
		XX_ASSERT( m_ctr );
		XX_ASSERT( m_ctr->view() == this );
		Inl_ViewController(m_ctr)->set_view(nullptr);
		m_ctr = nullptr;
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
	 * @func clear_children
	 */
	void clear_children() {
		if ( m_children ) {
			m_children->clear();
		}
	}
	
	/**
	 * @func gen_children
	 */
	void gen_children() {
		View* view = this->m_first;
		while (view) {
			m_children->push(view);
			view = view->m_next;
		}
	}

	/**
	 * @func gen_children2
	 */
	void gen_children2() {
		if ( m_first ) {
			if ( m_children ) {
				if ( m_children->length() == 0 ) {
					_inl(this)->gen_children();
				}
			} else {
				m_children = new Array<View*>();
				_inl(this)->gen_children();
			}
		} else {
			if ( !m_children ) {
				m_children = new Array<View*>();
			}
		}
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

inline static ViewController* get_parent_controller(View* view) {
	if (view) {
		view = view->top();
		if (view) {
			return view->controller();
		}
	}
	return nullptr;
}

/**
 * @func parent
 */
ViewController* ViewController::parent() {
	return get_parent_controller(view());
}

/**
 * @func load_view
 */
void ViewController::load_view(ViewXML* vx) {
	XX_WARN("unimplemented native load_view");
	XX_UNIMPLEMENTED();
}

ViewController::ViewController(): m_view(nullptr) {
	XX_ASSERT(RunLoop::is_main_loop());
	// LOG("ViewController");
}

ViewController::~ViewController() {
	if ( m_view ) {
		XX_ASSERT( m_view->m_ctr == this );
		// 控制器是由父视图所保持,如果这里还存在父视图那么这是错误的,被保持的对像不应该被析构
		XX_ASSERT( m_view->m_parent == nullptr );
		
		m_view->remove();
		if ( m_view ) {
			m_view->m_ctr = nullptr;
			m_view = nullptr;
		}
	}
}

/**
 * @func set_view
 */
void ViewController::view(View* view) throw(Error) {
	XX_ASSERT_ERR(view, ERR_CONTROLLER_VIEW_ERROR, "View cannot be empty");
	
	if ( view != m_view ) {
		View* old = m_view; m_view = nullptr;
		if (old) {
			if ( old->next() ) {
				Handle<ViewController> handle(this);
				View* next = old->next();
				old->remove();  // remove
				_inl(view)->set_ctr(this);
				next->before(view);
			} else if ( old->parent() ) {
				Handle<ViewController> handle(this);
				View* parent = old->parent();
				old->remove();  // remove
				_inl(view)->set_ctr(this);
				parent->append(view);
			} else {
				_inl(old)->remove();
				_inl(view)->set_ctr(this);
			}
		} else {
			_inl(view)->set_ctr(this);
		}
	}
}

String ViewController::id() const { return m_id; }

void ViewController::set_id(cString& value) throw(Error) {
	ViewController* ctr = get_parent_controller(view());
	if (ctr) {
		if ( value != m_id ) {
			ctr->del_member(m_id);
			m_id = value;
			if ( ! m_id.is_empty() ) {
				ctr->set_member(value, this);
			}
		}
	} else {
		m_id = value;
	}
}

/**
 * @overwrite
 */
Member* ViewController::find(cString& id) {
	XX_ASSERT(RunLoop::is_main_loop());
	auto i = m_members.find(id);
	if (i != m_members.end()) {
		return i.value();
	}
	return NULL;
}

/**
 * @overwrite
 */
void ViewController::set_member(cString& id, Member* member) throw(Error) {
	XX_ASSERT(RunLoop::is_main_loop());
	auto i = m_members.find(id);
	if (i != m_members.end()) {
		XX_ASSERT_ERR(member != i.value(), "ID member of the \"%s\" already exists", *id);
	} else {
		m_members.set(id, member);
	}
}

/**
 * @overwrite
 */
void ViewController::del_member(cString& id, Member* member) {
	XX_ASSERT(RunLoop::is_main_loop());
	if (member) {
		auto i = m_members.find(id);
		if (i != m_members.end()) {
			if (i.value() == member) {
				m_members.del(i);
			}
		}
	} else {
		m_members.del(id);
	}
}

/**
 * @func remove
 */
void ViewController::remove() {
	if ( m_view ) {
		m_view->remove();
	}
}

View::View()
: m_top(nullptr)
, m_parent(nullptr)
, m_prev(nullptr)
, m_next(nullptr)
, m_first(nullptr)
, m_last(nullptr)
, m_id()
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
, m_screen_visible(false)
, m_need_draw(true)
, m_child_change_flag(false)
, m_receive(false)
, m_ctr(nullptr)
, m_ctx_data(nullptr)
, m_action(nullptr)
, m_children(nullptr)
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
	
	if ( m_ctr ) {
		_inl(this)->del_ctr();
	}
	if ( is_mark_pre ) { // 删除标记
		_inl(this)->delete_mark();
	}
	
	if ((size_t)m_ctx_data > 0x1) {
		delete m_ctx_data; m_ctx_data = nullptr;
	}
	Release(m_children); m_children = nullptr;
	Release(m_classs); m_classs = nullptr;
}

/**
 * @func set_parent # 设置父视图
 */
void View::set_parent(View* parent) throw(Error) {
	// clear parent
	if (parent != m_parent) {
		// set top
		_inl(this)->set_top(parent->controller() ? parent : parent->m_top);
		_inl(this)->clear_parent();
		
		if ( !m_parent ) {
			retain(); // link to parent and retain ref
			if ( m_ctr )
				m_ctr->retain();
		}
		m_parent = parent;
		
		// 设置level
		int level = parent->m_level;
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
	if ( !(mark_value & M_REMOVING) ) {
		mark_value |= M_REMOVING;
		
		if (m_parent) {
			
			if ( m_ctr ) {
				XX_ASSERT( m_ctr->view() == this );
				m_ctr->trigger_remove_view(this); // 通知控制器
			}
			_inl(this)->inl_remove_all_child(); // 删除子视图
			
			blur(); // 辞去焦点
			
			trigger(GUI_EVENT_REMOVE_VIEW, true); // trigger remove view event
			
			if ( is_mark_pre ) { // 删除标记
				_inl(this)->full_delete_mark();
			}
			
			action(nullptr); // del action
			
			_inl(this)->clear_top();
			_inl(this)->clear_parent();
			
			ViewController* ctr = m_ctr;
			if (ctr) {
				_inl(this)->del_ctr();
				ctr->release();
			}
			
			off();
			m_level = 0;
			m_parent = m_top = m_prev = m_next = nullptr;
			revoke_mark_value(mark_value, M_REMOVING);
			release(); // Disconnect from parent view strong reference
		}
		else {
			// off();
			action(nullptr); // del action
			_inl(this)->inl_remove_all_child(); // 删除子视图
			if ( m_ctr ) 
				_inl(this)->del_ctr();
			revoke_mark_value(mark_value, M_REMOVING);
		}
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
 * @func inner_text
 */
void View::inner_text(cUcs2String& str) throw(Error) {
	remove_all_child();
	append_text( str );
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
 * #func append_to # 追加自身至父视图结尾
 * @arg parent {View*} # 父视图
 */
void View::append_to(View* parent) throw(Error) {
	parent->append(this);
}

/**
 * @get id {cString&} # 获取当前视图id
 * @const
 */
String View::id() const { return m_id; }

/**
 * @func set_id # 设置视图ID,会同时设置top控制器查询句柄.
 */
void View::set_id(cString& value) throw(Error) {
	if (m_top) {
		if (value != m_id) {
			m_top->controller()->del_member(m_id);
			m_id = value;
			if (!m_id.is_empty()) {
				m_top->controller()->set_member(value, this);
			}
		}
	} else {
		m_id = value;
	}
}

/**
 * @func before # 插入前
 * @arg view {View*} # 要插入的元素
 */
void View::before(View* view) throw(Error) {
	
	if (m_parent) {
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
 * @func move_to_before # 移动视图到上一个视图上面
 */
void View::move_to_before() {
	if (m_prev) {
		m_prev->before(this);
	}
}

/**
 * @func move_to_after # 移动视图到下一个视图下面
 */
void View::move_to_after() {
	if (m_next) {
		m_next->after(this);
	}
}

/**
 * @func move_to_first # 移动视图到所有视图的上面
 */
void View::move_to_first() {
	if (m_parent && m_parent->m_first) {
		m_parent->m_first->before(this);
	}
}

/**
 * @func move_to_last # 移动视图到所有视图的下面
 */
void View::move_to_last() {
	if (m_parent && m_parent->m_last) {
		m_parent->m_last->after(this);
	}
}

/**
 * @func remove_all_child # 删除所有子视图
 */
void View::remove_all_child() {
	_inl(this)->inl_remove_all_child();
}

/**
 * @func children
 */
View* View::children(uint index) {
	_inl(this)->gen_children2();
	if ( index < m_children->length() ) {
		return (*m_children)[index];
	}
	return nullptr;
}

/**
 * @func children_count
 */
uint View::children_count() {
	_inl(this)->gen_children2();
	return m_children->length();
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
	
	if ( m_screen_visible || need_draw ) {
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
 * @func set_screen_visible
 */
void View::set_screen_visible() {
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
		set_screen_visible();
	} else {
		if ( mark_value & M_OPACITY ) {
			m_final_opacity = m_parent->m_final_opacity * m_opacity;
		}
		if ( mark_value & M_SHAPE ) {
			set_screen_visible();
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
bool View::has_child(View* view) {
	if ( view && view->m_level < m_level ) {
		View* parent = view->m_parent;
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
