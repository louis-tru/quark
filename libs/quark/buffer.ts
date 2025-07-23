/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright © 2015-2016, blue.chu
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

const _buffer = __binding__('_buffer');

Object.assign(exports, _buffer);

export type Buffer = Uint8Array; //!<
export const Buffer = Uint8Array;

/**
 * All encoding algorithms of string
*/
export type Encoding = 'binary'|'ascii'|'base64'|'hex'|'utf-8'|'utf8'|'utf-16'|'utf16'|'ucs4';

/**
 * Specify the algorithm to encode the string and return the encoded data
*/
export declare function fromString(str: string, targetEn?: Encoding): Buffer;

/**
 * Returns the encoded length of the string
*/
export declare function byteLength(str: string, targetEn?: Encoding): Uint;

/**
 * Decode data into a string using the specified algorithm
*/
export declare function toString(src: Buffer, encoding?: Encoding, start?: number, end?: number): string;

/**
 * Zero buffer
*/
export const Zero = alloc(0);

/**
 * Alloc new buffer
*/
export function alloc(size: Uint, initFill?: Uint): Buffer {
	let buf = new Buffer(Number(size) || 0);
	if (initFill)
		buf.fill(Number(initFill) || 0);
	return buf;
}

/**
 * New buffer by array list
*/
export function concat(list: ArrayLike<number>[], length?: number): Buffer {
	if (length === undefined) {
		length = 0;
		for (let bytes of list) {
			if (bytes.length) {
				length += bytes.length;
			}
		}
	} else {
		length = Number(length) || 0;
	}

	if (list.length === 0 || length === 0)
		return Zero;

	let bf = new Buffer(length);
	let offset = 0;

	for (let bytes of list) {
		if (bytes.length) {
			bf.set(bytes, offset);
			offset += bytes.length;
		}
	}

	return bf;
}