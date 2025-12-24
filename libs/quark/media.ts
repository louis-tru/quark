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

import util from './util';
import event, { EventNoticer, NativeNotification, Notification, Event } from './event';
import type { Uint} from './defs';

/**
 * @enum MediaType
*/
export enum MediaType {
	/** Video type */
	kVideo = 1,
	/** Audio type */
	kAudio,
}

/**
 * @enum MediaSourceStatus
*/
export enum MediaSourceStatus {
	/** Normal status, Uninitialized */
	kNormal = 0,
	/** Opening status */
	kOpening,
	/** Playing status */
	kPlaying,
	/** Paused status */
	kPaused,
	/** Error status */
	kError,
	/** EOF, end status */
	kEOF,
};

/**
 * @enum AudioChannelLayoutMask
*/
export enum AudioChannelLayoutMask {
	/** none */
	kInvalid                = 0,
	/** front left */
	kFront_Left             = 0x00000001,
	/** front right */
	kFront_Right            = 0x00000002,
	/** front center */
	kFront_Center           = 0x00000004,
	/** low frequency */
	kLow_Frequency          = 0x00000008,
	/** back left */
	kBack_Left              = 0x00000010,
	/** back right */
	kBack_Right             = 0x00000020,
	/** front left of center */
	kFront_Left_Of_Center   = 0x00000040,
	/** front right of center */
	kFront_Right_Of_Center  = 0x00000080,
	/** back center */
	kBack_Center            = 0x00000100,
	/** side left */
	kSide_Left              = 0x00000200,
	/** side right */
	kSide_Right             = 0x00000400,
	/** top center */
	kTop_Center             = 0x00000800,
	/** top front left */
	kTop_Front_Left         = 0x00001000,
	/** top front center */
	kTop_Front_Center       = 0x00002000,
	/** top front right */
	kTop_Front_Right        = 0x00004000,
	/** top back left */
	kTop_Back_Left          = 0x00008000,
	/** top back center */
	kTop_Back_Center        = 0x00010000,
	/** top back right */
	kTop_Back_Right         = 0x00020000,
};

/**
 * @interface Stream
*/
export interface Stream {
	readonly type: MediaType;          //!< type
	readonly mime: string;             //!< mime type
	readonly language: string;         //!< language
	readonly codecId: number;          //!< codec id
	readonly codecTag: number;         //!< codec tag
	readonly format: number;           //!< codec av format output
	readonly profile: number;          //!< profile
	readonly level: number;            //!< level
	readonly width: number;            //!< video width
	readonly height: number;           //!< video height
	readonly bitrate: number;          //!< bit rate
	readonly sampleRate: number;       //!< audio sample rate
	readonly channels: number;         //!< audio channel count
	readonly channelLayout: number;    //!< audio channel layout to enum AudioChannelLayoutMask
	readonly duration: number;         //!< duration of the stream, in stream time base.
	readonly avgFramerate: number[];   //!< video frame average framerate
	readonly timeBase: number[];       //!< Unit of pts,dts on Packet,Frame by Numerator/Denominator (seconds)
	readonly index: number;            //!< stream index in source
	readonly hashCode: number;         //!< key params hash code
}

/**
 * @interface Player
*/
export interface Player {
	/** Presentation timestamp (ms or engine-specific units). */
	readonly pts: Uint;

	/** Playback volume, 0.0–1.0. */
	volume: number;

	/** Whether audio is currently muted. */
	mute: boolean;

	/** True if media is currently paused. */
	readonly isPause: boolean;

	/** Media container type. */
	readonly type: MediaType;

	/** Total duration of the media, in ms. */
	readonly duration: Uint;

	/**
	 * @get status:MediaSourceStatus The playback status of the current media source
	*/
	readonly status: MediaSourceStatus;

	/**
	 * set/get media source path
	*/
	src: string;

	/** Active video stream, if any. */
	readonly video: Stream | null;

	/** Active audio stream, if any. */
	readonly audio: Stream | null;

	/** Get the number of audio tracks in the current media source */
	readonly audioStreams: Uint;

	/** Begin or resume playback. */
	play(): void;

	/** Pause playback. */
	pause(): void;

	/** Stop playback completely. */
	stop(): void;

	/**
	 * Seek to a new position in the media.
	 * @param timeMs Target timestamp in milliseconds.
	 */
	seek(timeMs: Uint): void;

	/**
	 * Switch the currently playing audio by track index
	 * 
	 * @param index The index < [`Player.audioStreams`]
	*/
	switchAudio(index: Uint): void;
}

/**
 * @class AudioPlayer
 * @extends Notification
 * @implements Player
*/
export declare class AudioPlayer extends 
	Notification<Event<AudioPlayer>> implements Player
{
	/**
	 * @event onLoad
	 * 
	 * Trigger when the media source opened and start playing
	*/
	readonly onLoad: EventNoticer<Event<AudioPlayer, void>>;

	/**
	 * @event onStop
	 * 
	 * Trigger when stop playing
	*/
	readonly onStop: EventNoticer<Event<AudioPlayer, void>>;

	/**
	 * @event onError
	 * 
	 * Trigger when an error occurs
	*/
	readonly onError: EventNoticer<Event<AudioPlayer, Error>>;

	/**
	 * @event onBuffering
	 * 
	 * Trigger when the data needs buffering, event.data range for 0-1
	*/
	readonly onBuffering: EventNoticer<Event<AudioPlayer, number>>;

	readonly pts: Uint;
	volume: number;
	mute: boolean;
	readonly isPause: boolean;
	readonly type: MediaType;
	readonly duration: Uint;
	readonly status: MediaSourceStatus;
	src: string;
	readonly video: Stream | null;
	readonly audio: Stream | null;
	readonly audioStreams: Uint;
	play(): void;
	pause(): void;
	stop(): void;
	seek(timeMs: Uint): void;
	switchAudio(index: Uint): void;
}

class _AudioPlayer extends NativeNotification {
	@event readonly onLoad: EventNoticer<Event<AudioPlayer, void>>;
	@event readonly onStop: EventNoticer<Event<AudioPlayer, void>>;
	@event readonly onError: EventNoticer<Event<AudioPlayer, Error>>;
	@event readonly onBuffering: EventNoticer<Event<AudioPlayer, number>>;
}

exports.AudioPlayer = __binding__('_ui').AudioPlayer;

util.extendClass(exports.AudioPlayer, _AudioPlayer);