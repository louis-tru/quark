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

#ifndef __noug__display__
#define __noug__display__

#include "./util/util.h"
#include "./util/event.h"
#include "./math.h"
#include "./value.h"
#include "./util/cb.h"
#include "./util/loop.h"

namespace noug {

	class Application;

	/**
	* 提供的一些对显示与屏幕的常用方法属性与事件
	* @class Display
	*/
	class F_EXPORT Display: public Reference {
		F_HIDDEN_ALL_COPY(Display);
		public:

			struct DisplayRegion {
				float x, y, x2, y2, width, height;
			};

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
			* @thread main
			* @event onchange 显示端口变化事件
			*/
			F_Event(Change);

			/**
			* @thread main
			* @event onorientation 屏幕方向发生改变触发
			*/
			F_Event(Orientation);
			
			/**
			* @func size 当前视口尺寸
			*/
			inline Vec2 size() const { return _size; }
			
			/**
			* @func scale_value
			*/
			inline float scale() const { return _scale; }

			/**
			* @func set_size()
			*
			* width与height都设置为0时自动设置系统默认显示尺寸
			*
			* 设置视口为一个固定的逻辑尺寸,这个值改变时会触发change事件
			*
			* 同时只能指定一个项目,同时指定宽度与高度不生效
			* 如果设置为非零表示锁定尺寸,不管display_size怎样变化对于视图这个值永远保持不变
			*
			*/
			void set_size(float width = 0, float height = 0);
			
			/**
			 * @func phy_size()
			 */
			Vec2 phy_size() const;

			/**
			* @thread rebder
			* @func push_clip_region
			*/
			void push_clip_region(Region value);
			
			/**
			* @thread rebder
			* @func pop_clip_region()
			*/
			void pop_clip_region();
			
			/**
			* @func display_region
			*/
			inline DisplayRegion clip_region() const {
				return _clip_region.back();
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
			 * @func default_scale()
			 */
			inline float default_scale() const { return _default_scale; }
			inline DisplayRegion display_region() const { return _display_region; }

			/**
			 * @thread render
			 */
			void set_default_scale(float value);

			/**
			 * @thread render
			 */
			bool set_display_region(DisplayRegion display_region); // call from render loop

			/**
			 * @thread render
			 */
			void render(bool need = false/*force render*/); // call from render loop
			
			/**
			* @func default_atom_pixel
			*/
			static float default_atom_pixel();

			/**
			* @func default_status_bar_height
			*/
			static float default_status_bar_height();

		private:
		
			void updateState();
			void solve_next_frame();

			// member data
			Application*      _host;
			Vec2              _lock_size;  // 锁定视口的尺寸
			Vec2              _size;       // 当前视口尺寸
			float             _scale;   // 当前屏幕显示缩放比,这个值越大size越小显示的内容也越少
			float             _atom_pixel;
			float             _default_scale;
			List<Cb>          _next_frame;
			uint32_t          _fsp, _next_fsp;
			int64_t           _next_fsp_time;
			Array<DisplayRegion> _clip_region;
			DisplayRegion     _display_region;  /* 选择绘图表面有区域 */
	};

	typedef Display::DisplayRegion DisplayRegion;

}
#endif
