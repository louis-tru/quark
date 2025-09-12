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

#include "../util/util.h"
#include "../util/event.h"
#include "../util/loop.h"
#include "../render/pixel.h"
#include "./types.h"

#define Qk_Main() \
	int __qk_main_func__(int, char**); \
	Qk_Init_Func(__qk_main_func__) { qk::Application::setMain(&__qk_main_func__); } \
	int __qk_main_func__(int argc, char** argv)

namespace qk {
	class Screen;
	class DefaultTextOptions;
	class FontPool;
	class ImageSourcePool;
	class Window;
	class RootStyleSheets;

	/**
	 *
	 * About events in UI:
	 * All events in the UI are triggered in the `main loop`, and are not thread-safe,
	 * So adding event listeners must also be in the `main loop`.
	 *
	 * @class Application
	*/
	class Qk_EXPORT Application: public Object {
		Qk_DISABLE_COPY(Application);
	public:
		// @events
		Qk_Event(Load);
		Qk_Event(Unload);
		Qk_Event(Pause);
		Qk_Event(Resume);
		Qk_Event(Memorywarning);

		// @props
		Qk_DEFINE_PROP_GET(bool, isLoaded, Const);
		Qk_DEFINE_PROP_GET(DefaultTextOptions*, defaultTextOptions); //! default font settings
		Qk_DEFINE_PROP_GET(Screen*, screen); //! screen object
		Qk_DEFINE_PROP_GET(RunLoop*, loop); //! main run loop
		Qk_DEFINE_PROP_GET(FontPool*, fontPool); //! font and font families manage
		Qk_DEFINE_PROP_GET(ImageSourcePool*, imgPool); //! image loader and image cache
		Qk_DEFINE_PROP_GET(Window*, activeWindow); //! current active window
		//! Resource memory limit, cannot be less than 64MB, the default is 512MB.
		Qk_DEFINE_PROPERTY(uint64_t, maxResourceMemoryLimit, Const);
		Qk_DEFINE_ACCE_GET(cList<Window*>&, windows, Const); //! all window list

		/**
		 * @constructor
		*/
		Application();

		/**
		 * @method ~Application()
		*/
		~Application();

		/**
		 * start run application message loop
		*/
		void run();

		/**
		 * Clean up garbage and recycle memory resources, all=true clean up all resources
		*/
		void clear(bool all = false);

		/**
		 * The amount of memory used by the current resource data,
		 * including image textures and font cache and pathv cache
		*/
		uint32_t usedResourceMemory() const;

		/**
		 * @method openURL()
		*/
		void openURL(cString& url);

		/**
		 * @method send_email
		*/
		void sendEmail(cString& recipient,
										cString& subject,
										cString& body, cString& cc = String(), cString& bcc = String());

		/**
		 * Safe lock all window render threads, unlocked after the callback is completed
		 * 
		 * @method lockAllRenderThreads()
		*/
		void lockAllRenderThreads(Cb cb);

		/**
		 * 
		 * setting main function and it's on the sub thread calling
		 *
		 * @method setMain()
		 */
		static void setMain(int (*main)(int, char**));

		/**
		 * Create sub gui thread call by system, and root thread call,
		 * when creating new the Application object return
		 * 
		 * @method runMain(argc, argv, waitNewApp)
		*/
		static void runMain(int argc, char* argv[], bool waitNewApp);

		/**
		 * @method app Get current gui application entity
		*/
		static inline Application* shared() { return _shared; }

	private:
		static Application *_shared;   //! current shared application
		List<Window*>  _windows; // window list
		Mutex          _mutex;
		uint32_t       _tick; // tick check id

		Qk_DEFINE_INLINE_CLASS(Inl);
		friend class UILock;
		friend class Window;
	};

	typedef Application App;

	inline Application* shared_app() {
		return Application::shared();
	}

	Qk_EXPORT void post_message_main(Cb cb, bool sync);
}
#endif
