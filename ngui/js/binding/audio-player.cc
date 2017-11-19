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

#include "base/event-1.h"
#include "ngui/js/ngui.h"
#include "ngui/player.h"
#include "ngui/media-codec.h"
#include "ngui/audio-player.h"

/**
 * @ns ngui::js
 */

JS_BEGIN

typedef MultimediaSource::TrackInfo TrackInfo;

Local<JSValue> inl__track_to_jsvalue(const TrackInfo* track, Worker* worker) {
  if ( ! track ) {
    return worker->NewNull();
  }

  /**
   * @object TrackInfo
   * type {meun MediaType}
   * mime {String}
   * codecId {int}
   * codecTag {uint}
   * format {int}
   * profile {int}
   * level {int}
   * width {uint}
   * height {uint}
   * language {String}
   * bitrate {uint}
   * sampleRate {uint}
   * channelCount {uint64}
   * channelLayout {uint}
   * frameInterval {uint} ms
   * @end
   */
  Local<JSObject> obj = worker->NewObject().To<JSObject>();
  obj->Set(worker, worker->New("type",1), worker->New(track->type));
  obj->Set(worker, worker->New("mime",1), worker->New(track->mime));
  obj->Set(worker, worker->New("codecId",1), worker->New(track->codec_id));
  obj->Set(worker, worker->New("codecTag",1), worker->New(track->codec_tag));
  obj->Set(worker, worker->New("format",1), worker->New(track->format));
  obj->Set(worker, worker->New("profile",1), worker->New(track->profile));
  obj->Set(worker, worker->New("level",1), worker->New(track->level));
  obj->Set(worker, worker->New("width",1), worker->New(track->width));
  obj->Set(worker, worker->New("height",1), worker->New(track->height));
  obj->Set(worker, worker->New("language",1), worker->New(track->language));
  obj->Set(worker, worker->New("bitrate",1), worker->New(track->bitrate));
  obj->Set(worker, worker->New("sampleRate",1), worker->New(track->sample_rate));
  obj->Set(worker, worker->New("channelCount",1), worker->New(track->channel_count));
  obj->Set(worker, worker->New("channelLayout",1), worker->New(track->channel_layout));
  obj->Set(worker, worker->New("frameInterval",1), worker->New(track->frame_interval / 1000.0));
  
  return obj.To<JSValue>();
}

/**
 * @class WrapAudioPlayer
 */
class WrapAudioPlayer: public WrapObject {
 public:
  template<class T>
  void add_event_listener_1(const GUIEventName& name, cString& func, int id) {
    self<AudioPlayer>()->on(name, [this, func](Event<>& evt) {
      // if (worker()->is_terminate()) return;
      HandleScope scope(worker());
      // arg event
      Wrap<Event<>>* ev = Wrap<Event<>>::pack(&evt);
      
      ev->set_private_data(Cast::entity<T>()); // set data cast func
      
      Local<JSValue> args[2] = { ev->that(), worker()->New(true) };
      // call js trigger func
      call( worker()->New(func,1), 2, args );
    }, id);
  }
  
  /**
   * @func overwrite
   */
  virtual bool add_event_listener(cString& name, cString& func, int id) {
    auto i = GUI_EVENT_PLAYER_TABLE.find(name);
    if ( i == GUI_EVENT_PLAYER_TABLE.end() ) {
      return false;
    }
    if ( i.value() == GUI_EVENT_PLAYER_WAIT_BUFFER ) { // Float
      add_event_listener_1<Float>(i.value(), func, id);
    } else if ( i.value() == GUI_EVENT_PLAYER_ERROR ) { // Error
      add_event_listener_1<Error>(i.value(), func, id);
    } else if ( i.value() == GUI_EVENT_PLAYER_SEEK ) { // Uint64
      add_event_listener_1<Uint64>(i.value(), func, id);
    } else { // object
      add_event_listener_1<Object>(i.value(), func, id);
    }
    return true;
  }
  
  virtual bool remove_event_listener(cString& name, int id) {
    auto i = GUI_EVENT_PLAYER_TABLE.find(name);
    if ( i != GUI_EVENT_PLAYER_TABLE.end() ) {
      // off event listener
      self<AudioPlayer>()->off(i.value(), id);
      return true;
    }
    return true;
  }
  
