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

#ifndef __langou__display_port__
#define __langou__display_port__

#include "langou/utils/util.h"
#include "langou/utils/event.h"
#include "langou/utils/list.h"
#include "langou/mathe.h"
#include "langou/value.h"
#include "langou/utils/cb.h"

/**
 * @ns langou
 */

XX_NS(langou)

class Draw;
class Root;
class GUIApplication;
class PreRender;

/**
 * 提供的一些对显示与屏幕的常用方法属性与事件
 * @class DisplayPort
 */
class XX_EXPORT DisplayPort: public Reference {
	XX_HIDDEN_ALL_COPY(DisplayPort);
 public:

	enum Orientation {
		ORIENTATION_INVALID = -1,
		ORIENTATION_PORTRAIT = 0,
		ORIENTATION_LANDSCAPE,
		ORIENTATION_REVERSE_PORTRAIT,
		ORIENTATION_REVERSE_LANDSCAPE,
		ORIENTATION_USER,
		ORIENTATION_USER_PORTRAIT,
		ORIENTATION_USER_LANDSCAPE,
		ORIENTATION_USER_LOCKED,
	};
	
	enum StatusBarStyle {
		STATUS_BAR_STYLE_WHITE = 0,
		STATUS_BAR_STYLE_BLACK,
	};
	
	DisplayPort(GUIApplication* host);
	
	/**
	 * @destructor
	 */
	virtual ~DisplayPort();
	
	/**
	 * @event onchange 显示端口变化事件
	 */
	XX_EVENT(change);

	/**
	 * @event onorientation 屏幕方向发生改变触发
	 */
	XX_EVENT(orientation);
	
	/**
	 * @func phy_size 视口在屏幕上所占的实际物理像素的尺寸
	 */
	inline Vec2 phy_size() const { return m_phy_size; }
	
	/**
	 * @func size 当前视口尺寸
	 */
	inline Vec2 size() const { return m_size; }
	
	/**
	 * @func best_scale 最好的视口缩放
	 */
	float best_scale() const;
	
	/**
	 * @func scale 当前视口缩放
	 */
	inline float scale() const { return m_scale; }
	
	/**
	 * @func scale_value
	 */
	inline Vec2 scale_value() const { return m_scale_value; }
	
	/**
	 * @func set_lock_size()
	 *
	 * width与height都设置为0时自动设置一个最舒适的默认显示尺寸
	 *
	 * 设置锁定视口为一个固定的逻辑尺寸,这个值改变时会触发change事件
	 *
	 * 如果width设置为零表示不锁定宽度,系统会自动根据height值设置一个同等比例的宽度
	 * 如果设置为非零表示锁定宽度,不管display_port_size怎么变化对于编程者来说,这个值永远保持不变
	 *
	 * 如果height设置为零表示不锁定,系统会自动根据width值设置一个同等比例的高度
	 * 如果设置为非零表示锁定高度,不管display_port_size怎么变化对于编程者来说,这个值永远保持不变
	 *
	 */
	void lock_size(float width = 0, float height = 0);
	
	/**
	 * @func root_matrix
	 */
	inline const Mat4& root_matrix() const { return m_root_matrix; }
	
	/**
	 * @func draw_region
	 */
	inline const Region& draw_region() const { return m_draw_region.last(); }

	/**
	 * @func push_draw_region
	 */
	void push_draw_region(Region value);
	
	/**
	 * @func pop_draw_region
	 */
	inline void pop_draw_region() {
		XX_ASSERT( m_draw_region.length() > 1 );
		m_draw_region.pop();
	}
	
	/**
	 * @func atom_pixel
	 */
	inline float atom_pixel() const { return m_atom_pixel; }
	
	/**
	 * @func next_frame() 只能在主gui线程调用
	 */
	void next_frame(cCb& cb);

	/**
	 * @func keep_screen(keep)
	 */
	void keep_screen(bool keep);

	/**
	 * @func status_bar_height()
	 */
	float status_bar_height();

	/**
	 * @func set_visible_status_bar(visible)
	 */
	void set_visible_status_bar(bool visible);

	/**
	 * @func set_status_bar_style(style)
	 */
	void set_status_bar_style(StatusBarStyle style);

	/**
	 * @func request_fullscreen(fullscreen)
	 */
	void request_fullscreen(bool fullscreen);

	/**
	 * @func orientation()
	 */
	Orientation orientation();
	
	/**
	 * @func set_orientation(orientation)
	 */
	void set_orientation(Orientation orientation);
	
	/**
	 * @func fsp()
	 */
	inline uint fsp() const { return m_fsp; }

	/**
	 * @func default_atom_pixel
	 */
	static float default_atom_pixel();

	/**
	 * @func default_status_bar_height
	 */
	static float default_status_bar_height();

 private:

	void render_frame();
	void refresh();
	
	Vec2              m_phy_size;   // 视口在屏幕上所占的实际像素的尺寸
	Vec2              m_lock_size;  // 锁定视口的尺寸
	Vec2              m_size;       // 当前视口尺寸
	float             m_scale;      // 当前屏幕显示缩放比,这个值越大size越小显示的内容也越少
	Vec2              m_scale_value;
	PreRender*        m_pre_render;
	Draw*             m_draw_ctx;
	Mat4              m_root_matrix;
	float             m_atom_pixel;
	List<Region>      m_draw_region;
	GUIApplication*   m_host;
	List<Callback>    m_next_frame;
	uint  m_fsp, m_record_fsp;
	int64 m_record_fsp_time;
	
	XX_DEFINE_INLINE_CLASS(Inl);
	friend class  GUIApplication; // 友元类
};

XX_END
#endif

