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
#include "ftr/util/string.h"
#include <alsa/asoundlib.h>
#include <stdlib.h>
#include <stdio.h>

FX_NS(ftr)

#define DEFAULT_PCM_PERIOD_SIZE 4096
#define DEFAULT_PCM_PERIODS 3

/**
 * @class LinuxPCMPlayer
 */
class LinuxPCMPlayer: public Object, public PCMPlayer {
 public:
	typedef ObjectTraits Traits;

	virtual Object* to_object() { return this; }

	LinuxPCMPlayer()
		: m_pcm(NULL)
		, m_hw_params(NULL), m_sw_params(NULL), m_mixer(NULL)
		, m_period_size(DEFAULT_PCM_PERIOD_SIZE)
		, m_periods(DEFAULT_PCM_PERIODS)
		, m_channel_count(0), m_sample_rate(0)
		, m_volume(1)
		, m_mute(false)
	{
#if DEBUG
		char* PCM_PERIODS = getenv("PCM_PERIODS");
		char* PCM_PERIOD_SIZE = getenv("PCM_PERIOD_SIZE");
		if (PCM_PERIODS) {
			m_period_size = String(PCM_PERIODS).to_uint();
		}
		if (PCM_PERIOD_SIZE) {
			m_periods = String(PCM_PERIOD_SIZE).to_uint();
		}
#endif
	}

	virtual ~LinuxPCMPlayer() {
		if (m_mixer) {
			snd_mixer_close(m_mixer); m_mixer = NULL;
		}
		if (m_hw_params) {
			snd_pcm_hw_params_free(m_hw_params); m_hw_params = NULL;
		}
		if (m_sw_params) {
			snd_pcm_sw_params_free(m_sw_params); m_sw_params = NULL;
		}
		if (m_pcm) {
			snd_pcm_close(m_pcm); m_pcm = NULL;
		}

		DLOG("~LinuxPCMPlayer");
	}

	bool initialize(uint channel_count, uint sample_rate) {
	 #define CHECK() if (r < 0) return 0
		int r, dir = 0;
		m_channel_count = channel_count;
		m_sample_rate = sample_rate;

		r = snd_pcm_open(&m_pcm, "default",
			SND_PCM_STREAM_PLAYBACK, SND_PCM_NONBLOCK); CHECK();
		// hardware params
		r = snd_pcm_hw_params_malloc(&m_hw_params); CHECK();
		r = snd_pcm_hw_params_any(m_pcm, m_hw_params); CHECK();
		r = snd_pcm_hw_params_set_access(m_pcm, m_hw_params, SND_PCM_ACCESS_RW_INTERLEAVED); CHECK();
		r = snd_pcm_hw_params_set_format(m_pcm, m_hw_params, SND_PCM_FORMAT_S16); CHECK();
		r = snd_pcm_hw_params_set_channels(m_pcm, m_hw_params, channel_count); CHECK();
		r = snd_pcm_hw_params_set_rate_near(m_pcm, m_hw_params, &sample_rate, &dir); CHECK();
		r = snd_pcm_hw_params_set_period_size_near(m_pcm, m_hw_params, &m_period_size, &dir); CHECK();
		r = snd_pcm_hw_params_set_periods_near(m_pcm, m_hw_params, &m_periods, &dir); CHECK();
		// send params to device
		r = snd_pcm_hw_params(m_pcm, m_hw_params); CHECK();
		//
		//software params
		r = snd_pcm_sw_params_malloc(&m_sw_params); CHECK();
		r = snd_pcm_sw_params_current(m_pcm, m_sw_params); CHECK();
		r = snd_pcm_sw_params_set_tstamp_type(m_pcm, 
			m_sw_params, SND_PCM_TSTAMP_TYPE_MONOTONIC_RAW); CHECK();
		r = snd_pcm_sw_params_set_start_threshold(m_pcm, m_sw_params, m_period_size * 2); CHECK();
		r = snd_pcm_sw_params_set_stop_threshold(m_pcm, m_sw_params, m_period_size); CHECK();
		r = snd_pcm_sw_params_set_avail_min(m_pcm, m_sw_params, m_period_size); CHECK();
		// send sw params
		r = snd_pcm_sw_params( m_pcm, m_sw_params); CHECK();

		// pcm handle prepare
		r = snd_pcm_prepare(m_pcm); CHECK();

		return true;
	}

	virtual bool write(cBuffer& buffer) {
		int r;
		snd_pcm_uframes_t frames;
		int s16_len = buffer.length() / 2;
		int16* s16_bf = (int16*)*buffer;

		// set s16 pcm buffer volume
		if (m_volume < 1.0) {
			for (int i = 0; i < s16_len; i++) {
				*s16_bf = (*s16_bf) * m_volume;
				s16_bf++;
			}
		}

		/* buffer.len / 16(采样位数) / 8 * 声道数 */
		frames = buffer.length() / (16 / 8 * m_channel_count);
		r = snd_pcm_writei(m_pcm, *buffer, frames);
		if (r < 0) {
			DLOG("snd_pcm_writei err,%d", r);
			r = snd_pcm_recover(m_pcm, r, 0);
			DLOG("snd_pcm_recover ok,%d", r);
			return 0;
		}
		return true;
	}

	virtual float compensate() {
		return -1.0f;
	}

	virtual void flush() {
		snd_pcm_reset(m_pcm);
		snd_pcm_prepare(m_pcm);
	}

	virtual bool set_mute(bool value) {
		if (value != m_mute) {
			if (!set_volume2(value ? 0: m_volume)) {
				return false;
			}
			m_mute = value;
		}
		return true;
	}

	inline bool set_volume2(float value) {
		m_volume = value;
		return 1;
	}

	virtual bool set_volume(uint value) {
		float fvalue = value / 100.0;
		fvalue = FX_MIN(1.0, fvalue);
		if (fvalue != m_volume || m_mute) {
			if (!set_volume2(fvalue)) {
				return false;
			}
			if (fvalue) {
				m_mute = false;
			}
			m_volume = fvalue;
		}
		return true;
	}

	virtual uint buffer_size() {
		return m_period_size;
	}

 private:

	snd_pcm_t* m_pcm;
	snd_pcm_hw_params_t* m_hw_params;
	snd_pcm_sw_params_t* m_sw_params;
	snd_mixer_t* m_mixer;
	snd_pcm_uframes_t m_period_size;
	uint m_periods, m_channel_count, m_sample_rate;
	float m_volume;
	bool m_mute;
};

/**
 * @func create
 */
PCMPlayer* PCMPlayer::create(uint channel_count, uint sample_rate) {
	Handle<LinuxPCMPlayer> player = new LinuxPCMPlayer();
	if ( player->initialize(channel_count, sample_rate) ) {
		return player.collapse();
	}
	return NULL;
}

FX_END
