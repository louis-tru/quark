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
 * ***** END LICENSE BLOCK *****/

#ifndef __quark__media__pcm_player__
#define __quark__media__pcm_player__

#include "./media.h"

namespace qk {

	enum AudioChannelLayoutMask {
		kInvalid_AudioChannelLayoutMask                = 0,
		kFront_Left_AudioChannelLayoutMask             = 0x00000001,
		kFront_Right_AudioChannelLayoutMask            = 0x00000002,
		kFront_Center_AudioChannelLayoutMask           = 0x00000004,
		kLow_Frequency_AudioChannelLayoutMask          = 0x00000008,
		kBack_Left_AudioChannelLayoutMask              = 0x00000010,
		kBack_Right_AudioChannelLayoutMask             = 0x00000020,
		kFront_Left_Of_Center_AudioChannelLayoutMask   = 0x00000040,
		kFront_Right_Of_Center_AudioChannelLayoutMask  = 0x00000080,
		kBack_Center_AudioChannelLayoutMask            = 0x00000100,
		kSide_Left_AudioChannelLayoutMask              = 0x00000200,
		kSide_Right_AudioChannelLayoutMask             = 0x00000400,
		kTop_Center_AudioChannelLayoutMask             = 0x00000800,
		kTop_Front_Left_AudioChannelLayoutMask         = 0x00001000,
		kTop_Front_Center_AudioChannelLayoutMask       = 0x00002000,
		kTop_Front_Right_AudioChannelLayoutMask        = 0x00004000,
		kTop_Back_Left_AudioChannelLayoutMask          = 0x00008000,
		kTop_Back_Center_AudioChannelLayoutMask        = 0x00010000,
		kTop_Back_Right_AudioChannelLayoutMask         = 0x00020000,
	};

	/**
	* @class PCMPlayer
	*/
	class PCMPlayer: public Protocol {
	public:
		typedef MediaCodec::Frame Frame;
		typedef MediaSource::Stream Stream;

		/**
		* @method write
		*/
		virtual bool write(const Frame *frame) = 0;

		/**
		* @method flush
		*/
		virtual void flush() = 0;

		/**
		* @method set_mute
		*/
		virtual void set_mute(bool value) = 0;

		/**
		* @method set_volume 0-1
		*/
		virtual void set_volume(float value) = 0;

		/**
		* @method delay() Delay from writing PCM data to hearing sound, Unit frame
		*/
		virtual float delay() = 0;

		/**
		* @method create
		*/
		static PCMPlayer* create(const Stream &stream);

	};

}
#endif
