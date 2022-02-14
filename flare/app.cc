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
#include "./source.h"
#include "./source.h"
#include "./pre_render.h"
#include "./layout/text.h"
#include "./event.h"

F_EXPORT int (*__f_default_gui_main)(int, char**) = nullptr;
F_EXPORT int (*__f_gui_main)(int, char**) = nullptr;

namespace flare {

	typedef Application::Inl AppInl;

	struct RunMainWait {
		bool wait() {
			if (_exit) return false;
			Lock lock(_thread_mutex);
			_thread_cond.wait(lock);
			return !_exit;
		}
		void awaken() {
			ScopeLock scope(_thread_mutex);
			_thread_cond.notify_all();
		}
		void exit(int rc) {
			_exit = true;
			awaken();
			flare::exit(rc); // if sub thread end then exit
		}
		Mutex _thread_mutex;
		Condition _thread_cond;
		bool _exit;
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

	typedef void (*CbFunc) (CbData&, AppInl*);

	void AppInl::triggerLoad() {
		if (_is_load || !_keep)
			return;
		_loop->post(Cb((CbFunc)[](CbData& d, AppInl* app) {
			UILock lock(app);
			if (!app->_is_load) {
				app->_is_load = true;
				app->F_Trigger(Load);
			}
		}, this));
	}

	void AppInl::triggerPause() {
		_loop->post(Cb((CbFunc)[](CbData& d, AppInl* app) { app->F_Trigger(Pause); }, this));
	}

	void AppInl::triggerResume() {
		_loop->post(Cb((CbFunc)[](CbData& d, AppInl* app) { app->F_Trigger(Resume); }, this));
	}

	void AppInl::triggerBackground() {
		_loop->post(Cb((CbFunc)[](CbData& d, AppInl* app) { app->F_Trigger(Background); }, this));
	}

	void AppInl::triggerForeground() {
		_loop->post(Cb((CbFunc)[](CbData& d, AppInl* app) { app->F_Trigger(Foreground); }, this));
	}

	void AppInl::triggerMemorywarning() {
		clear();
		_loop->post(Cb((CbFunc)[](CbData&, AppInl* app){ app->F_Trigger(Memorywarning); }, this));
	}

	void AppInl::triggerUnload() {
		if (!_keep) return;
		typedef Callback<RunLoop::PostSyncData> Cb;

		_loop->post_sync(Cb([&](Cb::Data& d) {
			if (_is_load) {
				_is_load = false;
				F_DEBUG("onUnload()");
				F_Trigger(Unload);
			}
			if (_keep) {
				Thread::abort(_loop->thread_id());
				Release(_keep); // stop loop
				_keep = nullptr;
				_loop = nullptr;
			}
			d.data->complete();
		}));
	}

