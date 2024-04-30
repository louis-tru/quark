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

#include "./_view.h"
#include "../str.h"
#include "../../action/action.h"

/**
 * @ns qk::js
 */

Js_BEGIN

/**
 * @class WrapAction
 */
class WrapAction: public WrapObject {
	public:

	static void constructor(FunctionArgs args) {
		Js_Worker(args);
		Js_Throw("Forbidden access abstract");
	}
	
	/**
	 * @method play()
	 */
	static void play(FunctionArgs args) {
		Js_Worker(args); UILock lock;
		Js_Self(Action);
		self->play();
	}
	
	/**
	 * @method stop()
	 */
	static void stop(FunctionArgs args) {
		Js_Worker(args); UILock lock;
		Js_Self(Action);
		self->stop();
	}
	
	/**
	 * @method seek(ms)
	 * @param ms {int}
	 */
	static void seek(FunctionArgs args) {
		Js_Worker(args); UILock lock;
		if ( args.length() < 1 || !args[0]->IsInt32(worker) ) {
			Js_Throw(
				"* @method seek(ms)\n"
				"* @param ms {int}\n"
			);
		}
		Js_Self(Action);
		self->seek( uint64(1000) * args[0]->ToInt32Value(worker) );
	}
	
	/**
	 * @method seek_play(ms)
	 * @param ms {int}
	 */
	static void seek_play(FunctionArgs args) {
		Js_Worker(args); UILock lock;
		if ( args.length() < 1 || !args[0]->IsInt32(worker) ) {
			Js_Throw(
				"* @method seekPlay(ms)\n"
				"* @param ms {int}\n"
			);
		}
		Js_Self(Action);
		self->seek_play( uint64(1000) * args[0]->ToInt32Value(worker) );
	}
	
	/**
	 * @method seek_stop(ms)
	 * @param ms {int}
	 */
	static void seek_stop(FunctionArgs args) {
		Js_Worker(args); UILock lock;
		if ( args.length() < 1 || !args[0]->IsInt32(worker) ) {
			Js_Throw(
				"* @method seekStop(ms)\n"
				"* @param ms {int}\n"
			);
		}
		Js_Self(Action);
		self->seek_stop( uint64(1000) * args[0]->ToInt32Value(worker) );
	}
	
	/**
	 * @method clear()
	 */
	static void clear(FunctionArgs args) {
		Js_Worker(args); UILock lock;
		Js_Self(Action);
		self->clear();
	}
	
	/**
	 * @get loop {uint}
	 */
	static void loop(JSString* name, PropertyArgs args) {
		Js_Worker(args); UILock lock;
		Js_Self(Action);
		Js_Return( self->loop() );
	}
	
	/**
	 * @get looped {uint}
	 */
	static void looped(JSString* name, PropertyArgs args) {
		Js_Worker(args); UILock lock;
		Js_Self(Action);
		Js_Return( self->looped() );
	}
	
	/** 
	 * @get delay {uint} ms
	 */
	static void delay(JSString* name, PropertyArgs args) {
		Js_Worker(args); UILock lock;
		Js_Self(Action);
		Js_Return( self->delay() / 1000 );
	}

	/** 
	 * @get delayed {int} ms
	 */
	static void delayed(JSString* name, PropertyArgs args) {
		Js_Worker(args); UILock lock;
		Js_Self(Action);
		Js_Return( self->delayed() / 1000 );
	}

	/** 
	 * @get speed {float} 0.1-10
	 */
	static void speed(JSString* name, PropertyArgs args) {
		Js_Worker(args); UILock lock;
		Js_Self(Action);
		Js_Return( self->speed() );
	}

	/** 
	 * @get playing {bool}
	 */
	static void playing(JSString* name, PropertyArgs args) {
		Js_Worker(args); UILock lock;
		Js_Self(Action);
		Js_Return( self->playing() );
	}

	/** 
	 * @get duration {uint} ms
	 */
	static void duration(JSString* name, PropertyArgs args) {
		Js_Worker(args); UILock lock;
		Js_Self(Action);
		Js_Return( self->duration() / 1000 );
	}

	/** 
	 * @get parent {Action}
	 */
	static void parent(JSString* name, PropertyArgs args) {
		Js_Worker(args); UILock lock;
		Js_Self(Action);
		Action* action = self->parent();
		if ( action ) {
			Wrap<Action>* wrap = Wrap<Action>::pack(action);
			Js_Return( wrap->that() );
		} else {
			Js_Return_Null();
		}
	}

