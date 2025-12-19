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

export class ErrnoList {
	// -10000 ~ -19999
	ERR_UNKNOWN_ERROR: ErrnoCode = [-10000, 'UNKNOWN_ERROR']
	ERR_MODULE_NOT_FOUND: ErrnoCode = [-10001, 'ERR_MODULE_NOT_FOUND'] // e.g. quark module not found
	ERR_EXECUTE_TIMEOUT: ErrnoCode = [-10002, 'ERR_EXECUTE_TIMEOUT'] // call timeout() execution timeout
	ERR_NOT_OPEN_CONNECTION: ErrnoCode = [-10003, 'Not open connection'] // e.g. WebSocket not open
	ERR_WS_HANDSHAKE_FAIL: ErrnoCode = [-10004, 'handshake failed, connection closed']
	ERR_FORBIDDEN_ACCESS: ErrnoCode = [-10005, 'FORBIDDEN ACCESS']
	ERR_CONNECTION_DISCONNECTION: ErrnoCode = [-10006, 'Connection disconnection']
	ERR_METHOD_CALL_TIMEOUT: ErrnoCode = [-10007, 'method call timeout']
	ERR_UNABLE_PARSE_JSONB: ErrnoCode = [-10008, 'Unable to parse jsonb, data corrupted']
	ERR_BAD_ARGUMENT: ErrnoCode = [-10009, 'bad argument.']
};
