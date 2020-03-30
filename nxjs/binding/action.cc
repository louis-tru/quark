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

#include "ngui/js/ngui.h"
#include "ngui/js/str.h"
#include "ngui/action.h"

/**
 * @ns ngui::js
 */

JS_BEGIN

/**
 * @class WrapAction
 */
class WrapAction: public WrapObject {
 public:

	static void constructor(FunctionCall args) {
		JS_WORKER(args);
		JS_THROW_ERR("Forbidden access abstract");
	}
	
	/**
	 * @func play()
	 */
	static void play(FunctionCall args) {
		JS_WORKER(args); GUILock lock;
		JS_SELF(Action);
		self->play();
	}
	
	/**
	 * @func stop()
	 */
	static void stop(FunctionCall args) {
		JS_WORKER(args); GUILock lock;
		JS_SELF(Action);
		self->stop();
	}
	
	/**
	 * @func seek(ms)
	 * @arg ms {int}
	 */
	static void seek(FunctionCall args) {
		JS_WORKER(args); GUILock lock;
		if ( args.Length() < 1 || !args[0]->IsInt32(worker) ) {
			JS_THROW_ERR(
				"* @func seek(ms)\n"
				"* @arg ms {int}\n"
			);
		}
		JS_SELF(Action);
		self->seek( uint64(1000) * args[0]->ToInt32Value(worker) );
	}
	
	/**
	 * @func seek_play(ms)
	 * @arg ms {int}
	 */
	static void seek_play(FunctionCall args) {
		JS_WORKER(args); GUILock lock;
		if ( args.Length() < 1 || !args[0]->IsInt32(worker) ) {
			JS_THROW_ERR(
				"* @func seekPlay(ms)\n"
				"* @arg ms {int}\n"
			);
		}
		JS_SELF(Action);
		self->seek_play( uint64(1000) * args[0]->ToInt32Value(worker) );
	}
	
	/**
	 * @func seek_stop(ms)
	 * @arg ms {int}
	 */
	static void seek_stop(FunctionCall args) {
		JS_WORKER(args); GUILock lock;
		if ( args.Length() < 1 || !args[0]->IsInt32(worker) ) {
			JS_THROW_ERR(
				"* @func seekStop(ms)\n"
				"* @arg ms {int}\n"
			);
		}
		JS_SELF(Action);
		self->seek_stop( uint64(1000) * args[0]->ToInt32Value(worker) );
	}
	
	/**
	 * @func clear()
	 */
	static void clear(FunctionCall args) {
		JS_WORKER(args); GUILock lock;
		JS_SELF(Action);
		self->clear();
	}
	
