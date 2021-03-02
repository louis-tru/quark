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

#include "./action.h"
#include "./group.h"
#include "./keyframe.h"
#include "../views2/view.h"

#ifndef __ftr__action__1__
#define __ftr__action__1__

namespace ftr {

	typedef KeyframeAction::Frame Frame;
	typedef KeyframeAction::Property Property;

	FX_DEFINE_INLINE_MEMBERS(View, ActionInl) {
		public:
		inline ReturnValue& trigger(const NameType& name, GUIEvent& evt) {
			return View::trigger(name, evt);
		}
	};

	FX_DEFINE_INLINE_MEMBERS(Action, Inl) {
		public:
		#define _inl_action(self) static_cast<Action::Inl*>(static_cast<Action*>(self))
		void set_parent(Action* parent) throw(Error);
		View* first_view();
		void clear_parent();
		View* view();
		List<View*>& views();
		bool is_playing(); // is_playing with root action
		void trigger_action_loop(uint64_t delay, Action* root);
		void trigger_action_key_frame(uint64_t delay, uint32_t frame_index, Action* root);
		void update_duration(int64_t difference);
		void add_view(View* view) throw(Error);
		void del_view(View* view);
	};

	FX_DEFINE_INLINE_MEMBERS(ActionCenter, Inl) {
		public:
		#define _inl_action_center(self) static_cast<ActionCenter::Inl*>(self)
		void add(Action* action);
		void del(Action* action);
	};

	FX_DEFINE_INLINE_MEMBERS(GroupAction, Inl) {
		public:
		#define _inl_group_action(self) \
			static_cast<GroupAction::Inl*>(static_cast<GroupAction*>(self))
		void clear_all();
		uint64_t _remove(uint32_t index);
		void update_spawn_action_duration();
	};

	FX_DEFINE_INLINE_MEMBERS(KeyframeAction, Inl) {
		public:
		#define _inl_key_action(self) static_cast<KeyframeAction::Inl*>(self)
		void transition(uint32_t f1, uint32_t f2, float x, float y, Action* root);
		void transition(uint32_t f1, Action* root);
		uint64_t advance(uint64_t time_span, Action* root);
	};

	void update_spawn_action_duration(SpawnAction* act);

	/**
	* @class Property2
	*/
	template<class T>
	class Property2: public Property {
		public:
		typedef T    (View::*GetPropertyFunc)() const;
		typedef void (View::*SetPropertyFunc)(T value);
		
		inline Property2(uint32_t frame_count)
			: _frames(frame_count)
			, _get_property_func(nullptr)
			, _set_property_func(nullptr)
		{}
		
		virtual ~Property2() {}
		
		inline void set_property(List<View*>& views) {
			for ( auto i : views ) {
				if ( i ) {
					(i->*_set_property_func)(_transition);
				}
			}
		}
		
		inline T get_property(View* view) {
			return (view->*_get_property_func)();
		}
		
		virtual void transition(uint32_t f1, Action* root) {
			if ( _set_property_func ) {
				_transition = _frames[f1];
				set_property(_inl_action(root)->views());
			}
		}
		
		virtual void transition(uint32_t f1, uint32_t f2, float x, float y, Action* root) {
			if ( _set_property_func ) {
				T v1 = _frames[f1], v2 = _frames[f2];
				_transition = v1 - (v1 - v2) * y;
				set_property(_inl_action(root)->views());
			}
		}
		
		virtual void add_frame() {
			_frames.push( T() );
		}
		
		virtual void fetch(uint32_t frame, View* view) {
			if ( _get_property_func ) {
				_frames[frame] = get_property(view);
			}
		}
		
		virtual void default_value(uint32_t frame) {
			_frames[frame] = T();
		}
		
		inline T operator[](uint32_t index) {
			return _frames[index];
		}
		
		inline T frame(uint32_t index) {
			return _frames[index];
		}
		
		inline void frame(uint32_t index, T value) {
			_frames[index] = value;
		}
			
		protected:
		Array<T>  _frames;
		T               _transition;
		GetPropertyFunc _get_property_func;
		SetPropertyFunc _set_property_func;
	};

	/**
	* @class Property3
	*/
	template<class T, PropertyName Name>
	class Property3: public Property2<T> {
		public:
		typedef typename Property2<T>::GetPropertyFunc GetPropertyFunc;
		typedef typename Property2<T>::SetPropertyFunc SetPropertyFunc;
		
		inline Property3(uint32_t frame_count): Property2<T>(frame_count) { }
		
		virtual void bind_view(int view_type) {
			typedef PropertysAccessor::Accessor PropertyFunc;
			PropertyFunc func = PropertysAccessor::shared()->accessor(view_type, Name);
			this->_get_property_func = reinterpret_cast<GetPropertyFunc>(func.get_accessor);
			this->_set_property_func = reinterpret_cast<SetPropertyFunc>(func.set_accessor);
		}
	};

	FX_DEFINE_INLINE_MEMBERS(Frame, Inl) {
		public:
		#define _inl_frame(self) static_cast<KeyframeAction::Frame::Inl*>(self)

		template<PropertyName Name, class T> inline T property_value() {
			typedef Property2<T> Type;
			auto it = _host->_property.find(Name);
			if (it != _host->_property.end()) {
				return static_cast<Type*>(it->value)->frame(_index);
			}
			return T();
		}

		template<PropertyName Name, class T> inline void set_property_value(T value) {
			Dict<PropertyName, Property*>& property = _host->_property;
			typedef Property3<T, Name> Type;
			auto it = property.find(Name);
			if (it == property.end()) {
				Type* prop = new Type(_host->length());
				property[Name] = prop;
				prop->bind_view(_host->_bind_view_type);
				prop->frame(_index, value);
			} else {
				static_cast<Type*>(it->value)->frame(_index, value);
			}
		}
	};

}

#endif
