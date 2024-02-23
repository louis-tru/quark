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
#include "../util/list.h"

namespace qk {
	class View;
	class GroupAction;
	class SpawnAction;
	class SequenceAction;
	class KeyframeAction;

	/**
	* @class ActionCenter
	*/
	class Qk_EXPORT ActionCenter: public Object {
	public:

		ActionCenter();
		~ActionCenter();

		/**
		* @method advance() Action scheduling forward frame
		*/
		void advance(int64_t time);

	private:
		uint64_t _prev_time;
		List<Action*> _actions;
	};

	/**
	* @class Action
	*/
	class Qk_EXPORT Action: public Reference {
	public:
		Qk_DEFINE_PROP_GET(Action*, parent);
		Qk_DEFINE_PROP(uint32_t, loop, Const);
		Qk_DEFINE_PROP(uint64_t, delay, Const);
		Qk_DEFINE_PROP(float, speed, Const);
		Qk_DEFINE_PROP_ACC_GET(bool, playing, Const);
		Qk_DEFINE_PROP_ACC_GET(uint64_t, duration, Const);

		Action();
		~Action();

		/**
		* @overwrite
		*/
		void release() override;

		/**
		* @method clear
		*/
		virtual void clear() = 0;

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
		void seek(int64_t time);

		/**
		* @method seek_play
		*/
		void seek_play(int64_t time);

		/**
		* @method seek_stop
		*/
		void seek_stop(int64_t time);

	protected:
		virtual uint64_t advance(uint64_t time_span, bool restart, Action *root) = 0;
		virtual void seek_time(uint64_t time, Action *root) = 0;
		virtual void seek_before(int64_t time, Action *child) = 0;

		// props
		int64_t _looped, _delayed;
		int64_t _full_duration;
		List<View*> _views;
		List<Action*>::Iterator _id; // action id from action center or group action
		bool _playing; // for add action center play

		friend class ActionCenter;
		friend class View;

	private:
		void set_parent(Action* parent) throw(Error);
		void add_view(View* view) throw(Error);
		void del_parent();
		void del_view(View* view);
		void update_duration(int64_t diff);
	};

	/**
	* @class GroupAction
	*/
	class Qk_EXPORT GroupAction: public Action {
	public:
		/**
		* @method length
		*/
		inline uint32_t length() const { return _actions.length(); }

		/**
		* @method front
		*/
		inline Action* front() { return _actions.front(); }
		inline Action* back() { return _actions.back(); }

		/**
		* @method append
		*/
		virtual void append(Action *child) throw(Error);

		/**
		* @method insert
		*/
		virtual void insert(Action *after, Action *child) throw(Error);

		/**
		* @method remove_child
		*/
		virtual void remove_child(Action *child) throw(Error);

		/**
		* @overwrite
		*/
		virtual void clear() override;

	protected:
		virtual ~GroupAction();
		virtual void bind_view(View* view);

		List<Action*> _actions;
	};

	/**
	* @class SpawnAction
	*/
	class Qk_EXPORT SpawnAction: public GroupAction {
	public:
		virtual void append(Action* child) throw(Error);
		virtual void insert(Action *after, Action *child) throw(Error);
		virtual void remove_child(Action *child) throw(Error);
	private:
		virtual uint64_t advance(uint64_t time_span, bool restart, Action* root);
		virtual void seek_time(uint64_t time, Action* root);
		virtual void seek_before(int64_t time, Action* child);
	};

	/**
	* @class SequenceAction
	*/
	class Qk_EXPORT SequenceAction: public GroupAction {
	public:
		virtual void append(Action* child) throw(Error);
		virtual void insert(Action *after, Action *child) throw(Error);
		virtual void remove_child(Action *child) throw(Error);
		virtual void clear();
	private:
		virtual uint64_t advance(uint64_t time_span, bool restart, Action* root);
		virtual void seek_time(uint64_t time, Action* root);
		virtual void seek_before(int64_t time, Action* child);
	};

}
#endif