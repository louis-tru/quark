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

#ifndef __flare__display__
#define __flare__display__

#include "./util/util.h"
#include "./util/event.h"
#include "./math.h"
#include "./value.h"
#include "./util/cb.h"
#include "./util/thread.h"

namespace flare {

	class Application;

	/**
	* 提供的一些对显示与屏幕的常用方法属性与事件
	* @class Display
	*/
	class FX_EXPORT Display: public Reference {
		FX_HIDDEN_ALL_COPY(Display);
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
		
		Display(Application* host);
		
		/**
		* @destructor
		*/
		virtual ~Display();
		
		/**
		* @event onchange 显示端口变化事件
		*/
		FX_Event(Change);

		/**
		* @event onorientation 屏幕方向发生改变触发
		*/
		FX_Event(Orientation);
		
		/**
		* @func size 当前视口尺寸
		*/
		inline Vec2 size() const { return _size; }
		
		/**
		* @func scale_value
		*/
		inline Vec2 scale() const { return _scale; }
		
		/**
		* @func lock_size()
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
		inline const Mat4& root_matrix() const { return _root_matrix; }
		
		/**
		* @func draw_region
		*/
		inline Region display_region() const {
			return _display_region.back();
		}

		/**
		* @func push_display_region
		*/
		void push_display_region(Region value);
		
		/**
		* @func pop_display_region
		*/
		inline void pop_display_region() {
			ASSERT( _display_region.length() > 1 );
			_display_region.pop_back();
		}
		
		/**
		* @func atom_pixel
		*/
		inline float atom_pixel() const { return _atom_pixel; }
		
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
		inline uint32_t fsp() const { return _fsp; }

		/**
		 * @func best_display_scale()
		 */
		inline float best_display_scale() const { return _best_display_scale; }
		inline void set_best_display_scale(float value) { _best_display_scale = value; }
		inline Region surface_region() const { return _surface_region; }
		bool set_surface_region(Region surface_region); // call from render loop
		void render_frame(bool force = false); // call from render loop

		/**
		 * @func phy_size()
		 */
		Vec2 phy_size() const;

		/**
		* @func default_atom_pixel
		*/
		static float default_atom_pixel();

		/**
		* @func default_status_bar_height
		*/
		static float default_status_bar_height();

	private:
		Application*      _host;
		Vec2              _lock_size;  // 锁定视口的尺寸
		Vec2              _size;       // 当前视口尺寸
		Vec2              _scale;   // 当前屏幕显示缩放比,这个值越大size越小显示的内容也越少
		Mat4              _root_matrix;
		float             _atom_pixel;
		float             _best_display_scale;
		List<Region>      _display_region;
		List<Cb>          _next_frame;
		uint32_t          _fsp, _record_fsp;
		int64_t           _record_fsp_time;
		Region            _surface_region;  /* 选择绘图表面有区域 */
		Mutex             _Mutex;
		
		FX_DEFINE_INLINE_CLASS(Inl);
	};

}
#endif
