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

#include "../_js.h"
#include "./_event.h"
#include "./_view.h"
#include "../../views2/video.h"
#include "../../media/media-codec.h"

/**
 * @ns quark::js
 */

JS_BEGIN


typedef MultimediaSource::TrackInfo TrackInfo;

extern Local<JSValue> inl_track_to_jsvalue(const TrackInfo* track, Worker* worker);

/**
 * @class WrapVideo
 */
class WrapVideo: public WrapViewBase {
	public:

	static void constructor(FunctionCall args) {
		JS_ATTACH(args);
		JS_CHECK_APP();
		auto player = static_cast<Video*>(module_video->create(nullptr));
		if (!player) {
			JS_WORKER(args);
			JS_THROW_ERR("create Video fail");
		}
		New<WrapVideo>(args, player);
	}
	
	static void auto_play(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		JS_SELF(Video);
		JS_RETURN( self->auto_play() );
	}
	
	static void set_auto_play(Local<JSString> name, Local<JSValue> value, PropertySetCall args) {
		JS_WORKER(args); UILock lock;
		JS_SELF(Video);
		self->set_auto_play( value->ToBooleanValue(worker) );
	}
	
	static void source_status(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		JS_SELF(Video);
		JS_RETURN( self->source_status() );
	}
	
	static void status(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		JS_SELF(Video);
		JS_RETURN( self->status() );
	}
	
	static void mute(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		JS_SELF(Video);
		JS_RETURN( self->mute() );
	}
	
	static void set_mute(Local<JSString> name, Local<JSValue> value, PropertySetCall args) {
		JS_WORKER(args); UILock lock;
		JS_SELF(Video);
		self->set_mute( value->ToBooleanValue(worker) );
	}
	
	static void volume(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		JS_SELF(Video);
		JS_RETURN( self->volume() );
	}
	
	static void set_volume(Local<JSString> name, Local<JSValue> value, PropertySetCall args) {
		JS_WORKER(args); UILock lock;
		if ( !value->IsNumber(worker) ) {
			JS_THROW_ERR("* @set volume {uint} 0-100");
		}
		JS_SELF(Video);
		self->set_volume( value->ToNumberValue(worker) );
	}
	
	static void time(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		JS_SELF(Video);
		JS_RETURN( self->time() / 1000.0 );
	}
	
	static void duration(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		JS_SELF(Video);
		JS_RETURN( self->duration() / 1000.0 );
	}
	
	static void audio_track_index(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		JS_SELF(Video);
		JS_RETURN( self->audio_track_index() );
	}
	
	static void audio_track_count(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		JS_SELF(Video);
		JS_RETURN( self->audio_track_count() );
	}
	
	static void select_audio_track(FunctionCall args) {
		JS_WORKER(args); UILock lock;
		if (args.Length() < 1 || ! args[0]->IsUint32(worker) ) {
			JS_THROW_ERR(
				"* @func selectAudioTrack(index)\n"
				"* @arg index {uint} audio track index\n"
			);
		}
		JS_SELF(Video);
		self->select_audio_track( args[0]->ToUint32Value(worker) );
	}
	
	static void audio_track(FunctionCall args) {
		JS_WORKER(args);
		JS_SELF(Video);
		if (args.Length() < 1 || ! args[0]->IsUint32(worker) ) {
			JS_RETURN( inl_track_to_jsvalue(self->audio_track(), worker) );
		} else {
			JS_RETURN( inl_track_to_jsvalue(self->audio_track(args[0]->ToUint32Value(worker)), worker) );
		}
	}
	
	static void video_track(FunctionCall args) {
		JS_WORKER(args);
		JS_SELF(Video);
		JS_RETURN( inl_track_to_jsvalue(self->video_track(), worker) );
	}
	
	static void start(FunctionCall args) {
		JS_WORKER(args); UILock lock;
		JS_SELF(Video);
		self->start();
	}
	
	static void seek(FunctionCall args) {
		JS_WORKER(args); UILock lock;
		if (args.Length() < 1 || ! args[0]->IsNumber(worker) ) {
			JS_THROW_ERR(
				"* @func seek(time)\n"
				"* @arg time {uint} ms\n"
				"* @ret {bool} success\n"
			);
		}
		JS_SELF(Video);
		JS_RETURN( self->seek( args[0]->ToNumberValue(worker) * 1000.0 ));
	}
	
	static void pause(FunctionCall args) {
		JS_WORKER(args); UILock lock;
		JS_SELF(Video);
		self->pause();
	}
	
	static void resume(FunctionCall args) {
		JS_WORKER(args); UILock lock;
		JS_SELF(Video);
		self->resume();
	}
	
	static void stop(FunctionCall args) {
		JS_WORKER(args); UILock lock;
		JS_SELF(Video);
		self->stop();
	}
	
	static void disable_wait_buffer(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		JS_SELF(Video);
		JS_RETURN( self->disable_wait_buffer() );
	}
	
	static void set_disable_wait_buffer(Local<JSString> name,
																			Local<JSValue> value, PropertySetCall args) {
		JS_WORKER(args); UILock lock;
		JS_SELF(Video);
		self->disable_wait_buffer( value->ToBooleanValue(worker) );
	}
	
	static void video_width(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		JS_SELF(Video);
		JS_RETURN( self->video_width() );
	}
	
	static void video_height(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		JS_SELF(Video);
		JS_RETURN( self->video_height() );
	}

	static void binding(Local<JSObject> exports, Worker* worker) {
		worker->bindingModule("_value");

		JS_NEW_CLASS_FROM_ID(Video, module_video->tid, constructor, {
			JS_SET_CLASS_ACCESSOR(autoPlay, auto_play, set_auto_play);
			JS_SET_CLASS_ACCESSOR(sourceStatus, source_status);
			JS_SET_CLASS_ACCESSOR(status, status);
			JS_SET_CLASS_ACCESSOR(mute, mute, set_mute);
			JS_SET_CLASS_ACCESSOR(volume, volume, set_volume);
			JS_SET_CLASS_ACCESSOR(time, time);
			JS_SET_CLASS_ACCESSOR(duration, duration);
			JS_SET_CLASS_ACCESSOR(audioTrackIndex, audio_track_index);
			JS_SET_CLASS_ACCESSOR(audioTrackCount, audio_track_count);
			JS_SET_CLASS_ACCESSOR(disableWaitBuffer, disable_wait_buffer, set_disable_wait_buffer);
			JS_SET_CLASS_ACCESSOR(videoWidth, video_width);
			JS_SET_CLASS_ACCESSOR(videoHeight, video_height);
			JS_SET_CLASS_METHOD(selectAudioTrack, select_audio_track);
			JS_SET_CLASS_METHOD(audioTrack, audio_track);
			JS_SET_CLASS_METHOD(videoTrack, video_track);
			JS_SET_CLASS_METHOD(start, start);
			JS_SET_CLASS_METHOD(seek, seek);
			JS_SET_CLASS_METHOD(pause, pause);
			JS_SET_CLASS_METHOD(resume, resume);
			JS_SET_CLASS_METHOD(stop, stop);
		}, JS_TYPEID(Image));
		cls->Export(worker, "Video", exports);
		IMPL::js_class(worker)->set_class_alias(module_video->tid, View::VIDEO);
	}
};

void binding_video(Local<JSObject> exports, Worker* worker) {
	WrapVideo::binding(exports, worker);
}

JS_END
