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
#include "./app.inl"
// #include "./action/action.h"
// #include "./css/css.h"
#include "./font/pool.h"
#include "./texture.h"
#include "./pre-render.h"
#include "./layout/text.h"
#include "./event.h"

F_EXPORT int (*__fx_default_gui_main)(int, char**) = nullptr;
F_EXPORT int (*__fx_gui_main)(int, char**) = nullptr;

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

	UILock::UILock(Application* host): _host(host), _lock(false) {
		lock();
	}

	UILock::~UILock() {
		unlock();
	}

	void UILock::lock() {
		if (_host->_use_gui_lock_mutex) {
			if (!_lock) {
				_lock = true;
				_host->_gui_lock_mutex.lock();
			}
		}
	}

	void UILock::unlock() {
		if (_lock) {
			_host->_gui_lock_mutex.unlock();
			_lock = false;
		}
	}

	void AppInl::triggerLoad() {
		if (!_is_load) {
			_is_load = true;
			_main_loop->post(Cb([&](CbData& d) { UILock lock; F_Trigger(Load); }));
		}
	}

	void AppInl::triggerRender() {
		_display->render_frame();
	}

	void AppInl::triggerPause() {
		_main_loop->post(Cb([&](CbData& d) { F_Trigger(Pause); }));
	}

	void AppInl::triggerResume() {
		_main_loop->post(Cb([&](CbData& d) { F_Trigger(Resume); }));
	}

	void AppInl::triggerBackground() {
		_main_loop->post(Cb([&](CbData& d) { F_Trigger(Background); }));
	}

	void AppInl::triggerForeground() {
		_main_loop->post(Cb([&](CbData& d) { F_Trigger(Foreground); }));
	}

	void AppInl::triggerMemorywarning() {
		clear();
		_main_loop->post(Cb([&](CbData&){ F_Trigger(Memorywarning); }));
	}

	void AppInl::triggerUnload() {
		if (_is_load) {
			_is_load = false;
			typedef Callback<RunLoop::PostSyncData> Cb;
			_main_loop->post_sync(Cb([&](Cb::Data& d) {
				DLOG("AppInl", "onUnload()");
				F_Trigger(Unload);
				if (_root) {
					UILock lock;
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
		F_CHECK(!_root, "Root view already exists");
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
		F_ASSERT( !__fx_gui_main );
		__fx_gui_main = main;
	}

	/**
	* @func runMain()
	*/
	void Application::runMain(int argc, Char* argv[]) {
		static int _is_initialize = 0;
		F_ASSERT(!_is_initialize++, "Cannot multiple calls.");
		
		// 创建一个新子工作线程.这个函数必须由main入口调用
		Thread::spawn([argc, argv](Thread& t) {
			auto main = __fx_gui_main ? __fx_gui_main : __fx_default_gui_main;
			F_ASSERT( main, "No gui main");
			__fx_default_gui_main = nullptr;
			__fx_gui_main = nullptr;
			int rc = main(argc, argv); // 运行这个自定gui入口函数
			DLOG("App", "Application::start Exit");
			flare::exit(rc); // if sub thread end then exit
			return rc;
		}, "runMain");

		// 在调用Application::run()之前一直阻塞这个主线程
		while (!_shared || !_shared->_is_run) {
			__run_main_wait->wait();
		}
	}

	void Application::run_loop() {
		F_ASSERT(!_is_run, "UI program has been running");

		_is_run = true;
		_render_loop = RunLoop::current(); // 当前消息队列
		_render_keep = _render_loop->keep_alive("Application::run, render_loop"); // 保持
		
		_use_gui_lock_mutex = _render_loop != _main_loop;

		if (_use_gui_lock_mutex) {
			Inl2_RunLoop(_render_loop)->set_independent_mutex(&_gui_lock_mutex);
			Thread::awaken(_main_loop->thread_id()); // main loop awaken
		}
		__run_main_wait->awaken(); // 外部线程继续运行

		F_ASSERT(!_render_loop->runing());

		_render_loop->run(); // 运行gui消息循环,这个消息循环主要用来绘图

		Release(_render_keep); _render_keep = nullptr;

		_render_loop = nullptr;
		_is_run = false;
	}

	void Application::run_loop_on_new_thread() {
		F_ASSERT(RunLoop::is_main_loop()); // main loop call

		Thread::spawn([this](Thread& t) {
			DLOG("App", "run render loop ...");
			run_loop();
			DLOG("App", "run render loop end");
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
			DLOG("App", "Application onExit");
		}
		return code;
	}

	Application::Application()
		: F_Init_Event(Load)
		, F_Init_Event(Unload)
		, F_Init_Event(Background)
		, F_Init_Event(Foreground)
		, F_Init_Event(Pause)
		, F_Init_Event(Resume)
		, F_Init_Event(Memorywarning)
		, _is_run(false), _is_load(false)
		, _render_loop(nullptr), _main_loop(RunLoop::main_loop())
		, _render_keep(nullptr), _main_keep(nullptr)
		, _render(nullptr), _display(nullptr)
		, _root(nullptr), _focus_view(nullptr)
		, _default_text_settings(new DefaultTextSettings())
		, _dispatch(nullptr), _action_center(nullptr)
		, _pre_render(nullptr)
		, _max_texture_memory_limit(512 * 1024 * 1024) // init 512MB
		, _use_gui_lock_mutex(false)
	{
		_main_keep = _main_loop->keep_alive("Application::Application(), main_keep");
		Thread::F_On(ProcessSafeExit, on_process_safe_handle);
	}

	Application::~Application() {
		UILock lock;
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

		Thread::F_Off(ProcessSafeExit, on_process_safe_handle);
	}

	/**
	* @func initialize()
	*/
	void Application::initialize(cJSON& options) throw(Error) {
		F_CHECK(!_shared, "At the same time can only run a Application entity");
		_shared = this;
		UILock lock;
		HttpHelper::initialize(); // 初始http
		_pre_render = new PreRender(); F_DEBUG("new PreRender ok");
		_display = NewRetain<Display>(this); F_DEBUG("NewRetain<Display> ok"); // strong ref
		_render = Render::create(this, options); F_DEBUG("Render::create() ok");
		_font_pool = new FontPool(this);
		_tex_pool = new TexturePool(this);
		_dispatch = new EventDispatch(this); F_DEBUG("new EventDispatch ok");
		// _action_center = new ActionCenter(); F_DEBUG("new ActionCenter ok");
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
		_max_texture_memory_limit = F_MAX(limit, 64 * 1024 * 1024);
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
		
		F_WARN("APP", "Adjust texture memory fail");
		
		return false;
	}

}
