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

#ifndef __flare__app__
#define __flare__app__

#include "./util/util.h"
#include "./util/event.h"
#include "./util/loop.h"
#include "./util/json.h"
#include "./value.h"

#define F_Main() \
	int __f_main__(int, Char**); \
	F_INIT_BLOCK(__f_main__) { flare::Application::setMain(&__f_main__); } \
	int __f_main__(int argc, Char** argv)

namespace flare {

	class Application;
	class Display;
	class PreRender;
	class Render;
	class View;
	class Root;
	class EventDispatch;
	class ActionDirect;
	class DefaultTextSettings;
	class FontPool;
	class ImagePool;

	/*
	* 关于UI中的事件:
	* UI中所有事件都在`main loop`触发，并且不锁定`UILock`，
	* 所以添加事件监听器也必须在`main loop`。
	*/

	Application* app();

	/**
	* @class Application
	*/
	class F_EXPORT Application: public Object {
		F_HIDDEN_ALL_COPY(Application);
	 public:

		/**
		* 注意: 如果`main loop`与`render loop`运行在不同的线程,
		* 那么在主线程调用任何UI-API函数必须加锁。
		*/
		class F_EXPORT UILock {
		 public:
			UILock(Application* host = app());
			~UILock();
			void lock();
			void unlock();
		 private:
			Application* _host;
			bool _lock;
		};

		F_Event(Load);
		F_Event(Unload);
		F_Event(Background);
		F_Event(Foreground);
		F_Event(Pause);
		F_Event(Resume);
		F_Event(Memorywarning);

		Application(JSON opts = JSON::object());

		/**
		* @destructor
		*/
		virtual ~Application();

		/**
		* @func run()
		*/
		void run(bool is_loop = false) throw(Error);

		/**
		* @func pending() 挂起应用进程
		*/
		void pending();

		/**
		* @func is_loaded
		*/
		inline bool is_loaded() const { return _is_load; }

		/**
			* @func default_text_settings()
			*/
		inline DefaultTextSettings* default_text_settings() { return _default_text_settings; }
		inline Display* display() { return _display; }
		inline Root* root() { return _root; }
		inline View* focus_view() { return _focus_view; }
		inline RunLoop* loop() { return _loop; }
		inline ActionDirect* action_direct() { return _action_direct; }
		inline PreRender* pre_render() { return _pre_render; }
		inline Render* render() { return _render; }
		inline FontPool* font_pool() { return _font_pool; }
		inline ImagePool* img_pool() { return _img_pool; }

		/**
		* @func clear 清理垃圾回收内存资源, full=true 清理全部资源
		*/
		void clear(bool full = false);

		/**
		* @func max_image_memory_limit()
		*/
		uint64_t max_image_memory_limit() const;
		
		/**
		* @func set_max_image_memory_limit(limit) 设置纹理内存限制，不能小于64MB，默认为512MB.
		*/
		void set_max_image_memory_limit(uint64_t limit);
		
		/**
		* @func used_memory() 当前纹理数据使用的内存数量,包括图像纹理与字体纹理
		*/
		uint64_t used_image_memory() const;

		/**
		* @func adjust_image_memory()
		*/
		bool adjust_image_memory(uint64_t will_alloc_size);

		/**
		* @func open_url()
		*/
		void open_url(cString& url);
		
		/**
		* @func send_email
		*/
		void send_email(cString& recipient,
										cString& subject,
										cString& cc = String(),
										cString& bcc = String(), cString& body = String());

		/**
		 * 
		 * setting main function
		 *
		 * @func setMain()
		 */
		static void setMain(int (*main)(int, char**));
		
		/**
		* @func app Get current gui application entity
		*/
		static inline Application* shared() { return _shared; }

	 protected:
		
		/**
		* @func runMain(argc, argv) create sub gui thread, call by system, First thread call
		*/
		static void runMain(int argc, Char* argv[]);

	 private:
		static Application*  _shared;   // 当前应用程序
		bool                 _is_load;
		JSON                 _opts;
		RunLoop*             _loop;
		KeepLoop*            _keep;
		Display*             _display;     // 当前显示端口
		PreRender*           _pre_render;
		Render*              _render;
		Root*                _root;             // 根视图
		View*                _focus_view;       // 焦点视图
		DefaultTextSettings* _default_text_settings;
		EventDispatch*       _dispatch;
		ActionDirect*        _action_direct;
		RecursiveMutex       _render_mutex;
		FontPool*            _font_pool;        /* 字体纹理池 */
		ImagePool*           _img_pool;         /* 图像池 */
		uint64_t _max_image_memory_limit; // 纹理内存限制，不能小于64MB，默认为512MB.
		
		F_DEFINE_INLINE_CLASS(Inl);
		
		friend class UILock;
		friend Application* app();
		friend Display* display();
		friend PreRender* pre_render();
	};

	inline Application* app() { return Application::_shared; }
	inline Display* display() { return Application::_shared->_display; }
	inline PreRender* pre_render() { return Application::_shared->_pre_render; }
	inline Render* render() { return app()->render(); }

	typedef Application::UILock UILock;

}
#endif
