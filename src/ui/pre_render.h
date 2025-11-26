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

#ifndef __quark__task__
#define __quark__task__

#include "../util/list.h"
#include "../util/array.h"
#include "../util/thread.h"

namespace qk {
	class Window;
	class View;
	class View;
	class PreRender;

	/**
	 * @class RenderTask render task
	*/
	class Qk_EXPORT RenderTask {
	public:
		typedef List<RenderTask*>::Iterator ID;
		// define props
		Qk_DEFINE_PROP_GET(ID, task_id, Const);
		Qk_DEFINE_PROP_GET(PreRender*, pre);
		Qk_DEFINE_PROPERTY(int64_t, task_timeout, Const); // Unit is subtle
		inline RenderTask(): _task_timeout(0) {}
		virtual ~RenderTask();
		virtual bool run_task(int64_t time, int64_t deltaTime) = 0;
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

		void mark_layout(View *view, uint32_t depth); //!< @thread Rt only render thread call
		void unmark_layout(View *view, uint32_t depth); //!< @thread Rt only render thread call
		void mark_render(); //!< mark render state @thread Rt only render thread call

		/** 
		 * add pre render task, if task already in pre render then ignore add
		 * @thread Rt only render thread call 
		 */
		void addtask(Task* task);
		/**
		 * delete pre render task, if task not in pre render then ignore delete
		 * @thread Rt only render thread call
		*/
		void untask(Task* task);

		/**
		 * @struct AsyncCall data
		*/
		template<typename Self = void, typename Arg = uint64_t>
		struct AsyncCall {
			void *exec;
			Self *self;
			Arg   arg;
			inline Self* operator->() { return self; }
			typedef void (*Exec)(Self*, AsyncCall&);
		};

		/**
		 * Issue commands from the main thread and execute them in the rendering thread
		 * 
		 * Note that the Args parameter size cannot exceed 8 bytes
		*/
		template<typename Self = View, typename Arg = uint64_t>
		inline void async_call(typename AsyncCall<Self,Arg>::Exec ex, Self *self, Arg arg) {
			static_assert(sizeof(Arg) <= sizeof(uint64_t), "");
			_asyncCall.push({(void*)ex,self,*(uint64_t*)&arg});
		}

		/**
		 * Safely release object in render thread and set nullptr
		 * @param obj {T*} object pointer to be released
		 * @thread First, Called in the first thread
		*/
		template<typename T>
		inline void safeReleasep(T *&obj) {
			async_call<T>([](auto obj, auto arg) { Release(obj); }, obj, 0);
			obj = nullptr;
		}

		/**
		 * Safely release atomic object in render thread and set nullptr
		 * @param obj {std::atomic<T*>} atomic object pointer to be released
		 * @thread First, Called in the first thread
		*/
		template<typename T>
		inline void safeReleasep(std::atomic<T*> &obj) {
			if (obj.load() == nullptr) return;
			auto v = obj.exchange(nullptr);
			async_call<T>([](auto obj, auto arg) { Release(obj); }, v, 0);
		}

		/**
		 * Post message to application work loop
		 * @param cb {Cb} callback to be executed in the work loop
		 * @param toQueue {bool} whether to add the callback to the queue
		 * @thread Rt/Ft
		*/
		void post(Cb cb, bool toQueue = false);

		/**
		 * Post message to application work loop
		 * @thread Rt/Ft
		 * @param view {View} safe retain view object to work loop, if retain fail then cancel call
		*/
		bool post(Cb cb, View *view, bool toQueue = false);

	private:
		/**
		 * Solve the pre-rendering problem, return true if the view needs to be updated
		 * @thread Rt
		 */
		bool solve(int64_t time, int64_t deltaTime);
		void clearTasks();
		void asyncCommit(); // commit async cmd to ready, only main thread call
		void solveAsyncCall();
		void flushAsyncCall();

		struct LevelMarks: Array<View*> {
			LevelMarks();
			Iterator begin();
			void clear();
			void pop(uint32_t count);
		};

		Window *_window;
		int32_t _mark_total;
		List<Task*> _tasks;
		Array<LevelMarks> _marks; // marked view layout
		Array<AsyncCall<>> _asyncCall;
		Array<AsyncCall<>> _asyncCommit;
		Mutex _asyncCommitMutex;
		bool _is_render; // next frame render
		friend class Application;
		friend class Window;
		friend class View;
	};

}
#endif
