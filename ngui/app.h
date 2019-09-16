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

#ifndef __ngui__app__
#define __ngui__app__

#include "nxutils/util.h"
#include "nxutils/event.h"
#include "nxutils/loop.h"
#include "nxutils/json.h"
#include "ngui/value.h"

#define XX_GUI_MAIN() \
	int __xx_gui_main__(int, char**); \
	XX_INIT_BLOCK(__xx_gui_main__) { __xx_gui_main = __xx_gui_main__; } \
	int __xx_gui_main__(int argc, char** argv)

#define XX_CHECK_RENDER_THREAD() XX_CHECK(app()->has_current_render_thread())
#define XX_ASSERT_RENDER_THREAD() XX_ASSERT(app()->has_current_render_thread())

/**
 * gui入口程序,替代main入口函数gui启动时候会调用这个函数
 */
XX_EXPORT extern int (*__xx_gui_main)(int, char**);

/**
 * @ns trurh::gui
 */

XX_NS(ngui)

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

/**
 * 注意: 如果`main loop`与`render loop`运行在不同的线程,
 * 那么在主线程调用任何GUI-API函数必须加锁。
 */
class XX_EXPORT GUILock {
 public:
	GUILock();
	~GUILock();
	void lock();
	void unlock();
 private:
	void* m_d;
};

/*
 * 关于GUI中的事件:
 * GUI中所有事件都在`main loop`触发，并且不锁定`GUILock`，
 * 所以添加事件监听器也必须在`main loop`。
 */

/**
 * @class GUIApplication
 */
class XX_EXPORT GUIApplication: public Object {
	XX_HIDDEN_ALL_COPY(GUIApplication);
 public:

	XX_EVENT(load);
	XX_EVENT(unload);
	XX_EVENT(background);
	XX_EVENT(foreground);
	XX_EVENT(pause);
	XX_EVENT(resume);
	XX_EVENT(memorywarning);
	
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
	 * @func run 运行消息循环
	 */
	void run();	

	/**
	 * @func run_indep 在独立的线程运行消息循环
	 */
	void run_indep();

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
	inline bool is_loaded() const { return m_is_load; }
	
	/**
	 * @func draw_ctx 绘图上下文
	 */
	inline Draw* draw_ctx() { return m_draw_ctx; }
	
	/**
	 * @func display_port GUI程序显示端口
	 */
	inline DisplayPort* display_port() { return m_display_port; }
	
	/**
	 * @func root GUI程序的根视图
	 */
	inline Root* root() { return m_root; }
	
	/**
	 * @func focus_view
	 */
	inline View* focus_view() { return m_focus_view; }
	
	/**
	 * @func render_loop gui render loop
	 */
	inline RunLoop* render_loop() const { return m_render_loop; }
	
	/**
	 * @func work_loop work loop
	 */
	inline RunLoop* main_loop() const { return m_main_loop; }

	/**
	 * @func has_current_render_thread()
	 */
	bool has_current_render_thread() const;
	
	/**
	 * @func action_center
	 */
	inline ActionCenter* action_center() { return m_action_center; }
	
	/**
	 * @func app Get current gui application entity
	 */
	static inline GUIApplication* shared() { return m_shared; }

	// get default text attrs
	inline TextColor default_text_background_color() { return m_default_text_background_color; }
	inline TextColor default_text_color() { return m_default_text_color; }
	inline TextSize default_text_size() { return m_default_text_size; }
	inline TextStyle default_text_style() { return m_default_text_style; }
	inline TextFamily default_text_family() { return m_default_text_family; }
	inline TextShadow default_text_shadow() { return m_default_text_shadow; }
	inline TextLineHeight default_text_line_height() { return m_default_text_line_height; }
	inline TextDecoration default_text_decoration() { return m_default_text_decoration; }
	inline TextOverflow default_text_overflow() { return m_default_text_overflow; }
	inline TextWhiteSpace default_text_white_space() { return m_default_text_white_space; }
	// set default text attrs
	void set_default_text_background_color(TextColor value);
	void set_default_text_color(TextColor value);
	void set_default_text_size(TextSize value);
	void set_default_text_style(TextStyle value);
	void set_default_text_family(TextFamily value);
	void set_default_text_shadow(TextShadow value);
	void set_default_text_line_height(TextLineHeight value);
	void set_default_text_decoration(TextDecoration value);
	void set_default_text_overflow(TextOverflow value);
	void set_default_text_white_space(TextWhiteSpace value);
	
	/**
	 * @func max_texture_memory_limit()
	 */
	uint64 max_texture_memory_limit() const;
	
	/**
	 * @func set_max_texture_memory_limit(limit) 设置纹理内存限制，不能小于64MB，默认为512MB.
	 */
	void set_max_texture_memory_limit(uint64 limit);
	
	/**
	 * @func used_memory() 当前纹理数据使用的内存数量,包括图像纹理与字体纹理
	 */
	uint64 used_texture_memory() const;
	
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
	
 protected:
	
	/**
	 * @func runMain(argc, argv) create sub gui thread, call by system, First thread call
	 */
	static void runMain(int argc, char* argv[]);

 private:
	
	static GUIApplication*  m_shared;   // 当前应用程序
	bool  m_is_run, m_is_load;
	RunLoop  *m_render_loop, *m_main_loop;
	KeepLoop *m_render_keep, *m_main_keep;
	ThreadID m_render_id, m_main_id;
	Draw*                m_draw_ctx;         // 绘图上下文
	DisplayPort*         m_display_port;     // 显示端口
	Root*                m_root;             // 根视图
	View*                m_focus_view;       // 焦点视图
	TextColor            m_default_text_background_color; // default text attrs
	TextColor            m_default_text_color;
	TextSize             m_default_text_size;
	TextStyle            m_default_text_style;
	TextFamily           m_default_text_family;
	TextShadow           m_default_text_shadow;
	TextLineHeight       m_default_text_line_height;
	TextDecoration       m_default_text_decoration;
	TextOverflow         m_default_text_overflow;
	TextWhiteSpace       m_default_text_white_space; // text
	GUIEventDispatch*    m_dispatch;
	ActionCenter*        m_action_center;
	uint64 m_max_texture_memory_limit;
	
	XX_DEFINE_INLINE_CLASS(Inl);
	
	friend GUIApplication*  app();
	friend Root*            root();
	friend DisplayPort*     display_port();
};

inline GUIApplication* app() {
	return GUIApplication::m_shared;
}
inline Root* root() {
	return GUIApplication::m_shared ? GUIApplication::m_shared->m_root : nullptr;
}
inline DisplayPort* display_port() {
	return GUIApplication::m_shared ? GUIApplication::m_shared->m_display_port : nullptr;
}
inline RunLoop* main_loop() {
	return RunLoop::main_loop();
}

XX_END
#endif
