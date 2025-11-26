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
#include "../../util/thread.h"
#include "./android.h"
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>

#ifndef USE_ANDROID_OPENSLES_PCM_PLAYER
#define USE_ANDROID_OPENSLES_PCM_PLAYER 0
#endif

namespace qk {

	extern int get_channel_mask(uint32_t channel_count, uint32_t channel_layout);

#if USE_ANDROID_OPENSLES_PCM_PLAYER

	struct AudioEngine;
	static AudioEngine* audio_engine = nullptr;

	struct AudioEngine {

		AudioEngine(): engineObject(nullptr), engineEngine(nullptr), outputMixObject(nullptr) {
			SLresult result;

			// create engine
			result = slCreateEngine(&engineObject, 0, nullptr, 0, nullptr, nullptr); 
			Qk_ASSERT(SL_RESULT_SUCCESS == result);

			// realize the engine
			result = (*engineObject)->Realize(engineObject, SL_BOOLEAN_FALSE);
			Qk_ASSERT(SL_RESULT_SUCCESS == result);

			// get the engine interface, which is needed in order to create other objects
			result = (*engineObject)->GetInterface(engineObject, SL_IID_ENGINE, &engineEngine);
			Qk_ASSERT(SL_RESULT_SUCCESS == result);

			// create output mix,
			result = (*engineEngine)->CreateOutputMix(engineEngine, &outputMixObject, 0, 0, 0);
			Qk_ASSERT(SL_RESULT_SUCCESS == result);

			// realize the output mix
			result = (*outputMixObject)->Realize(outputMixObject, SL_BOOLEAN_FALSE);
			Qk_ASSERT(SL_RESULT_SUCCESS == result);
		}

		~AudioEngine() {

			// destroy output mix object, and invalidate all associated interfaces
			if (outputMixObject != nullptr) {
				(*outputMixObject)->Destroy(outputMixObject);
				outputMixObject = nullptr;
			}

			// destroy engine object, and invalidate all associated interfaces
			if (engineObject != nullptr) {
				(*engineObject)->Destroy(engineObject);
				engineObject = nullptr;
				engineEngine = nullptr;
			}
		}

		static AudioEngine* share() {
			if ( ! audio_engine ) {
				audio_engine = new AudioEngine();
			}
			return audio_engine;
		}

		// engine interfaces
		SLObjectItf engineObject;
		SLEngineItf engineEngine;
		// output mix interfaces
		SLObjectItf outputMixObject;
	};

	class AndroidPCMOpenSLES: public Object, public PCMPlayer {
	public:

		virtual Object* asObject() { return this; }

		AndroidPCMOpenSLES()
			: _max_volume_level(100)
			, _object(nullptr)
			, _play(nullptr)
			, _bufferQueue(nullptr)
			, _effectSend(nullptr)
			, _volume(nullptr)
		{
		}

		virtual ~AndroidPCMOpenSLES() {
			if (_object != nullptr) {
				(*_object)->Destroy(_object);
				_play = nullptr;
				_bufferQueue = nullptr;
				_effectSend = nullptr;
				_volume = nullptr;
			}
		}

		uint32_t min_buffer_size() {
			JNI::ScopeENV env;
			JNI::MethodInfo m("android/media/AudioTrack", "getMinBufferSize", "(III)I", true);
			int r = env->CallStaticIntMethod(m.clazz(), m.method(), _sampleRate,
										get_channel_mask(_channels, _channelMask), 2/*ENCODIF_PCM_16BIT*/);
			return r;
		}