	/**
	 * @get loop {uint}
	 */
	static void loop(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args); GUILock lock;
		JS_SELF(Action);
		JS_RETURN( self->loop() );
	}
	
	/**
	 * @get looped {uint}
	 */
	static void looped(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args); GUILock lock;
		JS_SELF(Action);
		JS_RETURN( self->looped() );
	}
	
	/** 
	 * @get delay {uint} ms
	 */
	static void delay(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args); GUILock lock;
		JS_SELF(Action);
		JS_RETURN( self->delay() / 1000 );
	}

	/** 
	 * @get delayed {int} ms
	 */
	static void delayed(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args); GUILock lock;
		JS_SELF(Action);
		JS_RETURN( self->delayed() / 1000 );
	}

	/** 
	 * @get speed {float} 0.1-10
	 */
	static void speed(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args); GUILock lock;
		JS_SELF(Action);
		JS_RETURN( self->speed() );
	}

	/** 
	 * @get playing {bool}
	 */
	static void playing(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args); GUILock lock;
		JS_SELF(Action);
		JS_RETURN( self->playing() );
	}

	/** 
	 * @get duration {uint} ms
	 */
	static void duration(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args); GUILock lock;
		JS_SELF(Action);
		JS_RETURN( self->duration() / 1000 );
	}

	/** 
	 * @get parent {Action}
	 */
	static void parent(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args); GUILock lock;
		JS_SELF(Action);
		Action* action = self->parent();
		if ( action ) {
			Wrap<Action>* wrap = Wrap<Action>::pack(action);
			JS_RETURN( wrap->that() );
		} else {
			JS_RETURN_NULL();
		}
	}

	/**
	 * @set playing {bool}
	 */
	static void set_playing(Local<JSString> name, Local<JSValue> value, PropertySetCall args) {
		JS_WORKER(args); GUILock lock;
		JS_SELF(Action);
		self->playing( value->ToBooleanValue(worker) );
	}

	/**
	 * @set loop {uint}
	 */
	static void set_loop(Local<JSString> name, Local<JSValue> value, PropertySetCall args) {
		JS_WORKER(args); GUILock lock;
		if ( !value->IsUint32(worker) ) {
			JS_THROW_ERR(
				
				"* @set loop {uint}\n"
				
			);
		}
		JS_SELF(Action);
		self->loop( value->ToUint32Value(worker) );
	}

	/**
	 * @set delay {uint} ms
	 */
	static void set_delay(Local<JSString> name, Local<JSValue> value, PropertySetCall args) {
		JS_WORKER(args); GUILock lock;
		if ( !value->IsUint32(worker) ) {
			JS_THROW_ERR(
				"* @set delay {uint} ms\n"
			);
		}
		JS_SELF(Action);
		self->delay( uint64(1000) * value->ToUint32Value(worker) );
	}

	/**
	 * @set speed {float} 0.1-10
	 */
	static void set_speed(Local<JSString> name, Local<JSValue> value, PropertySetCall args) {
		JS_WORKER(args); GUILock lock;
		if ( ! value->IsNumber(worker) ) {
			JS_THROW_ERR(
				"* @set speed {float} 0.1-10\n"
			);
		}
		JS_SELF(Action);
		self->speed( value->ToNumberValue(worker) );
	}

	static void null_set_accessor(Local<JSString> name, Local<JSValue> value, PropertySetCall args) {}

	static void binding(Local<JSObject> exports, Worker* worker) {
		JS_DEFINE_CLASS(Action, constructor, {
			JS_SET_CLASS_METHOD(play, play);
			JS_SET_CLASS_METHOD(stop, stop);
			JS_SET_CLASS_METHOD(seek, seek);
			JS_SET_CLASS_METHOD(seekPlay, seek_play);
			JS_SET_CLASS_METHOD(seekStop, seek_stop);
			JS_SET_CLASS_METHOD(clear, clear);
			JS_SET_CLASS_ACCESSOR(duration, duration);
			JS_SET_CLASS_ACCESSOR(parent, parent);
			JS_SET_CLASS_ACCESSOR(playing, playing, set_playing);
			JS_SET_CLASS_ACCESSOR(loop, loop, set_loop);
			JS_SET_CLASS_ACCESSOR(looped, looped);
			JS_SET_CLASS_ACCESSOR(delay, delay, set_delay);
			JS_SET_CLASS_ACCESSOR(delayed, delayed);
			JS_SET_CLASS_ACCESSOR(speed, speed, set_speed);
			JS_SET_CLASS_ACCESSOR(seq, nullptr, null_set_accessor);
			JS_SET_CLASS_ACCESSOR(spawn, nullptr, null_set_accessor);
			JS_SET_CLASS_ACCESSOR(keyframe, nullptr, null_set_accessor);
		}, nullptr);
	}
};

/**
 * @class WrapGroupAction
 */
class WrapGroupAction: public WrapObject {
 public:

	static void constructor(FunctionCall args) {
		JS_WORKER(args);
		JS_THROW_ERR("Forbidden access abstract");
	}
	
	/**
	 * @get length {uint}
	 */
	static void length(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args); GUILock lock;
		JS_SELF(GroupAction);
		JS_RETURN( self->length() );
	}
	
	/**
	 * @func append(child)
	 * @arg child {Action}
	 */
	static void append(FunctionCall args) {
		JS_WORKER(args); GUILock lock;
		if ( args.Length() < 1 || !worker->hasInstance<Action>(args[0]) ) {
			JS_THROW_ERR(
				"* @func append(child)\n"
				"* @arg child {Action}\n"
			);
		}
		JS_SELF(GroupAction);
		Action* child = Wrap<Action>::unpack(args[0].To<JSObject>())->self();
		self->append( child );
	}
	
	/**
	 * @func insert(index, child)
	 * @arg index {uint}
	 * @arg child {Action}
	 */
	static void insert(FunctionCall args) {
		JS_WORKER(args); GUILock lock;
		if (args.Length() < 2 || !args[0]->IsUint32(worker) || 
				!worker->hasInstance<Action>(args[1]) ) {
			JS_THROW_ERR(
				"* @func insert(index, child)\n"
				"* @arg index {uint}\n"
				"* @arg child {Action}\n"
			);
		}
		JS_SELF(GroupAction);
		Action* child = Wrap<Action>::unpack(args[1].To<JSObject>())->self();
		self->insert( args[0]->ToUint32Value(worker), child );
	}
	
	/**
	 * @func remove_child(index)
	 * @arg index {uint}
	 */
	static void remove_child(FunctionCall args) {
		JS_WORKER(args); GUILock lock;
		if ( args.Length() < 1 || !args[0]->IsUint32(worker) ) {
			JS_THROW_ERR(
				"* @func removeChild(index)\n"
				"* @arg index {uint}\n"
			);
		}
		JS_SELF(GroupAction);
		self->remove_child( args[0]->ToUint32Value(worker) );
	}
	
	static void children(FunctionCall args) {
		JS_WORKER(args); GUILock lock;
		if ( args.Length() < 1 || !args[0]->IsUint32(worker) ) {
			JS_THROW_ERR(
				"* @func children(index)\n"
				"* @arg index {uint}\n"
				"* @ret {Action} return child action\n"
			);
		}
		JS_SELF(GroupAction);
		
		uint index = args[0]->ToUint32Value(worker);
		if ( index < self->length() ) {
			Action* action = (*self)[ args[0]->ToUint32Value(worker) ];
			Wrap<Action>* wrap = Wrap<Action>::pack(action);
			JS_RETURN( wrap->that() );
		} else {
			JS_RETURN_NULL();
		}
	}
	
	static void binding(Local<JSObject> exports, Worker* worker) {
		JS_DEFINE_CLASS_NO_EXPORTS(GroupAction, constructor, {
			JS_SET_CLASS_ACCESSOR(length, length);
			JS_SET_CLASS_METHOD(append, append);
			JS_SET_CLASS_METHOD(insert, insert);
			JS_SET_CLASS_METHOD(removeChild, remove_child);
			JS_SET_CLASS_METHOD(children, children);
		}, Action);
	}
};