	void AppInl::on_process_exit_handle(Event<>& e) {
		// int rc = static_cast<const Int32*>(e.data())->value;
		triggerUnload();
		F_DEBUG("Application onExit");
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
	
	Application::Application(JSON opts)
		: F_Init_Event(Load)
		, F_Init_Event(Unload)
		, F_Init_Event(Background)
		, F_Init_Event(Foreground)
		, F_Init_Event(Pause)
		, F_Init_Event(Resume)
		, F_Init_Event(Memorywarning)
		, _is_load(false)
		, _opts(opts)
		, _loop(nullptr), _keep(nullptr)
		, _render(nullptr), _display(nullptr)
		, _root(nullptr), _focus_view(nullptr)
		, _default_text_settings(nullptr)
		, _dispatch(nullptr), _action_direct(nullptr)
		, _pre_render(nullptr), _font_pool(nullptr), _img_pool(nullptr)
		, _max_image_memory_limit(512 * 1024 * 1024) // init 512MB
	{
		F_CHECK(!_shared, "At the same time can only run a Application entity");
		_shared = this;

		F_On(SafeExit, &AppInl::on_process_exit_handle, _inl_app(this));
		// init
		_default_text_settings = new DefaultTextSettings();
		_pre_render = new PreRender(); F_DEBUG("new PreRender ok");
		_display = NewRetain<Display>(this); F_DEBUG("NewRetain<Display> ok"); // strong ref
		//_font_pool = new FontPool(this);
		_img_pool = new ImagePool(this);
		_dispatch = new EventDispatch(this); F_DEBUG("new EventDispatch ok");
		// _action_direct = new ActionDirect(); F_DEBUG("new ActionDirect ok");
	}

	Application::~Application() {
		UILock lock(this);
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
		// Release(_action_direct); _action_direct = nullptr;
		Release(_display);     _display = nullptr;
		Release(_pre_render);  _pre_render = nullptr;
		Release(_render);      _render = nullptr;
		Release(_keep);        _keep = nullptr; _loop = nullptr;
		Release(_font_pool);   _font_pool = nullptr;
		Release(_img_pool);    _img_pool = nullptr;

		F_Off(SafeExit, &AppInl::on_process_exit_handle, _inl_app(this));

		_shared = nullptr;
	}

	/**
	* @func run() init and run
	*/
	void Application::run(bool is_loop) throw(Error) {
		UILock lock(this);
		if (!_keep) { // init
			_render = Render::Make(this, Render::parseOptions(_opts)); F_DEBUG("Render::Make() ok");
			_loop = RunLoop::current();
			_keep = _loop->keep_alive("Application::run(), keep"); // 保持运行
			__run_main_wait->awaken(); // 外部线程继续运行
		}
		if (is_loop) { // run loop
			lock.unlock();
			_loop->run(); // run message loop
		}
	}

	/**
	* @func setMain()
	*/
	void Application::setMain(int (*main)(int, char**)) {
		F_ASSERT( !__f_gui_main );
		__f_gui_main = main;
	}

	/**
	* @func runMain()
	*/
	void Application::runMain(int argc, Char* argv[]) {
		static int _is_init = 0;
		F_ASSERT(!_is_init++, "Cannot multiple calls.");
		
		struct Args { int argc; Char** argv; } arg = { argc, argv };
		
		// 创建一个新子工作线程.这个函数必须由main入口调用
		Thread::create([](Thread& t, void* arg) {
			auto args = (Args*)arg;
			auto main = __f_gui_main ? __f_gui_main : __f_default_gui_main;
			F_ASSERT( main, "No gui main");
			__f_default_gui_main = nullptr;
			__f_gui_main = nullptr;
			int rc = main(args->argc, args->argv); // 运行这个自定gui入口函数
			F_DEBUG("Application::runMain() Exit");
			__run_main_wait->exit(rc);
		}, &arg, "runMain");

		// 在调用Application::run()之前一直阻塞这个主线程
		while (!_shared || !_shared->_keep) {
			if (!__run_main_wait->wait()) {
				break;
			}
		}
	}

	/**
	* @func clear([full]) 清理不需要使用的资源
	*/
	void Application::clear(bool full) {
		_render->post_message(Cb([this, full](CbData& e){
			_img_pool->clear(full);
			_font_pool->clear(full);
		}));
	}

	/**
	* @func max_image_memory_limit()
	*/
	uint64_t Application::max_image_memory_limit() const {
		return _max_image_memory_limit;
	}
	
	/**
	* @func set_max_image_memory_limit(limit) 设置纹理内存限制，不能小于64MB，默认为512MB.
	*/
	void Application::set_max_image_memory_limit(uint64_t limit) {
		_max_image_memory_limit = F_MAX(limit, 64 * 1024 * 1024);
	}
	
	/**
	* @func used_memory() 当前纹理数据使用的内存数量,包括图像纹理与字体纹理
	*/
	uint64_t Application::used_image_memory() const {
		return _img_pool->total_data_size() + _font_pool->total_data_size();
	}

	/**
	* @func adjust_image_memory()
	*/
	bool Application::adjust_image_memory(uint64_t will_alloc_size) {
		int i = 0;
		do {
			if (will_alloc_size + used_image_memory() <= _max_image_memory_limit) {
				return true;
			}
			clear();
			i++;
		} while(i < 3);
		
		F_WARN("Adjust image memory fail");
		
		return false;
	}

}
