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

	class LinuxPCMPlayer: public Object, public PCMPlayer {
	public:
		LinuxPCMPlayer()
			: _pcm(nullptr)
			, _hw_params(nullptr), _sw_params(nullptr)
			, _period_size(0), _periods(3)
			, _channels(0), _sample_rate(0), _channel_layout(0)
			, _delayed(0), _volume(1)
			, _mute(false)
		{
		}

		~LinuxPCMPlayer() override {
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

		bool init(const Stream& stream) {
			#define CHECK() if (r < 0) return false

			int r, dir = 0;
			_channels = stream.channels;
			_sample_rate = stream.sample_rate;
			_channel_layout = stream.channel_layout;
			_period_size = (32.0f / 1000.0f) * _sample_rate; // 32ms

			r = snd_pcm_open(&_pcm, "default", SND_PCM_STREAM_PLAYBACK, SND_PCM_NONBLOCK); CHECK();
			// hardware params
			r = snd_pcm_hw_params_malloc(&_hw_params); CHECK();
			r = snd_pcm_hw_params_any(_pcm, _hw_params); CHECK();
			r = snd_pcm_hw_params_set_access(_pcm, _hw_params, SND_PCM_ACCESS_RW_INTERLEAVED); CHECK();
			r = snd_pcm_hw_params_set_format(_pcm, _hw_params, SND_PCM_FORMAT_S16); CHECK();
			r = snd_pcm_hw_params_set_channels(_pcm, _hw_params, _channels); CHECK();
			r = snd_pcm_hw_params_set_rate_near(_pcm, _hw_params, &_sample_rate, &dir); CHECK();
			Qk_DLog("snd_pcm_hw_params_set_rate_near,dir=%d for _sample_rate", dir);
			r = snd_pcm_hw_params_set_periods_near(_pcm, _hw_params, &_periods, &dir); CHECK();
			Qk_DLog("snd_pcm_hw_params_set_periods_near,dir=%d for _periods", dir);
			r = snd_pcm_hw_params_set_period_size_near(_pcm, _hw_params, &_period_size, &dir); CHECK();
			Qk_DLog("snd_pcm_hw_params_set_period_size_near,dir=%d for _period_size", dir);
			// send hw params to device
			r = snd_pcm_hw_params(_pcm, _hw_params); CHECK();

			// software params
			r = snd_pcm_sw_params_malloc(&_sw_params); CHECK();
			r = snd_pcm_sw_params_current(_pcm, _sw_params); CHECK();
			r = snd_pcm_sw_params_set_tstamp_type(_pcm, _sw_params, SND_PCM_TSTAMP_TYPE_MONOTONIC_RAW); CHECK();
			r = snd_pcm_sw_params_set_start_threshold(_pcm, _sw_params, _period_size * 2); CHECK();
			r = snd_pcm_sw_params_set_stop_threshold(_pcm, _sw_params, _period_size); CHECK();
			r = snd_pcm_sw_params_set_avail_min(_pcm, _sw_params, _period_size); CHECK();
			// send sw params
			r = snd_pcm_sw_params( _pcm, _sw_params); CHECK();

			// pcm handle prepare
			r = snd_pcm_prepare(_pcm); CHECK();

			#undef CHECK
			return true;
		}

		Object* asObject() override {
			return this;
		}

		bool write(const Frame *frame) override {
			int r;
			snd_pcm_uframes_t frames; // frame count
			uint32_t s16_len = frame->linesize[0] >> 1;
			int16_t* s16_bf = reinterpret_cast<int16_t*>(frame->data[0]);

			if (_delayed == 0) {
				_delayed = float(_period_size) / frame->nb_samples;
			}

			// Qk_DLog("_delayed: %f, Frame: %d, %ld", _delayed, s16_len, time_monotonic());

			if (_mute) {
				for (int i = 0; i < s16_len; i++)
					*s16_bf++ = 0;
			} else if (_volume < 1.0) { // set s16 pcm buffer volume
				for (int i = 0; i < s16_len; i++) {
					*s16_bf++ = (*s16_bf) * _volume;
					s16_bf++;
				}
			}

			// frame->linesize[0] / 16(采样位数) / 8 * 声道数
			frames = frame->linesize[0] / (16 / 8 * _channels);
			r = snd_pcm_writei(_pcm, frame->data[0], frames);
			if (r < 0) {
				r = snd_pcm_recover(_pcm, r, 0);
				return false;
			}
			return true;
		}

		float delayed() override {
			return _delayed;
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

	private:
		snd_pcm_t* _pcm;
		snd_pcm_hw_params_t* _hw_params;
		snd_pcm_sw_params_t* _sw_params;
		snd_pcm_uframes_t _period_size;
		uint32_t        _periods;
		uint32_t        _channels, _sample_rate;
		uint64_t        _channel_layout;
		float           _volume;
		float           _delayed;
		bool            _mute;
	};

	PCMPlayer* PCMPlayer::create(const Stream &stream) {
		Sp<LinuxPCMPlayer> player = new LinuxPCMPlayer();
		if ( player->init(stream) ) {
			return player.collapse();
		}
		return nullptr;
	}

}
