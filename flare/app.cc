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

#include "./util/loop.h"
#include "./util/_working.h"
#include "./util/http.h"
// #include "./draw.h"
#include "./layout/root.h"
#include "./display-port.h"
#include "./_app.h"
// #include "./action/action.h"
// #include "./css/css.h"
#include "./font/pool.h"
#include "./_pre-render.h"
#include "../layout/text.h"

FX_EXPORT int (*__fx_default_gui_main)(int, char**) = nullptr;
FX_EXPORT int (*__fx_gui_main)(int, char**) = nullptr;

namespace flare {

	typedef GUIApplication::Inl AppInl;

	struct RunMainWait {
		void wait() {
			Lock lock(_thread_mutex);
			_thread_cond.wait(lock);
		}
		void awaken() {
			ScopeLock scope(_thread_mutex);
			_thread_cond.notify_all();
		}
		Mutex _thread_mutex;
		Condition _thread_cond;
	};

	// thread helper
	static auto *__run_main_wait = new RunMainWait();

	// global shared gui application 
	GUIApplication* GUIApplication::_shared = nullptr;

	GUILock::GUILock(GUIApplication* host): _host(host), _lock(false) {
		lock();
	}

	GUILock::~GUILock() {
		unlock();
	}

	void GUILock::lock() {
		if (!_lock) {
			_lock = true;
			_host->_gui_lock_mutex.lock();
		}
	}

	void GUILock::unlock() {
		if (_lock) {
			_host->_gui_lock_mutex.unlock();
			_lock = false;
		}
	}

	void AppInl::refresh_display() {
		_display_port->refresh();
	}

	void AppInl::triggerLoad() {
		if (!_is_load) {
			_is_load = true;
			_main_loop->post(Cb([&](CbData& d) { GUILock lock; FX_Trigger(Load); }));
		}
	}

	void AppInl::triggerRender() {
		_display_port->render_frame();
	}

	void AppInl::triggerPause() {
		_main_loop->post(Cb([&](CbData& d) { FX_Trigger(Pause); }));
	}

	void AppInl::triggerResume() {
		_main_loop->post(Cb([&](CbData& d) { FX_Trigger(Resume); }));
	}

	void AppInl::triggerBackground() {
		_main_loop->post(Cb([&](CbData& d) { FX_Trigger(Background); }));
	}

	void AppInl::triggerForeground() {
		_main_loop->post(Cb([&](CbData& d) { FX_Trigger(Foreground); }));
	}

	void AppInl::triggerMemorywarning() {
		clear();
		_main_loop->post(Cb([&](CbData&){ FX_Trigger(Memorywarning); }));
	}

	void AppInl::triggerUnload() {
		if (_is_load) {
			_is_load = false;
			typedef Callback<RunLoop::PostSyncData> Cb;
			_main_loop->post_sync(Cb([&](Cb::Data& d) {
				DLOG("AppInl::onUnload()");
				FX_Trigger(Unload);
				if (_root) {
					GUILock lock;
					_root->remove();
				}
				d.data->complete();
			}));
		}
	}

	/**
	* @func set_root
	*/
	void AppInl::set_root(Root* value) throw(Error) {
		FX_CHECK(!_root, "Root view already exists");
		_root = value;
		_root->retain();   // strong ref
		set_focus_view(value);
	}

	/**
	* @func set_focus_view()
	*/
	bool AppInl::set_focus_view(View* view) {
		if ( _focus_view != view ) {
			if ( view->final_visible() && view->can_become_focus() ) {
				if ( _focus_view ) {
					_focus_view->release();
				}
				_focus_view = view;
				_focus_view->retain(); // strong ref
				_dispatch->make_text_input(view->as_itext_input());
			} else {
				return false;
			}
		}
		return true;
	}

	/**
	* @func setMain()
	*/
	void GUIApplication::setMain(int (*main)(int, char**)) {
		ASSERT( !__fx_gui_main );
		__fx_gui_main = main;
	}