  /**
   * @constructor([src])
   * @arg [src] {String}
   */
  static void constructor(FunctionCall args) {
    JS_WORKER(args);
    if ( args.Length() > 0 && args[0]->IsString(worker) ) {
      New<WrapAudioPlayer>(args, new AudioPlayer(args[0]->ToStringValue(worker)));
    } else {
      New<WrapAudioPlayer>(args, new AudioPlayer());
    }
  }
  
  /**
   * @get auto_play {bool}
   */
  static void auto_play(Local<JSString> name, PropertyCall args) {
    JS_WORKER(args);
    JS_SELF(AudioPlayer);
    JS_RETURN( self->auto_play() );
  }
  
  /**
   * @set auto_play {bool}
   */
  static void set_auto_play(Local<JSString> name, Local<JSValue> value, PropertySetCall args) {
    JS_WORKER(args); GUILock lock;
    JS_SELF(AudioPlayer);
    self->set_auto_play( value->ToBooleanValue(worker) );
  }
  
  /**
   * @get source_status {enum MultimediaSourceStatus}
   */
  static void source_status(Local<JSString> name, PropertyCall args) {
    JS_WORKER(args);
    JS_SELF(AudioPlayer);
    JS_RETURN( self->source_status() );
  }
  
  /**
   * @get status {enum PlayerStatus}
   */
  static void status(Local<JSString> name, PropertyCall args) {
    JS_WORKER(args);
    JS_SELF(AudioPlayer);
    JS_RETURN( self->status() );
  }
  
  /**
   * @get mute {bool}
   */
  static void mute(Local<JSString> name, PropertyCall args) {
    JS_WORKER(args);
    JS_SELF(AudioPlayer);
    JS_RETURN( self->mute() );
  }
  
  /**
   * @set mute {bool}
   */
  static void set_mute(Local<JSString> name, Local<JSValue> value, PropertySetCall args) {
    JS_WORKER(args); GUILock lock;
    JS_SELF(AudioPlayer);
    self->set_mute( value->ToBooleanValue(worker) );
  }
  
  /**
   * @get volume {uint} 0-100
   */
  static void volume(Local<JSString> name, PropertyCall args) {
    JS_WORKER(args);
    JS_SELF(AudioPlayer);
    JS_RETURN( self->mute() );
  }
  
  /**
   * @set volume {uint} 0-100
   */
  static void set_volume(Local<JSString> name, Local<JSValue> value, PropertySetCall args) {
    JS_WORKER(args); GUILock lock;
    if ( !value->IsNumber(worker) ) {
      JS_THROW_ERR("* @set volume {uint} 0-100");
    }
    JS_SELF(AudioPlayer);
    self->set_volume( value->ToNumberValue(worker) );
  }
  
  /**
   * @get src {String}
   */
  static void src(Local<JSString> name, PropertyCall args) {
    JS_WORKER(args);
    JS_SELF(AudioPlayer);
    JS_RETURN( self->src() );
  }
  
  /**
   * @set src {String}
   */
  static void set_src(Local<JSString> name, Local<JSValue> value, PropertySetCall args) {
    JS_WORKER(args); GUILock lock;
    JS_SELF(AudioPlayer);
    self->set_src( value->ToStringValue(worker) );
  }
  
  /**
   * @get time {uint64} ms
   */
  static void time(Local<JSString> name, PropertyCall args) {
    JS_WORKER(args);
    JS_SELF(AudioPlayer);
    JS_RETURN( self->time() / 1000.0 );
  }
  
  /**
   * @get duration {uint64} ms
   */
  static void duration(Local<JSString> name, PropertyCall args) {
    JS_WORKER(args);
    JS_SELF(AudioPlayer);
    JS_RETURN( self->duration() / 1000.0 );
  }
  
  /**
   * @get track_index {uint}
   */
  static void track_index(Local<JSString> name, PropertyCall args) {
    JS_WORKER(args);
    JS_SELF(AudioPlayer);
    JS_RETURN( self->track_index() );
  }
  
  /**
   * @get track_count {uint}
   */
  static void track_count(Local<JSString> name, PropertyCall args) {
    JS_WORKER(args);
    JS_SELF(AudioPlayer);
    JS_RETURN( self->track_count() );
  }
  
  /**
   * @func select_track(index)
   * @arg index {uint} audio track index
   */
  static void select_track(FunctionCall args) {
    JS_WORKER(args); GUILock lock;
    if (args.Length() < 1 || ! args[0]->IsUint32(worker) ) {
      JS_THROW_ERR(
        "* @func selectTrack(index)\n"
        "* @arg index {uint} audio track index\n"
      );
    }
    JS_SELF(AudioPlayer);
    self->select_track( args[0]->ToUint32Value(worker) );
  }
  
