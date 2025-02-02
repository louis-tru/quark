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

#include <stdlib.h>
#include <stdio.h>
#include <alsa/asoundlib.h>
#include "../../media/pcm_player.h"
#include "../../util/handle.h"
#include "../../util/string.h"

namespace qk {
	#define DEFAULT_PCM_PERIOD_SIZE 4096
	#define DEFAULT_PCM_PERIODS 3

	class LinuxPCMPlayer: public Object, public PCMPlayer {
	public:

		LinuxPCMPlayer()
			: _pcm(nullptr)
			, _hw_params(nullptr), _sw_params(nullptr), _mixer(nullptr)
			, _period_size(DEFAULT_PCM_PERIOD_SIZE)
			, _periods(DEFAULT_PCM_PERIODS)
			, _channel_count(0), _sample_rate(0)
			, _volume(1)
			, _mute(false)
		{
#if Qk_DEBUG
			char* PCM_PERIODS = getenv("PCM_PERIODS");
			char* PCM_PERIOD_SIZE = getenv("PCM_PERIOD_SIZE");
			if (PCM_PERIODS) {
				_period_size = String(PCM_PERIODS).toNumber<uint32_t>();
			}
			if (PCM_PERIOD_SIZE) {
				_periods = String(PCM_PERIOD_SIZE).toNumber<uint32_t>();
			}
#endif
		}

		bool init(const Stream& stream) {
			#define CHECK() if (r < 0) return false

			int r, dir = 0;
			_channel_count = stream.channels;
			_sample_rate = stream.sample_rate;

			r = snd_pcm_open(&_pcm, "default", SND_PCM_STREAM_PLAYBACK, SND_PCM_NONBLOCK); CHECK();
			// hardware params
			r = snd_pcm_hw_params_malloc(&_hw_params); CHECK();
			r = snd_pcm_hw_params_any(_pcm, _hw_params); CHECK();
			r = snd_pcm_hw_params_set_access(_pcm, _hw_params, SND_PCM_ACCESS_RW_INTERLEAVED); CHECK();
			r = snd_pcm_hw_params_set_format(_pcm, _hw_params, SND_PCM_FORMAT_S16); CHECK();
			r = snd_pcm_hw_params_set_channels(_pcm, _hw_params, _channel_count); CHECK();
			r = snd_pcm_hw_params_set_rate_near(_pcm, _hw_params, &_sample_rate, &dir); CHECK();
			r = snd_pcm_hw_params_set_period_size_near(_pcm, _hw_params, &_period_size, &dir); CHECK();
			r = snd_pcm_hw_params_set_periods_near(_pcm, _hw_params, &_periods, &dir); CHECK();
			// send params to device
			r = snd_pcm_hw_params(_pcm, _hw_params); CHECK();
			//
			//software params
			r = snd_pcm_sw_params_malloc(&_sw_params); CHECK();
			r = snd_pcm_sw_params_current(_pcm, _sw_params); CHECK();
			r = snd_pcm_sw_params_set_tstamp_type(_pcm, 
				_sw_params, SND_PCM_TSTAMP_TYPE_MONOTONIC_RAW); CHECK();
			r = snd_pcm_sw_params_set_start_threshold(_pcm, _sw_params, _period_size * 2); CHECK();
			r = snd_pcm_sw_params_set_stop_threshold(_pcm, _sw_params, _period_size); CHECK();
			r = snd_pcm_sw_params_set_avail_min(_pcm, _sw_params, _period_size); CHECK();
			// send sw params
			r = snd_pcm_sw_params( _pcm, _sw_params); CHECK();

			// pcm handle prepare
			r = snd_pcm_prepare(_pcm); CHECK();

			return true;
		}

		~LinuxPCMPlayer() override {
			if (_mixer) {
				snd_mixer_close(_mixer); _mixer = nullptr;
			}
			if (_hw_params) {
				snd_pcm_hw_params_free(_hw_params); _hw_params = nullptr;
			}
			if (_sw_params) {
				snd_pcm_sw_params_free(_sw_params); _sw_params = nullptr;
			}
			if (_pcm) {
				snd_pcm_close(_pcm); _pcm = nullptr;
			}

			Qk_DLog("~LinuxPCMPlayer");
		}

		Object* asObject() override {
			return this;
		}

		bool write(const Frame *frame) override {
			// int r;
			// snd_pcm_uframes_t frames;
			// int s16_len = buffer.length() / 2;
			// int16_t* s16_bf = (int16_t*)*buffer;

			// if (_mute) {
			// 	for (int i = 0; i < s16_len; i++)
			// 		*s16_bf++ = 0;
			// } else if (_volume < 1.0) { // set s16 pcm buffer volume
			// 	for (int i = 0; i < s16_len; i++) {
			// 		*s16_bf++ = (*s16_bf) * _volume;
			// 		s16_bf++;
			// 	}
			// }

			// /* buffer.len / 16(采样位数) / 8 * 声道数 */
			// frames = buffer.length() / (16 / 8 * _channel_count);
			// r = snd_pcm_writei(_pcm, *buffer, frames);
			// if (r < 0) {
			// 	Qk_DLog("snd_pcm_writei err,%d", r);
			// 	r = snd_pcm_recover(_pcm, r, 0);
			// 	Qk_DLog("snd_pcm_recover ok,%d", r);
			// 	return 0;
			// }
			return true;
		}

		float delay() override {
			return 1.0f;
		}

		void flush() override {
			snd_pcm_reset(_pcm);
			snd_pcm_prepare(_pcm);
		}

		void set_mute(bool value) override {
			_mute = value;
		}

		void set_volume(float fvalue) override {
			fvalue = Qk_Min(1.0, fvalue);
			if (fvalue != _volume) {
				_volume = fvalue;
			}
		}

		virtual uint32_t buffer_size() {
			return _period_size;
		}

	private:
		snd_pcm_t* _pcm;
		snd_pcm_hw_params_t* _hw_params;
		snd_pcm_sw_params_t* _sw_params;
		snd_mixer_t* _mixer;
		snd_pcm_uframes_t _period_size;
		uint32_t _periods, _channel_count, _sample_rate;
		float _volume;
		bool _mute;
	};

	PCMPlayer* PCMPlayer::create(const Stream &stream) {
		Sp<LinuxPCMPlayer> player = new LinuxPCMPlayer();
		if ( player->init(stream) ) {
			return player.collapse();
		}
		return nullptr;
	}

}