	/**
	 * @set playing {bool}
	 */
	static void set_playing(JSString* name, JSValue* value, PropertySetArgs args) {
		Js_Worker(args); UILock lock;
		Js_Self(Action);
		self->playing( value->ToBooleanValue(worker) );
	}

	/**
	 * @set loop {uint}
	 */
	static void set_loop(JSString* name, JSValue* value, PropertySetArgs args) {
		Js_Worker(args); UILock lock;
		if ( !value->IsUint32(worker) ) {
			Js_Throw(
				
				"* @set loop {uint}\n"
				
			);
		}
		Js_Self(Action);
		self->loop( value->ToUint32Value(worker) );
	}

	/**
	 * @set delay {uint} ms
	 */
	static void set_delay(JSString* name, JSValue* value, PropertySetArgs args) {
		Js_Worker(args); UILock lock;
		if ( !value->IsUint32(worker) ) {
			Js_Throw(
				"* @set delay {uint} ms\n"
			);
		}
		Js_Self(Action);
		self->delay( uint64(1000) * value->ToUint32Value(worker) );
	}

	/**
	 * @set speed {float} 0.1-10
	 */
	static void set_speed(JSString* name, JSValue* value, PropertySetArgs args) {
		Js_Worker(args); UILock lock;
		if ( ! value->IsNumber(worker) ) {
			Js_Throw(
				"* @set speed {float} 0.1-10\n"
			);
		}
		Js_Self(Action);
		self->speed( value->ToNumberValue(worker) );
	}

	static void null_set_accessor(JSString* name, JSValue* value, PropertySetArgs args) {}

	static void binding(JSObject* exports, Worker* worker) {
		Js_Define_Class(Action, constructor, {
			Js_Set_Class_Method(play, play);
			Js_Set_Class_Method(stop, stop);
			Js_Set_Class_Method(seek, seek);
			Js_Set_Class_Method(seekPlay, seek_play);
			Js_Set_Class_Method(seekStop, seek_stop);
			Js_Set_Class_Method(clear, clear);
			Js_Set_Class_Accessor(duration, duration);
			Js_Set_Class_Accessor(parent, parent);
			Js_Set_Class_Accessor(playing, playing, set_playing);
			Js_Set_Class_Accessor(loop, loop, set_loop);
			Js_Set_Class_Accessor(looped, looped);
			Js_Set_Class_Accessor(delay, delay, set_delay);
			Js_Set_Class_Accessor(delayed, delayed);
			Js_Set_Class_Accessor(speed, speed, set_speed);
			Js_Set_Class_Accessor(seq, nullptr, null_set_accessor);
			Js_Set_Class_Accessor(spawn, nullptr, null_set_accessor);
			Js_Set_Class_Accessor(keyframe, nullptr, null_set_accessor);
		}, nullptr);
	}
};

/**
 * @class WrapGroupAction
 */
class WrapGroupAction: public WrapObject {
	public:

	static void constructor(FunctionArgs args) {
		Js_Worker(args);
		Js_Throw("Forbidden access abstract");
	}
	
	/**
	 * @get length {uint}
	 */
	static void length(JSString* name, PropertyArgs args) {
		Js_Worker(args); UILock lock;
		Js_Self(GroupAction);
		Js_Return( self->length() );
	}
	
	/**
	 * @method append(child)
	 * @param child {Action}
	 */
	static void append(FunctionArgs args) {
		Js_Worker(args); UILock lock;
		if ( args.length() < 1 || !worker->hasInstance<Action>(args[0]) ) {
			Js_Throw(
				"* @method append(child)\n"
				"* @param child {Action}\n"
			);
		}
		Js_Self(GroupAction);
		Action* child = Wrap<Action>::unpack(args[0].To<JSObject>())->self();
		self->append( child );
	}
	
	/**
	 * @method insert(index, child)
	 * @param index {uint}
	 * @param child {Action}
	 */
	static void insert(FunctionArgs args) {
		Js_Worker(args); UILock lock;
		if (args.length() < 2 || !args[0]->IsUint32(worker) || 
				!worker->hasInstance<Action>(args[1]) ) {
			Js_Throw(
				"* @method insert(index, child)\n"
				"* @param index {uint}\n"
				"* @param child {Action}\n"
			);
		}
		Js_Self(GroupAction);
		Action* child = Wrap<Action>::unpack(args[1].To<JSObject>())->self();
		self->insert( args[0]->ToUint32Value(worker), child );
	}
	
	/**
	 * @method remove_child(index)
	 * @param index {uint}
	 */
	static void remove_child(FunctionArgs args) {
		Js_Worker(args); UILock lock;
		if ( args.length() < 1 || !args[0]->IsUint32(worker) ) {
			Js_Throw(
				"* @method removeChild(index)\n"
				"* @param index {uint}\n"
			);
		}
		Js_Self(GroupAction);
		self->remove_child( args[0]->ToUint32Value(worker) );
	}
	
