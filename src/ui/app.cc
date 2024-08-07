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

#include "../util/loop.h"
#include "./screen.h"
#include "./app.h"
#include "../render/font/pool.h"
#include "../render/source.h"
#include "../render/canvas.h"
#include "./text/text_opts.h"
#include "./event.h"
#include "./window.h"
#include "./css/css.h"

Qk_EXPORT int (*__qk_run_main__)(int, char**) = nullptr;

namespace qk {
	typedef Application::Inl AppInl;
	// thread helper
	static auto __run_main_wait = new CondMutex;

	void view_prop_acc_init();

	// global shared gui application 
	Application* Application::_shared = nullptr;

	Application::Application(RunLoop *loop)
		: Qk_Init_Event(Load)
		, Qk_Init_Event(Unload)
		, Qk_Init_Event(Pause)
		, Qk_Init_Event(Resume)
		, Qk_Init_Event(Memorywarning)
		, _isLoaded(false)
		, _loop(loop), _keep(nullptr)
		, _screen(nullptr)
		, _defaultTextOptions(nullptr)
		, _fontPool(nullptr), _imgPool(nullptr)
		, _maxResourceMemoryLimit(512 * 1024 * 1024) // init 512MB
		, _activeWindow(nullptr)
		, _styleSheets(nullptr)
	{
		if (_shared)
			Qk_FATAL("At the same time can only run a Application entity");
		if (!_loop)
			Qk_FATAL("The current thread does not have a RunLoop");
		view_prop_acc_init();
		_shared = this;
		_screen = new Screen(this); // strong ref
		_fontPool = FontPool::Make();
		_imgPool = new ImageSourcePool(_loop);
		_defaultTextOptions = new DefaultTextOptions(_fontPool);
		_styleSheets = new RootStyleSheets();
		__run_main_wait->lock_notify_all(); // The external thread continues to run

		// keep loop
		_keep = _loop->keep_alive([](void *ctx) {
			auto app = (Application*)(ctx);
			if (app->_windows.length()) {
				for(auto w: app->_windows) {
					w->preRender().asyncCommit();
				}
			}
		}, this); // keep loop
	}

	Application::~Application() {
		_mutex.lock();
		for (auto i = _windows.begin(), e = _windows.end(); i != e;) {
			(*(i++))->close(); // destroy
		}
		_activeWindow =  nullptr;
		Release(_defaultTextOptions); _defaultTextOptions = nullptr;
		Release(_styleSheets); _styleSheets = nullptr;
		Release(_screen);    _screen = nullptr;
		Release(_fontPool);  _fontPool = nullptr;
		Release(_imgPool);   _imgPool = nullptr;
		delete _keep;        _keep = nullptr;  _loop = nullptr;

		_shared = nullptr;
		_mutex.unlock();
	}

	void Application::run() {
		if (!_loop->runing()) {
			_loop->run(); // run message loop
		}
	}

	void Application::setMain(int (*main)(int, char**)) {
		__qk_run_main__ = main;
	}

	void Application::runMain(int argc, char* argv[]) {
		struct Args { int argc; char** argv; };
		// Create a new child worker thread. This function must be called by the main entry
		thread_new([](void* arg) {
			auto args = (Args*)arg;
			Qk_ASSERT( __qk_run_main__, "No gui main");
			int rc = __qk_run_main__(args->argc, args->argv); // Run this custom gui entry function
			Qk_DEBUG("Application::runMain() thread_new() Exit");
			thread_try_abort_and_exit(rc); // if sub thread end then exit
			Qk_DEBUG("Application::runMain() thread_new() Exit ok");
		}, new Args{argc, argv}, "runMain");

		// Block this main thread until calling new Application
		while (!_shared) {
			__run_main_wait->lock_wait_for();
		}
	}

	void Application::clear(bool all) {
		Qk_Fatal_Assert(thread_self_id() == _loop->thread_id());
		for (auto i: _windows) {
			i->render()->getCanvas()->getPathvCache()->clear(all);
		}
		_imgPool->clear(all); // clear image cache
	}

	void Application::set_maxResourceMemoryLimit(uint64_t limit) {
		_maxResourceMemoryLimit = Qk_MAX(limit, 64 * 1024 * 1024);
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
		for (auto w: _windows)
			locks.push(new UILock(w));
		cb->resolve();
		for (auto lock: locks)
			delete lock;
	}

	// ------------------- A p p l i c a t i o n :: I n l -------------------

	typedef void (*CbFunc) (Cb::Data&, AppInl*);

	void AppInl::triggerLoad() {
		_loop->post(Cb((CbFunc)[](Cb::Data& d, AppInl* app) {
			if (app->_isLoaded || !app->_keep)
				return;
			ScopeLock lock(app->_mutex); // TODO ... is safe, is release app ?
			if (!app->_isLoaded) {
				app->_isLoaded = true;
				app->Qk_Trigger(Load);
			}
		}, this));
	}

	void AppInl::triggerUnload() {
		_loop->post(Cb([&](auto&d) {
			if (_keep) {
				ScopeLock lock(_mutex); // TODO ... is safe, is release app ?
				if (_isLoaded) {
					_isLoaded = false;
					Qk_Trigger(Unload);
					Qk_DEBUG("AppInl::onUnload()");
				}
				delete _keep; _keep = nullptr; // stop loop run
			}
		}));
	}

	void AppInl::triggerPause() {
		_loop->post(Cb((CbFunc)[](Cb::Data& d, AppInl* app) { app->Qk_Trigger(Pause); }, this));
	}

	void AppInl::triggerResume() {
		_loop->post(Cb((CbFunc)[](Cb::Data& d, AppInl* app) { app->Qk_Trigger(Resume); }, this));
	}

	void AppInl::triggerMemorywarning() {
		clear();
		_loop->post(Cb((CbFunc)[](Cb::Data&, AppInl* app){ app->Qk_Trigger(Memorywarning); }, this));
	}

	void AppInl::triggerBackground(Window *win) {
		_loop->post(Cb([win](Cb::Data& d) { win->Qk_Trigger(Background); }, win));
	}

	void AppInl::triggerForeground(Window *win) {
		_loop->post(Cb([win](Cb::Data& d) { win->Qk_Trigger(Foreground); }, win));
	}

	void AppInl::setActiveWindow(Window *win) {
		if (!win) { // key == nullptr, auto select key window
			if (_windows.length()) {
				_windows.front()->activate(); // select front window
				return;
			}
		}
		_activeWindow = win;
	}

}
