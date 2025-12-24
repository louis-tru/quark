/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright (c) 2015, Louis.chu
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of Louis.chu nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL Louis.chu BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * ***** END LICENSE BLOCK ***** */

import { LOG, Mv, Pv } from './tool'
import { Window, Video } from 'quark'
import { AudioPlayer, MediaType, MediaSourceStatus } from 'quark/media'

const resolve = require.resolve

export default async function(win: Window) {
	LOG('\nTest Audio:\n')

	const audio = new AudioPlayer();
	const src = resolve('all_we_know.mp3')

	Pv(audio, 'pts', 0)
	Pv(audio, 'volume', 1)
	Pv(audio, 'mute', false)
	Pv(audio, 'isPause', false)
	Pv(audio, 'type', MediaType.kAudio)
	Pv(audio, 'duration', e=>true)
	Pv(audio, 'status', MediaSourceStatus.kNormal)
	Pv(audio, 'src', src, e=>e.src=src)
	Pv(audio, 'video', null)
	Pv(audio, 'audio', e=>e!.type==MediaType.kAudio)
	Pv(audio, 'audioStreams', 1)
	Mv(audio, 'play', [])
	Mv(audio, 'seek', [1e5])
	Mv(audio, 'pause', [])
	Mv(audio, 'stop', [])
	Mv(audio, 'play', [])
	Mv(audio, 'switchAudio', [0])
	Mv(audio.onLoad, 'on', [function(){ LOG('---- audio onLoad') }])
	Mv(audio.onStop, 'on', [function(){ LOG('---- audio onStop') }])
	Mv(audio.onError, 'on', [function(){ LOG('---- audio onError') }])
	Mv(audio.onBuffering, 'on', [function(){ LOG('---- audio onBuffering') }])

	const video = new Video(win)
	Pv(video, 'pts', 0)
	Mv(video, 'appendTo', [win.root])
	Pv(video, 'width', e=>true, e=>e.style.width='match');
	Pv(video, 'src', e=>true, e=>e.src='http://192.168.1.100:1026/demo/examples/piper720p.mp4');
	Pv(video, 'volume', 1)
	Pv(video, 'mute', false)
	Pv(video, 'isPause', false)
	Pv(video, 'type', MediaType.kAudio)
	Pv(video, 'duration', e=>true)
	Pv(video, 'status', MediaSourceStatus.kNormal)
	Pv(video, 'src', src, e=>e.src=src)
	Pv(video, 'video', e=>e!.type==MediaType.kVideo)
	Pv(video, 'audio', e=>e!.type==MediaType.kAudio)
	Pv(video, 'audioStreams', 1)
	Mv(video, 'play', [])
	Mv(video, 'seek', [1e5])
	Mv(video, 'pause', [])
	Mv(video, 'stop', [])
	Mv(video, 'play', [])
	Mv(video, 'switchAudio', [0])
	Mv(video.onLoad, 'on', [function(){ LOG('---- video onLoad') }])
	Mv(video.onStop, 'on', [function(){ LOG('---- video onStop') }])
	Mv(video.onError, 'on', [function(){ LOG('---- video onError') }])
	Mv(video.onBuffering, 'on', [function(){ LOG('---- video onBuffering') }])
};