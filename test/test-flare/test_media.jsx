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

import { P, M, LOG, VM, VP } from './test'
import { Application, Root } from 'flare';
import 'flare/media';

new Application().start(<Root/>).onLoad = function() {

	F_LOG('\nTest Audio:\n')

	P(media, 'MEDIA_TYPE_AUDIO');
	P(media, 'MEDIA_TYPE_VIDEO');
	P(media, 'PLAYER_STATUS_STOP');
	P(media, 'PLAYER_STATUS_START');
	P(media, 'PLAYER_STATUS_PLAYING');
	P(media, 'PLAYER_STATUS_PAUSED');
	P(media, 'MULTIMEDIA_SOURCE_STATUS_UNINITIALIZED');
	P(media, 'MULTIMEDIA_SOURCE_STATUS_READYING');
	P(media, 'MULTIMEDIA_SOURCE_STATUS_READY');
	P(media, 'MULTIMEDIA_SOURCE_STATUS_WAIT');
	P(media, 'MULTIMEDIA_SOURCE_STATUS_FAULT');
	P(media, 'MULTIMEDIA_SOURCE_STATUS_EOF');
	P(media, 'CH_INVALID');
	P(media, 'CH_FRONT_LEFT');
	P(media, 'CH_FRONT_RIGHT');
	P(media, 'CH_FRONT_CENTER');
	P(media, 'CH_LOW_FREQUENCY');
	P(media, 'CH_BACK_LEFT');
	P(media, 'CH_BACK_RIGHT');
	P(media, 'CH_FRONT_LEFT_OF_CENTER');
	P(media, 'CH_FRONT_RIGHT_OF_CENTER');
	P(media, 'CH_BACK_CENTER');
	P(media, 'CH_SIDE_LEFT');
	P(media, 'CH_SIDE_RIGHT');
	P(media, 'CH_TOP_CENTER');
	P(media, 'CH_TOP_FRONT_LEFT');
	P(media, 'CH_TOP_FRONT_CENTER');
	P(media, 'CH_TOP_FRONT_RIGHT');
	P(media, 'CH_TOP_BACK_LEFT');
	P(media, 'CH_TOP_BACK_CENTER');
	P(media, 'CH_TOP_BACK_RIGHT');
	P(media, 'VIDEO_COLOR_FORMAT_INVALID');
	P(media, 'VIDEO_COLOR_FORMAT_YUV420P');
	P(media, 'VIDEO_COLOR_FORMAT_YUV420SP');
	P(media, 'VIDEO_COLOR_FORMAT_YUV411P');
	P(media, 'VIDEO_COLOR_FORMAT_YUV411SP');

	const audio = new media.AudioPlayer(resolve('all_we_know.mp3'))

	P(audio, 'autoPlay');
	P(audio, 'sourceStatus');
	P(audio, 'status');
	P(audio, 'mute')
	P(audio, 'volume')
	P(audio, 'src')
	P(audio, 'time')
	P(audio, 'duration')
	P(audio, 'trackIndex');
	P(audio, 'trackCount');
	P(audio, 'disableWaitBuffer')
	M(audio, 'selectTrack', [0]);
	M(audio, 'track', [0]);
	M(audio, 'start');
	M(audio, 'seek', [0]);
	M(audio, 'pause');
	M(audio, 'resume');
	//M(audio, 'stop');

	M(audio.onWaitBuffer, 'on', [function(){ LOG('---- audio onwait_buffer') }]);
	M(audio.onReady, 'on', [function(){ LOG('---- audio onready') }])
	M(audio.onStartPlay, 'on', [function(){ LOG('---- audio onstart_play') }])
	M(audio.onerror, 'on', [function(){ LOG('---- audio onerror') }])
	M(audio.onSourceEOF, 'on', [function(){ LOG('---- audio onsource_eof') }])
	M(audio.onPause, 'on', [function(){ LOG('---- audio onpause') }])
	M(audio.onResume, 'on', [function(){ LOG('---- audio onresume') }])
	M(audio.onStop, 'on', [function(){ LOG('---- audio onstop') }])
	M(audio.onSeek, 'on', [function(){ LOG('---- audio onseek') }])
	

	const video = new media.Video();
	M(video, 'appendTo', [this.root])
	P(video, 'width', 'full');
	P(video, 'margin', 'auto');
	P(video, 'src', 'http://192.168.1.100:1026/demo/examples/piper720p.mp4');
	//P(video, 'src', 'http://192.168.1.100:1026/test/flame-piper.2016.1080p.bluray.x264.mkv');
	
	M(video, 'appendText', ["Play Video"])

	P(video, 'autoPlay');
	P(video, 'sourceStatus');
	P(video, 'status');
	P(video, 'mute')
	P(video, 'volume')
	P(video, 'src')
	P(video, 'time')
	P(video, 'duration')
	P(video, 'trackIndex');
	P(video, 'trackCount');
	P(video, 'disableWaitBuffer')
	M(video, 'selectAudioTrack', [0]);
	M(video, 'audioTrack', [0]);
	M(video, 'videoTrack');
	M(video, 'start');
	M(video, 'seek', [0]);
	M(video, 'pause');
	M(video, 'resume');
	//M(video, 'stop');

	M(video.onWaitBuffer, 'on', [function(){ LOG('---- video onwait_buffer') }]);
	M(video.onReady, 'on', [function(){ LOG('---- video onready') }])
	M(video.onStartPlay, 'on', [function(){ LOG('---- video onstart_play') }])
	M(video.onError, 'on', [function(){ LOG('---- video onerror') }])
	M(video.onSourceEOF, 'on', [function(){ LOG('---- video onsource_eof') }])
	M(video.onPause, 'on', [function(){ LOG('---- video onpause') }])
	M(video.onResume, 'on', [function(){ LOG('---- video onresume') }])
	M(video.onStop, 'on', [function(){ LOG('---- video onstop') }])
	M(video.onSeek, 'on', [function(){ LOG('---- video onseek') }])

};