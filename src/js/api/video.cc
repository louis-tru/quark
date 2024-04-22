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

#include "../_js.h"
#include "./_event.h"
#include "./_view.h"
#include "../../views2/video.h"
#include "../../media/media-codec.h"

/**
 * @ns qk::js
 */

Js_BEGIN


typedef MultimediaSource::TrackInfo TrackInfo;

extern Local<JSValue> inl_track_to_jsvalue(const TrackInfo* track, Worker* worker);

/**
 * @class WrapVideo
 */
class WrapVideo: public WrapViewBase {
	public:

	static void constructor(FunctionCall args) {
		Js_ATTACH(args);
		Js_CHECK_APP();
		auto player = static_cast<Video*>(module_video->create(nullptr));
		if (!player) {
			Js_Worker(args);
			Js_Throw("create Video fail");
		}
		New<WrapVideo>(args, player);
	}
	
	static void auto_play(Local<JSString> name, PropertyCall args) {
		Js_Worker(args);
		Js_Self(Video);
		Js_Return( self->auto_play() );
	}
	
	static void set_auto_play(Local<JSString> name, Local<JSValue> value, PropertySetCall args) {
		Js_Worker(args); UILock lock;
		Js_Self(Video);
		self->set_auto_play( value->ToBooleanValue(worker) );
	}
	
	static void source_status(Local<JSString> name, PropertyCall args) {
		Js_Worker(args);
		Js_Self(Video);
		Js_Return( self->source_status() );
	}
	
	static void status(Local<JSString> name, PropertyCall args) {
		Js_Worker(args);
		Js_Self(Video);
		Js_Return( self->status() );
	}
	
	static void mute(Local<JSString> name, PropertyCall args) {
		Js_Worker(args);
		Js_Self(Video);
		Js_Return( self->mute() );
	}
	
	static void set_mute(Local<JSString> name, Local<JSValue> value, PropertySetCall args) {
		Js_Worker(args); UILock lock;
		Js_Self(Video);
		self->set_mute( value->ToBooleanValue(worker) );
	}
	
	static void volume(Local<JSString> name, PropertyCall args) {
		Js_Worker(args);
		Js_Self(Video);
		Js_Return( self->volume() );
	}
	
	static void set_volume(Local<JSString> name, Local<JSValue> value, PropertySetCall args) {
		Js_Worker(args); UILock lock;
		if ( !value->IsNumber(worker) ) {
			Js_Throw("* @set volume {uint} 0-100");
		}
		Js_Self(Video);
		self->set_volume( value->ToNumberValue(worker) );
	}
	
	static void time(Local<JSString> name, PropertyCall args) {
		Js_Worker(args);
		Js_Self(Video);
		Js_Return( self->time() / 1000.0 );
	}
	
	static void duration(Local<JSString> name, PropertyCall args) {
		Js_Worker(args);
		Js_Self(Video);
		Js_Return( self->duration() / 1000.0 );
	}
	
	static void audio_track_index(Local<JSString> name, PropertyCall args) {
		Js_Worker(args);
		Js_Self(Video);
		Js_Return( self->audio_track_index() );
	}
	
	static void audio_track_count(Local<JSString> name, PropertyCall args) {
		Js_Worker(args);
		Js_Self(Video);
		Js_Return( self->audio_track_count() );
	}
	
	static void select_audio_track(FunctionCall args) {
		Js_Worker(args); UILock lock;
		if (args.Length() < 1 || ! args[0]->IsUint32(worker) ) {
			Js_Throw(
				"* @func selectAudioTrack(index)\n"
				"* @arg index {uint} audio track index\n"
			);
		}
		Js_Self(Video);
		self->select_audio_track( args[0]->ToUint32Value(worker) );
	}
	
	static void audio_track(FunctionCall args) {
		Js_Worker(args);
		Js_Self(Video);
		if (args.Length() < 1 || ! args[0]->IsUint32(worker) ) {
			Js_Return( inl_track_to_jsvalue(self->audio_track(), worker) );
		} else {
			Js_Return( inl_track_to_jsvalue(self->audio_track(args[0]->ToUint32Value(worker)), worker) );
		}
	}
	
