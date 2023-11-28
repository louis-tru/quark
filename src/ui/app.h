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
	int __f_main__(int, Char**); \
	Qk_INIT_BLOCK(__f_main__) { qk::Application::setMain(&__f_main__); } \
	int __f_main__(int argc, Char** argv)

namespace qk {
	class Screen;
	class DefaultTextOptions;
	class FontPool;
	class ImageSourcePool;
	class Window;

	/**
	 *
	 * About events in UI:
	 * All events in the UI are triggered in the `main loop`, and are not thread-safe,
	 * So adding event listeners must also be in the `main loop`.
	 *
	 * @class Application
	*/
	class Qk_EXPORT Application: public Object {
		Qk_HIDDEN_ALL_COPY(Application);
	public:

		// @props
		Qk_DEFINE_PROP_GET(bool, isLoaded);
		Qk_DEFINE_PROP_GET(DefaultTextOptions*, defaultTextOptions); //! default font settings
		Qk_DEFINE_PROP_GET(Screen*, screen); //! screen object
		Qk_DEFINE_PROP_GET(RunLoop*, loop); //! main run loop
		Qk_DEFINE_PROP_GET(FontPool*, fontPool); //! font and font familys manage
		Qk_DEFINE_PROP_GET(ImageSourcePool*, imgPool); //! image loader and image cache
		Qk_DEFINE_PROP_GET(Window*, activeWindow); //! current active window
		//! Resource memory limit, cannot be less than 64MB, the default is 512MB.
		Qk_DEFINE_PROP(uint64_t, maxResourceMemoryLimit);
		Qk_DEFINE_PROP_ACC_GET(const List<Window*>&, windows); //! all window list

		// @events
		Qk_Event(Load);
		Qk_Event(Unload);
		Qk_Event(Pause);
		Qk_Event(Resume);
		Qk_Event(Memorywarning);

		/**
		 * @constructor
		*/
		Application(RunLoop *loop = RunLoop::current());

		/**
		 * @method ~Application()
		*/
		virtual ~Application();

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
										cString& cc = String(),
										cString& bcc = String(), cString& body = String());
		/**
		 * 
		 * setting main function
		 *
		 * @method setMain()
		 */
		static void setMain(int (*main)(int, char**));

		/**
		 * @method runMain(argc, argv) create sub gui thread, call by system, First thread call
		*/
		static void runMain(int argc, char* argv[]);

		/**
		 * @method app Get current gui application entity
		*/
		static inline Application* shared() { return _shared; }

	private:
		static Application *_shared;   //! current shared application
		KeepLoop*      _keep;
		List<Window*>  _windows; // window list
		Mutex          _mutex;

		Qk_DEFINE_INLINE_CLASS(Inl);
		friend class UILock;
		friend class Window;
	};

	typedef Application App;

	inline Application* shared_app() {
		return Application::shared();
	}

	//@private head
	Qk_DEFINE_INLINE_MEMBERS(Application, Inl) {
	public:
		void triggerLoad();
		void triggerUnload();
		void triggerPause();
		void triggerResume();
		void triggerBackground(Window *win);
		void triggerForeground(Window *win);
		void triggerMemorywarning();
		void setActiveWindow(Window *win);
	};
}
#endif