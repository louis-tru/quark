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

// @private

#ifndef __quark__task__
#define __quark__task__

#include "../util/list.h"
#include "../util/array.h"
#include "../util/loop.h"

namespace qk {
	class Window;
	class View;
	class Layout;
	class PreRender;

	/**
	 * @class RenderTask render task
	*/
	class Qk_EXPORT RenderTask {
	public:
		typedef List<RenderTask*>::Iterator ID;
		// define props
		Qk_DEFINE_PROP_GET(ID, task_id);
		Qk_DEFINE_PROP_GET(PreRender*, pre);
		Qk_DEFINE_PROP(int64_t, task_timeout); // Unit is subtle
		RenderTask(): _task_timeout(0) {}
		virtual ~RenderTask();
		virtual bool run_task(int64_t sys_time) = 0;
		inline bool is_register_task() const { return _task_id != ID(); }
		friend class PreRender;
	};

	/**
	 * @class PreRender pre render for render thread
	*/
	class PreRender {
	public:
		typedef RenderTask Task;
		/*
		* @constructor
		*/
		PreRender(Window *window);

		/**
		 * NOTE only render thread call
		 * @method mark_layout
		 */
		void mark_layout(Layout *layout, uint32_t depth);
		void unmark_layout(Layout *layout, uint32_t depth);
		void mark_render(); // mark render state
		void addtask(Task* task); // add pre render task
		void untask(Task* task); // delete pre render task

		/**
		 * Issue commands from the main thread and execute them in the rendering thread
		 * 
		 * Note that the Args parameter size cannot exceed 16 bytes
		 * 
		 * @method async_call()
		*/
		template<typename E, typename Args, typename Ctx = Layout>
		inline void async_call(E exec, Ctx *ctx, Args args) {
			typedef void (*Exec)(Ctx *ctx, Args args);
			async_call_((void*)static_cast<Exec>(exec), ctx, &args);
		}

	private:
		/**
		 * Solve the pre-rendering problem, return true if the view needs to be updated
		 * @method solve()
		 */
		bool solve();
		void solveMarks(); // solve layout marks
		void clearTasks();
		void asyncCommit(); // commit async cmd to ready, only main thread call
		void solveAsyncCall();
		void async_call_(void *exec, void *ctx, void *args);

		struct AsyncCall {
			struct Args {uint64_t value[2];};
			void *ctx;
			Args args;
			void (*exec)(void *ctx, Args args);
		};
		Window *_window;
		int32_t _mark_total;
		List<Task*>  _tasks;
		Array<Array<Layout*>> _marks; // marked view
		Array<AsyncCall> _asyncCall;
		Array<AsyncCall> _asyncCommit;
		bool _is_render; // next frame render
		friend class Application;
		friend class Window;
	};

}
#endif
