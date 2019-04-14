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

#include "media.h"
#include <uv.h>

XX_NS(qgr)

typedef PCMPlayer* (*create_pcm_player_f)(uint channel_count, uint sample_rate);
typedef AudioPlayer* (*create_audio_player_f)(cString& uri);
typedef Video* (*create_video_f)();

XX_EXPORT create_pcm_player_f __create_pcm_player_f = nullptr;
XX_EXPORT create_audio_player_f __create_audio_player_f = nullptr;
XX_EXPORT create_video_f __create_video_f = nullptr;

static int is_loaded_lib() {
	return __create_pcm_player_f && __create_audio_player_f && __create_video_f;
}

int initialize_media() {

	if (is_loaded_lib()) {
		return 1;
	}
	// try loading qgr-node
	uv_lib_t lib;
	int err = uv_dlopen("libqgr-media.so", &lib);
	if (err != 0) {
		XX_WARN("No qgr-media library loaded, %s", uv_dlerror(&lib));
	} else {
		if (is_loaded_lib()) {
			return 1;
		}
		XX_WARN("No qgr-media library loaded");
	}

	return 0;
}

PCMPlayer* create_pcm_player(uint channel_count, uint sample_rate) {
	return __create_pcm_player_f(channel_count, sample_rate);
}

AudioPlayer* create_audio_player(cString& uri) {
	return __create_audio_player_f(uri);
}

Video* create_video() {
	return __create_video_f();
}

XX_END
