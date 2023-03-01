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

#include "quark/media/pcm_player.h"
#include "quark/util/handle.h"
#include "quark/util/platform/android_jni.h"

namespace qk {

typedef JNI::MethodInfo MethodInfo;
typedef JNI::ScopeENV   ScopeENV;

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

/**
 * @func get_channel_mask
 * */
int get_channel_mask(uint32_t channel_count) {
	int channelMask = CHANNEL_OUT_DEFAULT;
	switch (channel_count) {
		default:
		case 1: // 1
			channelMask = CHANNEL_OUT_DEFAULT/*CHANNEL_OUT_FRONT_CENTER*/; break;
		case 2: // 2
			channelMask = CHANNEL_OUT_FRONT_LEFT | CHANNEL_OUT_FRONT_RIGHT; break;
		case 3: // 2.1
			channelMask = CHANNEL_OUT_FRONT_LEFT | CHANNEL_OUT_FRONT_RIGHT |
							CHANNEL_OUT_LOW_FREQUENCY; break;
		case 4: // 4
			channelMask = CHANNEL_OUT_FRONT_LEFT | CHANNEL_OUT_FRONT_RIGHT |
							CHANNEL_OUT_BACK_LEFT | CHANNEL_OUT_BACK_RIGHT; break;
		case 5: // 4.1
			channelMask = CHANNEL_OUT_FRONT_LEFT | CHANNEL_OUT_FRONT_RIGHT | CHANNEL_OUT_LOW_FREQUENCY |
							CHANNEL_OUT_BACK_LEFT | CHANNEL_OUT_BACK_RIGHT; break;
		case 6: // 5.1
			channelMask = CHANNEL_OUT_FRONT_LEFT | CHANNEL_OUT_FRONT_RIGHT | CHANNEL_OUT_FRONT_CENTER |
							CHANNEL_OUT_LOW_FREQUENCY |
							CHANNEL_OUT_BACK_LEFT | CHANNEL_OUT_BACK_LEFT; break;
		case 7: // 6.1
			channelMask = CHANNEL_OUT_FRONT_LEFT | CHANNEL_OUT_FRONT_RIGHT | CHANNEL_OUT_FRONT_CENTER |
							CHANNEL_OUT_LOW_FREQUENCY |
							CHANNEL_OUT_SIDE_LEFT | CHANNEL_OUT_SIDE_RIGHT |
							CHANNEL_OUT_BACK_CENTER; break;
		case 8: // 7.1
			channelMask = CHANNEL_OUT_FRONT_LEFT | CHANNEL_OUT_FRONT_RIGHT | CHANNEL_OUT_FRONT_CENTER |
							CHANNEL_OUT_LOW_FREQUENCY |
							CHANNEL_OUT_SIDE_LEFT | CHANNEL_OUT_SIDE_RIGHT |
							CHANNEL_OUT_BACK_LEFT | CHANNEL_OUT_BACK_RIGHT; break;
	}
	return channelMask;
}

/**
 * @class AndroidAudioTrack
 */
class AndroidAudioTrack: public Object, public PCMPlayer {
	public:
		typedef ObjectTraits Traits;

		virtual Object* to_object() { return this; }

		AndroidAudioTrack()
		: _sample_rate(0)
		, _channel_count(0)
		, _buffer_size(0)
		, _min_volume(0)
		, _max_volume(1)
		, _volume(70)
		, _self(NULL)
		, _clazz(NULL)
		, _buffer(NULL)
		{
			ScopeENV env;
			_buffer_rewind     = JNI::find_method("java/nio/ByteBuffer", "rewind", "()Ljava/nio/Buffer;");
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
			_clazz = (jclass)env->NewGlobalRef(_clazz);
		}

		virtual ~AndroidAudioTrack() {
			ScopeENV env;
			if ( _self ) {
				env->CallVoidMethod(_self, _stop);
				env->DeleteGlobalRef(_self);
			}
			if ( _clazz )
				env->DeleteGlobalRef(_clazz);
			if ( _buffer )
				env->DeleteGlobalRef(_buffer);
		}

		bool initialize(uint32_t channel_count, uint32_t sample_rate) {
			ScopeENV env;

			_channel_count = channel_count;
			_sample_rate = sample_rate;
			_buffer_size = min_buffer_size();
			_min_volume = env->CallStaticFloatMethod(_clazz, _getMinVolume);
			_max_volume = env->CallStaticFloatMethod(_clazz, _getMaxVolume);

			if ( _buffer_size <= 0 ) {
				_buffer_size = 4096 * _channel_count;
			}

			// new Audio track object
			_self = env->NewObject(_clazz, _constructor,
															3, /* STREAM_MUSIC */
															_sample_rate,
															get_channel_mask(_channel_count),
															2, /* ENCODIF_PCM_16BIT */
															_buffer_size * 2,
															1  /* MODE_STREAM */
			);

			Qk_ASSERT(_self);

			_self = env->NewGlobalRef(_self);

			// new buffer swap area
			uint32_t size = Qk_MAX(_buffer_size, 1024 * 32);
			_buffer = env->NewGlobalRef(env->NewDirectByteBuffer(malloc(size), size));

			// audio track play
			env->CallVoidMethod(_self, _play);

			return true;
		}

		/**
		* @overwrite
		* */
		virtual bool write(cBuffer& buffer) {
			ScopeENV env;
			// buffer rewind
			env->DeleteLocalRef(env->CallObjectMethod(_buffer, _buffer_rewind));
			// copy pcm data
			memcpy(env->GetDirectBufferAddress(_buffer), *buffer, buffer.length());
			// write pcm data
			int r = env->CallIntMethod(_self, _write, _buffer, buffer.length(), 1);

			return r == buffer.length();
		}
		
		/**
		* @overwrite
		*/
		virtual float compensate() {
			return -1.0;
		}

		/**
		* @overwrite
		* */
		virtual void flush() {
			JNI::ScopeENV env;
			env->CallVoidMethod(_self, _flush);
		}

		/**
		* @overwrite
		* */
		virtual bool set_mute(bool value) {
			if ( value ) {
				JNI::ScopeENV env;
				env->CallVoidMethod(_self, _setVolume, 0.0f);
			} else {
				set_volume(_volume);
			}
			return true;
		}

		/**
		* @overwrite
		* */
		virtual bool set_volume(uint32_t value) {
			JNI::ScopeENV env;
			_volume = Qk_MIN(100, value);
			jfloat f = _volume / 100.0;
			env->CallIntMethod(_self, _setVolume, f);
			return true;
		}

		/**
		* @func buffer_size
		* */
		virtual uint32_t buffer_size() {
			return _buffer_size;
		}

		int min_buffer_size() {
			JNI::ScopeENV env;
			int mask = get_channel_mask(Qk_MAX(_channel_count, 2));
			return env->CallStaticIntMethod(_clazz, _getMinBufferSize,
																			_sample_rate, mask, 2/*ENCODIN_PCM_16BIT*/);
		}

	private:
		uint32_t        _sample_rate;
		uint32_t        _channel_count;
		int         _buffer_size;
		float       _min_volume;
		float       _max_volume;
		uint32_t        _volume;
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

	/**
	* @func _inl_create_android_audio_track
	*/
	PCMPlayer* _inl_create_android_audio_track(uint32_t channel_count, uint32_t sample_rate) {
		Handle<AndroidAudioTrack> player = new AndroidAudioTrack();
		if ( player->initialize(channel_count, sample_rate) ) {
			return player.collapse();
		}
		return NULL;
	}

}