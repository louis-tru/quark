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

#ifndef __qgr__pre_render__
#define __qgr__pre_render__

#include "utils/util.h"
#include "utils/array.h"
#include "utils/list.h"

/**
 * @ns qgr
 */

XX_NS(qgr)

class View;

/**
 * @class PreRender 预渲染
 */
class XX_EXPORT PreRender: public Object {
	XX_HIDDEN_ALL_COPY(PreRender);
 public:
	
	PreRender();
	
	virtual ~PreRender();
	
	class XX_EXPORT Task {
	 public:
		typedef List<Task*>::Iterator ID;
		inline Task(): m_timeout(0) { }
		virtual ~Task();
		virtual bool run_task(int64 sys_time) = 0;
		void register_task();
		void unregister_task();
		inline bool is_register_task() const { return !m_task_id.is_null(); }
		inline ID get_task_id() const { return m_task_id; }
		inline void set_task_id(ID id) { m_task_id = id; }
		inline int64 get_task_timeout() const { return m_timeout; }
		inline void set_task_timeout(int64 timeout_us) { m_timeout = timeout_us; }
	 private:
		ID    m_task_id;
		int64 m_timeout;
	};
	
	/**
	 * @func solve 解决预先渲染问题,如果需要更新视图返回true
	 */
	bool solve(int64 now_time);
	
	/**
	 * @func mark_pre
	 */
	void mark_pre(View* view);
	
 private:
	
	bool         m_mark_pre;    // 是否有layout标记
	Array<View*> m_marks;       // 被标记的视图
	List<Task*>  m_tasks;
	static PreRender* m_pre_render;
	
	friend PreRender* pre_render();
	
	XX_DEFINE_INLINE_CLASS(Inl)
};

/**
 * @func pre_render
 */
inline PreRender* pre_render() {
	return PreRender::m_pre_render;
}

XX_END
#endif