/**
 * @class WrapSpawnAction
 */
class WrapSpawnAction: public WrapObject {
 public:
	
	static void constructor(FunctionCall args) {
		JS_ATTACH(args);
		JS_CHECK_APP();
		New<WrapSpawnAction>(args, new SpawnAction());
	}
	
	static void binding(Local<JSObject> exports, Worker* worker) {
		JS_DEFINE_CLASS(SpawnAction, constructor, {
		}, GroupAction);
	}
};

/**
 * @class WrapSequenceAction
 */
class WrapSequenceAction: public WrapObject {
 public:
	
	static void constructor(FunctionCall args) {
		JS_ATTACH(args);
		JS_CHECK_APP();
		New<WrapSequenceAction>(args, new SequenceAction());
	}

	static void binding(Local<JSObject> exports, Worker* worker) {
		JS_DEFINE_CLASS(SequenceAction, constructor, {
		}, GroupAction);
	}
};

typedef KeyframeAction::Frame Frame;

/**
 * @class WrapKeyframeAction
 */
class WrapKeyframeAction: public WrapObject {
 public:
	
	static void constructor(FunctionCall args) {
		JS_ATTACH(args);
		JS_CHECK_APP();
		New<WrapKeyframeAction>(args, new KeyframeAction());
	}
	
	/**
	 * @func hasProperty(name)
	 * @arg name {emun PropertyName} 
	 * @ret {bool}
	 */
	static void hasProperty(FunctionCall args) {
		JS_WORKER(args);
		if ( args.Length() < 1 || !args[0]->IsInt32(worker) ) {
			JS_THROW_ERR(
				"* @func hasProperty(name)\n"
				"* @arg name {emun PropertyName}\n"
				"* @ret {bool}\n"
			);
		}
		JS_SELF(KeyframeAction);
		JS_RETURN( self->has_property( static_cast<PropertyName>(args[0]->ToInt32Value(worker)) ));
	}
	
	/**
	 * @func matchProperty(name)
	 * @arg name {emun PropertyName} 
	 * @ret {bool}
	 */
	static void matchProperty(FunctionCall args) {
		JS_WORKER(args);
		if ( args.Length() < 1 || ! args[0]->IsInt32(worker) ) {
			JS_THROW_ERR(
				"* @func matchProperty(name)\n"
				"* @arg name {emun PropertyName}\n"
				"* @ret {bool}\n"
			);
		}
		JS_SELF(KeyframeAction);
		JS_RETURN( self->match_property( static_cast<PropertyName>(args[0]->ToInt32Value(worker)) ));
	}
	
	/**
	 * @func frame(index)
	 * @arg index {uint}
	 * @ret {Frame}
	 */
	static void frame(FunctionCall args) {
		JS_WORKER(args);
		if ( args.Length() < 1 || ! args[0]->IsUint32(worker) ) {
			JS_THROW_ERR("Bad argument.");
		}
		JS_SELF(KeyframeAction);
		uint index = args[0]->ToUint32Value(worker);
		if ( index < self->length() ) {
			Frame* frame = self->frame(index);
			JS_RETURN( Wrap<Frame>::pack(frame)->that() );
		} else {
			JS_RETURN_NULL();
		}
	}
	
