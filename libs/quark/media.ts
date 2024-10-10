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

import util from './util';
import event, { EventNoticer, NativeNotification, Notification, Event } from './event';

export enum MediaType {
	kVideo = 1,
	kAudio,
}

export enum MediaSourceStatus {
	kNormal = 0,
	kOpening,
	kPlaying,
	kPaused,
	kError,
	kEOF,
};

export enum AudioChannelLayoutMask {
	kInvalid                = 0,
	kFront_Left             = 0x00000001,
	kFront_Right            = 0x00000002,
	kFront_Center           = 0x00000004,
	kLow_Frequency          = 0x00000008,
	kBack_Left              = 0x00000010,
	kBack_Right             = 0x00000020,
	kFront_Left_Of_Center   = 0x00000040,
	kFront_Right_Of_Center  = 0x00000080,
	kBack_Center            = 0x00000100,
	kSide_Left              = 0x00000200,
	kSide_Right             = 0x00000400,
	kTop_Center             = 0x00000800,
	kTop_Front_Left         = 0x00001000,
	kTop_Front_Center       = 0x00002000,
	kTop_Front_Right        = 0x00004000,
	kTop_Back_Left          = 0x00008000,
	kTop_Back_Center        = 0x00010000,
	kTop_Back_Right         = 0x00020000,
};

export interface Stream {
	type: MediaType;          /* type */
	mime: string;             /* mime type */
	codecId: number;          /* codec id */
	codecTag: number;         /* codec tag */
	format: number;           /* codec av format output */
	profile: number;          /* profile */
	level: number;            /* level */
	width: number;            /* video width */
	height: number;           /* video height */
	language: string;         /* language */
	bitrate: number;          /* bit rate */
	sampleRate: number;       /* audio sample rate */
	channels: number;         /* audio channel count */
	channelLayout: number;    /* audio channel layout to enum AudioChannelLayoutMask */
	avgFramerate: number[];   /* video frame average framerate */
	timeBase: number[];       // Unit of pts,dts on Packet,Frame by Numerator/Denominator (seconds)
	index: number;            /* stream index in source */
	hashCode: number;         // key params hash code
}

export interface Player {
	readonly pts: number;
	volume: number;
	mute: boolean;
	readonly isPause: boolean;
	readonly type: MediaType;
	readonly duration: number;
	readonly status: MediaSourceStatus;
	src: string;
	readonly video: Stream | null;
	readonly audio: Stream | null;
	readonly audioStreams: number;
	play(): void;
	pause(): void;
	stop(): void;
	seek(timeMs: number): void;
	switch_audio(index: number): void;
}

export declare class AudioPlayer extends Notification<Event<AudioPlayer>> implements Player {
	readonly onLoad: EventNoticer<Event<AudioPlayer, void>>;
	readonly onStop: EventNoticer<Event<AudioPlayer, void>>;
	readonly onError: EventNoticer<Event<AudioPlayer, Error>>;
	readonly onBuffering: EventNoticer<Event<AudioPlayer, number>>;
	readonly pts: number;
	volume: number;
	mute: boolean;
	readonly isPause: boolean;
	readonly type: MediaType;
	readonly duration: number;
	readonly status: MediaSourceStatus;
	src: string;
	readonly video: Stream | null;
	readonly audio: Stream | null;
	readonly audioStreams: number;
	play(): void;
	pause(): void;
	stop(): void;
	seek(timeMs: number): void;
	switch_audio(index: number): void;
}

class _AudioPlayer extends NativeNotification {
	@event readonly onLoad: EventNoticer<Event<AudioPlayer, void>>;
	@event readonly onStop: EventNoticer<Event<AudioPlayer, void>>;
	@event readonly onError: EventNoticer<Event<AudioPlayer, Error>>;
	@event readonly onBuffering: EventNoticer<Event<AudioPlayer, number>>;
}

exports.AudioPlayer = __binding__('_ui').AudioPlayer;

util.extendClass(exports.AudioPlayer, _AudioPlayer);