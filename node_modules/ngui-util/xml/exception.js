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

var util = require('../util');

var ExceptionMessage = [];

var Exception = util.class('Exception', {
  constructor: function (code, message) {
    var error;
		if (message instanceof Error) {
			error = message;
		} else {
			error = this;
			Error.call(this, ExceptionMessage[code]);
			this.message = ExceptionMessage[code];
			if (Error.captureStackTrace)
				Error.captureStackTrace(this, Exception);
		}
		error.code = code;
		if (message)
			this.message = this.message + ": " + message;
		return error;
	}
});

exports = {
	Exception: Exception,
	INDEX_SIZE_ERR: (ExceptionMessage[1] = 'Index size error', 1),
	DOMSTRING_SIZE_ERR: (ExceptionMessage[2] = 'DOMString size error', 2),
	HIERARCHY_REQUEST_ERR: (ExceptionMessage[3] = 'Hierarchy request error', 3),
	WRONG_DOCUMENT_ERR: (ExceptionMessage[4] = 'Wrong document', 4),
	INVALID_CHARACTER_ERR: (ExceptionMessage[5] = 'Invalid character', 5),
	NO_DATA_ALLOWED_ERR: (ExceptionMessage[6] = 'No data allowed', 6),
	NO_MODIFICATION_ALLOWED_ERR: (ExceptionMessage[7] = 'No modification allowed', 7),
	NOT_FOUND_ERR: (ExceptionMessage[8] = 'Not found', 8),
	NOT_SUPPORTED_ERR: (ExceptionMessage[9] = 'Not supported', 9),
	INUSE_ATTRIBUTE_ERR: (ExceptionMessage[10] = 'Attribute in use', 10),
	//level2
	INVALID_STATE_ERR: (ExceptionMessage[11] = 'Invalid state', 11),
	SYNTAX_ERR: (ExceptionMessage[12] = 'Syntax error', 12),
	INVALID_MODIFICATION_ERR: (ExceptionMessage[13] = 'Invalid modification', 13),
	NAMESPACE_ERR: (ExceptionMessage[14] = 'Invalid namespace', 14),
	INVALID_ACCESS_ERR: (ExceptionMessage[15] = 'Invalid access', 15)
};
