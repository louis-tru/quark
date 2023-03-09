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

#ifndef __quark__app__
#define __quark__app__

#include "./util/util.h"
#include "./util/event.h"
#include "./util/loop.h"
#include "./types.h"
#include "./render/render.h"

#define Qk_Main() \
	int __f_main__(int, Char**); \
	Qk_INIT_BLOCK(__f_main__) { qk::Application::setMain(&__f_main__); } \
	int __f_main__(int argc, Char** argv)

namespace qk {

	class Display;
	class PreRender;
	class View;
	class Root;
	class EventDispatch;
	class ActionDirect;
	class DefaultTextOptions;
	class FontPool;
	class ImageSourcePool;

	/*
	* 关于UI中的事件:
	* UI中所有事件都在`main loop`触发，并且不锁定`UILock`，
	* 所以添加事件监听器也必须在`main loop`。
	*/

	Application* app();

	/**
	* @class Application
	*/
	class Qk_EXPORT Application: public Object {
		Qk_HIDDEN_ALL_COPY(Application);
	public:

		struct Options {
			Render::Options render;
		};

		Qk_Event(Load);
		Qk_Event(Unload);
		Qk_Event(Background);
		Qk_Event(Foreground);
		Qk_Event(Pause);
		Qk_Event(Resume);
		Qk_Event(Memorywarning);

		/**
		* 注意: 如果`main loop`与`render loop`运行在不同的线程,
		* 那么在主线程调用任何UI-API函数必须加锁。
		*/
		class Qk_EXPORT UILock {
		public:
			UILock(Application* host = app());
			~UILock();
			void lock();
			void unlock();
		private:
			Application* _host;
			bool _lock;
		};

		Application(Options opts = {});

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
		 * @func options application options
		 */
		inline const Options& options() const { return _opts; }

		Qk_DEFINE_PROP_GET(bool, is_loaded);
		Qk_DEFINE_PROP_GET(DefaultTextOptions*, default_text_options); // 默认文本设置
		Qk_DEFINE_PROP_GET(Display*, display); // 当前显示端口
		Qk_DEFINE_PROP_GET(Root*, root); // 根视图
		Qk_DEFINE_PROP_GET(RunLoop*, loop); // 运行消息循环
		Qk_DEFINE_PROP_GET(ActionDirect*, action_direct); // 动作管理器
		Qk_DEFINE_PROP_GET(PreRender*, pre_render); // 预渲染器
		Qk_DEFINE_PROP_GET(Render*, render); // 渲染器
		Qk_DEFINE_PROP_GET(FontPool*, font_pool); // 字体管理器
		Qk_DEFINE_PROP_GET(ImageSourcePool*, img_pool); // 图片加载器
		Qk_DEFINE_PROP_GET(EventDispatch*, dispatch); // event dispatch

		/**
		* @func clean 清理垃圾回收内存资源, all=true 清理全部资源
		*/
		void clean(bool all = false);

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
		* @func runMain(argc, argv) create sub gui thread, call by system, First thread call
		*/
		static void runMain(int argc, char* argv[]);

		/**
		* @func app Get current gui application entity
		*/
		static inline Application* shared() { return _shared; }

	private:
		void set_root(Root* value) throw(Error);
		void handleExit(Event<>& e);

		static Application*  _shared;   // 当前应用程序
		Options        _opts;
		KeepLoop*      _keep;
		// inline EventDispatch* dispatch() { return _dispatch; }
		RecursiveMutex _render_mutex;
		uint64_t       _max_image_memory_limit; // 纹理内存限制，不能小于64MB，默认为512MB.

		Qk_DEFINE_INLINE_CLASS(Inl);

		friend class UILock;
		friend class Root;
		friend Application* app();
		friend Display* display();
		friend PreRender* pre_render();
	};

	inline Application* app() { return Application::_shared; }
	inline Display* display() { return Application::_shared->_display; }
	inline PreRender* pre_render() { return Application::_shared->_pre_render; }
	inline Render* render() { return app()->render(); }

	typedef Application::UILock UILock;

	//@private head
	Qk_DEFINE_INLINE_MEMBERS(Application, Inl) {
	public:
		#define _inl_app(self) static_cast<Application::Inl*>(self)
		void triggerLoad();
		void triggerUnload();
		void triggerPause();
		void triggerResume();
		void triggerBackground();
		void triggerForeground();
		void triggerMemorywarning();
	};
	//@end
}
#endif
