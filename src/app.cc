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

#include "./util/loop.h"
#include "./screen.h"
#include "./app.h"
#include "./render/font/pool.h"
#include "./render/source.h"
#include "./text/text_opts.h"
#include "./event.h"

Qk_EXPORT int (*__f_default_gui_main)(int, char**) = nullptr;
Qk_EXPORT int (*__f_gui_main)        (int, char**) = nullptr;

namespace qk {
	typedef Application::Inl AppInl;
	// thread helper
	static auto __run_main_wait = new CondMutex;

	// global shared gui application 
	Application* Application::_shared = nullptr;

	UILock::UILock(Application* host): _host(host), _lock(false) {
		lock();
	}

	UILock::~UILock() {
		unlock();
	}

	void UILock::lock() {
		if (!_lock) {
			_lock = true;
			_host->_render_mutex.lock();
		}
	}

	void UILock::unlock() {
		if (_lock) {
			_host->_render_mutex.unlock();
			_lock = false;
		}
	}

	Application::Application(RunLoop *loop)
		: Qk_Init_Event(Load)
		, Qk_Init_Event(Unload)
		, Qk_Init_Event(Background)
		, Qk_Init_Event(Foreground)
		, Qk_Init_Event(Pause)
		, Qk_Init_Event(Resume)
		, Qk_Init_Event(Memorywarning)
		, _is_loaded(false)
		, _loop(loop), _keep(nullptr)
		, _screen(nullptr)
		, _default_text_options(nullptr)
		, _font_pool(nullptr), _img_pool(nullptr)
		, _max_image_memory_limit(512 * 1024 * 1024) // init 512MB
	{
		if (_shared)
			Qk_FATAL("At the same time can only run a Application entity");
		_shared = this;

		Qk_On(ProcessExit, &Application::handleExit, this);
		// init
		_screen = New<Screen>(this); // strong ref
		_font_pool = FontPool::Make();
		_img_pool = new ImageSourcePool(this);
		_default_text_options = new DefaultTextOptions(_font_pool);
	}

	Application::~Application() {
		UILock lock(this);

		// TODO delete windows ..
		delete _default_text_options; _default_text_options = nullptr;
		Release(_screen);     _screen = nullptr;
		delete _keep;         _keep = nullptr;  _loop = nullptr;
		Release(_font_pool);  _font_pool = nullptr;
		Release(_img_pool);   _img_pool = nullptr;

		Qk_Off(ProcessExit, &Application::handleExit, this);

		_shared = nullptr;
	}

	void Application::run() {
		if (!_keep) {
			_keep = _loop->keep_alive("Application::run(), keep"); // keep loop
		}
		__run_main_wait->lock_notify_all(); // The external thread continues to run

		if (!_loop->runing()) {
			_loop->run(); // run message loop
			Qk_DEBUG("_loop->run() end");
		}
	}

	void Application::setMain(int (*main)(int, char**)) {
		__f_gui_main = main;
	}

	void Application::runMain(int argc, char* argv[]) {
		struct Args { int argc; char** argv; } arg = { argc, argv };

		// Create a new child worker thread. This function must be called by the main entry
		thread_new([](void* arg) {
			auto args = (Args*)arg;
			auto main = __f_gui_main ? __f_gui_main : __f_default_gui_main;
			Qk_ASSERT( main, "No gui main");
			int rc = main(args->argc, args->argv); // Run this custom gui entry function
			Qk_DEBUG("Application::runMain() thread_new() Exit");
			thread_try_abort_and_exit(rc); // if sub thread end then exit
			Qk_DEBUG("Application::runMain() thread_new() Exit ok");
		}, &arg, "runMain");

		// Block this main thread until calling Application::run()
		while (!_shared || !_shared->_keep) {
			__run_main_wait->lock_wait_for();
		}
	}

	void Application::clear(bool all) {
		UILock(this);
		_img_pool->clear(all);
		// TODO clear windows cache ..
	}

	uint64_t Application::max_image_memory_limit() const {
		return _max_image_memory_limit;
	}

	void Application::set_max_image_memory_limit(uint64_t limit) {
		_max_image_memory_limit = Qk_MAX(limit, 64 * 1024 * 1024);
	}

	uint64_t Application::used_image_memory() const {
		return _img_pool->total_data_size();
	}

	bool Application::adjust_image_memory(uint64_t will_alloc_size) {
		int i = 0;
		do {
			if (will_alloc_size + used_image_memory() <= _max_image_memory_limit) {
				return true;
			}
			clear();
			i++;
		} while(i < 3);
		
		Qk_WARN("Adjust image memory fail");
		
		return false;
	}

	void Application::handleExit(Event<>& e) {
		Qk_DEBUG("Application::handleExit(), rc=%d", static_cast<const Int32*>(e.data())->value);
		Inl_Application(this)->triggerUnload();
	}

	// ------------------- A p p l i c a t i o n :: I n l -------------------

	typedef void (*CbFunc) (Cb::Data&, AppInl*);

	void AppInl::triggerLoad() {
		_loop->post(Cb((CbFunc)[](Cb::Data& d, AppInl* app) {
			if (app->_is_loaded || !app->_keep)
				return;
			UILock lock(app);
			if (!app->_is_loaded) {
				app->_is_loaded = true;
				app->Qk_Trigger(Load);
			}
		}, this));
	}

	void AppInl::triggerUnload() {
		typedef Callback<RunLoop::PostSyncData> Cb;
		if (!_loop) return; // Block access after object is deleted

		_loop->post_sync(Cb([&](Cb::Data& d) {
			if (_keep) {
				if (_is_loaded) {
					_is_loaded = false;
					Qk_DEBUG("AppInl::onUnload()");
					Qk_Trigger(Unload);
				}
				thread_abort(_loop->thread_id()); // abort Signal
				delete _keep; // stop loop
				_keep = nullptr;
			}
			d.data->complete();
		}));
	}

	void AppInl::triggerPause() {
		_loop->post(Cb((CbFunc)[](Cb::Data& d, AppInl* app) { app->Qk_Trigger(Pause); }, this));
	}

	void AppInl::triggerResume() {
		_loop->post(Cb((CbFunc)[](Cb::Data& d, AppInl* app) { app->Qk_Trigger(Resume); }, this));
	}

	void AppInl::triggerBackground() {
		_loop->post(Cb((CbFunc)[](Cb::Data& d, AppInl* app) { app->Qk_Trigger(Background); }, this));
	}

	void AppInl::triggerForeground() {
		_loop->post(Cb((CbFunc)[](Cb::Data& d, AppInl* app) { app->Qk_Trigger(Foreground); }, this));
	}

	void AppInl::triggerMemorywarning() {
		clear();
		_loop->post(Cb((CbFunc)[](Cb::Data&, AppInl* app){ app->Qk_Trigger(Memorywarning); }, this));
	}

}