	/**
	 * @func add([time[,curve]]|[style])
	 * arg [time=0] {uint}
	 * arg [curve] {Curve}
	 * arg [style] {Object}
	 * @ret {Frame}
	 */
	static void add(FunctionCall args) {
		JS_WORKER(args); GUILock lock;
		uint64 time = 0;
		
		if ( args.Length() > 0 ) {
			if ( args[0]->IsObject(worker) && ! args[0]->IsNull(worker) ) {
				JS_HANDLE_SCOPE();
				
				Local<JSObject> arg = args[0].To<JSObject>();
				Local<JSArray> names = arg->GetPropertyNames(worker);
				Local<JSValue> t = arg->Get(worker, worker->strs()->time());
				if ( !t.IsEmpty() ) {
					if ( t->IsNumber(worker) ) {
						time = uint64(1000) * t->ToNumberValue(worker);
					} else {
						js_throw_value_err(t, "KeyframeAction.add([time = %s])");
					}
				} else { // js error
					return;
				}
				JS_SELF(KeyframeAction);
				Frame* frame = self->add(time);
				Local<JSObject> handle = Wrap<Frame>::pack(frame, JS_TYPEID(Frame))->that();
				
				for ( uint i = 0, len = names->Length(worker); i < len; i++ ) {
					Local<JSValue> key = names->Get(worker, i);
					Local<JSValue> val = arg->Get(worker, key );
					if ( ! handle->Set(worker, key, val) ) { // js error
						return;
					}
				}
				
				JS_RETURN( handle );
			} else if ( args[0]->IsNumber(worker) ) {
				time = uint64(1000) * args[0]->ToNumberValue(worker);
			}
		}
		
		JS_SELF(KeyframeAction);
		
		Frame* frame = nullptr;
		if ( args.Length() > 1 && !args[1]->IsUndefined(worker) ) {
			js_parse_value(Curve, args[1], "KeyframeAction.add([time[,curve = %s]])");
			frame = self->add(time, out);
		} else {
			frame = self->add(time);
		}
		
		Wrap<Frame>* wrap = Wrap<Frame>::pack(frame, JS_TYPEID(Frame));
		JS_RETURN( wrap->that() );
	}
	
	/**
	 * @get first {Frame}
	 */
	static void first(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		JS_SELF(KeyframeAction);
		if ( self->length() ) {
			Frame* frame = self->first();
			JS_RETURN( Wrap<Frame>::pack(frame)->that() );
		} else {
			JS_RETURN_NULL();
		}
	}
	
	/**
	 * @get last {Frame}
	 */
	static void last(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		JS_SELF(KeyframeAction);
		if ( self->length() ) {
			Frame* frame = self->last();
			JS_RETURN( Wrap<Frame>::pack(frame)->that() );
		} else {
			JS_RETURN_NULL();
		}
	}
	
	/**
	 * @get length {uint}
	 */
	static void length(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		JS_SELF(KeyframeAction);
		JS_RETURN( self->length() );
	}
	
	/**
	 * @get position {uint} get play frame position
	 */
	static void position(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		JS_SELF(KeyframeAction);
		JS_RETURN( self->position() );
	}
	
	/**
	 * @get time {uint} ms get play time position
	 */
	static void time(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		JS_SELF(KeyframeAction);
		JS_RETURN( self->time() / 1000 );
	}
	
 public:
	static void binding(Local<JSObject> exports, Worker* worker) {
		JS_DEFINE_CLASS(KeyframeAction, constructor, {
			JS_SET_CLASS_METHOD(hasProperty, hasProperty);
			JS_SET_CLASS_METHOD(matchProperty, matchProperty);
			JS_SET_CLASS_METHOD(frame, frame);
			JS_SET_CLASS_METHOD(add, add);
			JS_SET_CLASS_ACCESSOR(first, first);
			JS_SET_CLASS_ACCESSOR(last, last);
			JS_SET_CLASS_ACCESSOR(length, length);
			JS_SET_CLASS_ACCESSOR(position, position);
			JS_SET_CLASS_ACCESSOR(time, time);
		}, Action);
	}
};

void binding_frame(Local<JSObject> exports, Worker* worker);

/**
 * @class NativeAction
 */
class BindingAction {
 public:
	static void binding(Local<JSObject> exports, Worker* worker) {
		worker->bindingModule("_value");

		WrapAction::binding(exports, worker);
		WrapGroupAction::binding(exports, worker);
		WrapSpawnAction::binding(exports, worker);
		WrapSequenceAction::binding(exports, worker);
		WrapKeyframeAction::binding(exports, worker);

		binding_frame(exports, worker);
	}
};

JS_REG_MODULE(_action, BindingAction);
JS_END
