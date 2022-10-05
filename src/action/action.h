/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright (c) 2015, xuewen.chu
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of xuewen.chu nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL xuewen.chu BE LIABLE FOR ANY
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

#include "../value.h"
#include "../bezier.h"
#include "../event.h"
#include "../property.h"
#include "../fill.h"
#include "../util/list.h"

Qk_NAMESPACE_START

class View;
class ActionCenter;
class GroupAction;
class SpawnAction;
class SequenceAction;
class KeyframeAction;

/**
* @class Action
*/
class Qk_EXPORT Action: public Reference {
public:
	Action();
	
	/**
	* @destructor
	*/
	virtual ~Action();
	
	/**
	* @overwrite
	*/
	virtual void release();
	
	/**
	* @func play
	*/
	void play();
	
	/**
	* @func stop
	*/
	void stop();
	
	/**
	* @func seek
	*/
	void seek(int64_t time);
	
	/**
	* @func seek_play
	*/
	void seek_play(int64_t time);
	
	/**
	* @func seek_play
	*/
	void seek_stop(int64_t time);
	
	/**
	* @func loop get
	*/
	inline uint32_t loop() const { return _loop; }
	
	/**
	* @func loopd get
	*/
	inline uint32_t looped() const { return _loopd; }

	/**
	* @func delay get
	*/
	inline uint64_t delay() const { return _delay; }
	
	/**
	* @func delayd get
	*/
	int64_t delayed() const { return _delayd; }
	
	/**
	* @func speed get
	*/
	inline float speed() const { return _speed; }
	
	/**
	* @func playing
	*/
	bool playing() const;
	
	/**
	* @func playing
	*/
	void playing(bool value);
	
	/**
	* @func parent get
	*/
	inline Action* parent() { return _parent; }
	
	/**
	* @func loop set
	*/
	inline void loop(uint32_t value) { _loop = value; }
	
	/**
	* @func delay set
	*/
	void delay(uint64_t value);
	
	/**
	* @func speed set
	*/
	inline void speed(float value) {
		_speed = Qk_MIN(10, Qk_MAX(value, 0.1));
	}
	
	/**
	* @func duration
	*/
	inline uint64_t duration() { return _full_duration - _delay; }
	
	/**
	* @func clear
	*/
	virtual void clear() = 0;
	
	/**
	* @func as_keyframe
	*/
	virtual GroupAction* as_group() { return nullptr; }
	
	/**
	* @func as_spawn
	*/
	virtual SpawnAction* as_spawn() { return nullptr; }
	
	/**
	* @func as_sequence
	*/
	virtual SequenceAction* as_sequence() { return nullptr; }
	
	/**
	* @func as_keyframe
	*/
	virtual KeyframeAction* as_keyframe() { return nullptr; }
	
private:
	/**
	* @func advance
	*/
	virtual uint64_t advance(uint64_t time_span, bool restart, Action* root) = 0;
	
	/**
	* @func seek_time
	*/
	virtual void seek_time(uint64_t time, Action* root) = 0;
	
	/**
	* @func seek_before to root action
	*/
	virtual void seek_before(int64_t time, Action* child) = 0;
	
	/**
	* @func bind_view
	*/
	virtual void bind_view(View* view) = 0;
	
protected:
	Action* _parent;
	int     _loop, _loopd;
	int64_t _full_duration, _delay, _delayd;
	float   _speed;
	
	List<View*> _views;
	
	struct Wrap {
		Action* value;
		bool play;
	};
	
	typedef List<Wrap>::Iterator ActionCenterId;
	
	ActionCenterId _action_center_id;
	
	Qk_DEFINE_INLINE_CLASS(Inl);
	
	friend class ActionCenter;
	friend class GroupAction;
	friend class SpawnAction;
	friend class SequenceAction;
	friend class KeyframeAction;
};

/**
* @class ActionCenter
*/
class Qk_EXPORT ActionCenter: public Object {
	public:
	
	ActionCenter();
	
	/**
	* @destructor
	*/
	virtual ~ActionCenter();
	
	/**
	* @func advance
	*/
	void advance(int64_t now_time);
	
	/**
	* @func shared
	*/
	static ActionCenter* shared();
	
	private:
	typedef List<Action::Wrap> Actions;
	
	uint64_t _prev_sys_time;
	Actions  _actions;
	
	Qk_DEFINE_INLINE_CLASS(Inl);
};

Qk_NAMESPACE_END
#endif