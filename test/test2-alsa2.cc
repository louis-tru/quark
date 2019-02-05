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

#if defined(__linux__)

#include <alsa/asoundlib.h>

int test2_alsa2(int argc, char *argv[])
{
	int i;
	int ret;
	int buf[128];
	unsigned int val;
	int dir=0;
	char *buffer;
	int size;
	snd_pcm_uframes_t frames;
	snd_pcm_uframes_t periodsize;
	snd_pcm_t *playback_handle;//PCM设备句柄pcm.h
	snd_pcm_hw_params_t *hw_params;//硬件信息和PCM流配置

	if (argc != 2) {
		printf("error: alsa_play_test [music name]\n");
		exit(1);
	}
	printf("play song %s by wolf\n", argv[1]);
	FILE *fp = fopen(argv[1], "rb");
	if(fp == NULL)
		return 0;
	fseek(fp, 100, SEEK_SET);
	
	//1. 打开PCM，最后一个参数为0意味着标准配置
	ret = snd_pcm_open(&playback_handle, "default", SND_PCM_STREAM_PLAYBACK, 0);
	if (ret < 0) {
		perror("snd_pcm_open");
		exit(1);
	}
	
	//2. 分配snd_pcm_hw_params_t结构体
	ret = snd_pcm_hw_params_malloc(&hw_params);
	if (ret < 0) {
		perror("snd_pcm_hw_params_malloc");
		exit(1);
	}

	//3. 初始化hw_params
	ret = snd_pcm_hw_params_any(playback_handle, hw_params);
	if (ret < 0) {
		perror("snd_pcm_hw_params_any");
		exit(1);
	}

	//4. 初始化访问权限
	ret = snd_pcm_hw_params_set_access(playback_handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED);
	if (ret < 0) {
		perror("snd_pcm_hw_params_set_access");
		exit(1);
	}

	//5. 初始化采样格式SND_PCM_FORMAT_U8,8位
	ret = snd_pcm_hw_params_set_format(playback_handle, hw_params, SND_PCM_FORMAT_U8);
	if (ret < 0) {
		perror("snd_pcm_hw_params_set_format");
		exit(1);
	}

	//6. 设置采样率，如果硬件不支持我们设置的采样率，将使用最接近的
	//val = 44100,有些录音采样频率固定为8KHz
	
	val = 8000;
	ret = snd_pcm_hw_params_set_rate_near(playback_handle, hw_params, &val, &dir);
	if (ret < 0) {
		perror("snd_pcm_hw_params_set_rate_near");
		exit(1);
	}
	//7. 设置通道数量
	ret = snd_pcm_hw_params_set_channels(playback_handle, hw_params, 2);
	if (ret < 0) {
		perror("snd_pcm_hw_params_set_channels");
		exit(1);
	}
	
	/* Set period size to 32 frames. */
	frames = 32;
	periodsize = frames * 2;
	ret = snd_pcm_hw_params_set_buffer_size_near(playback_handle, hw_params, &periodsize);
	if (ret < 0)
	{
		printf("Unable to set buffer size %li : %s\n", frames * 2, snd_strerror(ret));
	}
	periodsize /= 2;

	ret = snd_pcm_hw_params_set_period_size_near(playback_handle, hw_params, &periodsize, 0);
	if (ret < 0)
	{
		printf("Unable to set period size %li : %s\n", periodsize,  snd_strerror(ret));
	}

	//8. 设置hw_params
	ret = snd_pcm_hw_params(playback_handle, hw_params);
	if (ret < 0) {
		perror("snd_pcm_hw_params");
		exit(1);
	}
	
	/* Use a buffer large enough to hold one period */
	snd_pcm_hw_params_get_period_size(hw_params, &frames, &dir);

	size = frames * 2; /* 2 bytes/sample, 2 channels */
	buffer = (char *) malloc(size);
	fprintf(stderr, "size = %d\n",size);
	
	while (1)
	{
		ret = fread(buffer, 1, size, fp);
		if (ret == 0)
		{
			fprintf(stderr, "end of file on input\n");
			break;
		}
		else if (ret != size) 
		{
		}

		//9. 写音频数据到PCM设备
		while(ret = snd_pcm_writei(playback_handle, buffer, frames)<0)
		{
			usleep(2000);
			if (ret == -EPIPE)
			{
				/* EPIPE means underrun */
				fprintf(stderr, "underrun occurred\n");
				//完成硬件参数设置，使设备准备好
				snd_pcm_prepare(playback_handle);
			}
			else if (ret < 0)
			{
				fprintf(stderr,
					"error from writei: %s\n",
					snd_strerror(ret));
			}
		}
		
	}

	//10. 关闭PCM设备句柄
	snd_pcm_close(playback_handle);
	
	return 0;
}
#else
int test2_alsa2(int argc, char *argv[]){}
#endif
