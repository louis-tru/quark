/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright (c) 2015, Louis.chu
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of Louis.chu nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL Louis.chu BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * ***** END LICENSE BLOCK ***** */

#include "./ui.h"
#include "../render/font/pool.h"
#include "../render/source.h"
#include "../render/canvas.h"
#include "./text/text_opts.h"
#include "./event.h"
#include "./css/css.h"

namespace qk {
	int (*__qk_run_main__)(int, char**) = nullptr;
	int (*__qk_run_main1__)(int, char**) = nullptr;

	// thread helper
	static auto _run_main_wait = new CondMutex;

	void view_prop_acc_init();
	cArray<Window*>* getWindowResidentPool();

	// global shared gui application 
	Application* Application::_shared = nullptr;

	Application::Application()
		: Qk_Init_Event(Load)
		, Qk_Init_Event(Unload)
		, Qk_Init_Event(Pause)
		, Qk_Init_Event(Resume)
		, Qk_Init_Event(Memorywarning)
		, _isLoaded(false)
		, _loop(first_loop())
		, _screen(nullptr)
		, _defaultTextOptions(nullptr)
		, _fontPool(nullptr), _imgPool(nullptr)
		, _maxResourceMemoryLimit(512 * 1024 * 1024) // init 512MB
		, _activeWindow(nullptr)
		, _clipboard(nullptr)
		, _tick(0), _timer(0)
	{
		Qk_CHECK(!_shared, "At the same time can only run a Application entity");
		check_is_first_loop();
		view_prop_acc_init();

		_shared = this;
		_clipboard = new Clipboard();
		_screen = new Screen(this); // strong ref
		_fontPool = shared_fontPool();
		_imgPool = shared_imgPool();
		_defaultTextOptions = new DefaultTextOptions(FontPool::shared());
		_run_main_wait->lock_and_notify_all(); // The external thread continues to run

		static uint32_t ticks = 0;
		struct Func {
			static void tick(Cb::Data& e, Application *self) {
				// static int64_t lastTime = 0;
				// static int64_t count = 0;
				// int64_t time = time_monotonic();
				// int64_t diff = time - lastTime;
				// lastTime = time;
				// Qk_Log("Tick delay: %lld ms, count: %lld", diff / 1000, count++);
				for (auto w: self->_windows) {
					w->pre_render().asyncCommit(); // commit async cmd to ready
				}
			}
			static void timer(Cb::Data& e, Application *self) {
				Inl_Application(self)->resolve_delay_tasks(false);
			}
		};

		// start tick check
		_tick = _loop->tick_prepare(Cb(&Func::tick, this), -1); // every loop tick
		// start delay tasks timer
		_timer = _loop->timer(Cb(&Func::timer, this), 5e3, -1); // 5 second timer
	}

	Application::~Application() {
		check_is_first_loop();
		// first clear all of image cache
		_imgPool->clear(true);
		// close all windows
		for (auto i = _windows.begin(), e = _windows.end(); i != e;) {
			(*(i++))->close(); // destroy
		}
		Qk_DLog("Application::~Application()");

		// Flush all pending asynchronous calls for every resident window.
		// This is required during application shutdown to ensure that no
		// delayed callbacks are left referencing logically destroyed windows
		// or released UI objects.
		Window::flushAsyncCall();

		_activeWindow =  nullptr;
		Inl_Application(this)->resolve_delay_tasks(true);
		Releasep(_defaultTextOptions);
		Releasep(_screen);
		Releasep(_clipboard);
	 	_loop->tick_stop(_tick); // stop tick
		_loop->timer_stop(_timer); // stop timer
		_shared = nullptr;
	}

	void Application::run() {
		if (!_loop->runing()) {
			_loop->run(); // run message loop
		}
	}

	void Application::setMain(int (*main)(int, char**)) {
		__qk_run_main1__ = main;
	}

	void Application::runMain(int argc, char* argv[], bool waitNewApp) {
		struct Args { int argc; char** argv; };
		// Create a new child worker thread. This function must be called by the main entry
		thread_new([](auto t, auto arg) {
			int rc = 0;
			auto args = (Args*)arg;
			auto main = __qk_run_main1__ ? __qk_run_main1__: __qk_run_main__;
			// Qk_CHECK(main, "Not found the Main function, Use Qk_Main() define");
			if (main)
				rc = main(args->argc, args->argv); // Run this custom gui entry function
			Qk_DLog("Application::runMain() thread_new() Exit");
			abort_exit(rc); // if sub thread end then exit
			Qk_DLog("Application::runMain() thread_new() Exit ok");
		}, new Args{argc, argv}, "Application::runMain");

		if (waitNewApp) {
			// Block this main thread until calling new Application
			while (!_shared) {
				_run_main_wait->lock_and_wait_for();
			}
		}
	}