	static void video_track(FunctionCall args) {
		Js_Worker(args);
		Js_Self(Video);
		Js_Return( inl_track_to_jsvalue(self->video_track(), worker) );
	}
	
	static void start(FunctionCall args) {
		Js_Worker(args); UILock lock;
		Js_Self(Video);
		self->start();
	}
	
	static void seek(FunctionCall args) {
		Js_Worker(args); UILock lock;
		if (args.Length() < 1 || ! args[0]->IsNumber(worker) ) {
			Js_Throw(
				"* @func seek(time)\n"
				"* @arg time {uint} ms\n"
				"* @ret {bool} success\n"
			);
		}
		Js_Self(Video);
		Js_Return( self->seek( args[0]->ToNumberValue(worker) * 1000.0 ));
	}
	
	static void pause(FunctionCall args) {
		Js_Worker(args); UILock lock;
		Js_Self(Video);
		self->pause();
	}
	
	static void resume(FunctionCall args) {
		Js_Worker(args); UILock lock;
		Js_Self(Video);
		self->resume();
	}
	
	static void stop(FunctionCall args) {
		Js_Worker(args); UILock lock;
		Js_Self(Video);
		self->stop();
	}
	
	static void disable_wait_buffer(Local<JSString> name, PropertyCall args) {
		Js_Worker(args);
		Js_Self(Video);
		Js_Return( self->disable_wait_buffer() );
	}
	
	static void set_disable_wait_buffer(Local<JSString> name,
																			Local<JSValue> value, PropertySetCall args) {
		Js_Worker(args); UILock lock;
		Js_Self(Video);
		self->disable_wait_buffer( value->ToBooleanValue(worker) );
	}
	
	static void video_width(Local<JSString> name, PropertyCall args) {
		Js_Worker(args);
		Js_Self(Video);
		Js_Return( self->video_width() );
	}
	
	static void video_height(Local<JSString> name, PropertyCall args) {
		Js_Worker(args);
		Js_Self(Video);
		Js_Return( self->video_height() );
	}

	static void binding(Local<JSObject> exports, Worker* worker) {
		worker->bindingModule("_value");

		Js_NEW_CLASS_FROM_ID(Video, module_video->tid, constructor, {
			Js_Set_Class_Accessor(autoPlay, auto_play, set_auto_play);
			Js_Set_Class_Accessor(sourceStatus, source_status);
			Js_Set_Class_Accessor(status, status);
			Js_Set_Class_Accessor(mute, mute, set_mute);
			Js_Set_Class_Accessor(volume, volume, set_volume);
			Js_Set_Class_Accessor(time, time);
			Js_Set_Class_Accessor(duration, duration);
			Js_Set_Class_Accessor(audioTrackIndex, audio_track_index);
			Js_Set_Class_Accessor(audioTrackCount, audio_track_count);
			Js_Set_Class_Accessor(disableWaitBuffer, disable_wait_buffer, set_disable_wait_buffer);
			Js_Set_Class_Accessor(videoWidth, video_width);
			Js_Set_Class_Accessor(videoHeight, video_height);
			Js_Set_Class_Method(selectAudioTrack, select_audio_track);
			Js_Set_Class_Method(audioTrack, audio_track);
			Js_Set_Class_Method(videoTrack, video_track);
			Js_Set_Class_Method(start, start);
			Js_Set_Class_Method(seek, seek);
			Js_Set_Class_Method(pause, pause);
			Js_Set_Class_Method(resume, resume);
			Js_Set_Class_Method(stop, stop);
		}, Js_Typeid(Image));
		cls->Export(worker, "Video", exports);
		IMPL::js_class(worker)->set_class_alias(module_video->tid, View::VIDEO);
	}
};

void binding_video(Local<JSObject> exports, Worker* worker) {
	WrapVideo::binding(exports, worker);
}

Js_END
