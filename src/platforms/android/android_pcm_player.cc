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
#include "../../util/loop.h"
#include "./android.h"
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>

#ifndef USE_ANDROID_OPENSLES_PCM_PLAYER
#define USE_ANDROID_OPENSLES_PCM_PLAYER 0
#endif

namespace qk {

	/**
	* @method get_channel_mask
	* */
	extern int get_channel_mask(uint32_t channel_count);

#if USE_ANDROID_OPENSLES_PCM_PLAYER

	struct AudioEngine;
	static AudioEngine* fx_share_engine = nullptr;

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
			if ( ! fx_share_engine ) {
				fx_share_engine = new AudioEngine();
			}
			return fx_share_engine;
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
			, bqPlayerObject(nullptr)
			, bqPlayerPlay(nullptr)
			, bqPlayerBufferQueue(nullptr)
			, bqPlayerEffectSend(nullptr)
			, bqPlayerVolume(nullptr)
			, _buffer_size(0)
		{
		}

		virtual ~AndroidPCMOpenSLES() {
			// destory player object
			if (bqPlayerObject != nullptr) {
				(*bqPlayerObject)->Destroy(bqPlayerObject);
				bqPlayerPlay = nullptr;
				bqPlayerBufferQueue = nullptr;
				bqPlayerEffectSend = nullptr;
				bqPlayerVolume = nullptr;
			}
		}

		bool initialize(const Stream &stream) {
			AudioEngine* engine = AudioEngine::share();
			if ( ! engine ) {
				return false;
			}

			// stream.channel_layout
			_channel_count = stream.channels;
			_sample_rate = stream.sample_rate;
			_buffer_size = min_buffer_size();

			SLresult result;
			SLuint32 channelMask;
			SLuint32 samplesPerSec;

			switch (channel_count) {
				case 1: // 1
					channelMask = SL_SPEAKER_FRONT_CENTER; break;
				case 2: // 2
					channelMask = SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT; break;
				case 3: // 2.1
					channelMask = SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT |
												SL_SPEAKER_LOW_FREQUENCY; break;
				case 4: // 4
					channelMask = SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT |
												SL_SPEAKER_BACK_LEFT | SL_SPEAKER_BACK_RIGHT; break;
				case 5: // 4.1
					channelMask = SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT | SL_SPEAKER_LOW_FREQUENCY |
												SL_SPEAKER_BACK_LEFT | SL_SPEAKER_BACK_RIGHT; break;
				case 6: // 5.1
					channelMask = SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT | SL_SPEAKER_FRONT_CENTER |
												SL_SPEAKER_LOW_FREQUENCY |
												SL_SPEAKER_BACK_LEFT | SL_SPEAKER_BACK_LEFT; break;
				case 7: // 6.1
					channelMask = SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT | SL_SPEAKER_FRONT_CENTER |
												SL_SPEAKER_LOW_FREQUENCY |
												SL_SPEAKER_SIDE_LEFT | SL_SPEAKER_SIDE_RIGHT |
												SL_SPEAKER_BACK_CENTER; break;
				case 8: // 7.1
					channelMask = SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT | SL_SPEAKER_FRONT_CENTER |
												SL_SPEAKER_LOW_FREQUENCY |
												SL_SPEAKER_SIDE_LEFT | SL_SPEAKER_SIDE_RIGHT |
												SL_SPEAKER_BACK_LEFT | SL_SPEAKER_BACK_RIGHT; break;
				default: return false;
			}

			switch (sample_rate) {
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
																			channel_count,
																			samplesPerSec,
																			SL_PCMSAMPLEFORMAT_FIXED_16,
																			SL_PCMSAMPLEFORMAT_FIXED_16,
																			channelMask,
																			SL_BYTEORDER_LITTLEENDIAN };
			SLDataSource audioSrc = { &loc_bufq, &format_pcm };

			// configure audio sink
			SLDataLocator_OutputMix loc_outmix  = { SL_DATALOCATOR_OUTPUTMIX, engine->outputMixObject };
			SLDataSink              audioSnk    = { &loc_outmix, nullptr };

			// create audio player
			const SLInterfaceID ids[3]  = { SL_IID_BUFFERQUEUE, SL_IID_EFFECTSEND, SL_IID_VOLUME };
			const SLboolean     req[3]  = { SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE };

			result = (*engine->engineEngine)->CreateAudioPlayer(engine->engineEngine,
																													&bqPlayerObject,
																													&audioSrc, &audioSnk, 3, ids, req);
			Qk_ASSERT(SL_RESULT_SUCCESS == result);

			// realize the player
			result = (*bqPlayerObject)->Realize(bqPlayerObject, SL_BOOLEAN_FALSE);
			Qk_ASSERT(SL_RESULT_SUCCESS == result);

			// get the play interface
			result = (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_PLAY, &bqPlayerPlay);
			Qk_ASSERT(SL_RESULT_SUCCESS == result);

			// get the buffer queue interface
			result = (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_BUFFERQUEUE, &bqPlayerBufferQueue);
			Qk_ASSERT(SL_RESULT_SUCCESS == result);

			// get the effect send interface
			result = (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_EFFECTSEND, &bqPlayerEffectSend);
			Qk_ASSERT(SL_RESULT_SUCCESS == result);

			// get the volume interface
			result = (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_VOLUME, &bqPlayerVolume);
			Qk_ASSERT(SL_RESULT_SUCCESS == result);

			// get max volume level
			result = (*bqPlayerVolume)->GetMaxVolumeLevel(bqPlayerVolume, &_max_volume_level);
			Qk_ASSERT(SL_RESULT_SUCCESS == result);

			// set playing status
			result = (*bqPlayerPlay)->SetPlayState(bqPlayerPlay, SL_PLAYSTATE_PLAYING);
			Qk_ASSERT(SL_RESULT_SUCCESS == result);

			Qk_DLog("createAudioPlayer finish");

			return true;
		}

