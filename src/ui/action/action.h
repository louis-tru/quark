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

	/**
	* @class Action
	*/
	class Qk_EXPORT Action: public Reference {
		Qk_HIDDEN_ALL_COPY(Action);
	public:
		typedef List<Action*>::Iterator Id;

		Qk_DEFINE_PROP_GET(Action*, parent);
		Qk_DEFINE_PROP(uint32_t, loop, Const);
		Qk_DEFINE_PROP_GET(uint32_t, duration, Const);
		Qk_DEFINE_PROP(float, speed, Const);
		Qk_DEFINE_PROP_ACC_GET(bool, playing, Const);

		Action();
		~Action();

		/**
		* @overwrite
		*/
		void release() override;

		/**
		* @method play
		*/
		void play();

		/**
		* @method stop
		*/
		void stop();

		/**
		* @method seek
		*/
		void seek(uint32_t time);

		/**
		* @method seek_play
		*/
		void seek_play(uint32_t time);

		/**
		* @method seek_stop
		*/
		void seek_stop(uint32_t time);

		/**
		 * @method before
		*/
		void before(Action *act) throw(Error);

		/**
		 * @method after
		*/
		void after(Action *act) throw(Error);

		/**
		 * @method remove from parent action
		*/
		void remove() throw(Error);

		/**
		* @method append child action
		*/
		virtual void append(Action *child) throw(Error) = 0;

		/**
		* @method clear action
		*/
		virtual void clear() = 0;

		/**
		 * @method targets bind
		*/
		inline cSet<Layout*>& targets() const {
			return _targets;
		}

	private:
		void set_parent(Action* parent) throw(Error);
		void add_target(Layout* t) throw(Error);
		void del_parent();
		void del_target(Layout* t);
		void trigger_action_loop(uint32_t delay, Action* root);
		void trigger_action_key_frame(uint32_t delay, uint32_t frame_index, Action* root);

	protected:
		virtual uint32_t advance(uint32_t time_span, bool restart, Action *root) = 0;
		virtual void seek_time(uint32_t time, Action *root) = 0;
		virtual void seek_before(int32_t time, Action *child) = 0;
		virtual void update_duration(int32_t diff);

		// props
		int32_t _looped;
		Set<Layout*> _targets;
		Id _id; // action id from action center or group action
		bool _runAdvance; // run advance for action center

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

		virtual ~GroupAction();

		/**
		* @method length for actions
		*/
		inline uint32_t length() const { return _actions.length(); }

		/**
		* @method first action
		*/
		inline Action* first() { return _actions.front(); }

		/**
		* @method last action
		*/
		inline Action* last() { return _actions.back(); }

		/**
		* @overwrite
		*/
		virtual void clear() override;
		virtual void append(Action *child) throw(Error) override;

	protected:
		virtual void insert(Id after, Action *child) throw(Error);
		virtual void remove_child(Id id) throw(Error) = 0;

		List<Action*> _actions;

		friend class Action;
	};

	/**
	* @class SpawnAction
	*/
	class Qk_EXPORT SpawnAction: public GroupAction {
	public:
		virtual void append(Action* child) throw(Error);
	private:
		virtual uint32_t advance(uint32_t time_span, bool restart, Action* root);
		virtual void seek_time(uint32_t time, Action* root);
		virtual void seek_before(int32_t time, Action* child);
		virtual void insert(Id after, Action *child) throw(Error);
		virtual void remove_child(Id id) throw(Error);
		virtual void update_duration(int32_t diff);
	};

	/**
	* @class SequenceAction
	*/
	class Qk_EXPORT SequenceAction: public GroupAction {
	public:
		virtual void append(Action* child) throw(Error);
		virtual void clear();
	private:
		virtual uint32_t advance(uint32_t time_span, bool restart, Action* root);
		virtual void seek_time(uint32_t time, Action* root);
		virtual void seek_before(int32_t time, Action* child);
		virtual void insert(Id after, Action *child) throw(Error);
		virtual void remove_child(Id id) throw(Error);

		List<Action*>::Iterator _playIdx;
	};

	/**
	* @class ActionCenter
	*/
	class Qk_EXPORT ActionCenter: public Object {
	public:
		ActionCenter();
		~ActionCenter();

		/**
		* @method advance() Action scheduling forward frame
		* @thread render
		*/
		void advance(uint32_t time);

	private:
		uint32_t _prevTime;
		List<Action*> _actions;

		friend class Action;
	};

}
#endif
