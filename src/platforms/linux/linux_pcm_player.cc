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

#include "quark/media/pcm_player.h"
#include "quark/util/handle.h"
#include "quark/util/string.h"
#include <alsa/asoundlib.h>
#include <stdlib.h>
#include <stdio.h>

namespace quark {

	#define DEFAULT_PCM_PERIOD_SIZE 4096
	#define DEFAULT_PCM_PERIODS 3

	/**
	* @class UnixPCMPlayer
	*/
	class UnixPCMPlayer: public Object, public PCMPlayer {
	public:
		typedef ObjectTraits Traits;

		virtual Object* to_object() { return this; }

		UnixPCMPlayer()
			: _pcm(NULL)
			, _hw_params(NULL), _sw_params(NULL), _mixer(NULL)
			, _period_size(DEFAULT_PCM_PERIOD_SIZE)
			, _periods(DEFAULT_PCM_PERIODS)
			, _channel_count(0), _sample_rate(0)
			, _volume(1)
			, _mute(false)
		{
			#if DEBUG
				Char* PCM_PERIODS = getenv("PCM_PERIODS");
				Char* PCM_PERIOD_SIZE = getenv("PCM_PERIOD_SIZE");
				if (PCM_PERIODS) {
					_period_size = String(PCM_PERIODS).to_uint();
				}
				if (PCM_PERIOD_SIZE) {
					_periods = String(PCM_PERIOD_SIZE).to_uint();
				}
			#endif
		}

		virtual ~UnixPCMPlayer() {
			if (_mixer) {
				snd_mixer_close(_mixer); _mixer = NULL;
			}
			if (_hw_params) {
				snd_pcm_hw_params_free(_hw_params); _hw_params = NULL;
			}
			if (_sw_params) {
				snd_pcm_sw_params_free(_sw_params); _sw_params = NULL;
			}
			if (_pcm) {
				snd_pcm_close(_pcm); _pcm = NULL;
			}

			Qk_DEBUG("~UnixPCMPlayer");
		}

		bool initialize(uint32_t channel_count, uint32_t sample_rate) {
		#define CHECK() if (r < 0) return 0
			int r, dir = 0;
			_channel_count = channel_count;
			_sample_rate = sample_rate;

			r = snd_pcm_open(&_pcm, "default",
				SND_PCM_STREAM_PLAYBACK, SND_PCM_NONBLOCK); CHECK();
			// hardware params
			r = snd_pcm_hw_params_malloc(&_hw_params); CHECK();
			r = snd_pcm_hw_params_any(_pcm, _hw_params); CHECK();
			r = snd_pcm_hw_params_set_access(_pcm, _hw_params, SND_PCM_ACCESS_RW_INTERLEAVED); CHECK();
			r = snd_pcm_hw_params_set_format(_pcm, _hw_params, SND_PCM_FORMAT_S16); CHECK();
			r = snd_pcm_hw_params_set_channels(_pcm, _hw_params, channel_count); CHECK();
			r = snd_pcm_hw_params_set_rate_near(_pcm, _hw_params, &sample_rate, &dir); CHECK();
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

		virtual bool write(cBuffer& buffer) {
			int r;
			snd_pcm_uframes_t frames;
			int s16_len = buffer.length() / 2;
			int16* s16_bf = (int16*)*buffer;

			// set s16 pcm buffer volume
			if (_volume < 1.0) {
				for (int i = 0; i < s16_len; i++) {
					*s16_bf = (*s16_bf) * _volume;
					s16_bf++;
				}
			}

			/* buffer.len / 16(采样位数) / 8 * 声道数 */
			frames = buffer.length() / (16 / 8 * _channel_count);
			r = snd_pcm_writei(_pcm, *buffer, frames);
			if (r < 0) {
				Qk_DEBUG("snd_pcm_writei err,%d", r);
				r = snd_pcm_recover(_pcm, r, 0);
				Qk_DEBUG("snd_pcm_recover ok,%d", r);
				return 0;
			}
			return true;
		}

		virtual float compensate() {
			return -1.0f;
		}

		virtual void flush() {
			snd_pcm_reset(_pcm);
			snd_pcm_prepare(_pcm);
		}

		virtual bool set_mute(bool value) {
			if (value != _mute) {
				if (!set_volume2(value ? 0: _volume)) {
					return false;
				}
				_mute = value;
			}
			return true;
		}

		inline bool set_volume2(float value) {
			_volume = value;
			return 1;
		}

		virtual bool set_volume(uint32_t value) {
			float fvalue = value / 100.0;
			fvalue = Qk_MIN(1.0, fvalue);
			if (fvalue != _volume || _mute) {
				if (!set_volume2(fvalue)) {
					return false;
				}
				if (fvalue) {
					_mute = false;
				}
				_volume = fvalue;
			}
			return true;
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

	/**
	* @func create
	*/
	PCMPlayer* PCMPlayer::create(uint32_t channel_count, uint32_t sample_rate) {
		Handle<UnixPCMPlayer> player = new UnixPCMPlayer();
		if ( player->initialize(channel_count, sample_rate) ) {
			return player.collapse();
		}
		return NULL;
	}

}
