// @private head
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

#ifndef __quark__pre_render__
#define __quark__pre_render__

#include "./util/util.h"
#include "./util/list.h"

/**
 * @ns quark
 */

namespace quark {

	class Application;
	class Layout;

	/**
	* @class PreRender 预渲染
	*/
	class Qk_EXPORT PreRender: public Object {
		Qk_HIDDEN_ALL_COPY(PreRender);
	public:

		PreRender(Application* host);
		virtual ~PreRender();

		class Qk_EXPORT Task {
		public:
			typedef List<Task*>::Iterator ID;
			inline Task(): _task_timeout(0) {}
			virtual ~Task();
			virtual bool run_task(int64_t sys_time) = 0;
			void register_task();
			void unregister_task();
			inline bool is_register_task() const { return _task_id != ID(); }
			// define props
			Qk_DEFINE_PROP(ID, task_id);
			Qk_DEFINE_PROP(int64_t, task_timeout); // Unit is subtle
		};

		Qk_DEFINE_PROP_GET(Application*, host);

		/**
			* @func solve 解决预先渲染问题,如果需要更新视图返回true
			*/
		bool solve(int64_t now_time);

		/**
			* @func mark
			*/
		void mark(Layout *layout, uint32_t depth);
		void delete_mark(Layout *layout, uint32_t depth);
		void mark_none();

	private:
		void add_task(Task* task);
		void del_task(Task* task);
		void solve_mark();

		// member data
		int32_t _mark_total;
		List<Task*>  _tasks;
		Array<Array<Layout*>> _marks; // 被标记的视图
		bool _is_render;
	};

}
#endif