	static void children(FunctionArgs args) {
		Js_Worker(args); UILock lock;
		if ( args.length() < 1 || !args[0]->IsUint32(worker) ) {
			Js_Throw(
				"* @method children(index)\n"
				"* @param index {uint}\n"
				"* @return {Action} return child action\n"
			);
		}
		Js_Self(GroupAction);
		
		uint32_t index = args[0]->ToUint32Value(worker);
		if ( index < self->length() ) {
			Action* action = (*self)[ args[0]->ToUint32Value(worker) ];
			Wrap<Action>* wrap = Wrap<Action>::pack(action);
			Js_Return( wrap->that() );
		} else {
			Js_Return_Null();
		}
	}
	
	static void binding(JSObject* exports, Worker* worker) {
		Js_Define_Class_NO_EXPORTS(GroupAction, constructor, {
			Js_Set_Class_Accessor(length, length);
			Js_Set_Class_Method(append, append);
			Js_Set_Class_Method(insert, insert);
			Js_Set_Class_Method(removeChild, remove_child);
			Js_Set_Class_Method(children, children);
		}, Action);
	}
};

/**
 * @class WrapSpawnAction
 */
class WrapSpawnAction: public WrapObject {
	public:
	
	static void constructor(FunctionArgs args) {
		Js_ATTACH(args);
		Js_CHECK_APP();
		New<WrapSpawnAction>(args, new SpawnAction());
	}
	
	static void binding(JSObject* exports, Worker* worker) {
		Js_Define_Class(SpawnAction, constructor, {
		}, GroupAction);
	}
};

/**
 * @class WrapSequenceAction
 */
class WrapSequenceAction: public WrapObject {
	public:
	
	static void constructor(FunctionArgs args) {
		Js_ATTACH(args);
		Js_CHECK_APP();
		New<WrapSequenceAction>(args, new SequenceAction());
	}

	static void binding(JSObject* exports, Worker* worker) {
		Js_Define_Class(SequenceAction, constructor, {
		}, GroupAction);
	}
};

typedef KeyframeAction::Frame Frame;

/**
 * @class WrapKeyframeAction
 */
class WrapKeyframeAction: public WrapObject {
	public:
	
	static void constructor(FunctionArgs args) {
		Js_ATTACH(args);
		Js_CHECK_APP();
		New<WrapKeyframeAction>(args, new KeyframeAction());
	}
	
	/**
	 * @method hasProperty(name)
	 * @param name {emun PropertyName} 
	 * @return {bool}
	 */
	static void hasProperty(FunctionArgs args) {
		Js_Worker(args);
		if ( args.length() < 1 || !args[0]->IsInt32(worker) ) {
			Js_Throw(
				"* @method hasProperty(name)\n"
				"* @param name {emun PropertyName}\n"
				"* @return {bool}\n"
			);
		}
		Js_Self(KeyframeAction);
		Js_Return( self->has_property( static_cast<PropertyName>(args[0]->ToInt32Value(worker)) ));
	}
	
	/**
	 * @method matchProperty(name)
	 * @param name {emun PropertyName} 
	 * @return {bool}
	 */
	static void matchProperty(FunctionArgs args) {
		Js_Worker(args);
		if ( args.length() < 1 || ! args[0]->IsInt32(worker) ) {
			Js_Throw(
				"* @method matchProperty(name)\n"
				"* @param name {emun PropertyName}\n"
				"* @return {bool}\n"
			);
		}
		Js_Self(KeyframeAction);
		Js_Return( self->match_property( static_cast<PropertyName>(args[0]->ToInt32Value(worker)) ));
	}
	
	/**
	 * @method frame(index)
	 * @param index {uint}
	 * @return {Frame}
	 */
	static void frame(FunctionArgs args) {
		Js_Worker(args);
		if ( args.length() < 1 || ! args[0]->IsUint32(worker) ) {
			Js_Throw("Bad argument.");
		}
		Js_Self(KeyframeAction);
		uint32_t index = args[0]->ToUint32Value(worker);
		if ( index < self->length() ) {
			Frame* frame = self->frame(index);
			Js_Return( Wrap<Frame>::pack(frame)->that() );
		} else {
			Js_Return_Null();
		}
	}
	
