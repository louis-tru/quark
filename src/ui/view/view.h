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

#include "../event.h"

namespace qk {
	class Action;
	class Window;
	class Layout;

	/**
		* The basic elements of UI tree
		*
	 * @class View
		*/
	class Qk_EXPORT View: public Notification<UIEvent, UIEventName> {
		Qk_HIDDEN_ALL_COPY(View);
	public:
		// @props
		Qk_DEFINE_PROP(Action*, action); // the objects that automatically adjust view properties
		Qk_DEFINE_PROP_GET(Window*, window); // window object
		Qk_DEFINE_PROP_GET(Layout*, layout); // layout backend
		Qk_DEFINE_PROP_GET(View*, parent);
		Qk_DEFINE_PROP_GET(View*, prev);
		Qk_DEFINE_PROP_GET(View*, next);
		Qk_DEFINE_PROP_GET(View*, first);
		Qk_DEFINE_PROP_GET(View*, last);
		/*
		* 视图节点在UI树中所处的层级，0表示还没有加入到UI视图树中
		* 这个值受`View::_visible`影响, View::_visible=false时_level=0
		*/
		Qk_DEFINE_PROP_GET(uint32_t, level);
		// 设置视图的可见性，这个值设置为`false`时视图为不可见且不占用任何布局空间
		Qk_DEFINE_PROP(bool, visible);
		Qk_DEFINE_PROP_ACC(bool, is_focus); // keyboard focus view

		/**
		 * @constructor
		*/
		View();

		/**
		 * @destructor
		*/
		virtual ~View();

		template<class T = View> inline T* prepend_new() {
			return New<T>()->template prepend_to<T>(this);
		}

		template<class T = View> inline T* append_new() {
			return New<T>()->template append_to<T>(this);
		}

		/**
		 * Prepend subview to parent
		 *
		 * @method prepend_to(parent)
		 */
		template<class T = View> inline T* prepend_to(View* parent) {
			parent->prepend(this); return static_cast<T*>(this);
		}

		/**
		 * Append subview to parent
		 *
		 * @method append_to(parent)
		 */
		template<class T = View> inline T* append_to(View* parent) {
			parent->append(this); return static_cast<T*>(this);
		}

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
		 *
		 * Can it be the focus
		 *
		 * @method can_become_focus()
		 */
		virtual bool can_become_focus();

		/**
		 * @method is_self_child(child)
		 */
		bool is_self_child(View *child);

	private:
		void clear_link(); // Cleaning up associated view information
		void clear_level(Window *win); //  clear layout depth
		void set_level(uint32_t level, Window *win); // settings depth
		void set_visible_(bool visible, uint32_t level);
		void set_parent(View* parent);
		bool is_root();

		friend class Window;
		Qk_DEFINE_INLINE_CLASS(InlEvent);
	};

}
#endif
