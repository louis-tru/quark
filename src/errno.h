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

#ifndef __quark_errno__
#define __quark_errno__

namespace qk {

	enum {
		ERR_MEDIA_UNKNOWN         = -20029,
		ERR_MEDIA_INVALID_SOURCE  = -20030,
		ERR_MEDIA_NETWORK_ERROR   = -20031,
		ERR_VIDEO_NEW_CODEC_FAIL  = -20032,
		ERR_AUDIO_NEW_CODEC_FAIL  = -20033,
		ERR_ACTION_ILLEGAL_CHILD  = -20034,
		ERR_ACTION_ILLEGAL_ROOT   = -20035,
		ERR_ACTION_ILLEGAL_PARENT  = -20036,
		ERR_ACTION_KEYFRAME_CANNOT_APPEND = -20046,
		ERR_CONTROLLER_VIEW_ERROR = -20037,
		ERR_ONLY_VIEW_CONTROLLER_ERROR = -20038,
		ERR_CONTROLLER_LOOP_SET_VIEW_ERROR = -20039,
		ERR_IMAGE_LOAD_ERROR = -20040,
		ERR_RUN_MAIN_EXCEPTION = -20043,
		ERR_UNCAUGHT_EXCEPTION = -20044,
		ERR_UNHANDLED_REJECTION = -20045,
		ERR_ACTION_SET_WINDOW_NO_MATCH = -20046,
		ERR_ACTION_DISABLE_MULTIPLE_TARGET = -20047,
	};

}

#endif
