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

#ifndef __quark__window__
#define __quark__window__

#include "../util/util.h"
#include "../util/event.h"
#include "../util/cb.h"
#include "../render/math.h"
#include "../render/render.h"
#include "./pre_render.h"
#include "./types.h"

namespace qk {
	class Application;
	class UIRender;
	class EventDispatch;
	class WindowImpl; // window platform impl
	class RenderTask;
	class Root;
	class Window;
	class RootStyleSheets;
	class ActionCenter;
	class FontPool;

	/**
	 * Note: If `main loop` and `render loop` run in different threads,
	 * Then any UI-API function called in the main thread must be locked.
	 */
	class Qk_EXPORT UILock {
	public:
		UILock(Window *win);
		~UILock();
		void lock();
		void unlock();
	private:
		Window *_win;
		bool _lock;
	};

	/**
	 * @class Window system window ui components
	*/
	class Qk_EXPORT Window: public Reference, public RenderBackend::Delegate {
		Qk_HIDDEN_ALL_COPY(Window);
	public:

		struct Options {
			ColorType colorType; // init window color type
			uint16_t  msaa; // init window gpu render msaa count
			uint16_t  fps; // init window render fsp limit
			Rect      frame; // init window params
			String    title; // init window title
			Color     backgroundColor; // init window background color
		};

		struct RegionSize/*: Region*/ {
			Vec2 origin,end;
			Vec2 size; // full surface
		};

		/**
		 * @event onChange show port change event
		*/
		Qk_Event(Change);
		Qk_Event(Background); // @event onBackground, window into background
		Qk_Event(Foreground); // @event onForeground, window into foreground

		/**
		*
		* Vec2,width与height都设置为0时自动设置系统默认显示尺寸
		*
		* 设置视口为一个固定的逻辑尺寸,这个值改变时会触发change事件
		*
		* 同时只能指定一个项目,同时指定宽度与高度不生效
		* 如果设置为非零表示锁定尺寸,不管size怎样变化对于视图这个值永远保持不变
		*/
		Qk_DEFINE_PROP(Vec2, size, Const); //!< current viewport size
		//!< display scale, the larger the value, the smaller the size and the less content displayed
		Qk_DEFINE_PROP_GET(float, scale, Const);
		Qk_DEFINE_PROP_GET(float, defaultScale, Const); //!< default display scale
		Qk_DEFINE_PROP_GET(RegionSize, surfaceRegion, Const); //!< Select the area on the drawing surface
		Qk_DEFINE_PROP_GET(uint32_t, fsp, Const); //!< current fsp
		Qk_DEFINE_PROP_GET(float, atomPixel, Const); // atom pixel size
		Qk_DEFINE_PROP_GET(Root*, root); //! root view
		Qk_DEFINE_PROP_GET(Application*, host); //! application host
		Qk_DEFINE_PROP_GET(Render*, render); //! render object
		Qk_DEFINE_PROP_GET(EventDispatch*, dispatch); //! event dispatch
		Qk_DEFINE_PROP_GET(RootStyleSheets*, styleSheets); //! root style sheets
		Qk_DEFINE_PROP(Color, backgroundColor, Const); //! background color
		Qk_DEFINE_PROP_GET(WindowImpl*, impl); //! window platform impl
		Qk_DEFINE_PROP_GET(ActionCenter*, actionCenter); //! Action scheduling
		Qk_DEFINE_PROP_ACC_GET(FontPool*, fontPool); //! Font pool
		Qk_DEFINE_PROP_ACC_GET(RunLoop*, loop); //! host main loop
		/**
		 * @prop surfaceSize
		 * returns surface only display region and size
		 */
		Qk_DEFINE_PROP_ACC_GET(Vec2, surfaceSize, Const);

		/**
		 * @static
		 * @method Make(opts) create new window object
		*/
		static Window* Make(Options opts);

		/**
		 * @destructor
		*/
		virtual ~Window();

		/**
		* @method getClipRegion
		*/
		const RegionSize& getClipRegion() const {
			return _clipRegion.back();
		}

		/**
		* @method clipRegion
		*/
		void clipRegion(Region value);

		/**
		 * @method clipRestore()
		*/
		void clipRestore();

		/**
		* @method nextFrame()
		*/
		void nextFrame(cCb& cb);

		/**
		 * @method activate() activate window
		*/
		void activate();

		/**
		 * @method close() close window object
		*/
		void close();

		/**
		 * @method pending() suspend ui window object
		 */
		void pending();

		/**
		 * @func preRender()
		*/
		inline PreRender& preRender() {
			return _preRender;
		}

		/**
		 * @method set_fullscreen(fullscreen)
		*/
		void setFullscreen(bool fullscreen);

		/**
		* @method setCursorStyle
		*/
		void setCursorStyle(CursorStyle cursor, bool low = false);

	private:
		void reload();
		void solveNextFrame();
		void onRenderBackendReload(Region region, Vec2 size, float defaultScale) override;
		bool onRenderBackendDisplay() override;
		void openImpl(Options &opts);
		void closeImpl();
		bool Destroy(); // destroy window and protform window

		/**
		 * Create an application object before creating a window
		 * 
		 * @constructor
		 * @param opts {Options} create options
		*/
		Window(Options &opts);

		// props data
		UIRender       *_uiRender;
		Vec2           _lockSize;  //!< Lock the size of the viewport
		List<Cb>       _nextFrame;
		uint32_t       _fspTick;
		int64_t        _fspTime;
		Array<RegionSize> _clipRegion;
		List<Window*>::Iterator _id;
		RecursiveMutex _renderMutex;
		PreRender _preRender;
		friend class WindowImpl;
		friend class UILock;
	};

}
#endif
