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

#include "./util/errno.h"

namespace qk {

	enum {
		ERR_MEDIA_INVALID_SOURCE  = -11030,
		ERR_MEDIA_SOURCE_READ_ERROR = -11031,
		ERR_VIDEO_NEW_CODEC_FAIL  = -11032,
		ERR_AUDIO_NEW_CODEC_FAIL  = -11033,
		ERR_ACTION_ILLEGAL_CHILD  = -11034,
		ERR_ACTION_ILLEGAL_ROOT   = -11035,
		ERR_ACTION_ILLEGAL_PARENT  = -11036,
		ERR_ACTION_PLAYING_CONFLICT  = -11048,
		ERR_ACTION_KEYFRAME_CANNOT_APPEND = -11046,
		ERR_CONTROLLER_VIEW_ERROR = -11037,
		ERR_ONLY_VIEW_CONTROLLER_ERROR = -11038,
		ERR_CONTROLLER_LOOP_SET_VIEW_ERROR = -11039,
		ERR_IMAGE_LOAD_ERROR = -11040,
		ERR_RUN_MAIN_EXCEPTION = -11043,
		ERR_UNCAUGHT_EXCEPTION = -11044,
		ERR_UNHANDLED_REJECTION = -11045,
		ERR_ACTION_SET_WINDOW_NO_MATCH = -11046,
		ERR_ACTION_DISABLE_MULTIPLE_TARGET = -11047,
	};
}

#endif