	void Application::clear(bool all) {
		check_is_first_loop();
		for (auto i: _windows) {
			i->render()->getCanvas()->getPathvCache()->clear(all);
		}
		_imgPool->clear(all); // clear image cache
	}

	void Application::set_maxResourceMemoryLimit(uint64_t limit) {
		_maxResourceMemoryLimit = Qk_Max(limit, 64 * 1024 * 1024);
	}

	uint32_t Application::usedResourceMemory() const {
		auto capacity = _imgPool->capacity();
		for (auto i: _windows) {
			capacity += i->render()->getCanvas()->getPathvCache()->capacity();
		}
		return capacity;
	}

	const List<Window*>& Application::windows() const { //! window list
		return _windows;
	}

	void Application::lockAllRenderThreads(Cb cb) {
		ScopeLock lock(_mutex);
		Array<UILock*> locks;
		for (auto w: _windows) {
			locks.push(new UILock(w));
		}
		cb->resolve();
		for (auto lock: locks) {
			delete lock;
		}
	}

	// ------------------- A p p l i c a t i o n :: I n l -------------------

	typedef Cb::Static<AppInl> CbFunc;

	void AppInl::triggerLoad() {
		_loop->post(Cb((CbFunc)[](Cb::Data& d, AppInl* app) {
			if (!app->_isLoaded) {
				app->_isLoaded = true;
				app->Qk_Trigger(Load);
			}
		}, this), true);
	}

	void AppInl::triggerUnload() {
		_loop->post_sync(Callback<RunLoop::PostSyncData>([this](auto &d) {
			if (_isLoaded) {
				_isLoaded = false;
				Qk_Trigger(Unload);
				_loop->tick_stop(_tick);
				Qk_DLog("AppInl::onUnload()");
			}
			d.data->complete();
		}, this));
	}

	void AppInl::triggerPause() {
		_loop->post(Cb((CbFunc)[](Cb::Data& d, AppInl* app) { app->Qk_Trigger(Pause); }, this));
	}

	void AppInl::triggerResume() {
		_loop->post(Cb((CbFunc)[](Cb::Data& d, AppInl* app) { app->Qk_Trigger(Resume); }, this));
	}

	void AppInl::triggerMemorywarning() {
		_loop->post(Cb((CbFunc)[](Cb::Data&, AppInl* app){
			app->clear();
			app->Qk_Trigger(Memorywarning);
		}, this));
	}

	void AppInl::triggerBackground(Window *win) {
		_loop->post(Cb([win](Cb::Data& d) {
			win->Qk_Trigger(Background);
			Qk_Trigger(Background, win);
		}, win));
	}

	void AppInl::triggerForeground(Window *win) {
		_loop->post(Cb([win](Cb::Data& d) {
			win->Qk_Trigger(Foreground);
			Qk_Trigger(Foreground, win);
		}, win));
	}

	void AppInl::triggerOrientation() {
		_loop->post(Cb([](Cb::Data& d, AppInl* app) {
			app->screen()->Qk_Trigger(Orientation);
		}, this));
	}

	void AppInl::setActiveWindow(Window *win) {
		if (win == nullptr) { // key == nullptr, auto select key window
			if (_windows.length()) {
				_windows.front()->activate(); // select front window
				return;
			}
		}
		_activeWindow = win;
	}

	void AppInl::add_delay_task(Cb cb) {
		_delayTasksMutex.lock();
		_delayTasks.push_back({cb,2});
		_delayTasksMutex.unlock();
	}

	void AppInl::resolve_delay_tasks(bool all) {
		if (_delayTasks.length() == 0)
			return;
		_delayTasksMutex.lock();
		auto tasks = std::move(_delayTasks);
		_delayTasksMutex.unlock();
		if (all) {
			for (auto t: tasks)
				t.first->resolve();
			return;
		}
		for (auto begin = tasks.begin(); begin != tasks.end();) {
			auto t = begin++;
			if (--t->second == 0) {
				t->first->resolve();
				tasks.erase(t);
			}
		}
		if (tasks.length()) {
			_delayTasksMutex.lock();
			_delayTasks.splice(_delayTasks.begin(), tasks); // put back
			_delayTasksMutex.unlock();
		}
	}
}
