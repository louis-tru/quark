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

#include "./util/util.h"
#include "./util/event.h"
#include "./util/loop.h"
#include "./types.h"
#include "./render/pixel.h"

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
		Qk_DEFINE_PROP_GET(bool, is_loaded);
		Qk_DEFINE_PROP_GET(DefaultTextOptions*, default_text_options); //! default font settings
		Qk_DEFINE_PROP_GET(Screen*, screen); //! screen object
		Qk_DEFINE_PROP_GET(RunLoop*, loop); //! main run loop
		Qk_DEFINE_PROP_GET(FontPool*, font_pool); //! font and font familys manage
		Qk_DEFINE_PROP_GET(ImageSourcePool*, img_pool); //! image loader and image cache

		// @events
		Qk_Event(Load);
		Qk_Event(Unload);
		Qk_Event(Background);
		Qk_Event(Foreground);
		Qk_Event(Pause);
		Qk_Event(Resume);
		Qk_Event(Memorywarning);

		/**
		 * Note: If `main loop` and `render loop` run in different threads,
		 * Then any UI-API function called in the main thread must be locked.
		 */
		class Qk_EXPORT UILock {
		public:
			UILock(Application* host = _shared);
			~UILock();
			void lock();
			void unlock();
		private:
			Application* _host;
			bool _lock;
		};

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
		 * @method pending() suspend ui application process
		 */
		void pending();

		/**
		 * Clean up garbage and recycle memory resources, all=true clean up all resources
		*/
		void clear(bool all = false);

		/**
		 * @method max_image_memory_limit()
		*/
		uint64_t max_image_memory_limit() const;

		/**
		 * Set the texture memory limit, which cannot be less than 64MB, and the default is 512MB.
		*/
		void set_max_image_memory_limit(uint64_t limit);

		/**
		 * The amount of memory used by the current texture data, including image textures and font textures
		*/
		uint64_t used_image_memory() const;

		/**
		 * @method adjust_image_memory()
		*/
		bool adjust_image_memory(uint64_t will_alloc_size);

		/**
		 * @method open_url()
		*/
		void open_url(cString& url);

		/**
		 * @method send_email
		*/
		void send_email(cString& recipient,
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
		void handleExit(Event<>& e);

		static Application*  _shared;   //! current shared application
		KeepLoop*      _keep;
		RecursiveMutex _render_mutex;
		//! Texture memory limit, cannot be less than 64MB, the default is 512MB.
		uint64_t       _max_image_memory_limit;

		Qk_DEFINE_INLINE_CLASS(Inl);
		friend class UILock;
	};

	typedef Application::UILock UILock;
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
		void triggerBackground();
		void triggerForeground();
		void triggerMemorywarning();
	};
	//@end
}
#endif