  /**
   * @func track([index])
   * @arg [track=curent_track] {uint} default use current track index
   * @ret {object TrackInfo}
   */
  static void track(FunctionCall args) {
    JS_WORKER(args);
    JS_SELF(AudioPlayer);
    if (args.Length() < 1 || !args[0]->IsUint32(worker) ) {
      JS_RETURN( inl__track_to_jsvalue(self->track(), worker) );
    } else {
      JS_RETURN( inl__track_to_jsvalue(self->track(args[0]->ToUint32Value(worker)), worker) );
    }
  }
  
  /**
   * @func start()
   */
  static void start(FunctionCall args) {
    JS_WORKER(args); GUILock lock;
    JS_SELF(AudioPlayer);
    self->start();
  }
  
  /**
   * @func seek(time)
   * @arg time {uint} ms
   * @ret {bool} success
   */
  static void seek(FunctionCall args) {
    JS_WORKER(args); GUILock lock;
    if (args.Length() < 1 || ! args[0]->IsNumber(worker) ) {
      JS_THROW_ERR(
        "* @func seek(time)\n"
        "* @arg time {uint} ms\n"
        "* @ret {bool} success\n"
      );
    }
    JS_SELF(AudioPlayer);
    JS_RETURN( self->seek( args[0]->ToNumberValue(worker) * 1000.0 ));
  }
  
  /**
   * @func pause()
   */
  static void pause(FunctionCall args) {
    JS_WORKER(args); GUILock lock;
    JS_SELF(AudioPlayer);
    self->pause();
  }
  
  /**
   * @func resume()
   */
  static void resume(FunctionCall args) {
    JS_WORKER(args); GUILock lock;
    JS_SELF(AudioPlayer);
    self->resume();
  }
  
  /**
   * @func stop()
   */
  static void stop(FunctionCall args) {
    JS_WORKER(args);
    JS_SELF(AudioPlayer);
    self->stop();
  }
  
  /**
   * @get disable_wait_buffer {bool}
   */
  static void disable_wait_buffer(Local<JSString> name, PropertyCall args) {
    JS_WORKER(args);
    JS_SELF(AudioPlayer);
    JS_RETURN( self->disable_wait_buffer() );
  }
  
  /**
   * @set disable_wait_buffer {bool}
   */
  static void set_disable_wait_buffer(Local<JSString> name,
                                      Local<JSValue> value, PropertySetCall args) {
    JS_WORKER(args); GUILock lock;
    JS_SELF(AudioPlayer);
    self->disable_wait_buffer( value->ToBooleanValue(worker) );
  }
  
  static void binding(Local<JSObject> exports, Worker* worker) {
    
    JS_DEFINE_CLASS(AudioPlayer, constructor, {
      JS_SET_CLASS_ACCESSOR(autoPlay, auto_play, set_auto_play);
      JS_SET_CLASS_ACCESSOR(sourceStatus, source_status);
      JS_SET_CLASS_ACCESSOR(status, status);
      JS_SET_CLASS_ACCESSOR(mute, mute, set_mute);
      JS_SET_CLASS_ACCESSOR(volume, volume, set_volume);
      JS_SET_CLASS_ACCESSOR(src, src, set_src);
      JS_SET_CLASS_ACCESSOR(time, time);
      JS_SET_CLASS_ACCESSOR(duration, duration);
      JS_SET_CLASS_ACCESSOR(trackIndex, track_index);
      JS_SET_CLASS_ACCESSOR(trackCount, track_count);
      JS_SET_CLASS_ACCESSOR(disableWaitBuffer,
                          disable_wait_buffer, set_disable_wait_buffer);
      JS_SET_CLASS_METHOD(selectTrack, select_track);
      JS_SET_CLASS_METHOD(track, track);
      JS_SET_CLASS_METHOD(start, start);
      JS_SET_CLASS_METHOD(seek, seek);
      JS_SET_CLASS_METHOD(pause, pause);
      JS_SET_CLASS_METHOD(resume, resume);
      JS_SET_CLASS_METHOD(stop, stop);
    }, nullptr);
  }
};

void binding_audio(Local<JSObject> exports, Worker* worker) {
  WrapAudioPlayer::binding(exports, worker);
}

JS_END
