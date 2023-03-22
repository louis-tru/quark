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

#ifndef __quark__display__
#define __quark__display__

#include "./util/util.h"
#include "./util/event.h"
#include "./math.h"
#include "./types.h"
#include "./util/cb.h"
#include "./util/loop.h"

namespace qk {
	class Application;

	/**
	 * @class Display Provide some common method properties and events for display and screen
	*/
	class Qk_EXPORT Display: public Reference {
		Qk_HIDDEN_ALL_COPY(Display);
	public:

		struct RegionSize/*: Region*/ {
			Vec2 origin,end;
			Vec2 size; // full surface
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
		 * @event onChange show port change event
		*/
		Qk_Event(Change);

		/**
		 * @event onOrientation Triggered when the screen orientation changes
		*/
		Qk_Event(Orientation);
		
		/**
		* @method size 当前视口尺寸
		*/
		inline Vec2 size() const { return _size; }
		
		/**
		 * @method scale_value
		 */
		inline float scale() const { return _scale; }

		/**
		* @method set_size()
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
		* @thread render
		* @method push_clip_region
		*/
		void push_clip_region(Region value);
		
		/**
		 * @thread render
		 * @method pop_clip_region()
		*/
		void pop_clip_region();
		
		/**
		* @method clip_region
		*/
		inline const RegionSize& clip_region() const {
			return _clip_region.back();
		}

		/**
		* @method atom_pixel
		*/
		inline float atom_pixel() const { return _atom_pixel; }
		
		/**
		* @method next_frame()
		*/
		void next_frame(cCb& cb);

		/**
		* @method keep_screen(keep)
		*/
		void keep_screen(bool keep);

		/**
		* @method status_bar_height()
		*/
		float status_bar_height();

		/**
		* @method set_visible_status_bar(visible)
		*/
		void set_visible_status_bar(bool visible);

		/**
		* @method set_status_bar_style(style)
		*/
		void set_status_bar_style(StatusBarStyle style);

		/**
		 * @method request_fullscreen(fullscreen)
		*/
		void request_fullscreen(bool fullscreen);

		/**
		 * @method orientation()
		*/
		Orientation orientation();
		
		/**
		 * @method set_orientation(orientation)
		*/
		void set_orientation(Orientation orientation);
		
		/**
		 * @method fsp()
		*/
		inline uint32_t fsp() const { return _fsp; }

		/**
		 * returns surface only display region and size
		 */
		inline Vec2 surface_size() const {
			return _surface_region.end - _surface_region.origin;
		}

		/**
		 * returns the default display scale
		 */
		inline float      default_scale() const { return _default_scale; }
		inline RegionSize surface_region() const { return _surface_region; }

		/**
		 * settings the display screen surface pixel region and size
		 */
		bool set_surface_region(RegionSize region, float defaultScale);

		/**
		 * pre render
		 * @thread render
		 */
		bool pre_render(); //!< call from render loop

		/**
		 * @thread render
		 */
		void render(); //!< call from render loop
		
		/**
		 * @method default_atom_pixel
		*/
		static float default_atom_pixel();

		/**
		 * @method default_status_bar_height
		*/
		static float default_status_bar_height();

	private:
		void updateState(void *lock);
		void solve_next_frame();

		// member data
		Application*      _host;
		Vec2              _set_size;  //!< Lock the size of the viewport
		Vec2              _size;   //!< current viewport size
		//!< display scale, the larger the value, the smaller the size and the less content displayed
		float             _scale;
		float             _atom_pixel;
		float             _default_scale;
		List<Cb>          _next_frame;
		uint32_t          _fsp, _next_fsp;
		int64_t           _next_fsp_time;
		Array<RegionSize> _clip_region;
		RegionSize        _surface_region; //!< Select the area on the drawing surface
	};

}
#endif
