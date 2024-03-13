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
	class Layout;
	class View;
	class GroupAction;
	class SpawnAction;
	class SequenceAction;
	class KeyframeAction;
	class Window;
	class PreRender;

	/**
	* @class Action
	*/
	class Qk_EXPORT Action: public Reference {
		Qk_HIDDEN_ALL_COPY(Action);
	public:
		typedef List<Action*>::Iterator Id;

		Qk_DEFINE_PROP_GET(Window*, window, Protected);
		Qk_DEFINE_PROP(uint32_t, loop, Const); // @RT get
		Qk_DEFINE_PROP_GET(uint32_t, duration, Const); // @RT get
		Qk_DEFINE_PROP(float, speed, Const); // @RT get
		Qk_DEFINE_PROP_ACC_GET(bool, playing, Const); // @RT get

		Action(Window *win);
		~Action();

		/**
		 * @overwrite
		*/
		void release() override;

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
		void seek(uint32_t time);

		/**
		 * @async
		* @method seek_play
		*/
		void seek_play(uint32_t time);

		/**
		 * @async
		* @method seek_stop
		*/
		void seek_stop(uint32_t time);

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
		virtual void append(Action *child) = 0;

		/**
		* @async
		* @method clear action
		*/
		virtual void clear();

	private:
		void set_target(Layout* t);
		void del_target(Layout* t);
		int  set_parent_RT(Action* parent);
		void del_parent_RT();
		void play_RT();
		void stop_RT();
		void seek_RT(uint32_t time);
		void trigger_ActionLoop_RT(uint32_t delay, Action* root);
		void trigger_ActionKeyframe_RT(uint32_t delay, uint32_t frame_index, Action* root);
	protected:
		virtual uint32_t advance_RT(uint32_t time_span, bool restart, Action *root) = 0;
		virtual void seek_time_RT(uint32_t time, Action *root) = 0;
		virtual void seek_before_RT(uint32_t time, Action *child) = 0;
		virtual void update_duration_RT(int32_t diff);
		virtual void clear_RT() = 0;

		// Props
		Action *_parent; // @RT
		Layout *_target; // @RT
		uint32_t _looped; // @RT
		Id _id; // @RT action id from action center or group action

		friend class View;
		friend class ActionCenter;
		friend class GroupAction;
		friend class SpawnAction;
		friend class SequenceAction;
		friend class KeyframeAction;
	};

	/**
	* @class GroupAction
	*/
	class Qk_EXPORT GroupAction: public Action {
	public:
		GroupAction(Window *win);
		~GroupAction();
	protected:
		virtual void insert_RT(Id after, Action *child) = 0;
		virtual void remove_child_RT(Id id) = 0;
		virtual void clear_RT() override;
		List<Action*> _actions_RT;
		friend class Action;
	};

	/**
	* @class SpawnAction
	*/
	class Qk_EXPORT SpawnAction: public GroupAction {
	public:
		SpawnAction(Window *win);
		virtual void append(Action* child);
	private:
		virtual uint32_t advance_RT(uint32_t time_span, bool restart, Action* root);
		virtual void seek_time_RT(uint32_t time, Action* root);
		virtual void seek_before_RT(uint32_t time, Action* child);
		virtual void insert_RT(Id after, Action *child);
		virtual void remove_child_RT(Id id);
		virtual void update_duration_RT(int32_t diff);
	};

	/**
	* @class SequenceAction
	*/
	class Qk_EXPORT SequenceAction: public GroupAction {
	public:
		SequenceAction(Window *win);
		virtual void append(Action* child);
	private:
		virtual uint32_t advance_RT(uint32_t time_span, bool restart, Action* root);
		virtual void seek_time_RT(uint32_t time, Action* root);
		virtual void seek_before_RT(uint32_t time, Action* child);
		virtual void insert_RT(Id after, Action *child);
		virtual void remove_child_RT(Id id);
		virtual void clear_RT();
		List<Action*>::Iterator _playIdx_RT;
	};

	/**
	* @class ActionCenter
	*/
	class ActionCenter: public Object {
		Qk_HIDDEN_ALL_COPY(ActionCenter)
	public:
		ActionCenter();
		~ActionCenter();
	private:
		struct Action_Wrap {
			Action* value; bool _runAdvance;
		};
		uint32_t _prevTime_RT;
		List<Action_Wrap> _actions_RT;
		/**
		* @method advance_RT() Action scheduling forward frame
		* @thread render
		*/
		void advance_RT(uint32_t time);

		friend class Action;
		friend class PreRender;
	};

}
#endif
