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

import utils from './util';
import event, {
	EventNoticer, NativeNotification, Notification, Event, UIEvent,
} from './event';
import { Image } from './_view';

const _media = __binding__('_media');

Object.assign(exports, _media);

export enum MediaType {
	MEDIA_TYPE_AUDIO,
	MEDIA_TYPE_VIDEO,
}

export enum PlayerStatus {
	PLAYER_STATUS_STOP = 0,
	PLAYER_STATUS_START,
	PLAYER_STATUS_PLAYING,
	PLAYER_STATUS_PAUSED,
};

export enum MultimediaSourceStatus {
	MULTIMEDIA_SOURCE_STATUS_UNINITIALIZED = 0,
	MULTIMEDIA_SOURCE_STATUS_READYING,
	MULTIMEDIA_SOURCE_STATUS_READY,
	MULTIMEDIA_SOURCE_STATUS_WAIT,
	MULTIMEDIA_SOURCE_STATUS_FAULT,
	MULTIMEDIA_SOURCE_STATUS_EOF,
}

export enum VideoColorFormat {
	VIDEO_COLOR_FORMAT_YUV420P = 18,
	VIDEO_COLOR_FORMAT_YUV420SP = 19,
	VIDEO_COLOR_FORMAT_YUV411P = 20,
	VIDEO_COLOR_FORMAT_YUV411SP = 21,
	VIDEO_COLOR_FORMAT_INVALID = 200000
}

export enum AudioChannelMask {
	CH_INVALID                = 0,
	CH_FRONT_LEFT             = 0x00000001,
	CH_FRONT_RIGHT            = 0x00000002,
	CH_FRONT_CENTER           = 0x00000004,
	CH_LOW_FREQUENCY          = 0x00000008,
	CH_BACK_LEFT              = 0x00000010,
	CH_BACK_RIGHT             = 0x00000020,
	CH_FRONT_LEFT_OF_CENTER   = 0x00000040,
	CH_FRONT_RIGHT_OF_CENTER  = 0x00000080,
	CH_BACK_CENTER            = 0x00000100,
	CH_SIDE_LEFT              = 0x00000200,
	CH_SIDE_RIGHT             = 0x00000400,
	CH_TOP_CENTER             = 0x00000800,
	CH_TOP_FRONT_LEFT         = 0x00001000,
	CH_TOP_FRONT_CENTER       = 0x00002000,
	CH_TOP_FRONT_RIGHT        = 0x00004000,
	CH_TOP_BACK_LEFT          = 0x00008000,
	CH_TOP_BACK_CENTER        = 0x00010000,
	CH_TOP_BACK_RIGHT         = 0x00020000,
}

export interface TrackInfo {
	type: MediaType;
	mime: string;
	codecId: number;
	codecTag: number;
	format: number;
	profile: number;
	level: number;
	width: number;
	height: number;
	language: string;
	bitrate: number;
	sampleRate: number;
	channelCount: number;
	channelLayout: number;
	frameInterval: number;
}

declare class NativeAudioPlayer extends Notification<Event<any, AudioPlayer>> {
	src: string;
	autoPlay: boolean;
	readonly sourceStatus: MultimediaSourceStatus;
	readonly status: PlayerStatus;
	mute: boolean;
	volume: number; // 0-100
	readonly time: number;
	readonly duration: number;
	readonly audioTrackIndex: number;
	readonly audioTrackCount: number;
	disableWaitBuffer: boolean;
	selectAudioTrack(index: number): void;
	audioTrack(index?: number): TrackInfo | null;
	start(): void;
	seek(timeMs: number): boolean;
	pause(): void;
	resume(): void;
	stop(): void;
}

/**
 * @class Video
 */
export declare class Video extends Image {
	readonly onWaitBuffer: EventNoticer<UIEvent<number>>;
	readonly onReady: EventNoticer<UIEvent<void>>;
	readonly onStartPlay: EventNoticer<UIEvent<void>>;
	readonly onSourceEnd: EventNoticer<UIEvent<void>>;
	readonly onPause: EventNoticer<UIEvent<void>>;
	readonly onResume: EventNoticer<UIEvent<void>>;
	readonly onStop: EventNoticer<UIEvent<void>>;
	readonly onSeek: EventNoticer<UIEvent<number>>;
	readonly videoWidth: number;
	readonly videoHeight: number;
	videoTrack(): TrackInfo | null;
	autoPlay: boolean;
	readonly sourceStatus: MultimediaSourceStatus;
	readonly status: PlayerStatus;
	mute: boolean;
	volume: number; // 0-100
	readonly time: number;
	readonly duration: number;
	readonly audioTrackIndex: number;
	readonly audioTrackCount: number;
	disableWaitBuffer: boolean;
	selectAudioTrack(index: number): void;
	audioTrack(index?: number): TrackInfo | null;
	start(): void;
	seek(timeMs: number): boolean;
	pause(): void;
	resume(): void;
	stop(): void;
}

/**
 * @class AudioPlayer
 */
export class AudioPlayer extends (_media.AudioPlayer as typeof NativeAudioPlayer) {
	@event readonly onWaitBuffer: EventNoticer<Event<number, AudioPlayer>>;
	@event readonly onReady: EventNoticer<Event<void, AudioPlayer>>;
	@event readonly onStartPlay: EventNoticer<Event<void, AudioPlayer>>;
	@event readonly onError: EventNoticer<Event<Error, AudioPlayer>>;
	@event readonly onSourceEnd: EventNoticer<Event<void, AudioPlayer>>;
	@event readonly onPause: EventNoticer<Event<void, AudioPlayer>>;
	@event readonly onResume: EventNoticer<Event<void, AudioPlayer>>;
	@event readonly onStop: EventNoticer<Event<void, AudioPlayer>>;
	@event readonly onSeek: EventNoticer<Event<number, AudioPlayer>>;
}

class _Video {
	@event readonly onWaitBuffer: EventNoticer;
	@event readonly onReady: EventNoticer;
	@event readonly onStartPlay: EventNoticer;
	@event readonly onSourceEnd: EventNoticer;
	@event readonly onPause: EventNoticer;
	@event readonly onResume: EventNoticer;
	@event readonly onStop: EventNoticer;
	@event readonly onSeek: EventNoticer;
}

utils.extendClass(_media.Video, _Video);
utils.extendClass(AudioPlayer, NativeNotification);