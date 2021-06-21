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

#define FX_Main() \
	int __fx_main__(int, Char**); \
	FX_INIT_BLOCK(__fx_main__) { setMain(&__fx_main__); } \
	int __fx_main__(int argc, Char** argv)

#define FX_ASSERT_STRICT_RENDER_THREAD() ASSERT(app()->has_current_render_thread())
#define FX_ASSERT_RENDER_THREAD() ASSERT(app()->has_current_render_thread())

namespace flare {

	class DrawOption;
	class Draw;
	class GLDraw;
	class DisplayPort;
	class View;
	class Root;
	class GUIEventDispatch;
	class ActionCenter;
	class PropertysAccessor;
	class CSSManager;
	class PreRender;
	class DefaultTextSettings;

	/*
	* 关于GUI中的事件:
	* GUI中所有事件都在`main loop`触发，并且不锁定`GUILock`，
	* 所以添加事件监听器也必须在`main loop`。
	*/

	GUIApplication* app();

	/**
	* @class GUIApplication
	*/
	class FX_EXPORT GUIApplication: public Object {
		FX_HIDDEN_ALL_COPY(GUIApplication);
		public:

		/**
		* 注意: 如果`main loop`与`render loop`运行在不同的线程,
		* 那么在主线程调用任何GUI-API函数必须加锁。
		*/
		class FX_EXPORT GUILock {
			public:
			GUILock(GUIApplication* host = app());
			~GUILock();
			void lock();
			void unlock();
			private:
			GUIApplication* _host;
			bool _lock;
		};

		FX_Event(Load);
		FX_Event(Unload);
		FX_Event(Background);
		FX_Event(Foreground);
		FX_Event(Pause);
		FX_Event(Resume);
		FX_Event(Memorywarning);

		GUIApplication();
		
		/**
		* @destructor
		*/
		virtual ~GUIApplication();

		/**
		* @func initialize()
		*/
		void initialize(cJSON& options = JSON::object()) throw(Error);

		/**
		* @func run_loop 运行gui消息循环
		*/
		void run_loop();

		/**
		* @func run_loop_on_new_thread 在新的线程运行gui消息循环
		*/
		void run_loop_on_new_thread();

		/**
		* @func clear 清理垃圾回收内存资源, full=true 清理全部资源
		*/
		void clear(bool full = false);

		/**
		* @func pending() 挂起应用进程
		*/
		void pending();

		/**
		* @func is_loaded
		*/
		inline bool is_loaded() const { return _is_load; }

		/**
		* @func draw_ctx 绘图上下文
		*/
		inline Draw* draw_ctx() { return _draw_ctx; }
		
		/**
		* @func display_port GUI程序显示端口
		*/
		inline DisplayPort* display_port() { return _display_port; }
		
		/**
		* @func root GUI程序的根视图
		*/
		inline Root* root() { return _root; }
		
		/**
		* @func focus_view
		*/
		inline View* focus_view() { return _focus_view; }
		
		/**
		* @func render_loop gui render loop
		*/
		inline RunLoop* render_loop() const { return _render_loop; }
		
		/**
		* @func work_loop work loop
		*/
		inline RunLoop* main_loop() const { return _main_loop; }

		/**
		* @func has_current_render_thread()
		*/
		bool has_current_render_thread() const;

		/**
		* @func action_center
		*/
		inline ActionCenter* action_center() { return _action_center; }
		
		/**
		* @func app Get current gui application entity
		*/
		static inline GUIApplication* shared() { return _shared; }

		/**
		 * @func default_text_settings() default text settings
		 */
		inline DefaultTextSettings* default_text_settings() { return _default_text_settings; }
		
		/**
		* @func max_texture_memory_limit()
		*/
		uint64_t max_texture_memory_limit() const;
		
		/**
		* @func set_max_texture_memory_limit(limit) 设置纹理内存限制，不能小于64MB，默认为512MB.
		*/
		void set_max_texture_memory_limit(uint64_t limit);
		
		/**
		* @func used_memory() 当前纹理数据使用的内存数量,包括图像纹理与字体纹理
		*/
		uint64_t used_texture_memory() const;
		
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
										cString& bcc = String(),
										cString& body = String());

		/**
		 * @func pre_render()
		 */
		inline PreRender* pre_render() {
			return _pre_render;
		}

		/**
		 * 
		 * setting main function
		 *
		 * @func setMain()
		 */
		static void setMain(int (*main)(int, char**));
		
		protected:
		
		/**
		* @func runMain(argc, argv) create sub gui thread, call by system, First thread call
		*/
		static void runMain(int argc, Char* argv[]);

		private:
		static GUIApplication* _shared;   // 当前应用程序
		bool  _is_run, _is_load;
		RunLoop  *_render_loop, *_main_loop;
		KeepLoop *_render_keep, *_main_keep;
		Draw*                _draw_ctx;         // 绘图上下文
		DisplayPort*         _display_port;     // 显示端口
		PreRender*           _pre_render;
		Root*                _root;             // 根视图
		View*                _focus_view;       // 焦点视图
		DefaultTextSettings* _default_text_settings;
		GUIEventDispatch*    _dispatch;
		ActionCenter*        _action_center;
		uint64_t             _max_texture_memory_limit;
		RecursiveMutex       _gui_lock_mutex;
		
		FX_DEFINE_INLINE_CLASS(Inl);
		
		friend class GUILock;
		friend GUIApplication* app();
	};

	inline GUIApplication* app() {
		return GUIApplication::_shared;
	}

	typedef GUIApplication::GUILock GUILock;

}
#endif
