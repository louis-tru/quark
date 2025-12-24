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

/* Extend base types Definitions */

/**
 * Any type
 */
export type Any = any;

/**
 * 32-bit integer, Range: -2147483648 to 2147483647
 * @global
*/
export type Int = number;

/**
 * 32-bit signed integer, Range: 0 to 4294967295
 * @global
*/
export type Uint = number;

/**
 * 16-bit signed integer, Range: 0 to 65535
 * @global
*/
export type Uint16 = number;

/**
 * 16-bit integer, Range: -32768 to 32767
 * @global
*/
export type Int16 = number;

/**
 * 8-bit signed integer, Range: 0 to 255
 * @global
*/
export type Uint8 = number;

/**
 * 8-bit integer, Range: -128 to 127
 * @global
*/
export type Int8 = number;

/**
 * 32-bit floating point， Range: -3.402823466E+38 to 3.402823466E+38
 * @global
*/
export type Float = number;

/**
 * The [errno,message,description] of Array
 * @type ErrnoCode:[number,string,string?]
*/
export type ErrnoCode = [number, string, string?];

/**
 * The argument type of Error.new
*/
export type ErrorNewArg = ErrnoCode | Error | string | ErrorDescribe;
