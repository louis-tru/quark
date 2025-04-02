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

#include "../../media/pcm_player.h"
#include "../../util/handle.h"
#include "./android.h"

namespace qk {
	typedef JNI::MethodInfo MethodInfo;
	typedef JNI::ScopeENV   ScopeENV;
	typedef MediaSource::Stream SStream;

	enum {
		/** Default audio channel mask */
		CHANNEL_OUT_DEFAULT = 1,

		// Output channel mask definitions below are translated to the native values defined in
		//  in /system/media/audio/include/system/audio.h in the JNI code of AudioTrack
		CHANNEL_OUT_FRONT_LEFT = 0x4,
		CHANNEL_OUT_FRONT_RIGHT = 0x8,
		CHANNEL_OUT_FRONT_CENTER = 0x10,
		CHANNEL_OUT_LOW_FREQUENCY = 0x20,
		CHANNEL_OUT_BACK_LEFT = 0x40,
		CHANNEL_OUT_BACK_RIGHT = 0x80,
		CHANNEL_OUT_FRONT_LEFT_OF_CENTER = 0x100,
		CHANNEL_OUT_FRONT_RIGHT_OF_CENTER = 0x200,
		CHANNEL_OUT_BACK_CENTER = 0x400,
		CHANNEL_OUT_SIDE_LEFT = 0x800,
		CHANNEL_OUT_SIDE_RIGHT = 0x1000,
		/** @hide */
		CHANNEL_OUT_TOP_CENTER = 0x2000,
		/** @hide */
		CHANNEL_OUT_TOP_FRONT_LEFT = 0x4000,
		/** @hide */
		CHANNEL_OUT_TOP_FRONT_CENTER = 0x8000,
		/** @hide */
		CHANNEL_OUT_TOP_FRONT_RIGHT = 0x10000,
		/** @hide */
		CHANNEL_OUT_TOP_BACK_LEFT = 0x20000,
		/** @hide */
		CHANNEL_OUT_TOP_BACK_CENTER = 0x40000,
		/** @hide */
		CHANNEL_OUT_TOP_BACK_RIGHT = 0x80000,
	};

	int get_channel_mask(uint32_t channel_count, uint32_t channel_layout) {
		if (channel_count < 2) {
			return CHANNEL_OUT_DEFAULT;
		}
		int channels = 0;
		int layout = 0;

		if (channel_layout & kFront_Left_AudioChannelLayoutMask) {
			layout |= CHANNEL_OUT_FRONT_LEFT; channels++;
		}
		if (channel_layout & kFront_Right_AudioChannelLayoutMask) {
			layout |= CHANNEL_OUT_FRONT_RIGHT; channels++;
		}
		if (channel_layout & kFront_Center_AudioChannelLayoutMask) {
			layout |= CHANNEL_OUT_FRONT_CENTER; channels++;
		}
		if (channel_layout & kLow_Frequency_AudioChannelLayoutMask) {
			layout |= CHANNEL_OUT_LOW_FREQUENCY; channels++;
		}
		if (channel_layout & kBack_Left_AudioChannelLayoutMask) {
			layout |= CHANNEL_OUT_BACK_LEFT; channels++;
		}
		if (channel_layout & kBack_Right_AudioChannelLayoutMask) {
			layout |= CHANNEL_OUT_BACK_RIGHT; channels++;
		}
		if (channel_layout & kFront_Left_Of_Center_AudioChannelLayoutMask) {
			layout |= CHANNEL_OUT_FRONT_LEFT_OF_CENTER; channels++;
		}
		if (channel_layout & kFront_Right_Of_Center_AudioChannelLayoutMask) {
			layout |= CHANNEL_OUT_FRONT_RIGHT_OF_CENTER; channels++;
		}
		if (channel_layout & kBack_Center_AudioChannelLayoutMask) {
			layout |= CHANNEL_OUT_BACK_CENTER; channels++;
		}
		if (channel_layout & kSide_Left_AudioChannelLayoutMask) {
			layout |= CHANNEL_OUT_SIDE_LEFT; channels++;
		}
		if (channel_layout & kSide_Right_AudioChannelLayoutMask) {
			layout |= CHANNEL_OUT_SIDE_RIGHT; channels++;
		}
		if (channel_layout & kTop_Center_AudioChannelLayoutMask) {
			layout |= CHANNEL_OUT_TOP_CENTER; channels++;
		}
		if (channel_layout & kTop_Front_Left_AudioChannelLayoutMask) {
			layout |= CHANNEL_OUT_TOP_FRONT_LEFT; channels++;
		}
		if (channel_layout & kTop_Front_Center_AudioChannelLayoutMask) {
			layout |= CHANNEL_OUT_TOP_FRONT_CENTER; channels++;
		}
		if (channel_layout & kTop_Front_Right_AudioChannelLayoutMask) {
			layout |= CHANNEL_OUT_TOP_FRONT_RIGHT; channels++;
		}
		if (channel_layout & kTop_Back_Left_AudioChannelLayoutMask) {
			layout |= CHANNEL_OUT_TOP_BACK_LEFT; channels++;
		}
		if (channel_layout & kTop_Back_Center_AudioChannelLayoutMask) {
			layout |= CHANNEL_OUT_TOP_BACK_CENTER; channels++;
		}
		if (channel_layout & kTop_Back_Right_AudioChannelLayoutMask) {
			layout |= CHANNEL_OUT_TOP_BACK_RIGHT; channels++;
		}
		Qk_ASSERT_EQ(channels, channel_layout);

		return layout;
	}

	class AndroidAudioTrack: public Object, public PCMPlayer {
	public:

		Object* asObject() override { return this; }

