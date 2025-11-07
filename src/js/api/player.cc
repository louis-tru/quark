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

#include "./ui.h"
#include "../../ui/view/video.h"

namespace qk { namespace js {
	typedef MediaSource::Stream AvStream;

	JSValue* stream_to_jsvalue(const AvStream* str, Worker* worker) {
		if ( !str ) {
			return worker->newNull();
		}
		auto rv = worker->newObject();
		rv->set(worker, worker->newStringOneByte("type"), worker->newValue(str->type));
		rv->set(worker, worker->newStringOneByte("mime"), worker->newValue(str->mime));
		rv->set(worker, worker->newStringOneByte("language"), worker->newValue(str->language));
		rv->set(worker, worker->newStringOneByte("codecId"), worker->newValue(str->codec_id));
		rv->set(worker, worker->newStringOneByte("codecTag"), worker->newValue(str->codec_tag));
		rv->set(worker, worker->newStringOneByte("format"), worker->newValue(str->format));
		rv->set(worker, worker->newStringOneByte("profile"), worker->newValue(str->profile));
		rv->set(worker, worker->newStringOneByte("level"), worker->newValue(str->level));
		rv->set(worker, worker->newStringOneByte("width"), worker->newValue(str->width));
		rv->set(worker, worker->newStringOneByte("height"), worker->newValue(str->height));
		rv->set(worker, worker->newStringOneByte("bitrate"), worker->newValue(str->bitrate));
		rv->set(worker, worker->newStringOneByte("sampleRate"), worker->newValue(str->sample_rate));
		rv->set(worker, worker->newStringOneByte("channels"), worker->newValue(str->channels));
		rv->set(worker, worker->newStringOneByte("channelLayout"), worker->newValue(str->channel_layout));
		rv->set(worker, worker->newStringOneByte("duration"), worker->newValue(str->duration));
		rv->set(worker, worker->newStringOneByte("avgFramerate"),
			worker->newValue(Array<uint32_t>{ uint32_t(str->avg_framerate[0]), str->avg_framerate[1] })
		);
		rv->set(worker, worker->newStringOneByte("timeBase"),
			worker->newValue(Array<uint32_t>{ str->time_base[0], str->time_base[1] })
		);
		rv->set(worker, worker->newStringOneByte("index"), worker->newValue(str->index));
		rv->set(worker, worker->newStringOneByte("hashCode"), worker->newValue(str->hash_code));
		return rv;
	}

	void inheritPlayer(JSClass* cls, Worker* worker) {
		typedef Object Type;

		Js_Class_Accessor_Get(pts, {
			Js_UISelf(Player);
			Js_Return(worker->types()->jsvalue(self->pts()));
		});

		Js_UIObject_Accessor(Player, float, volume, volume);
		Js_UIObject_Accessor(Player, bool, mute, mute);

		Js_Class_Accessor_Get(isPause, {
			Js_UISelf(Player);
			Js_Return(worker->types()->jsvalue(self->is_pause()));
		});

		Js_Class_Accessor_Get(type, {
			Js_UISelf(Player);
			Js_Return(worker->types()->jsvalue(self->type()));
		});

		Js_Class_Accessor_Get(duration, {
			Js_UISelf(Player);
			Js_Return(worker->types()->jsvalue(self->duration()));
		});

		Js_Class_Accessor_Get(status, {
			Js_UISelf(Player);
			Js_Return(worker->types()->jsvalue(self->status()));
		});

		Js_UIObject_Accessor(Player, String, src, src);

		// Qk_DEFINE_ACCE_GET(MediaSource*, media_source);

		Js_Class_Accessor_Get(video, {
			Js_UISelf(Player);
			Js_Return(stream_to_jsvalue(self->video(), worker));
		});

		Js_Class_Accessor_Get(audio, {
			Js_UISelf(Player);
			Js_Return(stream_to_jsvalue(self->audio(), worker));
		});

		Js_Class_Accessor_Get(audioStreams, {
			Js_UISelf(Player);
			Js_Return(worker->types()->jsvalue(self->audio_streams()));
		});

		Js_Class_Method(play, {
			Js_UISelf(Player);
			self->play();
		});

		Js_Class_Method(pause, {
			Js_UISelf(Player);
			self->pause();
		});

		Js_Class_Method(stop, {
			Js_UISelf(Player);
			self->stop();
		});

		Js_Class_Method(seek, {
			uint32_t time;
			if (args.length() < 1 || !args[0]->asUint32(worker).to(time)) {
				Js_Throw(
					"@method Player.seek(timeMs)\n"
					"@param timeMs {uint32_t}\n"
				);
			}
			Js_UISelf(Player);
			self->seek(time * 1e3);
		});

		Js_Class_Method(switchAudio, {
			uint32_t index;
			if (args.length() < 1 || !args[0]->asUint32(worker).to(index)) {
				Js_Throw(
					"@method Player.switch_audio(index)\n"
					"@param index {uint32_t}\n"
				);
			}
			Js_UISelf(Player);
			self->switch_audio(index);
		});
	}

	struct MixAudioPlayer: MixUIObject {
		NotificationBasic* asNotificationBasic() {
			return self<AudioPlayer>();
		}
		Player* asPlayer() override {
			return self<AudioPlayer>();
		}
		static void binding(JSObject* exports, Worker* worker) {
			Js_Define_Class(AudioPlayer, 0, {
				auto audio = AudioPlayer::Make();
				New<MixAudioPlayer>(args, *audio);
			});
			inheritPlayer(cls, worker);
			cls->exports("AudioPlayer", exports);
		}
	};

	struct MixVideo: MixViewObject {
		Player* asPlayer() override {
			return self<Video>();
		}
		static void binding(JSObject* exports, Worker* worker) {
			Js_Define_Class(Video, Image, { Js_NewView(Video); });
			inheritPlayer(cls, worker);
			cls->exports("Video", exports);
		}
	};

	void binding_player(JSObject* exports, Worker* worker) {
		MixAudioPlayer::binding(exports, worker);
		MixVideo::binding(exports, worker);
	}
} }
