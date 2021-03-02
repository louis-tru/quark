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

#include "./_property.h"

namespace ftr {

	#define fx_def_property(ENUM, TYPE, NAME) \
		TYPE Frame::NAME() { \
			return _inl_frame(this)->property_value<ENUM, TYPE>(); \
		}\
		void Frame::set_##NAME(TYPE value) { \
			_inl_frame(this)->set_property_value<ENUM>(value); \
		}

	FX_EACH_PROPERTY_TABLE(fx_def_property)
	#undef fx_def_accessor

	/**
	* @func time set
	*/
	void Frame::set_time(uint64_t value) {
		if ( _host && _index && value != _time ) {
			uint32_t next = _index + 1;
			if ( next < _host->length() ) {
				uint64_t max_time = _host->frame(next)->time();
				_time = FX_MIN(value, max_time);
			} else { // no next
				_time = value;
			}
		}
	}

	/**
	* @func fetch property
	*/
	void Frame::fetch(View* view) {
		if ( view && view->view_type() == _host->_bind_view_type ) {
			for ( auto& i : _host->_property ) {
				i.value->fetch(_index, view);
			}
		} else {
			view = _inl_action(_host)->view();
			if ( view ) {
				for ( auto& i : _host->_property ) {
					i.value->fetch(_index, view);
				}
			}
		}
	}

	/**
	* @func flush recovery default property value
	*/
	void Frame::flush() {
		for ( auto& i : _host->_property ) {
			i.value->default_value(_index);
		}
	}

	void KeyframeAction::Inl::transition(uint32_t f1, uint32_t f2, float x, float y, Action* root) {
		for ( auto& i : _property ) {
			i.value->transition(f1, f2, x, y, root);
		}
	}
	
	void KeyframeAction::Inl::transition(uint32_t f1, Action* root) {
		for ( auto& i : _property ) {
			i.value->transition(f1, root);
		}
	}
	
	uint64_t KeyframeAction::Inl::advance(uint64_t time_span, Action* root) {
		
		start:
		
		uint32_t f1 = _frame;
		uint32_t f2 = f1 + 1;
		
		if ( f2 < length() ) {
			advance:
			
			if ( ! _inl_action(root)->is_playing() ) { // is playing
				return 0;
			}
			
			int64_t time = _time + time_span;
			int64_t time1 = _frames[f1]->time();
			int64_t time2 = _frames[f2]->time();
			int64_t t = time - time2;
			
			if ( t < 0 ) {
				
				time_span = 0;
				_time = time;
				float x = (time - time1) / float(time2 - time1);
				float y = _frames[f1]->curve().solve(x, 0.001);
				transition(f1, f2, x, y, root);
				
			} else if ( t > 0 ) {
				time_span = t;
				_frame = f2;
				_time = time2;
				_inl_action(this)->trigger_action_key_frame(t, f2, root); // trigger event action_key_frame
				
				f1 = f2; f2++;
				
				if ( f2 < length() ) {
					goto advance;
				} else {
					if ( _loop && _full_duration > _delay ) {
						goto loop;
					} else {
						transition(f1, root);
					}
				}
			} else { // t == 0
				time_span = 0;
				_time = time;
				_frame = f2;
				transition(f2, root);
				_inl_action(this)->trigger_action_key_frame(0, f2, root); // trigger event action_key_frame
			}
			
		} else { // last frame
			
			if ( _loop && _full_duration > _delay ) {
				loop:
				
				if ( _loop > 0 ) {
					if ( _loopd < _loop ) { // 可经继续循环
						_loopd++;
					} else { //
						transition(f1, root);
						goto end;
					}
				}
				
				_frame = 0;
				_time = 0;
				_inl_action(this)->trigger_action_loop(time_span, root);
				_inl_action(this)->trigger_action_key_frame(time_span, 0, root);
				goto start;
			}
		}
		
		end:
		return time_span;
	}

	void KeyframeAction::seek_before(int64_t time, Action* child) {
		FX_UNIMPLEMENTED();
	}

	void KeyframeAction::seek_time(uint64_t time, Action* root) {
		
		int64_t t = time - _delay;
		if ( t < 0 ) {
			_delayd = time;
			_frame = -1;
			_time = 0; return;
		} else {
			_delayd = _delay;
			time = t;
		}
		
		_loopd = 0;// 重置循环
		
		if ( length() ) {
			Frame* frame = nullptr;
			
			for ( auto& i: _frames ) {
				if ( time < i->time() ) {
					break;
				}
				frame = i;
			}
			
			_frame = frame->index();
			_time = FX_MIN(int64_t(time), _full_duration - _delay);
			
			uint32_t f1 = _frame;
			uint32_t f2 = f1 + 1;
			
			if ( f2 < length() ) {
				int64_t time1 = frame->time();
				int64_t time2 = _frames[f2]->time();
				float x = (_time - time1) / float(time2 - time1);
				float t = frame->curve().solve(x, 0.001);
				_inl_key_action(this)->transition(f1, f2, x, t, root);
			} else { // last frame
				_inl_key_action(this)->transition(f1, root);
			}
			
			if ( _time == int64_t(frame->time()) ) {
				_inl_action(this)->trigger_action_key_frame(0, _frame, root);
			}
		}
	}

	KeyframeAction::~KeyframeAction() {
		clear();
	}

	uint64_t KeyframeAction::advance(uint64_t time_span, bool restart, Action* root) {
		
		time_span *= _speed;
		
		if ( _frame == -1 || restart ) { // no start play
			
			if ( restart ) { // restart
				_delayd = 0;
				_loopd = 0;
				_frame = -1;
				_time = 0;
			}

			if ( _delay > _delayd ) { // 需要延时
				int64_t time = _delay - _delayd - time_span;
				if ( time >= 0 ) {
					_delayd += time_span;
					return 0;
				} else {
					_delayd = _delay;
					time_span = -time;
				}
			}
			
			if ( length() ) {
				_frame = 0;
				_time = 0;
				_inl_key_action(this)->transition(0, root);
				_inl_action(this)->trigger_action_key_frame(time_span, 0, root);
				
				if ( time_span == 0 ) {
					return 0;
				}
				
				if ( length() == 1 ) {
					return time_span / _speed;
				}
			} else {
				return time_span / _speed;
			}
		}
		
		return _inl_key_action(this)->advance(time_span, root) / _speed;
	}

	void KeyframeAction::bind_view(View* view) {
		int view_type = view->view_type();
		if ( view_type != _bind_view_type ) {
			_bind_view_type = view_type;
			for ( auto& i : _property ) {
				i.value->bind_view(view_type);
			}
		}
	}

	/**
	* @func add new frame
	*/
	Frame* KeyframeAction::add(uint64_t time, const FixedCubicBezier& curve) {
		
		if ( length() ) {
			Frame* frame = last();
			int64_t duration = time - frame->time();
			if ( duration <= 0 ) {
				time = frame->time();
			} else {
				_inl_action(this)->update_duration(duration);
			}
		} else {
			time = 0;
		}
		
		Frame* frame = new Frame(this, _frames.length(), curve);
		_frames.push(frame);
		frame->_time = time;
		
		for ( auto& i : _property ) {
			i.value->add_frame();
		}
		
		return frame;
	}

	/**
	* @func clear all frame and property
	*/
	void KeyframeAction::clear() {
		
		for (auto& i : _frames) {
			i->_host = nullptr;
			Release(i);
		}
		for (auto& i : _property) {
			delete i.value;
		}
		_frames.clear();
		_property.clear();
		
		if ( _full_duration ) {
			_inl_action(this)->update_duration( _delay - _full_duration );
		}
	}

	bool KeyframeAction::has_property(PropertyName name) {
		return _property.count(name);
	}

	/**
	* @func match_property
	*/
	bool KeyframeAction::match_property(PropertyName name) {
		return PropertysAccessor::shared()->has_accessor(_bind_view_type, name);
	}
}
