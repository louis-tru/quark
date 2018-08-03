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

#ifndef __ngui_errno__
#define __ngui_errno__

/**
 * @ns ngui
 */

namespace ngui {

	enum {
		ERR_UNKNOWN_ERROR         = -10000,
		ERR_HTTP_STATUS_ERROR     = -10001,
		ERR_INL_ERROR             = -10003,
		ERR_INVALID_PATH          = -10004,
		ERR_SYNTAX_ERROR          = -10005,
		ERR_ASYNC_ERROR           = -10006,
		ERR_FILE_ALREADY_OPEN     = -10007,
		ERR_FILE_NOT_OPEN         = -10008,
		ERR_ALLOCATE_MEMORY_FAIL  = -10009,
		ERR_NOT_RUN_LOOP          = -10010,
		ERR_FILE_NOT_EXISTS       = -10011,
		ERR_ZIP_IN_FILE_NOT_EXISTS= -10012,
		ERR_DUPLICATE_LISTENER    = -10013,
		ERR_PARSE_HOSTNAME_ERROR  = -10014,
		ERR_CONNECT_ALREADY_OPEN  = -10015,
		ERR_SOCKET_NOT_WRITABLE   = -10016,
		ERR_SENDIXX_CANNOT_MODIFY = -10017,
		ERR_FILE_UNEXPECTED_SHUTDOWN    = -10018,
		ERR_CONNECT_UNEXPECTED_SHUTDOWN = -10019,
		ERR_REPEAT_CALL           = - 10020,
		ERR_HTTP_FORM_SIZE_LIMIT  = - 10021,
		ERR_CANNOT_RUN_SYNC_IO    = -10022,
		ERR_NOT_SUPPORTED_FILE_PROTOCOL = -10023,
		ERR_SSL_HANDSHAKE_FAIL    = -10024,
		ERR_SSL_UNKNOWN_ERROR     = -10025,
		ERR_NOT_OPTN_TCP_CONNECT  = -10026,
		ERR_COPY_TARGET_DIRECTORY_NOT_EXISTS  = -10027,
		ERR_FILE_OPENING          = -10028,
		ERR_MEDIA_UNKNOWN         = -10029,
		ERR_MEDIA_INVALID_SOURCE  = -10030,
		ERR_MEDIA_NETWORK_ERROR   = -10031,
		ERR_VIDEO_NEW_CODEC_FAIL  = -10032,
		ERR_AUDIO_NEW_CODEC_FAIL  = -10033,
		ERR_ACTION_ILLEGAL_CHILD  = -10034,
		ERR_ACTION_ILLEGAL_ROOT   = -10035,
		ERR_ACTION_ILLEGAL_VIEW_TYPE  = -10036,
		ERR_CONTROLLER_VIEW_ERROR = -10037,
		ERR_ONLY_VIEW_CONTROLLER_ERROR = -10038,
		ERR_CONTROLLER_LOOP_SET_VIEW_ERROR = -10039,
		ERR_IMAGE_LOAD_ERROR = -10040,
		ERR_BACKGROUND_ONLY_HOST = 10041,
		ERR_BACKGROUND_NEXT_LOOP_REF = 10042,
	};

}

#endif
