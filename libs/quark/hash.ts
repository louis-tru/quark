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

import {Buffer, alloc} from './buffer';
import _md5 from './_md5';
import _sha1 from './_sha1';
import _sha256 from './_sha256';
import type {Uint,Uint8} from './defs';

export const md5 = _md5; //!<
export const sha1 = _sha1; //!<
export const sha256 = _sha256; //!<

const rnds16 = alloc(16);

function getRnds(len: Uint) {
	return len == 16 ? rnds16: alloc(len);
}

/**
 * Math.random()-based (RNG)
 * If all else fails, use Math.random().  It's fast, but is of unspecified
 * quality.
 * */
export function rng(len: Uint): Buffer {
	var rnds = getRnds(len);
	for (var i = 0, r = 0; i < 16; i++) {
		if ((i & 0x03) === 0) r = Math.random() * 0x100000000;
		rnds[i] = r >>> ((i & 0x03) << 3) & 0xff;
	}
	return rnds;
}

/**
 * Generate a 16-byte random number.
*/
export function rng16(): Buffer {
	return rng(16);
}

/**
 * Convert array of 16 byte values to UUID string format of the form:
 * XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX
 */
const byteToHex: string[] = [];
for (let i = 0; i < 256; ++i) {
	byteToHex[i] = (i + 0x100).toString(16).substring(1);
}

function bytesToUuid(buf: ArrayLike<Uint8>, offset?: Uint) {
	var i = offset || 0;
	var bth = byteToHex;
	// join used to fix memory issue caused by concatenation: https://bugs.chromium.org/p/v8/issues/detail?id=3175#c4
	return ([bth[buf[i++]], bth[buf[i++]], 
	bth[buf[i++]], bth[buf[i++]], '-',
	bth[buf[i++]], bth[buf[i++]], '-',
	bth[buf[i++]], bth[buf[i++]], '-',
	bth[buf[i++]], bth[buf[i++]], '-',
	bth[buf[i++]], bth[buf[i++]],
	bth[buf[i++]], bth[buf[i++]],
	bth[buf[i++]], bth[buf[i++]]]).join('');
}

/**
 * Generate a version 4 (random) UUID.
 * @param random Optional 16-byte array to use as random data.
 * @return A version 4 UUID string.
*/
export function uuid_v4(random?: Uint8Array): string {
	let rnds = random || rng16();

	// Per 4.4, set bits for version and `clock_seq_hi_and_reserved`
	rnds[6] = (rnds[6] & 0x0f) | 0x40;
	rnds[8] = (rnds[8] & 0x3f) | 0x80;

	return bytesToUuid(rnds);
}

