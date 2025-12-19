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

const Zero = BigInt(0);
const Eight = BigInt(8);
const ThirtyTwo = BigInt(32);
const Uint8Max = BigInt(0xff);
const Uint32Max = BigInt(0xffffffff);
const Int64Min = BigInt('-9223372036854775808'); // -0x8000000000000000n
const Int64Max = BigInt('9223372036854775807'); // 0x7fffffffffffffffn
const Uint64Max = BigInt('18446744073709551615'); // 0xffffffffffffffffn

function _readBigUIntBE(self, offset, end) {
	let num = Zero;
	while (offset < end) {
		num <<= Eight;
		num |= BigInt(self[offset]);
		offset++;
	}
	return num;
}

function _readBigUIntLE(self, offset, end) {
	let num = Zero;
	while (offset < end) {
		end--;
		num <<= Eight;
		num |= BigInt(self[end]);
	}
	return num;
}

function _readBigInt64BE(self, offset) {
	const hi = 
		(self[offset++] << 24) + // Overflow
		self[offset++] * 2 ** 16 +
		self[offset++] * 2 ** 8 +
		self[offset++];
	const lo =
		self[offset++] * 2 ** 24 +
		self[offset++] * 2 ** 16 +
		self[offset++] * 2 ** 8 +
		self[offset++];
	return (BigInt(hi) << ThirtyTwo) + BigInt(lo);
}

function _readBigInt64LE(self, offset) {
	const lo =
		self[offset++] +
		self[offset++] * 2 ** 8 +
		self[offset++] * 2 ** 16 +
		self[offset++] * 2 ** 24;
	const hi = 
		self[offset++] +
		self[offset++] * 2 ** 8 +
		self[offset++] * 2 ** 16 +
		(self[offset++] << 24); // Overflow
	return (BigInt(hi) << ThirtyTwo) + BigInt(lo);
}

function _readBigUInt64BE(self, offset) {
	const hi = 
		self[offset++] * 2 ** 24 +
		self[offset++] * 2 ** 16 +
		self[offset++] * 2 ** 8 +
		self[offset++];
	const lo = 
		self[offset++] * 2 ** 24 +
		self[offset++] * 2 ** 16 +
		self[offset++] * 2 ** 8 +
		self[offset++];
	return (BigInt(hi) << ThirtyTwo) + BigInt(lo);
}

function _readBigUInt64LE(self, bigint) {
	const lo = 
		self[bigint++] +
		self[bigint++] * 2 ** 8 +
		self[bigint++] * 2 ** 16 +
		self[bigint++] * 2 ** 24;
	const hi = 
		self[bigint++] +
		self[bigint++] * 2 ** 8 +
		self[bigint++] * 2 ** 16 +
		self[bigint++] * 2 ** 24;
	return (BigInt(hi) << ThirtyTwo) + BigInt(lo);
}

function _writeBigIntLE(bytes, bigint) {
	let i = 0;
	do {
		bytes.push(Number(bigint & Uint8Max));
		bigint >>= Eight;
		i++;
	} while(bigint || i < 8);
	return i;
}

function writeBigU_Int64BE(buf, value, offset, min, max) {
	checkInt(value, min, max, buf, offset, 7);

	let lo = Number(value & Uint32Max);
	buf[offset + 7] = lo;
	lo = lo >> 8;
	buf[offset + 6] = lo;
	lo = lo >> 8;
	buf[offset + 5] = lo;
	lo = lo >> 8;
	buf[offset + 4] = lo;
	let hi = Number(value >> ThirtyTwo & Uint32Max);
	buf[offset + 3] = hi;
	hi = hi >> 8;
	buf[offset + 2] = hi;
	hi = hi >> 8;
	buf[offset + 1] = hi;
	hi = hi >> 8;
	buf[offset] = hi;
	return Number(offset) + 8;
}

function _writeBigInt64BE(self, value, offset = 0) {
	return writeBigU_Int64BE(
		self, value, offset, Int64Min, Int64Max);
}

function _writeBigUInt64BE(self, value, offset = 0) {
	return writeBigU_Int64BE(self, value, offset, Zero, Uint64Max);
}

let checkInt = null;
module.exports = {
	_set(_checkInt) { checkInt = _checkInt },
	_readBigUIntBE,
	_readBigUIntLE,
	_readBigInt64BE,
	_readBigInt64LE,
	_readBigUInt64BE,
	_readBigUInt64LE,
	_writeBigIntLE,
	_writeBigInt64BE,
	_writeBigUInt64BE,
};