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

#if defined(__linux__) and !defined(__ANDROID__)

#include <alsa/asoundlib.h>

static char *device = "default";    /*default playback device */

int test2_alsa(int argc, char *argv[])
{
	int err, numSamples;
	snd_pcm_t *handle;
	snd_pcm_sframes_t frames;
	numSamples = atoi(argv[1]);
	char* samples = (char*) malloc((size_t) numSamples);
	FILE *inFile = fopen(argv[2], "rb");
	int numRead = fread(samples, 1, numSamples, inFile);
	fclose(inFile);

	if ((err=snd_pcm_open(&handle, device, SND_PCM_STREAM_PLAYBACK, 0)) < 0){
		printf("Playback open error: %s\n", snd_strerror(err));
		exit(EXIT_FAILURE);
	}

	if ((err =
			snd_pcm_set_params(handle,SND_PCM_FORMAT_U8,
			SND_PCM_ACCESS_RW_INTERLEAVED, 1, 44100, 1, 100000) ) < 0 )
	{
		printf("Playback open error: %s\n", snd_strerror(err));
		exit(EXIT_FAILURE);
	}

	printf("snd_pcm_writei\n");
	frames = snd_pcm_writei(handle, samples, numSamples);
	printf("snd_pcm_writei ok\n");

	if (frames < 0) {
		printf("snd_pcm_recover\n");
		frames = snd_pcm_recover(handle, frames, 0);
		printf("snd_pcm_recover ok\n");
	}

	if (frames < 0) {
		printf("snd_pcm_writei failed: %s\n", snd_strerror(err));
	}

	snd_pcm_close(handle);
	return 0;
}

#else
int test2_alsa(int argc, char *argv[]){}
#endif
