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

#include "utils/loop-1.h"
#include "utils/http.h"
#include "draw.h"
#include "root.h"
#include "display-port.h"
#include "app-1.h"
#include "action.h"
#include "css.h"

XX_EXPORT int (*__xx_default_gui_main)(int, char**) = nullptr;
XX_EXPORT int (*__XX_GUI_MAIN)(int, char**) = nullptr;

XX_NS(shark)

typedef GUIApplication::Inl AppInl;

// global shared gui application 
GUIApplication* GUIApplication::m_shared = nullptr;

struct thread_control {
	Mutex root_thread_mutex;
	Condition root_thread_cond;
	RecursiveMutex gui_thread_mutex;

	void root_thread_awaken() {
		ScopeLock scope(root_thread_mutex);
		root_thread_cond.notify_one();
	}

	void root_thread_sleep(bool* ok = nullptr) {
		Lock lock(root_thread_mutex);
		do {
			root_thread_cond.wait(lock);
		} while(ok && !*ok);
	}

};

static auto *tctr = new thread_control();

GUILock::GUILock(): m_d(nullptr) {
	lock();
}

GUILock::~GUILock() {
	unlock();
}

void GUILock::lock() {
	if (!m_d) {
		m_d = &tctr->gui_thread_mutex;
		tctr->gui_thread_mutex.lock();
	}
}

void GUILock::unlock() {
	if (m_d) {
		reinterpret_cast<RecursiveMutex*>(m_d)->unlock();
		m_d = nullptr;
	}
}

void AppInl::refresh_display() {
	m_display_port->refresh();
}

void AppInl::onLoad() {
	m_is_load = true;
	m_main_loop->post(Cb([&](Se& d) { GUILock lock; XX_TRIGGER(load); }));
}

void AppInl::onRender() {
	m_display_port->render_frame();
}

void AppInl::onPause() {
	m_main_loop->post(Cb([&](Se& d) { XX_TRIGGER(pause); }));
}

void AppInl::onResume() {
	m_main_loop->post(Cb([&](Se& d) { XX_TRIGGER(resume); }));
}

void AppInl::onBackground() {
	m_main_loop->post(Cb([&](Se& d) { XX_TRIGGER(background); }));
}

void AppInl::onForeground() {
	m_main_loop->post(Cb([&](Se& d) { XX_TRIGGER(foreground); }));
}

void AppInl::onMemorywarning() {
	clear();
	m_main_loop->post(Cb([&](Se&){ XX_TRIGGER(memorywarning); }));
}

void AppInl::onUnload() {
	if (m_is_load) {
		m_is_load = false;
		m_main_loop->post_sync(Cb([&](Se& d) {
			XX_TRIGGER(unload);
			if (m_root) {
				GUILock lock;
				m_root->remove();
			}
		}));
	}
}
/**
 * @func set_root
 */
void AppInl::set_root(Root* value) throw(Error) {
	XX_ASSERT_ERR(!m_root, "Root view already exists");
	m_root = value;
	m_root->retain();   // strong ref
	set_focus_view(value);
}

/**
 * @func set_focus_view()
 */
bool AppInl::set_focus_view(View* view) {
	if ( m_focus_view != view ) {
		if ( view->final_visible() && view->can_become_focus() ) {
			if ( m_focus_view ) {
				m_focus_view->release();
			}
			m_focus_view = view;
			m_focus_view->retain(); // strong ref
			m_dispatch->make_text_input(dynamic_cast<ITextInput*>(view));
		} else {
			return false;
		}
	}
	return true;
}

/**
 * @func start()
 */
void GUIApplication::start(int argc, char* argv[]) {
	static int is_initialize = 0;
	XX_CHECK(!is_initialize++, "Cannot multiple calls.");
	
	// 创建一个新子工作线程.这个函数必须由main入口调用
	SimpleThread::detach([argc, argv](SimpleThread& t) {
		XX_CHECK( __xx_default_gui_main );
		auto main = __XX_GUI_MAIN ? __XX_GUI_MAIN : __xx_default_gui_main;
		__xx_default_gui_main = nullptr;
		__XX_GUI_MAIN = nullptr;
		int rc = main(argc, argv); // 运行这个自定gui入口函数
		::exit(rc); // if sub thread end then exit
	}, "gui");

	// 在调用GUIApplication::run()之前一直阻塞这个主线程
	while (!m_shared || !m_shared->m_is_run) {
		tctr->root_thread_sleep();
	}
}

/**
 * @func run()
 */
void GUIApplication::run() {
	XX_CHECK(!m_is_run, "GUI program has been running");
	
	m_is_run = true;
	m_main_loop = RunLoop::main_loop();
	m_main_keep = m_main_loop->keep_alive();
	m_render_loop = RunLoop::current(); // 当前消息队列
	m_render_keep = m_render_loop->keep_alive(); // 保持
	if (m_main_loop != m_render_loop) {
		Inl2_RunLoop(m_render_loop)->set_independent_mutex(&tctr->gui_thread_mutex);
	}
	tctr->root_thread_awaken(); // 根线程继续运行
	
	XX_CHECK(!m_render_loop->runing());
	m_render_loop->run(); // 运行gui消息循环,这个消息循环主要用来绘图
}

void GUIApplication::exit() {
	_inl_app(this)->onUnload();
	XX_ASSERT(m_main_loop);
	Release(m_render_keep); // stop render loop
	m_render_keep = nullptr;
	m_main_loop->stop();
}