	/**
	* @func runMain()
	*/
	void GUIApplication::runMain(int argc, Char* argv[]) {
		static int _is_initialize = 0;
		ASSERT(!_is_initialize++, "Cannot multiple calls.");
		
		// 创建一个新子工作线程.这个函数必须由main入口调用
		Thread::spawn([argc, argv](Thread& t) {
			ASSERT( __fx_default_gui_main );
			auto main = __fx_gui_main ? __fx_gui_main : __fx_default_gui_main;
			__fx_default_gui_main = nullptr;
			__fx_gui_main = nullptr;
			int rc = main(argc, argv); // 运行这个自定gui入口函数
			DLOG("GUIApplication::start Exit");
			flare::exit(rc); // if sub thread end then exit
			return rc;
		}, "runMain");

		// 在调用GUIApplication::run()之前一直阻塞这个主线程
		while (!_shared || !_shared->_is_run)) {
			__run_main_wait->wait();
		}
	}

	void GUIApplication::run_loop() {
		ASSERT(!_is_run, "GUI program has been running");

		_is_run = true;
		_render_loop = RunLoop::current(); // 当前消息队列
		_render_keep = _render_loop->keep_alive("GUIApplication::run, render_loop"); // 保持

		if (_render_loop != _main_loop) {
			Inl2_RunLoop(_render_loop)->set_independent_mutex(&_gui_lock_mutex);
			Thread::awaken(_main_loop->thread_id()); // main loop awaken
		}
		__run_main_wait->awaken(); // 外部线程继续运行

		ASSERT(!_render_loop->runing());

		_render_loop->run(); // 运行gui消息循环,这个消息循环主要用来绘图

		Release(_render_keep); _render_keep = nullptr;

		_render_loop = nullptr;
		_is_run = false;
	}

	void GUIApplication::run_loop_on_new_thread() {
		ASSERT(RunLoop::is_main_loop()); // main loop call

		Thread::spawn([this](Thread& t) {
			DLOG("run render loop ...");
			run_loop();
			DLOG("run render loop end");
			return 0;
		}, "render_loop");

		Thread::sleep(); // main loop sleep, await run loop ok
	}

	static void on_process_safe_handle(Event<>& e, Object* data) {
		int rc = static_cast<const Int32*>(e.data())->value;
		if (app()) {
			e.return_value = _inl_app(app())->onExit(rc);
		}
	}

	int AppInl::onExit(int code) {
		if (_render_keep) {
			onUnload();
			auto render_loop_id = _render_loop->thread_id();
			Release(_render_keep); _render_keep = nullptr; // stop render loop
			Release(_main_keep); _main_keep = nullptr; // stop main loop
			Thread::abort(render_loop_id);
			DLOG("GUIApplication onExit");
		}
		return code;
	}

	GUIApplication::GUIApplication()
	: FX_Init_Event(Load)
	, FX_Init_Event(Unload)
	, FX_Init_Event(Background)
	, FX_Init_Event(Foreground)
	, FX_Init_Event(Pause)
	, FX_Init_Event(Resume)
	, FX_Init_Event(Memorywarning)
	, _is_run(false)
	, _is_load(false)
	, _render_loop(nullptr)
	, _main_loop(RunLoop::main_loop())
	, _render_keep(nullptr)
	, _main_keep(nullptr)
	// , _draw_ctx(nullptr)
	, _display_port(nullptr)
	, _root(nullptr)
	, _focus_view(nullptr)
	, _default_text_settings(new DefaultTextSettings())
	, _dispatch(nullptr)
	// , _action_center(nullptr)
	, _pre_render(nullptr)
	{
		_main_keep = _main_loop->keep_alive("GUIApplication::GUIApplication(), main_keep");
		Thread::FX_On(ProcessSafeExit, on_process_safe_handle);
	}

	GUIApplication::~GUIApplication() {
		GUILock lock;
		if (_root) {
			_root->remove();
			_root->release(); _root = nullptr;
		}
		if ( _focus_view ) {
			_focus_view->release();
			_focus_view = nullptr;
		}
		Release(_default_text_settings); _default_text_settings = nullptr;
		// Release(_draw_ctx);      _draw_ctx = nullptr;
		Release(_dispatch);      _dispatch = nullptr;
		// Release(_action_center); _action_center = nullptr;
		Release(_display_port);  _display_port = nullptr;
		Release(_pre_render);    _pre_render = nullptr;
		Release(_render_keep);   _render_keep = nullptr;
		Release(_main_keep);     _main_keep = nullptr;

		_render_loop = nullptr;
		_main_loop = nullptr;
		_shared = nullptr;

		Thread::FX_Off(ProcessSafeExit, on_process_safe_handle);
	}

	/**
	* @func initialize()
	*/
	void GUIApplication::initialize(cJSON& options) throw(Error) {
		GUILock lock;
		FX_CHECK(!_shared, "At the same time can only run a GUIApplication entity");
		_shared = this;
		HttpHelper::initialize(); // 初始http
		_inl_app(this)->initialize(options);
		FX_DEBUG("Inl_GUIApplication initialize ok");
		_display_port = NewRetain<DisplayPort>(this); // strong ref
		FX_DEBUG("NewRetain<DisplayPort> ok");
		_pre_render = new PreRender();
		FX_DEBUG("new PreRender ok");
		// _draw_ctx->font_pool()->set_display_port(_display_port);
		// FX_DEBUG("_draw_ctx->font_pool()->set_display_port() ok");
		_dispatch = new GUIEventDispatch(this);
		// _action_center = new ActionCenter();
	}

	/**
	* @func has_current_render_thread()
	*/
	bool GUIApplication::has_current_render_thread() const {
		return _render_loop && _render_loop->thread_id() == Thread::current_id();
	}

	/**
	* @func clear([full]) 清理不需要使用的资源
	*/
	void GUIApplication::clear(bool full) {
		// _render_loop->post(Cb([&, full](CbData& e){ _draw_ctx->clear(full); }));
	}

	uint64_t GUIApplication::max_texture_memory_limit() const {
		// return _draw_ctx->max_texture_memory_limit();
		return 0;
	}

	void GUIApplication::set_max_texture_memory_limit(uint64_t limit) {
		// _draw_ctx->set_max_texture_memory_limit(limit);
	}

	uint64_t GUIApplication::used_texture_memory() const {
		// return _draw_ctx->used_texture_memory();
		return 0;
	}

}
