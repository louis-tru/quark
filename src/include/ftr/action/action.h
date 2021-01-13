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

#ifndef __ftr__action__
#define __ftr__action__

#include "ftr/util/array.h"
#include "ftr/util/list.h"
#include "ftr/util/map.h"
#include "ftr/value.h"
#include "ftr/math/bezier.h"
#include "ftr/event.h"
#include "ftr/property.h"
#include "ftr/background.h"

namespace ftr {

	class View;
	class ActionCenter;
	class GroupAction;
	class SpawnAction;
	class SequenceAction;
	class KeyframeAction;

	/**
	* @class Action
	*/
	class FX_EXPORT Action: public Reference {
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
		void seek(int64 time);
		
		/**
		* @func seek_play
		*/
		void seek_play(int64 time);
		
		/**
		* @func seek_play
		*/
		void seek_stop(int64 time);
		
		/**
		* @func loop get
		*/
		inline uint loop() const { return _loop; }
		
		/**
		* @func loopd get
		*/
		inline uint looped() const { return _loopd; }

		/**
		* @func delay get
		*/
		inline uint64 delay() const { return _delay; }
		
		/**
		* @func delayd get
		*/
		int64 delayed() const { return _delayd; }
		
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
		inline void loop(uint value) { _loop = value; }
		
		/**
		* @func delay set
		*/
		void delay(uint64 value);
		
		/**
		* @func speed set
		*/
		inline void speed(float value) {
			_speed = FX_MIN(10, FX_MAX(value, 0.1));
		}
		
		/**
		* @func duration
		*/
		inline uint64 duration() { return _full_duration - _delay; }
		
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
		virtual uint64 advance(uint64 time_span, bool restart, Action* root) = 0;
		
		/**
		* @func seek_time
		*/
		virtual void seek_time(uint64 time, Action* root) = 0;
		
		/**
		* @func seek_before to root action
		*/
		virtual void seek_before(int64 time, Action* child) = 0;
		
		/**
		* @func bind_view
		*/
		virtual void bind_view(View* view) = 0;
		
		protected:
		
		struct Wrap {
			Action* value;
			bool play;
		};
		
		Action* _parent;
		int     _loop;
		int     _loopd;
		int64   _full_duration;
		int64   _delay;
		int64   _delayd;
		float   _speed;
		List<View*> _views;
		List<Wrap>::Iterator _action_center_id;
		
		FX_DEFINE_INLINE_CLASS(Inl);
		
		friend class ActionCenter;
		friend class GroupAction;
		friend class SpawnAction;
		friend class SequenceAction;
		friend class KeyframeAction;
	};

}
#endif