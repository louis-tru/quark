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

#include "./pre-render.h"
#include "./draw.h"
#include "./div.h"
#include "./css.h"

namespace ftr {

	PreRender* PreRender::_pre_render(NULL);

	FX_DEFINE_INLINE_MEMBERS(PreRender, Inl) {
		public:
		
		void delete_mark_pre(View* view) {
			view->_prev_pre_mark->_next_pre_mark = view->_next_pre_mark;
			view->_next_pre_mark->_prev_pre_mark = view->_prev_pre_mark;
			view->_prev_pre_mark = nullptr;
			view->_next_pre_mark = nullptr;
		}
		
		void solve_pre() {
			
			if (_mark_pre) {
				
				// 1.从外至内,计算明确的布局宽度与高度
				
				// 忽略第0层级,第0层级的视图是无效的
				auto i = ++_marks.begin();
				
				StyleSheetsScope* sss = nullptr;
				
				// 解决所有的布局视图的size,与style
				for ( ; !i.is_null(); i++ ) {
					View* begin = i.value();
					View* view = begin->_next_pre_mark;
					
					while ( begin != view ) {
						
						if ( view->mark_value & View::M_STYLE ) { // style
							View* scope = view->parent();
							if ( sss ) {
								if ( sss->bottom_scope() != scope ) {
									Release(sss);
									sss = new StyleSheetsScope(scope);
								}
							} else {
								sss = new StyleSheetsScope(scope);
							}
							view->refresh_styles(sss);
						}
						
						View* next = view->_next_pre_mark;
						Layout* layout = view->as_layout();
						if ( layout ) {
							if (view->mark_value & View::M_LAYOUT) {
								layout->set_layout_explicit_size();
							}
						} else {
							delete_mark_pre(view);
						}
						view = next;
					}
				}
				
				Release(sss);
				
				// 2.从内至外,设置偏移并挤压不明确的高度与宽度
				
				i = --_marks.end(); auto end = _marks.begin();
				
				// 解决所有的布局视图的位置
				for ( ; i != end; i-- ) {
					View* begin = i.value();
					View* view = begin->_next_pre_mark;
					while (begin != view) {
						
						Layout* layout = static_cast<Layout*>(view);
						if ( view->mark_value & Box::M_CONTENT_OFFSET ) {
							layout->set_layout_content_offset();
						}
						
						View* next = view->_next_pre_mark;
						
						if ( view->mark_value & Box::M_LAYOUT_THREE_TIMES ) {
							
						} else {
							delete_mark_pre(view);
						}
						view = next;
					}
				}
				
				// 3.从外至内进一步最终解决局宽度与高度还有偏移位置
				i = ++_marks.begin();
				
				for ( ; !i.is_null(); i++ ) {
					View* begin = i.value();
					View* view = begin->_next_pre_mark;
					
					while (begin != view) {
						
						Layout* layout = static_cast<Layout*>(view);
						Layout* parent = layout->_parent_layout; ASSERT( parent );
						
						if ( parent->as_div() ) { // in div
							bool horizontal =
								(static_cast<Div*>(parent)->content_align() == ContentAlign::LEFT ||
								static_cast<Div*>(parent)->content_align() == ContentAlign::RIGHT);
							layout->set_layout_three_times(horizontal, false);
						} else {
							ASSERT( parent->as_hybrid() );
							layout->set_layout_three_times(true, true);
						}
						
						View* next = view->_next_pre_mark;
						view->_prev_pre_mark = nullptr; // 删除标记
						view->_next_pre_mark = nullptr;
						view = next;
					}
					begin->_prev_pre_mark = begin;
					begin->_next_pre_mark = begin;
				}
				
				_mark_pre = false;
			}
		}
		
		/**
		* @func add_task
		*/
		void add_task(Task* task) {
			if ( task->get_task_id().is_null() ) {
				Task::ID id = _tasks.push(task);
				task->set_task_id( id );
			}
		}
		
		/**
		* @func del_task
		*/
		void del_task(Task* task) {
			Task::ID id = task->get_task_id();
			if ( !id.is_null() ) {
				id.value()->set_task_id( Task::ID() );
				id.value() = nullptr;
			}
		}
		
	};

	/**
	* @class BeginView
	* @bases View
	*/
	class BeginView: public View {};

	/**
	* @constructor
	*/
	PreRender::PreRender() {
		ASSERT(!_pre_render); // "At the same time can only run a MarkManager entity"
		_pre_render = this;
		BeginView* begin = new BeginView();
		_marks.push(begin);
		begin->_prev_pre_mark = begin;
		begin->_next_pre_mark = begin;
	}

	/**
	* @destructor
	*/
	PreRender::~PreRender() {
		auto it = _marks.begin();
		auto end = _marks.end();
		for (; it != end; it++) {
			Release(it.value());
		}
		_pre_render = nullptr;
	}

	/**
	* @func mark_pre
	* @arg {View*} view
	*/
	void PreRender::mark_pre(View* view) {
		if ( view->_level ) {
			if ( !view->_next_pre_mark ) {
				if ( _marks.length() <= view->_level ) {
					int len = view->_level + 1;
					for (int i = _marks.length(); i < len; i++) {
						View* begin = new BeginView();
						_marks.push(begin);
						begin->_prev_pre_mark = begin;
						begin->_next_pre_mark = begin;
					}
				}
				View* begin = _marks[view->_level];
				view->_prev_pre_mark = begin->_prev_pre_mark;
				view->_next_pre_mark = begin;
				begin->_prev_pre_mark = view;
				view->_prev_pre_mark->_next_pre_mark = view;
			}
			_mark_pre = true;
		}
	}

	/**
	* 解决标记需要更新的视图
	*/
	bool PreRender::solve(int64_t now_time) {
		
		bool rv = false;
		
		if ( _tasks.length() ) { // solve task
			
			for ( auto i = _tasks.begin(), end = _tasks.end(); i != end; ) {
				auto j = i++;
				Task* task = j.value();
				if ( task ) {
					if ( now_time > task->get_task_timeout() ) {
						if ( task->run_task(now_time) ) {
							rv = true;
						}
					}
				} else {
					_tasks.del(j);
				}
			}
		}
		
		Inl_PreRender(this)->solve_pre();
	
		return rv;
	}

	PreRender::Task::~Task() {
		unregister_task();
	}

	void PreRender::Task::register_task() {
		if ( pre_render() ) {
			Inl_PreRender(pre_render())->add_task(this);
		}
	}

	void PreRender::Task::unregister_task() {
		if ( pre_render() ) {
			Inl_PreRender(pre_render())->del_task(this);
		}
	}

}
