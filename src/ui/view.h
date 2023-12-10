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

#ifndef __quark__ui__view__
#define __quark__ui__view__

#include "./event.h"
#include "./pre_render.h"

namespace qk {
	class Action;
	class Layout;
	class Button;

	#define Qk_Define_View(ViewName, Base) \
		typedef ViewName##Layout Layout; \
		inline ViewName(ViewName##Layout *layout): Base(layout) {}

	#define Qk_IMPL_VIEW_PROP_ACC_GET(cls, type, name, ...) \
		type cls::name() Qk_DEFINE_PROP_MODIFIER##__VA_ARGS__ { return layout<cls##Layout>()->name(); }
	#define Qk_IMPL_VIEW_PROP_ACC_SET(cls, type, name) \
		void cls::set_##name(type val) { \
			preRender().async_call([](auto ctx, auto val) { ctx->set_##name(val); }, layout<cls##Layout>(), val); \
		}
	#define Qk_IMPL_VIEW_PROP_ACC(cls, type, name, ...) \
		Qk_IMPL_VIEW_PROP_ACC_GET(cls, type, name, ##__VA_ARGS__) Qk_IMPL_VIEW_PROP_ACC_SET(cls, type, name)

	/**
		* The basic elements of UI view tree
		*
		* This tree is used to isolate operations from rendering and layout, 
		* and to map relationships with the layout tree. The layout tree delay follows the view tree structure.
		*
		* This object view can only be called during the main thread operation.
		*
		* @class View
		*/
	class Qk_EXPORT View: public Notification<UIEvent, UIEventName, Reference> {
		Qk_HIDDEN_ALL_COPY(View);
	public:
		typedef Layout Layout;

		/*
		* @field window
		*/
		Qk_DEFINE_PROP_ACC_GET(Window*, window);
		/**
		 * the objects that automatically adjust view properties
		*/
		Qk_DEFINE_PROP(Action*, action);
		Qk_DEFINE_PROP_GET(Layout*, layout);
		Qk_DEFINE_PROP_GET(View*, parent);
		Qk_DEFINE_PROP_GET(View*, prev);
		Qk_DEFINE_PROP_GET(View*, next);
		Qk_DEFINE_PROP_GET(View*, first);
		Qk_DEFINE_PROP_GET(View*, last);
		/**
		 *  can affect the transparency of subviews
		 */
		Qk_DEFINE_PROP_ACC(float, opacity);
		/**
		 * @field level
		*/
		Qk_DEFINE_PROP_ACC_GET(uint32_t, level);
		/**
		 * Set the visibility of the view. When this value is set to 'false',
		 * the view is invisible and does not occupy any layout space
		*/
		Qk_DEFINE_PROP_ACC(bool, visible);
		/**
		 * keyboard focus view
		*/
		Qk_DEFINE_PROP_ACC(bool, is_focus);
		/**
		 * Do views need to receive or handle system event throws? In most cases,
		 * these events do not need to be handled, which can improve overall event processing efficiency
		*/
		Qk_DEFINE_PROP_ACC(bool, receive);

		/**
		 * @constructor
		*/
		View(Layout *layout);

		/**
		 * @destructor
		*/
		virtual ~View();

		template<class T = Layout> T* layout() const {
			return static_cast<T*>(_layout);
		}

		template<class View = View> inline View* newView();

		template<class View = View> inline View prepend_new() {
			return newView<View>()->template prepend_to<View>(this);
		}

		template<class View = View> inline View* append_new() {
			return newView<View>()->template append_to<View>(this);
		}

		template<class T = View> inline T* prepend_to(View* parent) {
			parent->prepend(this); return static_cast<T*>(this);
		}

		template<class T = View> inline T* append_to(View* parent) {
			parent->append(this); return static_cast<T*>(this);
		}

		/**
		 *
		 * focus keyboard
		 *
		 * @method focus()
		 */
		bool focus();

		/**
		 *
		 * Unfocus keyboard
		 *
		 * @method blur()
		 */
		bool blur();

		/**
		 * @method is_self_child(child)
		 */
		bool is_self_child(View *child);

		/**
			*
			* Add a sibling view to the front
			*
			* @method before(view)
			*/
		void before(View* view);

		/**
			*
			* Add a sibling view to the back
			*
			* @method after(view)
			*/
		void after(View* view);

		/**
			* 
			* Append subview to front
			* 
			* @method prepend(child)
			*/
		void prepend(View* child);

		/**
			*
			* Append subview to end
			*
			* @method append(child)
			*/
		void append(View* child);

		/**
		 *
		 * Remove self from parent view
		 *
		 * @method remove()
		 */
		void remove();

		/**
		 *
		 * remove all subview
		 *
		 * @method remove_all_child()
		 */
		void remove_all_child();

		/**
		 *
		 * Can it be the focus
		 *
		 * @method can_become_focus()
		 */
		virtual bool can_become_focus();

		/**
		 * 
		 * Returns button object
		 * 
		 * @method as_buttn()
		*/
		virtual Button* as_button();

		/**
		 * @override
		 */
		virtual void release() override;

		/**
		 * @method preRender()
		*/
		PreRender& preRender();

	private:
		/**
		 * @method set_parent(parent) setting parent view
		 */
		void set_parent(View* parent);
		void clear_link(); // Cleaning up associated view information

		friend class EventDispatch;

		Qk_DEFINE_INLINE_CLASS(InlEvent);
	};
}
#endif