		AndroidAudioTrack()
			: _sampleRate(0)
			, _channels(0), _channel_layout(0)
			, _channelMask(0), _minBufferSize(0)
			, _min_volume(0), _max_volume(1)
			, _volume(1)
			, _self(nullptr)
			, _clazz(nullptr)
			, _buffer(nullptr)
		{
			ScopeENV env;
			_clazz             = JNI::find_clazz("android/media/AudioTrack");
			_getMinBufferSize  = JNI::find_static_method(_clazz, "getMinBufferSize", "(III)I");
			_getMinVolume      = JNI::find_static_method(_clazz, "getMinVolume", "()F");
			_getMaxVolume      = JNI::find_static_method(_clazz, "getMaxVolume", "()F");
			_constructor       = JNI::find_method(_clazz, "<init>", "(IIIIII)V");
			_play              = JNI::find_method(_clazz, "play", "()V");
			_stop              = JNI::find_method(_clazz, "stop", "()V");
			_pause             = JNI::find_method(_clazz, "pause", "()V");
			_write             = JNI::find_method(_clazz, "write", "(Ljava/nio/ByteBuffer;II)I");
			_setVolume         = JNI::find_method(_clazz, "setVolume", "(F)I");
			_flush             = JNI::find_method(_clazz, "flush", "()V");
			_clazz             = (jclass)env->NewGlobalRef(_clazz);
			_buffer_rewind     = JNI::find_method("java/nio/ByteBuffer", "rewind", "()Ljava/nio/Buffer;");
		}

		virtual ~AndroidAudioTrack() {
			ScopeENV env;
			if ( _self ) {
				env->CallVoidMethod(_self, _stop);
				env->DeleteGlobalRef(_self);
			}
			if ( _clazz ) {
				env->DeleteGlobalRef(_clazz);
			}
			if ( _buffer ) {
				env->DeleteGlobalRef(_buffer);
			}
		}

		bool init(const Stream &stream) {
			ScopeENV env;
			_channels = stream.channels;
			_sampleRate = stream.sample_rate;
			_channel_layout = 0xffffffff & stream.channel_layout;
			_channelMask = get_channel_mask(_channels, _channel_layout);
			_minBufferSize = env->CallStaticIntMethod(_clazz, _getMinBufferSize, _sampleRate,
				_channelMask, 2/*ENCODIN_PCM_16BIT*/);
			_min_volume = env->CallStaticFloatMethod(_clazz, _getMinVolume);
			_max_volume = env->CallStaticFloatMethod(_clazz, _getMaxVolume);

			uint32_t bufferSize = _channels * _sampleRate * 2 * (32.0f / 1000.0f); // 32ms
			if (_minBufferSize <= 0) {
				_minBufferSize = bufferSize;
			}

			// new android.media.AudioTrack
			_self = env->NewObject(_clazz, _constructor,
															3, /* STREAM_MUSIC */
															_sampleRate,
															_channelMask,
															2, /* ENCODIF_PCM_16BIT */
															_minBufferSize * 2, // Use double size
															1 /* MODE_STREAM */
			);
			if (!_self) return false;

			_self = env->NewGlobalRef(_self);

			bufferSize = Qk_Max(_minBufferSize, bufferSize);
			_bufferHold = Buffer((char*)malloc(bufferSize), bufferSize);
			_buffer = env->NewGlobalRef(env->NewDirectByteBuffer(_bufferHold.val(), bufferSize));

			// audio track play
			env->CallVoidMethod(_self, _play);

			return true;
		}

		bool write(const Frame *frame) override {
			ScopeENV env;
			// buffer rewind
			// env->DeleteLocalRef(env->CallObjectMethod(_buffer, _buffer_rewind));
			Qk_ASSERT(_bufferHold.length() >= frame->linesize[0]);
			// copy pcm data
			Qk_ASSERT_EQ(env->GetDirectBufferAddress(_buffer), _bufferHold.val());
			memcpy(_bufferHold.val(), frame->data[0], frame->linesize[0]);
			// write pcm data
			int r = env->CallIntMethod(_self, _write, _buffer, frame->linesize[0], 1);
			return true;
		}

		void flush() override {
			ScopeENV env;
			env->CallVoidMethod(_self, _flush);
		}

		void set_mute(bool value) override {
			if ( value ) {
				ScopeENV env;
				env->CallVoidMethod(_self, _setVolume, 0.0f);
			} else {
				set_volume(_volume);
			}
		}

		void set_volume(float value) override {
			ScopeENV env;
			_volume = Qk_Min(1.0, value);
			jfloat f = _volume;
			env->CallIntMethod(_self, _setVolume, f);
		}

		float delayed() override {
			return -1.0;
		}

	private:
		uint32_t    _sampleRate;
		uint32_t    _channels;
		uint32_t    _channel_layout;
		int         _channelMask;
		int         _minBufferSize, _bufferSize;
		float       _min_volume, _max_volume;
		float       _volume;
		Buffer      _bufferHold;
		jobject     _self;
		jclass      _clazz;
		jobject     _buffer;
		jmethodID   _getMinBufferSize;
		jmethodID   _getMinVolume;
		jmethodID   _getMaxVolume;
		jmethodID   _constructor;
		jmethodID   _play;
		jmethodID   _stop;
		jmethodID   _pause;
		jmethodID   _write;
		jmethodID   _setVolume;
		jmethodID   _flush;
		jmethodID   _buffer_rewind;
	};

	PCMPlayer* create_android_audio_track(const SStream &stream) {
		Handle<AndroidAudioTrack> player = new AndroidAudioTrack();
		if ( player->init(stream) ) {
			return player.collapse();
		}
		return nullptr;
	}

}