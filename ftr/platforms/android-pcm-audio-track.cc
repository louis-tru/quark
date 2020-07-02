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

#include "ftr/pcm-player.h"
#include "ftr/util/handle.h"
#include "ftr/util/android-jni.h"

FX_NS(ftr)

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
int get_channel_mask(uint channel_count) {
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
	typedef DefaultTraits Traits;

	virtual Object* to_object() { return this; }

	AndroidAudioTrack()
	: m_sample_rate(0)
	, m_channel_count(0)
	, m_buffer_size(0)
	, m_min_volume(0)
	, m_max_volume(1)
	, m_volume(70)
	, m_self(NULL)
	, m_clazz(NULL)
	, m_buffer(NULL)
	{
		ScopeENV env;
		m_buffer_rewind     = JNI::find_method("java/nio/ByteBuffer", "rewind", "()Ljava/nio/Buffer;");
		m_clazz             = JNI::find_clazz("android/media/AudioTrack");
		m_getMinBufferSize  = JNI::find_static_method(m_clazz, "getMinBufferSize", "(III)I");
		m_getMinVolume      = JNI::find_static_method(m_clazz, "getMinVolume", "()F");
		m_getMaxVolume      = JNI::find_static_method(m_clazz, "getMaxVolume", "()F");
		m_constructor       = JNI::find_method(m_clazz, "<init>", "(IIIIII)V");
		m_play              = JNI::find_method(m_clazz, "play", "()V");
		m_stop              = JNI::find_method(m_clazz, "stop", "()V");
		m_pause             = JNI::find_method(m_clazz, "pause", "()V");
		m_write             = JNI::find_method(m_clazz, "write", "(Ljava/nio/ByteBuffer;II)I");
		m_setVolume         = JNI::find_method(m_clazz, "setVolume", "(F)I");
		m_flush             = JNI::find_method(m_clazz, "flush", "()V");
		m_clazz = (jclass)env->NewGlobalRef(m_clazz);
	}

	virtual ~AndroidAudioTrack() {
		ScopeENV env;
		if ( m_self ) {
			env->CallVoidMethod(m_self, m_stop);
			env->DeleteGlobalRef(m_self);
		}
		if ( m_clazz )
			env->DeleteGlobalRef(m_clazz);
		if ( m_buffer )
			env->DeleteGlobalRef(m_buffer);
	}

	bool initialize(uint channel_count, uint sample_rate) {
		ScopeENV env;

		m_channel_count = channel_count;
		m_sample_rate = sample_rate;
		m_buffer_size = min_buffer_size();
		m_min_volume = env->CallStaticFloatMethod(m_clazz, m_getMinVolume);
		m_max_volume = env->CallStaticFloatMethod(m_clazz, m_getMaxVolume);

		if ( m_buffer_size <= 0 ) {
			m_buffer_size = 4096 * m_channel_count;
		}

		// new Audio track object
		m_self = env->NewObject(m_clazz, m_constructor,
														3, /* STREAM_MUSIC */
														m_sample_rate,
														get_channel_mask(m_channel_count),
														2, /* ENCODIFX_PCM_16BIT */
														m_buffer_size * 2,
														1  /* MODE_STREAM */
		);

		ASSERT(m_self);

		m_self = env->NewGlobalRef(m_self);

		// new buffer swap area
		uint size = FX_MAX(m_buffer_size, 1024 * 32);
		m_buffer = env->NewGlobalRef(env->NewDirectByteBuffer(malloc(size), size));

		// audio track play
		env->CallVoidMethod(m_self, m_play);

		return true;
	}

	/**
	 * @overwrite
	 * */
	virtual bool write(cBuffer& buffer) {
		ScopeENV env;
		// buffer rewind
		env->DeleteLocalRef(env->CallObjectMethod(m_buffer, m_buffer_rewind));
		// copy pcm data
		memcpy(env->GetDirectBufferAddress(m_buffer), *buffer, buffer.length());
		// write pcm data
		int r = env->CallIntMethod(m_self, m_write, m_buffer, buffer.length(), 1);

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
		env->CallVoidMethod(m_self, m_flush);
	}

	/**
	 * @overwrite
	 * */
	virtual bool set_mute(bool value) {
		if ( value ) {
			JNI::ScopeENV env;
			env->CallVoidMethod(m_self, m_setVolume, 0.0f);
		} else {
			set_volume(m_volume);
		}
		return true;
	}

	/**
	 * @overwrite
	 * */
	virtual bool set_volume(uint value) {
		JNI::ScopeENV env;
		m_volume = FX_MIN(100, value);
		jfloat f = m_volume / 100.0;
		env->CallIntMethod(m_self, m_setVolume, f);
		return true;
	}

	/**
	 * @func buffer_size
	 * */
	virtual uint buffer_size() {
		return m_buffer_size;
	}

	int min_buffer_size() {
		JNI::ScopeENV env;
		int mask = get_channel_mask(FX_MAX(m_channel_count, 2));
		return env->CallStaticIntMethod(m_clazz, m_getMinBufferSize,
																		m_sample_rate, mask, 2/*ENCODIFX_PCM_16BIT*/);
	}

 private:
	uint        m_sample_rate;
	uint        m_channel_count;
	int         m_buffer_size;
	float       m_min_volume;
	float       m_max_volume;
	uint        m_volume;
	jobject     m_self;
	jclass      m_clazz;
	jobject     m_buffer;
	jmethodID   m_getMinBufferSize;
	jmethodID   m_getMinVolume;
	jmethodID   m_getMaxVolume;
	jmethodID   m_constructor;
	jmethodID   m_play;
	jmethodID   m_stop;
	jmethodID   m_pause;
	jmethodID   m_write;
	jmethodID   m_setVolume;
	jmethodID   m_flush;
	jmethodID   m_buffer_rewind;
};

/**
 * @func _inl_create_android_audio_track
 */
PCMPlayer* _inl_create_android_audio_track(uint channel_count, uint sample_rate) {
	Handle<AndroidAudioTrack> player = new AndroidAudioTrack();
	if ( player->initialize(channel_count, sample_rate) ) {
		return player.collapse();
	}
	return NULL;
}

FX_END