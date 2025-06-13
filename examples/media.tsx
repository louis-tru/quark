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

import { ViewController, _CVD, Video } from 'quark';
import { AudioPlayer } from 'quark/media';
import { Page } from './tool';
import { ClickEvent } from 'quark/event';

// const src_720 = 'http://quarks.cc/media/2017-09-11_15_41_19.mp4';
const src_720 = 'http://quarks.cc/media/piper720p.mp4';
const audio_src = 'http://quarks.cc/media/all_we_know.mp3';

const resolve = require.resolve;

var audio_player: AudioPlayer | null = null;

class Destroy extends ViewController {
	protected triggerDestroy() {
		if (audio_player)
			audio_player.stop();
	}
}

export default (self: Page)=>{

	function PlayVideo(evt: ClickEvent) {
		StopAudio(evt);
		var v = self.asRef<Video>('video');
		v.src = src_720;
		v.play();
	}

	function PlayAudio(evt: ClickEvent) {
		StopVideo(evt);
		if ( !audio_player ) {
			audio_player = new AudioPlayer();
		}
		audio_player.src = audio_src;
		audio_player.play();
	}

	function StopVideo(evt: ClickEvent) {
		self.asRef<Video>('video').stop();
	}

	function StopAudio(evt: ClickEvent) {
		if ( audio_player ) {
			audio_player.stop();
			audio_player = null;
		}
	}

	function Stop(evt: ClickEvent) {
		StopVideo(evt);
		StopAudio(evt);
	}

	function Seek(evt: ClickEvent) {
		if ( audio_player ) {
			audio_player.seek(10000); // 10s
		} else {
			//evt.sender.ownerAs().find<Video>('video').seek(100000); // 100s
			self.asRef<Video>('video').seek(100000); // 100s
		}
	}

	return (
		<box width="match">
			<Destroy />
			<button class="long_btn" onClick={PlayVideo}>Play Video</button>
			<button class="long_btn" onClick={PlayAudio}>Play Audio</button>
			<button class="long_btn" onClick={Stop}>Stop</button>

			<video
				ref="video"
				marginTop={10}
				// borderRadius={20}
				// border="8 #f00"
				// clip={false}
				width="match" 
				backgroundColor="#000"
			/>
		</box>
	);
}