GUIApplication::GUIApplication()
: XX_INIT_EVENT(load)
, XX_INIT_EVENT(unload)
, XX_INIT_EVENT(background)
, XX_INIT_EVENT(foreground)
, XX_INIT_EVENT(pause)
, XX_INIT_EVENT(resume)
, XX_INIT_EVENT(memorywarning)
, m_is_run(false)
, m_is_load(false)
, m_render_loop(nullptr)
, m_main_loop(nullptr)
, m_render_keep(nullptr)
, m_main_keep(nullptr)
, m_draw_ctx(nullptr)
, m_display_port(nullptr)
, m_root(nullptr)
, m_focus_view(nullptr)
, m_default_text_background_color({ TextAttrType::VALUE, Color(0, 0, 0, 0) })
, m_default_text_color({ TextAttrType::VALUE, Color(0, 0, 0) })
, m_default_text_size({ TextAttrType::VALUE, 16 })
, m_default_text_style({ TextAttrType::VALUE, TextStyleEnum::REGULAR })
, m_default_text_family(TextAttrType::VALUE, FontPool::get_font_familys_id(String()))
, m_default_text_shadow({ TextAttrType::VALUE, { 0, 0, 0, Color(0, 0, 0) } })
, m_default_text_line_height({ TextAttrType::VALUE, { 0 } })
, m_default_text_decoration({ TextAttrType::VALUE, TextDecorationEnum::NONE })
, m_default_text_overflow({ TextAttrType::VALUE, TextOverflowEnum::NORMAL })
, m_default_text_white_space({ TextAttrType::VALUE, TextWhiteSpaceEnum::NORMAL })
, m_dispatch(nullptr)
, m_action_center(nullptr)
{
}

GUIApplication::~GUIApplication() {
	GUILock lock;
	if (m_root) {
		m_root->remove();
		m_root->release(); m_root = nullptr;
	}
	if ( m_focus_view ) {
		m_focus_view->release();
		m_focus_view = nullptr;
	}
	Release(m_draw_ctx);      m_draw_ctx = nullptr;
	Release(m_dispatch);      m_dispatch = nullptr;
	Release(m_action_center); m_action_center = nullptr;
	Release(m_display_port);  m_display_port = nullptr;
	Release(m_render_keep);   m_render_keep = nullptr;
	Release(m_main_keep);     m_main_keep = nullptr;
	m_render_loop = nullptr;
	m_main_loop = nullptr;
	m_shared = nullptr;
}

/**
 * @func initialize()
 */
void GUIApplication::initialize(cJSON& options) throw(Error) {
	GUILock lock;
	XX_CHECK_ERR(!m_shared, "At the same time can only run a GUIApplication entity");
	m_shared = this;
	HttpHelper::initialize(); // 初始http
	_inl_app(this)->initialize(options);
	XX_DEBUG("Inl_GUIApplication initialize ok");
	m_display_port = NewRetain<DisplayPort>(this); // strong ref
	XX_DEBUG("NewRetain<DisplayPort> ok");
	m_draw_ctx->font_pool()->set_display_port(m_display_port);
	XX_DEBUG("m_draw_ctx->font_pool()->set_display_port() ok");
	m_dispatch = new GUIEventDispatch(this);
	m_action_center = new ActionCenter();
}

/**
 * @func clear([full]) 清理不需要使用的资源
 */
void GUIApplication::clear(bool full) {
	m_render_loop->post(Cb([&, full](Se& e){ m_draw_ctx->clear(full); }));
}

void GUIApplication::set_default_text_background_color(TextColor value) {
	if ( value.type == TextAttrType::VALUE ) {
		m_default_text_background_color = value;
	}
}
void GUIApplication::set_default_text_color(TextColor value) {
	if ( value.type == TextAttrType::VALUE ) {
		m_default_text_color = value;
	}
}
void GUIApplication::set_default_text_size(TextSize value) {
	if ( value.type == TextAttrType::VALUE ) {
		m_default_text_size = value;
	}
}
void GUIApplication::set_default_text_style(TextStyle value) {
	if ( value.type == TextAttrType::VALUE ) {
		m_default_text_style = value;
	}
}
void GUIApplication::set_default_text_family(TextFamily value) {
	if ( value.type == TextAttrType::VALUE ) {
		m_default_text_family = value;
	}
}
void GUIApplication::set_default_text_shadow(TextShadow value) {
	if ( value.type == TextAttrType::VALUE ) {
		m_default_text_shadow = value;
	}
}
void GUIApplication::set_default_text_line_height(TextLineHeight value) {
	if ( value.type == TextAttrType::VALUE ) {
		m_default_text_line_height = value;
	}
}
void GUIApplication::set_default_text_decoration(TextDecoration value) {
	if ( value.type == TextAttrType::VALUE ) {
		m_default_text_decoration = value;
	}
}
void GUIApplication::set_default_text_overflow(TextOverflow value) {
	if ( value.type == TextAttrType::VALUE ) {
		m_default_text_overflow = value;
	}
}
void GUIApplication::set_default_text_white_space(TextWhiteSpace value) {
	if ( value.type == TextAttrType::VALUE ) {
		m_default_text_white_space = value;
	}
}

uint64 GUIApplication::max_texture_memory_limit() const {
	return m_draw_ctx->max_texture_memory_limit();
}

void GUIApplication::set_max_texture_memory_limit(uint64 limit) {
	m_draw_ctx->set_max_texture_memory_limit(limit);
}

uint64 GUIApplication::used_texture_memory() const {
	return m_draw_ctx->used_texture_memory();
}

XX_END