		bool init(const Stream &stream) {
			auto engine = AudioEngine::share();
			if (!engine) return false;

			_sampleRate = stream.sample_rate;
			_channels = stream.channels;
			_channelMask = 0xffffffff & stream.channel_layout;
			_bufferSize = min_buffer_size();

			SLresult result;
			SLuint32 samplesPerSec;

			switch (_sampleRate) {
				case 8000 : samplesPerSec = SL_SAMPLINGRATE_8; break;
				case 11025: samplesPerSec = SL_SAMPLINGRATE_11_025; break;
				case 12000: samplesPerSec = SL_SAMPLINGRATE_12; break;
				case 16000: samplesPerSec = SL_SAMPLINGRATE_16; break;
				case 22050: samplesPerSec = SL_SAMPLINGRATE_22_05; break;
				case 24000: samplesPerSec = SL_SAMPLINGRATE_24; break;
				case 32000: samplesPerSec = SL_SAMPLINGRATE_32; break;
				case 44100: samplesPerSec = SL_SAMPLINGRATE_44_1; break;
				case 48000: samplesPerSec = SL_SAMPLINGRATE_48; break;
				case 64000: samplesPerSec = SL_SAMPLINGRATE_64; break; // not support
				case 88200: samplesPerSec = SL_SAMPLINGRATE_88_2; break;
				case 192000: samplesPerSec = SL_SAMPLINGRATE_192; break;
				default: return false;
			}

			// configure audio source
			SLDataLocator_AndroidSimpleBufferQueue loc_bufq = { SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, 2 };

			SLDataFormat_PCM format_pcm = { SL_DATAFORMAT_PCM,
																			_channels,
																			samplesPerSec,
																			SL_PCMSAMPLEFORMAT_FIXED_16,
																			SL_PCMSAMPLEFORMAT_FIXED_16,
																			_channelMask,
																			SL_BYTEORDER_LITTLEENDIAN };
			SLDataSource audioSrc = { &loc_bufq, &format_pcm };

			// configure audio sink
			SLDataLocator_OutputMix loc_outmix  = { SL_DATALOCATOR_OUTPUTMIX, engine->outputMixObject };
			SLDataSink              audioSnk    = { &loc_outmix, nullptr };

			// create audio player
			const SLInterfaceID ids[3]  = { SL_IID_BUFFERQUEUE, SL_IID_EFFECTSEND, SL_IID_VOLUME };
			const SLboolean     req[3]  = { SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE };

			result = (*engine->engineEngine)->CreateAudioPlayer(engine->engineEngine,
																													&_object,
																													&audioSrc, &audioSnk, 3, ids, req);
			Qk_ASSERT(SL_RESULT_SUCCESS == result);

			// realize the player
			result = (*_object)->Realize(_object, SL_BOOLEAN_FALSE);
			Qk_ASSERT(SL_RESULT_SUCCESS == result);

			// get the play interface
			result = (*_object)->GetInterface(_object, SL_IID_PLAY, &_play);
			Qk_ASSERT(SL_RESULT_SUCCESS == result);

			// get the buffer queue interface
			result = (*_object)->GetInterface(_object, SL_IID_BUFFERQUEUE, &_bufferQueue);
			Qk_ASSERT(SL_RESULT_SUCCESS == result);

			// get the effect send interface
			result = (*_object)->GetInterface(_object, SL_IID_EFFECTSEND, &_effectSend);
			Qk_ASSERT(SL_RESULT_SUCCESS == result);

			// get the volume interface
			result = (*_object)->GetInterface(_object, SL_IID_VOLUME, &_volume);
			Qk_ASSERT(SL_RESULT_SUCCESS == result);

			// get max volume level
			result = (*_volume)->GetMaxVolumeLevel(_volume, &_max_volume_level);
			Qk_ASSERT(SL_RESULT_SUCCESS == result);

			// set playing status
			result = (*_play)->SetPlayState(_play, SL_PLAYSTATE_PLAYING);
			Qk_ASSERT(SL_RESULT_SUCCESS == result);

			Qk_DLog("createAudioPlayer finish");

			return true;
		}

		bool write(const Frame *frame) override {
			SLresult result;
			ScopeLock scope(_lock);
			// input pcm buffer
			result = (*_bufferQueue)->Enqueue(_bufferQueue, frame->data[0], frame->linesize[0]);
			return result != SL_RESULT_BUFFER_INSUFFICIENT;
		}

		void flush() override {
			SLresult result;
			ScopeLock scope(_lock);
			result = (*_bufferQueue)->Clear(_bufferQueue);
			Qk_ASSERT(SL_RESULT_SUCCESS == result);
		}

		void set_mute(bool value) override {
			SLresult result;
			result = (*_volume)->SetMute(_volume, value);
		}

		void set_volume(float value) override {
			if ( _max_volume_level ) {
				SLresult result;
				value = Qk_Min(1.0, value);
				result = (*_volume)->SetVolumeLevel(_volume, value * _max_volume_level);
			}
		}

		float delayed() override {
			return -1.0;
		}

	private:
		Mutex         _lock;
		SLint16       _max_volume_level;
		uint32_t      _sampleRate;
		uint32_t      _channels;
		SLuint32      _channelMask;
		uint32_t      _bufferSize;

		// engine interfaces
		SLObjectItf engineObject;
		SLEngineItf engineEngine;

		// output mix interfaces
		SLObjectItf outputMixObject;

		// buffer queue player interfaces
		SLObjectItf _object;
		SLPlayItf _play;
		SLAndroidSimpleBufferQueueItf _bufferQueue;
		SLEffectSendItf _effectSend;
		SLVolumeItf _volume;
	};

#endif

	typedef MediaSource::Stream SStream;

	PCMPlayer* create_android_audio_track(const SStream &stream);

	PCMPlayer* PCMPlayer::create(const SStream &stream) {
#if USE_ANDROID_OPENSLES_PCM_PLAYER
		Sp<AndroidPCMOpenSLES> player = new AndroidPCMOpenSLES();
		if ( player->init(stream) ) {
			return player.collapse();
		} else {
			return create_android_audio_track(stream);
		}
#else
		return create_android_audio_track(stream);
#endif
	}

}