	/**
	 * @method add([time[,curve]]|[style])
	 * arg [time=0] {uint}
	 * arg [curve] {Curve}
	 * arg [style] {Object}
	 * @return {Frame}
	 */
	static void add(FunctionArgs args) {
		Js_Worker(args); UILock lock;
		uint64_t time = 0;
		
		if ( args.length() > 0 ) {
			if ( args[0]->IsObject(worker) && ! args[0]->IsNull(worker) ) {
				Js_Handle_Scope();
				
				JSObject* arg = args[0].To<JSObject>();
				Local<JSArray> names = arg->GetPropertyNames(worker);
				JSValue* t = arg->Get(worker, worker->strs()->time());
				if ( !t.IsEmpty() ) {
					if ( t->IsNumber(worker) ) {
						time = uint64(1000) * t->ToNumberValue(worker);
					} else {
						Js_Throw_value_err(t, "KeyframeAction.add([time = %s])");
					}
				} else { // js error
					return;
				}
				Js_Self(KeyframeAction);
				Frame* frame = self->add(time);
				JSObject* handle = Wrap<Frame>::pack(frame, Js_Typeid(Frame))->that();
				
				for ( uint32_t i = 0, len = names->Length(worker); i < len; i++ ) {
					JSValue* key = names->Get(worker, i);
					JSValue* val = arg->Get(worker, key );
					if ( ! handle->Set(worker, key, val) ) { // js error
						return;
					}
				}
				
				Js_Return( handle );
			} else if ( args[0]->IsNumber(worker) ) {
				time = uint64(1000) * args[0]->ToNumberValue(worker);
			}
		}
		
		Js_Self(KeyframeAction);
		
		Frame* frame = nullptr;
		if ( args.length() > 1 && !args[1]->IsUndefined(worker) ) {
			js_parse_value(Curve, args[1], "KeyframeAction.add([time[,curve = %s]])");
			frame = self->add(time, out);
		} else {
			frame = self->add(time);
		}
		
		Wrap<Frame>* wrap = Wrap<Frame>::pack(frame, Js_Typeid(Frame));
		Js_Return( wrap->that() );
	}
	
	/**
	 * @get first {Frame}
	 */
	static void first(JSString* name, PropertyArgs args) {
		Js_Worker(args);
		Js_Self(KeyframeAction);
		if ( self->length() ) {
			Frame* frame = self->first();
			Js_Return( Wrap<Frame>::pack(frame)->that() );
		} else {
			Js_Return_Null();
		}
	}
	
	/**
	 * @get last {Frame}
	 */
	static void last(JSString* name, PropertyArgs args) {
		Js_Worker(args);
		Js_Self(KeyframeAction);
		if ( self->length() ) {
			Frame* frame = self->last();
			Js_Return( Wrap<Frame>::pack(frame)->that() );
		} else {
			Js_Return_Null();
		}
	}
	
	/**
	 * @get length {uint}
	 */
	static void length(JSString* name, PropertyArgs args) {
		Js_Worker(args);
		Js_Self(KeyframeAction);
		Js_Return( self->length() );
	}
	
	/**
	 * @get position {uint} get play frame position
	 */
	static void position(JSString* name, PropertyArgs args) {
		Js_Worker(args);
		Js_Self(KeyframeAction);
		Js_Return( self->position() );
	}
	
	/**
	 * @get time {uint} ms get play time position
	 */
	static void time(JSString* name, PropertyArgs args) {
		Js_Worker(args);
		Js_Self(KeyframeAction);
		Js_Return( self->time() / 1000 );
	}
	
 public:
	static void binding(JSObject* exports, Worker* worker) {
		Js_Define_Class(KeyframeAction, constructor, {
			Js_Set_Class_Method(hasProperty, hasProperty);
			Js_Set_Class_Method(matchProperty, matchProperty);
			Js_Set_Class_Method(frame, frame);
			Js_Set_Class_Method(add, add);
			Js_Set_Class_Accessor(first, first);
			Js_Set_Class_Accessor(last, last);
			Js_Set_Class_Accessor(length, length);
			Js_Set_Class_Accessor(position, position);
			Js_Set_Class_Accessor(time, time);
		}, Action);
	}
};

void binding_frame(JSObject* exports, Worker* worker);

/**
 * @class NativeAction
 */
class BindingAction {
	public:
	static void binding(JSObject* exports, Worker* worker) {
		worker->bindingModule("_value");

		WrapAction::binding(exports, worker);
		WrapGroupAction::binding(exports, worker);
		WrapSpawnAction::binding(exports, worker);
		WrapSequenceAction::binding(exports, worker);
		WrapKeyframeAction::binding(exports, worker);

		binding_frame(exports, worker);
	}
};

Js_REG_MODULE(_action, BindingAction);
Js_END
