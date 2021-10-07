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
#include "./util/working.h"
#include "./util/http.h"
#include "./render/render.h"
#include "./layout/root.h"
#include "./display.h"
#include "./_app.h"
// #include "./action/action.h"
// #include "./css/css.h"
#include "./font/pool.h"
#include "./texture.h"
#include "./pre-render.h"
#include "./layout/text.h"
#include "./event.h"

FX_EXPORT int (*__fx_default_gui_main)(int, char**) = nullptr;
FX_EXPORT int (*__fx_gui_main)(int, char**) = nullptr;

namespace flare {

	typedef Application::Inl AppInl;

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
	Application* Application::_shared = nullptr;

	GUILock::GUILock(Application* host): _host(host), _lock(false) {
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

	void AppInl::triggerLoad() {
		if (!_is_load) {
			_is_load = true;
			_main_loop->post(Cb([&](CbData& d) { GUILock lock; FX_Trigger(Load); }));
		}
	}

	void AppInl::triggerRender() {
		_display->render_frame();
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
			if ( view->layout_depth() && view->can_become_focus() ) {
				if ( _focus_view ) {
					_focus_view->release();
				}
				_focus_view = view;
				_focus_view->retain(); // strong ref
				// TODO ...
				// _dispatch->make_text_input(view->as_itext_input());
			} else {
				return false;
			}
		}
		return true;
	}

	/**
	* @func setMain()
	*/
	void Application::setMain(int (*main)(int, char**)) {
		ASSERT( !__fx_gui_main );
		__fx_gui_main = main;
	}

	/**
	* @func runMain()
	*/
	void Application::runMain(int argc, Char* argv[]) {
		static int _is_initialize = 0;
		ASSERT(!_is_initialize++, "Cannot multiple calls.");
		
		// 创建一个新子工作线程.这个函数必须由main入口调用
		Thread::spawn([argc, argv](Thread& t) {
			auto main = __fx_gui_main ? __fx_gui_main : __fx_default_gui_main;
			ASSERT( main, "No gui main");
			__fx_default_gui_main = nullptr;
			__fx_gui_main = nullptr;
			int rc = main(argc, argv); // 运行这个自定gui入口函数
			DLOG("Application::start Exit");
			flare::exit(rc); // if sub thread end then exit
			return rc;
		}, "runMain");

		// 在调用Application::run()之前一直阻塞这个主线程
		while (!_shared || !_shared->_is_run) {
			__run_main_wait->wait();
		}
	}

	void Application::run_loop() {
		ASSERT(!_is_run, "GUI program has been running");

		_is_run = true;
		_render_loop = RunLoop::current(); // 当前消息队列
		_render_keep = _render_loop->keep_alive("Application::run, render_loop"); // 保持

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

	void Application::run_loop_on_new_thread() {
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
			DLOG("Application onExit");
		}
		return code;
	}

	Application::Application()
		: FX_Init_Event(Load)
		, FX_Init_Event(Unload)
		, FX_Init_Event(Background)
		, FX_Init_Event(Foreground)
		, FX_Init_Event(Pause)
		, FX_Init_Event(Resume)
		, FX_Init_Event(Memorywarning)
		, _is_run(false), _is_load(false)
		, _render_loop(nullptr), _main_loop(RunLoop::main_loop())
		, _render_keep(nullptr), _main_keep(nullptr)
		, _render(nullptr), _display(nullptr)
		, _root(nullptr), _focus_view(nullptr)
		, _default_text_settings(new DefaultTextSettings())
		, _dispatch(nullptr), _action_center(nullptr)
		, _pre_render(nullptr)
		, _max_texture_memory_limit(512 * 1024 * 1024) // init 512MB
	{
		_main_keep = _main_loop->keep_alive("Application::Application(), main_keep");
		Thread::FX_On(ProcessSafeExit, on_process_safe_handle);
	}

	Application::~Application() {
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
		Release(_dispatch);      _dispatch = nullptr;
		// Release(_action_center); _action_center = nullptr;
		Release(_display);       _display = nullptr;
		Release(_pre_render);    _pre_render = nullptr;
		Release(_render);      _render = nullptr;
		Release(_render_keep);   _render_keep = nullptr;
		Release(_main_keep);     _main_keep = nullptr;
		Release(_font_pool); _font_pool = nullptr;
		Release(_tex_pool); _tex_pool = nullptr;

		_render_loop = nullptr;
		_main_loop = nullptr;
		_shared = nullptr;

		Thread::FX_Off(ProcessSafeExit, on_process_safe_handle);
	}

	/**
	* @func initialize()
	*/
	void Application::initialize(cJSON& options) throw(Error) {
		GUILock lock;
		FX_CHECK(!_shared, "At the same time can only run a Application entity");
		_shared = this;
		HttpHelper::initialize(); // 初始http
		_pre_render = new PreRender(); FX_DEBUG("new PreRender ok");
		_display = NewRetain<Display>(this); FX_DEBUG("NewRetain<Display> ok"); // strong ref
		_render = Render::create(this, options); FX_DEBUG("Render::create() ok");
		_font_pool = new FontPool(this);
		_tex_pool = new TexturePool(this);
		_dispatch = new EventDispatch(this); FX_DEBUG("new EventDispatch ok");
		// _action_center = new ActionCenter(); FX_DEBUG("new ActionCenter ok");
	}

	/**
	* @func has_current_render_thread()
	*/
	bool Application::has_current_render_thread() const {
		return _render_loop && _render_loop->thread_id() == Thread::current_id();
	}

	/**
	* @func clear([full]) 清理不需要使用的资源
	*/
	void Application::clear(bool full) {
		_render_loop->post(Cb([&, full](CbData& e){
			_tex_pool->clear(full);
			_font_pool->clear(full);
		}));
	}

	/**
	* @func max_texture_memory_limit()
	*/
	uint64_t Application::max_texture_memory_limit() const {
		return _max_texture_memory_limit;
	}
	
	/**
	* @func set_max_texture_memory_limit(limit) 设置纹理内存限制，不能小于64MB，默认为512MB.
	*/
	void Application::set_max_texture_memory_limit(uint64_t limit) {
		_max_texture_memory_limit = FX_MAX(limit, 64 * 1024 * 1024);
	}
	
	/**
	* @func used_memory() 当前纹理数据使用的内存数量,包括图像纹理与字体纹理
	*/
	uint64_t Application::used_texture_memory() const {
		return _tex_pool->total_data_size() + _font_pool->total_data_size();
	}

	/**
	* @func adjust_texture_memory()
	*/
	bool Application::adjust_texture_memory(uint64_t will_alloc_size) {
		int i = 0;
		do {
			if (will_alloc_size + used_texture_memory() <= _max_texture_memory_limit) {
				return true;
			}
			clear();
			i++;
		} while(i < 3);
		
		FX_WARN("Adjust texture memory fail");
		
		return false;
	}

}
