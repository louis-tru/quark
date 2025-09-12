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

#ifndef __quark__action__action__
#define __quark__action__action__

#include "../types.h"
#include "../../util/list.h"

namespace qk {
	class View;
	class ActionGroup;
	class SpawnAction;
	class SequenceAction;
	class KeyframeAction;
	class Window;
	class PreRender;
	class CStyleSheets;
	class CStyleSheetsClass;

	/**
	* @class Action
	* @note Release calls can only be made on the main thread
	*/
	class Qk_EXPORT Action: public Reference {
		Qk_DISABLE_COPY(Action);
	public:
		typedef List<Action*>::Iterator Id;

		Qk_DEFINE_PROP_GET(Window*, window, Protected);
		Qk_DEFINE_PROPERTY(uint32_t, loop, Const); // 0xffffffff for infinite loop
		Qk_DEFINE_PROP_GET(uint32_t, duration, Const); // Duration of the action in milliseconds
		Qk_DEFINE_PROPERTY(float, speed, Const); // Range from 0.01 to 1e2, default 1.0
		Qk_DEFINE_ACCESSOR(bool, playing, Const);

		Action(Window *win);
		~Action();

		/**
		 * @method destroy()
		 */
		void destroy() override;

		/**
		 * @async
		* @method play
		*/
		void play();

		/**
		 * @async
		* @method stop async
		*/
		void stop();

		/**
		 * @async
		* @method seek async
		*/
		void seek(uint32_t timeMs);

		/**
		 * @async
		* @method seek_play
		*/
		void seek_play(uint32_t timeMs);

		/**
		 * @async
		* @method seek_stop
		*/
		void seek_stop(uint32_t timeMs);

		/**
		 * @async
		 * @method before
		*/
		void before(Action *act);

		/**
		* @async
		* @method after
		*/
		void after(Action *act);

		/**
		* @async
		* @method remove from parent action
		*/
		void remove();

		/**
		* @async
		* @method append child action
		*/
		virtual void append(Action *child) throw(Error) = 0;

		/**
		* @async
		* @method clear action
		*/
		virtual void clear() = 0;

		/**
		 * Safely use and hold action objects in rendering thread,
		 * Because action objects may be destroyed at any time on the main thread
		 * @method tryRetain() Returns safe self hold
		*/
		Action* tryRetain_Rt();

	private:
		void set_target(View* t);
		void del_target(View* t);
		int  set_parent(ActionGroup* parent);
		void del_parent();
		void play_Rt(); // Rt
		void stop_Rt();
		void seek_Rt(uint32_t time);
		void trigger_ActionLoop_Rt(uint32_t delay, Action* root);
		void trigger_ActionKeyframe_Rt(uint32_t delay, uint32_t frame_index, Action* root);
		// Only allow the inner call, make sure not to access any members in non-RT threads.
		void release_inner_Rt(); // Only release keyframe action.
		bool next_loop_Rt();
	protected:
		virtual uint32_t advance_Rt(uint32_t time_span, bool restart, Action *root) = 0;
		virtual void seek_time_Rt(uint32_t time, Action *root) = 0;
		virtual void seek_before_Rt(uint32_t time, Action *child) = 0;
		virtual void setDuration(int32_t diff);

		// Props
		ActionGroup *_parent;
		View *_target;
		uint32_t _looped_Rt; // @thread Rt
		Id _id_Rt; // @thread Rt action id from action center or group action

		friend class View;
		friend class ActionCenter;
		friend class ActionGroup;
		friend class SpawnAction;
		friend class SequenceAction;
		friend class KeyframeAction;
		friend class Sprite;
	};

	/**
	* @class ActionGroup
	*/
	class Qk_EXPORT ActionGroup: public Action {
	public:
		ActionGroup(Window *win);
		virtual void append(Action* child) throw(Error) override ;
	protected:
		virtual void insertChild(Id after, Action *child);
		virtual void removeChild(Id id) = 0;
		virtual void clear() override;
		virtual bool isSequence();
		Set<Action*> _actions;
		List<Action*> _actions_Rt;
		friend class Action;
	};

	/**
	* @class SpawnAction
	*/
	class Qk_EXPORT SpawnAction: public ActionGroup {
	public:
		SpawnAction(Window *win);
	private:
		uint32_t advance_Rt(uint32_t time_span, bool restart, Action* root) override;
		void seek_time_Rt(uint32_t time, Action* root) override;
		void seek_before_Rt(uint32_t time, Action* child) override;
		void insertChild(Id after, Action *child) override;
		void removeChild(Id id) override;
		void setDuration(int32_t diff) override;
	};

	/**
	* @class SequenceAction
	*/
	class Qk_EXPORT SequenceAction: public ActionGroup {
	public:
		SequenceAction(Window *win);
	private:
		bool isSequence() override;
		uint32_t advance_Rt(uint32_t time_span, bool restart, Action* root) override;
		void seek_time_Rt(uint32_t time, Action* root) override;
		void seek_before_Rt(uint32_t time, Action* child) override;
		void insertChild(Id after, Action *child) override;
		void removeChild(Id id) override;
		Id _play_Rt;
		friend class ActionGroup;
	};

	/**
	* @class ActionCenter
	*/
	class ActionCenter: public Object {
		Qk_DISABLE_COPY(ActionCenter)
	public:
		Qk_DEFINE_PROP_GET(Window*, window)
		ActionCenter(Window *window);
		~ActionCenter();
	private:
		void addCSSTransition_Rt(View *view, CStyleSheets *css);
		void removeCSSTransition_Rt(View *view);

		struct Action_Wrap {
			Action* value;
			bool _runAdvance;
		};
		uint32_t _prevTime_Rt;
		List<Action_Wrap> _actions_Rt;
		Dict<uint64_t, Array<Action*>> _CSSTransitions_Rt;

		/**
		* @method advance_Rt() Action scheduling forward frame
		* @thread render
		*/
		void advance_Rt(uint32_t timeMs);

		friend class View;
		friend class Action;
		friend class PreRender;
		friend class CStyleSheetsClass;
	};

}
#endif