		virtual bool write(const Frame *frame) {
			SLresult result;
			// ScopeLock scope(_lock);
			// // input pcm buffer
			// result = (*bqPlayerBufferQueue)->Enqueue(bqPlayerBufferQueue, *buffer, buffer.length());
			// return result != SL_RESULT_BUFFER_INSUFFICIENT;
			return true;
		}

		virtual void flush() {
			SLresult result;
			// lock
			ScopeLock scope(_lock);
			// clear buffer
			result = (*bqPlayerBufferQueue)->Clear(bqPlayerBufferQueue);
			Qk_ASSERT(SL_RESULT_SUCCESS == result);
		}

		virtual void set_mute(bool value) {
			SLresult result;
			result = (*bqPlayerVolume)->SetMute(bqPlayerVolume, value);
			// return SL_RESULT_SUCCESS == result;
		}

		virtual void set_volume(float value) {
			if ( _max_volume_level ) {
				SLresult result;
				result = (*bqPlayerVolume)->SetVolumeLevel(bqPlayerVolume, value / 100 * _max_volume_level);
				// return SL_RESULT_SUCCESS == result;
			}
		}

		virtual float delayed() {
			return -1.0;
		}

		uint32_t min_buffer_size() {
			JNI::ScopeENV env;
			JNI::MethodInfo m("android/media/AudioTrack", "getMinBufferSize", "(III)I", true);
			int r = env->CallStaticIntMethod(m.clazz(), m.method(), _sample_rate,
																			get_channel_mask(_channel_count), 2/*ENCODIF_PCM_16BIT*/);
			return r;
		}

	private:
		Mutex         _lock;
		SLint16       _max_volume_level;
		uint32_t      _sample_rate;
		uint32_t      _channel_count;
		uint32_t      _buffer_size;

		// engine interfaces
		SLObjectItf engineObject;
		SLEngineItf engineEngine;

		// output mix interfaces
		SLObjectItf outputMixObject;

		// buffer queue player interfaces
		SLObjectItf bqPlayerObject;
		SLPlayItf bqPlayerPlay;
		SLAndroidSimpleBufferQueueItf bqPlayerBufferQueue;
		SLEffectSendItf bqPlayerEffectSend;
		SLVolumeItf bqPlayerVolume;
	};

#endif

	typedef MediaSource::Stream SStream;

	PCMPlayer* create_android_audio_track(const SStream &stream);

	PCMPlayer* PCMPlayer::create(const SStream &stream) {
#if USE_ANDROID_OPENSLES_PCM_PLAYER
		Sp<AndroidPCMOpenSLES> player = new AndroidPCMOpenSLES();
		if ( player->initialize(stream) ) {
			return player.collapse();
		} else {
			return create_android_audio_track(stream);
		}
#else
		return create_android_audio_track(stream);
#endif
	}

}
