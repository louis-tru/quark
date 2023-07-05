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

#include "./pre_render.h"
#include "./layout/layout.h"
#include "./app.h"
#include "./text/text_opts.h"

namespace qk {

	void PreRender::addtask(Task* task) {
		if ( task->task_id() == Task::ID() ) {
			Task::ID id = _tasks.pushBack(task);
			task->_task_id = id;
			task->_pre = this;
		}
	}

	void PreRender::untask(Task* task) {
		Task::ID id = task->task_id();
		if ( id != Task::ID() ) {
			(*id)->_pre = nullptr;
			(*id)->_task_id = Task::ID();
			(*id) = nullptr;
		}
	}

	void PreRender::solve_mark() {
		TextConfig cfg(_host->default_text_options(), _host->default_text_options()->base());

		do {
			{ // forward iteration
				for (auto& levelMarks: _marks) {
					for (auto& layout: levelMarks) {
						if (layout) {
							if ( !layout->layout_forward(layout->layout_mark()) ) {
								// simple delete mark
								layout->_mark_index = -1;
								layout = nullptr;
								_mark_total--;
							}
						}
					}
				}
			}
			if (_mark_total > 0) { // reverse iteration
				for (int i = _marks.length() - 1; i >= 0; i--) {
					auto& levelMarks = _marks[i];
					for (auto& layout: levelMarks) {
						if (layout) {
							if ( !layout->layout_reverse(layout->layout_mark()) ) {
								// simple delete mark recursive
								layout->_mark_index = -1;
								layout = nullptr;
								_mark_total--;
							}
						}
					}
				}
			}
			Qk_ASSERT(_mark_total >= 0);
		} while (_mark_total);

		for (auto& levelMarks: _marks) {
			levelMarks.clear();
		}
		_is_render = true;
	}

	PreRender::PreRender(Application* host)
		: _host(host)
		, _mark_total(0)
		, _marks(0)
		, _is_render(false)
	{}

	PreRender::~PreRender() {}

	/**
	 * Work around flagging views that need to be updated
	 */
	bool PreRender::solve() {
		int64_t now_time = time_monotonic();
		// _host->action_direct()->advance(now_time); // advance action

		if ( _tasks.length() ) { // solve task
			auto i = _tasks.begin(), end = _tasks.end();
			while ( i != end ) {
				Task* task = *i;
				if ( task ) {
					if ( now_time > task->task_timeout() ) {
						if ( task->run_task(now_time) ) {
							_is_render = true;
						}
					}
					i++;
				} else {
					_tasks.erase(i++);
				}
			}
		}

		if (_mark_total) { // solve marks
			solve_mark();
		}

		if (_is_render) {
			_is_render = false;
			return true;
		} else {
			return false;
		}
	}

	void PreRender::mark(Layout *layout, uint32_t depth) {
		Qk_ASSERT(depth);
		_marks.extend(depth + 1);
		auto& arr = _marks[depth];
		layout->_mark_index = arr.length();
		arr.push(layout);
		_mark_total++;
	}

	void PreRender::delete_mark(Layout *layout, uint32_t depth) {
		Qk_ASSERT(depth);
		auto& arr = _marks[depth];
		auto last = arr[arr.length() - 1];
		if (last != layout) {
			arr[layout->_mark_index] = last;
		}
		arr.pop();
		layout->_mark_index = -1;
		_mark_total--;
		_is_render = true;
	}

	void PreRender::mark_none() {
		_is_render = true;
	}

	PreRender::Task::~Task() {
		if (_pre) {
			_pre->untask(this);
		}
	}

	void PreRender::Task::set_task_timeout(int64_t timeout_us) {
		_task_timeout = timeout_us;
	}
}
