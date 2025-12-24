/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright (c) 2015, Louis.chu
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, self list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, self list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of Louis.chu nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from self software without specific prior written permission.
 * 
 * self SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL Louis.chu BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF self
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * ***** END LICENSE BLOCK ***** */

///////////////////////////////////////////////////////////////////////////////
// BigInt support.
let _bigint: any;
if ((globalThis as any).BigInt) {
	(function(ok: any, req: any) {
		if (typeof __binding__ == 'function') { // quark
			ok(__binding__('quark/_bigint'));
		} else if (globalThis.document) { // webpack amd
			import('./_bigint.js').then((e: any)=>ok(e)); // bigint syntax, webpack delay load
		} else { // node cjs
			ok(req('./_bigint'));
		}
	})(function(bigint: any) {
		_bigint = bigint;
		_bigint._set(checkInt);
	}, typeof require === 'function' ? require : null);
}

function readBigUIntBE(self: Uint8Array, offset: number = 0, end: number = self.length): bigint {
	validateNumber(offset, 'offset');
	validateNumber(end, 'end');
	if (end > self.length)
		boundsError(offset, self.length - (end - offset), 'end');
	return _bigint._readBigUIntBE(self, offset, end);
}

function readBigUIntLE(self: Uint8Array, offset: number = 0, end: number = self.length): bigint {
	validateNumber(offset, 'offset');
	validateNumber(end, 'end');
	if (end > self.length)
		boundsError(offset, self.length - (end - offset), 'end');
	return _bigint._readBigUIntLE(self, offset, end);
}

function writeBigIntLE(bytes: number[], bigint: bigint): number {
	return _bigint._writeBigIntLE(bytes, bigint);
}
///////////////////////////////////////////////////////////////////////////////

// Temporary buffers to convert numbers.
const float32Array = new Float32Array(1);
const uInt8Float32Array = new Uint8Array(float32Array.buffer);
const float64Array = new Float64Array(1);
const uInt8Float64Array = new Uint8Array(float64Array.buffer);

// Check endianness.
float32Array[0] = -1; // 0xBF800000
// Either it is [0, 0, 128, 191] or [191, 128, 0, 0]. It is not possible to
// check self with `os.endianness()` because that is determined at compile time.
const bigEndian = uInt8Float32Array[3] === 0;

export function ERR_BUFFER_OUT_OF_BOUNDS(name?: string) {
	if (name) {
		return new RangeError(`"${name}" is outside of buffer bounds`);
	}
	return new RangeError('Attempt to access memory outside buffer bounds');
}

export function ERR_OUT_OF_RANGE(str: string, range: string, input: any) {
	return new RangeError(`ERR_OUT_OF_RANGE ${str}, ${range}, ${input}`);
}

export function ERR_INVALID_ARG_TYPE(value: any, types: string | string[], arg: string = ''): TypeError {
	if (!Array.isArray(types))
		types = [types];
	return new TypeError(`ERR_INVALID_ARG_TYPE ${arg} [${types.join('|')}] ${value}`);
}

export function validateNumber(value: any, name?: string) {
	if (typeof value !== 'number')
		throw ERR_INVALID_ARG_TYPE(value, 'number', name);
		// throw new TypeError(`ERR_INVALID_ARG_TYPE ${name} number ${value}`);
}

function checkBounds(buf: Uint8Array, offset: number, byteLength: number) {
	validateNumber(offset, 'offset');
	if (buf[offset] === undefined || buf[offset + byteLength] === undefined)
		boundsError(offset, buf.length - (byteLength + 1));
}

function checkInt(value: any, min: any, max: any, buf: Uint8Array, offset: number, byteLength: number) {
	if (value > max || value < min) {
		const n = typeof min === 'bigint' ? 'n' : '';
		let range;
		if (byteLength > 3) {
			if (min == 0) {
				range = `>= 0${n} and < 2${n} ** ${(byteLength + 1) * 8}${n}`;
			} else {
				range = `>= -(2${n} ** ${(byteLength + 1) * 8 - 1}${n}) and < 2 ** ` +
								`${(byteLength + 1) * 8 - 1}${n}`;
			}
		} else {
			range = `>= ${min}${n} and <= ${max}${n}`;
		}
		throw ERR_OUT_OF_RANGE('value', range, value);
	}
	checkBounds(buf, offset, byteLength);
}

function boundsError(value: number, length: number, type?: string) {
	if (Math.floor(value) !== value) {
		validateNumber(value, type);
		throw ERR_OUT_OF_RANGE(type || 'offset', 'an integer', value);
	}

	if (length < 0)
		throw ERR_BUFFER_OUT_OF_BOUNDS();

	throw ERR_OUT_OF_RANGE(type || 'offset',
														 `>= ${type ? 1 : 0} and <= ${length}`,
														 value);
}

// Read integers.
function readUInt8(self: Uint8Array, offset: number = 0) {
	validateNumber(offset, 'offset');
	const val = self[offset];
	if (val === undefined)
		boundsError(offset, self.length - 1);

	return val;
}

function readInt8(self: Uint8Array, offset: number = 0) {
	validateNumber(offset, 'offset');
	const val = self[offset];
	if (val === undefined)
		boundsError(offset, self.length - 1);

	return val | (val & 2 ** 7) * 0x1fffffe;
}

function readInt16(self: Uint8Array, le: boolean, offset: number = 0) {
	validateNumber(offset, 'offset');
	const first = self[offset];
	const last = self[offset + 1];
	if (first === undefined || last === undefined)
		boundsError(offset, self.length - 2);

	const val = le ? last * 2 ** 8 + first : first * 2 ** 8 + last;
	return val | (val & 2 ** 15) * 0x1fffe; // sign bit check
}

function readUInt16(self: Uint8Array, le: boolean, offset: number = 0) {
	validateNumber(offset, 'offset');
	const first = self[offset];
	const last = self[offset + 1];
	if (first === undefined || last === undefined)
		boundsError(offset, self.length - 2);

	return le ? last * 2 ** 8 + first : first * 2 ** 8 + last;
}

function readInt24(buf: Uint8Array, le: boolean, offset: number = 0) {
	validateNumber(offset, 'offset');
	const first = buf[offset];
	const last = buf[offset + 2];
	if (first === undefined || last === undefined)
		boundsError(offset, buf.length - 3);

	const val = le ?
		last * 2 ** 16 + buf[++offset] * 2 ** 8 + first :
		first * 2 ** 16 + buf[++offset] * 2 ** 8 + last;
	return val | (val & 2 ** 23) * 0x1fe;
}

function readUInt24(buf: Uint8Array, le: boolean, offset: number = 0) {
	validateNumber(offset, 'offset');
	const first = buf[offset];
	const last = buf[offset + 2];
	if (first === undefined || last === undefined)
		boundsError(offset, buf.length - 3);

	return le ?
		last * 2 ** 16 + buf[++offset] * 2 ** 8 + first :
		first * 2 ** 16 + buf[++offset] * 2 ** 8 + last;
}

function readInt32(self: Uint8Array, le: boolean, offset: number = 0) {
	validateNumber(offset, 'offset');
	const first = self[offset];
	const last = self[offset + 3];
	if (first === undefined || last === undefined)
		boundsError(offset, self.length - 4);

	if (le) {
		return (last << 24) + // Overflow
			self[offset+2] * 2 ** 16 +
			self[offset+1] * 2 ** 8 +
			first;
	} else {
		return (first << 24) + // Overflow
			self[++offset] * 2 ** 16 +
			self[++offset] * 2 ** 8 +
			last;
	}
}

function readUInt32(self: Uint8Array, le: boolean, offset: number = 0) {
	validateNumber(offset, 'offset');
	const first = self[offset];
	const last = self[offset + 3];
	if (first === undefined || last === undefined)
		boundsError(offset, self.length - 4);

	if (le) {
		return last * 2 ** 24 +
			self[offset+2] * 2 ** 16 +
			self[offset+1] * 2 ** 8 +
			first;
	} else {
		return first * 2 ** 24 +
			self[++offset] * 2 ** 16 +
			self[++offset] * 2 ** 8 +
			last;
	}
}

function readInt40(self: Uint8Array, le: boolean, offset: number = 0) {
	validateNumber(offset, 'offset');
	const first = self[offset];
	const last = self[offset + 4];
	if (first === undefined || last === undefined)
		boundsError(offset, self.length - 5);

	if (le) {
		return (last | (last & 2 ** 7) * 0x1fffffe) * 2 ** 32 +
		self[offset+3] * 2 ** 24 +
		self[offset+2] * 2 ** 16 +
		self[offset+1] * 2 ** 8 +
		first;
	} else {
		return (first | (first & 2 ** 7) * 0x1fffffe) * 2 ** 32 +
			self[++offset] * 2 ** 24 +
			self[++offset] * 2 ** 16 +
			self[++offset] * 2 ** 8 +
			last;
	}
}

function readUInt40(self: Uint8Array, le: boolean, offset: number = 0) {
	validateNumber(offset, 'offset');
	const first = self[offset];
	const last = self[offset + 4];
	if (first === undefined || last === undefined)
		boundsError(offset, self.length - 5);

	if (le) {
		return last * 2 ** 32 +
			self[offset+3] * 2 ** 24 +
			self[offset+2] * 2 ** 16 +
			self[offset+1] * 2 ** 8 +
			first;
	} else {
		return first * 2 ** 32 +
			self[++offset] * 2 ** 24 +
			self[++offset] * 2 ** 16 +
			self[++offset] * 2 ** 8 +
			last;
	}
}

function readInt48(self: Uint8Array, le: boolean, offset: number = 0) {
	validateNumber(offset, 'offset');
	const first = self[offset];
	const last = self[offset + 5];
	if (first === undefined || last === undefined)
		boundsError(offset, self.length - 6);

	if (le) {
		const val = self[offset + 4] + last * 2 ** 8;
		return (val | (val & 2 ** 15) * 0x1fffe) * 2 ** 32 +
			self[offset + 3] * 2 ** 24 +
			self[offset + 2] * 2 ** 16 +
			self[offset + 1] * 2 ** 8 +
			first;
	} else {
		const val = self[++offset] + first * 2 ** 8;
		return (val | (val & 2 ** 15) * 0x1fffe) * 2 ** 32 +
			self[++offset] * 2 ** 24 +
			self[++offset] * 2 ** 16 +
			self[++offset] * 2 ** 8 +
			last;
	}
}

function readUInt48(self: Uint8Array, le: boolean, offset: number = 0) {
	validateNumber(offset, 'offset');
	const first = self[offset];
	const last = self[offset + 5];
	if (first === undefined || last === undefined)
		boundsError(offset, self.length - 6);

	if (le) {
		return (last * 2 ** 8 + self[offset+4]) * 2 ** 32 +
			self[offset+3] * 2 ** 24 +
			self[offset+2] * 2 ** 16 +
			self[offset+1] * 2 ** 8 +
			first;
	} else {
		return (first * 2 ** 8 + self[++offset]) * 2 ** 32 +
			self[++offset] * 2 ** 24 +
			self[++offset] * 2 ** 16 +
			self[++offset] * 2 ** 8 +
			last;
	}
}

function readBigInt64(self: Uint8Array, le: boolean, offset: number = 0): bigint {
	validateNumber(offset, 'offset');
	const first = self[offset];
	const last = self[offset + 7];
	if (first === undefined || last === undefined)
		boundsError(offset, self.length - 8);

	if (_bigint) {
		return le ?
			<bigint>_bigint._readBigInt64LE(self, offset) :
			<bigint>_bigint._readBigInt64BE(self, offset);
	}
	throw new Error('Not support bigint');
}

function readBigUInt64(self: Uint8Array, le: boolean, offset: number = 0): bigint {
	validateNumber(offset, 'offset');
	const first = self[offset];
	const last = self[offset + 7];
	if (first === undefined || last === undefined)
		boundsError(offset, self.length - 8);

	if (_bigint) {
		return le ?
			<bigint>_bigint._readBigUInt64LE(self, offset) :
			<bigint>_bigint._readBigUInt64BE(self, offset);
	}
	throw new Error('Not support bigint');
}

function readBigInt64BE_Compatible(self: Uint8Array, offset: number = 0): bigint | number {
	validateNumber(offset, 'offset');
	const first = self[offset];
	const last = self[offset + 7];
	if (first === undefined || last === undefined)
		boundsError(offset, self.length - 8);

	if (_bigint) {
		return <bigint>_bigint._readBigInt64BE(self, offset);
	}

	const hi = 
		(first << 24) + // Overflow
		self[++offset] * 2 ** 16 +
		self[++offset] * 2 ** 8 +
		self[++offset];

	const lo = 
		self[++offset] * 2 ** 24 +
		self[++offset] * 2 ** 16 +
		self[++offset] * 2 ** 8 +
		last;

	return hi * 2 ** 32 + lo;
}

function readBigUInt64BE_Compatible(self: Uint8Array, offset: number = 0): bigint | number {
	validateNumber(offset, 'offset');
	const first = self[offset];
	const last = self[offset + 7];
	if (first === undefined || last === undefined)
		boundsError(offset, self.length - 8);

	if (_bigint) {
		return <bigint>_bigint._readBigUInt64BE(self, offset)
	}

	const hi = first * 2 ** 24 +
	self[++offset] * 2 ** 16 +
	self[++offset] * 2 ** 8 +
	self[++offset];

	const lo = self[++offset] * 2 ** 24 +
		self[++offset] * 2 ** 16 +
		self[++offset] * 2 ** 8 +
		last;
	return hi * 2 ** 32 + lo;
}

function readInt(self: Uint8Array, le: boolean, offset: number = 0, byteLength = 4) {
	validateNumber(offset, 'offset');

	if (byteLength === 6)
		return readInt48(self, le, offset);
	if (byteLength === 5)
		return readInt40(self, le, offset);
	if (byteLength === 4)
		return readInt32(self, le, offset);
	if (byteLength === 3)
		return readInt24(self, le, offset);
	if (byteLength === 2)
		return readInt16(self, le, offset);
	if (byteLength === 1)
		return readInt8(self, offset);

	boundsError(byteLength, 6, 'byteLength');

	return 0;
}

function readUInt(self: Uint8Array, le: boolean, offset: number = 0, byteLength = 4) {
	validateNumber(offset, 'offset');

	if (byteLength === 6)
		return readUInt48(self, le, offset);
	if (byteLength === 5)
		return readUInt40(self, le, offset);
	if (byteLength === 4)
		return readUInt32(self, le, offset);
	if (byteLength === 3)
	return readUInt24(self, le, offset);
	if (byteLength === 2)
		return readUInt16(self, le, offset);
	if (byteLength === 1)
		return readUInt8(self, offset);

	boundsError(byteLength, 6, 'byteLength');

	return 0;
}

// Read floats
function readFloatBackwards(self: Uint8Array, offset: number = 0) {
	validateNumber(offset, 'offset');
	const first = self[offset];
	const last = self[offset + 3];
	if (first === undefined || last === undefined)
		boundsError(offset, self.length - 4);

	uInt8Float32Array[3] = first;
	uInt8Float32Array[2] = self[++offset];
	uInt8Float32Array[1] = self[++offset];
	uInt8Float32Array[0] = last;
	return float32Array[0];
}

function readFloatForwards(self: Uint8Array, offset: number = 0) {
	validateNumber(offset, 'offset');
	const first = self[offset];
	const last = self[offset + 3];
	if (first === undefined || last === undefined)
		boundsError(offset, self.length - 4);

	uInt8Float32Array[0] = first;
	uInt8Float32Array[1] = self[++offset];
	uInt8Float32Array[2] = self[++offset];
	uInt8Float32Array[3] = last;
	return float32Array[0];
}

function readDoubleBackwards(self: Uint8Array, offset: number = 0) {
	validateNumber(offset, 'offset');
	const first = self[offset];
	const last = self[offset + 7];
	if (first === undefined || last === undefined)
		boundsError(offset, self.length - 8);

	uInt8Float64Array[7] = first;
	uInt8Float64Array[6] = self[++offset];
	uInt8Float64Array[5] = self[++offset];
	uInt8Float64Array[4] = self[++offset];
	uInt8Float64Array[3] = self[++offset];
	uInt8Float64Array[2] = self[++offset];
	uInt8Float64Array[1] = self[++offset];
	uInt8Float64Array[0] = last;
	return float64Array[0];
}

function readDoubleForwards(self: Uint8Array, offset: number = 0) {
	validateNumber(offset, 'offset');
	const first = self[offset];
	const last = self[offset + 7];
	if (first === undefined || last === undefined)
		boundsError(offset, self.length - 8);

	uInt8Float64Array[0] = first;
	uInt8Float64Array[1] = self[++offset];
	uInt8Float64Array[2] = self[++offset];
	uInt8Float64Array[3] = self[++offset];
	uInt8Float64Array[4] = self[++offset];
	uInt8Float64Array[5] = self[++offset];
	uInt8Float64Array[6] = self[++offset];
	uInt8Float64Array[7] = last;
	return float64Array[0];
}

// Write integers.
function writeU_Int8(buf: Uint8Array, value: number, offset: number, min: number, max: number) {
	value = +value;
	// `checkInt()` can not be used here because it checks two entries.
	validateNumber(offset, 'offset');
	if (value > max || value < min) {
		throw ERR_OUT_OF_RANGE('value', `>= ${min} and <= ${max}`, value);
	}
	if (buf[offset] === undefined)
		boundsError(offset, buf.length - 1);

	buf[offset] = value;
	return offset + 1;
}

function writeU_Int16BE(buf: Uint8Array, value: number, offset: number, min: number, max: number) {
	value = +value;
	checkInt(value, min, max, buf, offset, 1);

	buf[offset++] = (value >>> 8);
	buf[offset++] = value;
	return offset;
}

function writeU_Int32BE(buf: Uint8Array, value: number, offset: number, min: number, max: number) {
	value = +value;
	checkInt(value, min, max, buf, offset, 3);

	buf[offset + 3] = value;
	value = value >>> 8;
	buf[offset + 2] = value;
	value = value >>> 8;
	buf[offset + 1] = value;
	value = value >>> 8;
	buf[offset] = value;
	return offset + 4;
}

function writeU_Int24BE(buf: Uint8Array, value: number, offset: number, min: number, max: number) {
	value = +value;
	checkInt(value, min, max, buf, offset, 2);

	buf[offset + 2] = value;
	value = value >>> 8;
	buf[offset + 1] = value;
	value = value >>> 8;
	buf[offset] = value;
	return offset + 3;
}

function writeU_Int40BE(buf: Uint8Array, value: number, offset: number, min: number, max: number) {
	value = +value;
	checkInt(value, min, max, buf, offset, 4);

	buf[offset++] = Math.floor(value * 2 ** -32);
	buf[offset + 3] = value;
	value = value >>> 8;
	buf[offset + 2] = value;
	value = value >>> 8;
	buf[offset + 1] = value;
	value = value >>> 8;
	buf[offset] = value;
	return offset + 4;
}

function writeU_Int48BE(buf: Uint8Array, value: number, offset: number, min: number, max: number) {
	value = +value;
	checkInt(value, min, max, buf, offset, 5);

	const newVal = Math.floor(value * 2 ** -32);
	buf[offset++] = (newVal >>> 8);
	buf[offset++] = newVal;
	buf[offset + 3] = value;
	value = value >>> 8;
	buf[offset + 2] = value;
	value = value >>> 8;
	buf[offset + 1] = value;
	value = value >>> 8;
	buf[offset] = value;
	return offset + 4;
}

function writeInt8(self: Uint8Array, value: number, offset: number = 0) {
	return writeU_Int8(self, value, offset, -0x80, 0x7f);
}

function writeUInt8(self: Uint8Array, value: number, offset: number = 0) {
	return writeU_Int8(self, value, offset, 0, 0xff);
}

function writeInt16BE(self: Uint8Array, value: number, offset: number = 0) {
	return writeU_Int16BE(self, value, offset, -0x8000, 0x7fff);
}

function writeUInt16BE(self: Uint8Array, value: number, offset: number = 0) {
	return writeU_Int16BE(self, value, offset, 0, 0xffff);
}

function writeInt32BE(self: Uint8Array, value: number, offset: number = 0) {
	return writeU_Int32BE(self, value, offset, -0x80000000, 0x7fffffff);
}

function writeUInt32BE(self: Uint8Array, value: number, offset: number = 0) {
	return writeU_Int32BE(self, value, offset, 0, 0xffffffff);
}

function writeInt48BE(self: Uint8Array, value: number, offset: number = 0) {
	return writeU_Int48BE(self, value, offset, -0x800000000000, 0x7fffffffffff);
}

function writeUInt48BE(self: Uint8Array, value: number, offset: number = 0) {
	return writeU_Int48BE(self, value, offset, 0, 0xffffffffffff);
}

function writeBigInt64BE(self: Uint8Array, value: bigint, offset: number = 0) {
	if (_bigint)
		return _bigint._writeBigInt64BE(self, value, offset);
	else
		throw new Error('Not support bigint');
}

function writeBigUInt64BE(self: Uint8Array, value: bigint, offset: number = 0) {
	if (_bigint)
		return _bigint._writeBigUInt64BE(self, value, offset);
	else
		throw new Error('Not support bigint');
}

function writeIntBE(self: Uint8Array, value: number, offset: number = 0, byteLength = 4) {
	if (byteLength === 6)
		return writeU_Int48BE(self, value, offset, -0x800000000000, 0x7fffffffffff);
	if (byteLength === 5)
		return writeU_Int40BE(self, value, offset, -0x8000000000, 0x7fffffffff);
	if (byteLength === 3)
		return writeU_Int24BE(self, value, offset, -0x800000, 0x7fffff);
	if (byteLength === 4)
		return writeU_Int32BE(self, value, offset, -0x80000000, 0x7fffffff);
	if (byteLength === 2)
		return writeU_Int16BE(self, value, offset, -0x8000, 0x7fff);
	if (byteLength === 1)
		return writeU_Int8(self, value, offset, -0x80, 0x7f);

	boundsError(byteLength, 6, 'byteLength');

	return 0;
}

function writeUIntBE(self: Uint8Array, value: number, offset: number = 0, byteLength = 4) {
	if (byteLength === 6)
		return writeU_Int48BE(self, value, offset, 0, 0xffffffffffff);
	if (byteLength === 5)
		return writeU_Int40BE(self, value, offset, 0, 0xffffffffff);
	if (byteLength === 3)
		return writeU_Int24BE(self, value, offset, 0, 0xffffff);
	if (byteLength === 4)
		return writeU_Int32BE(self, value, offset, 0, 0xffffffff);
	if (byteLength === 2)
		return writeU_Int16BE(self, value, offset, 0, 0xffff);
	if (byteLength === 1)
		return writeU_Int8(self, value, offset, 0, 0xff);

	boundsError(byteLength, 6, 'byteLength');

	return 0;
}

// Write floats.
function writeFloatForwards(self: Uint8Array, val: number, offset: number = 0) {
	val = +val;
	checkBounds(self, offset, 3);

	float32Array[0] = val;
	self[offset++] = uInt8Float32Array[0];
	self[offset++] = uInt8Float32Array[1];
	self[offset++] = uInt8Float32Array[2];
	self[offset++] = uInt8Float32Array[3];
	return offset;
}

function writeFloatBackwards(self: Uint8Array, val: number, offset: number = 0) {
	val = +val;
	checkBounds(self, offset, 3);

	float32Array[0] = val;
	self[offset++] = uInt8Float32Array[3];
	self[offset++] = uInt8Float32Array[2];
	self[offset++] = uInt8Float32Array[1];
	self[offset++] = uInt8Float32Array[0];
	return offset;
}

function writeDoubleForwards(self: Uint8Array, val: number, offset: number = 0) {
	val = +val;
	checkBounds(self, offset, 7);

	float64Array[0] = val;
	self[offset++] = uInt8Float64Array[0];
	self[offset++] = uInt8Float64Array[1];
	self[offset++] = uInt8Float64Array[2];
	self[offset++] = uInt8Float64Array[3];
	self[offset++] = uInt8Float64Array[4];
	self[offset++] = uInt8Float64Array[5];
	self[offset++] = uInt8Float64Array[6];
	self[offset++] = uInt8Float64Array[7];
	return offset;
}

function writeDoubleBackwards(self: Uint8Array, val: number, offset: number = 0) {
	val = +val;
	checkBounds(self, offset, 7);

	float64Array[0] = val;
	self[offset++] = uInt8Float64Array[7];
	self[offset++] = uInt8Float64Array[6];
	self[offset++] = uInt8Float64Array[5];
	self[offset++] = uInt8Float64Array[4];
	self[offset++] = uInt8Float64Array[3];
	self[offset++] = uInt8Float64Array[2];
	self[offset++] = uInt8Float64Array[1];
	self[offset++] = uInt8Float64Array[0];
	return offset;
}

var readFloatBE = bigEndian ? readFloatForwards : readFloatBackwards;
var readFloatLE = bigEndian ? readFloatBackwards : readFloatForwards;
var readDoubleBE = bigEndian ? readDoubleForwards : readDoubleBackwards;
var readDoubleLE = bigEndian ? readDoubleBackwards : readDoubleForwards;
var writeFloatBE = bigEndian ? writeFloatForwards : writeFloatBackwards;
var writeFloatLE = bigEndian ? writeFloatBackwards : writeFloatForwards;
var writeDoubleBE = bigEndian ? writeDoubleForwards : writeDoubleBackwards;
var writeDoubleLE = bigEndian ? writeDoubleBackwards : writeDoubleForwards;

export default {
	// read
	readInt8, readUInt8,
	readInt16,
	readUInt16,
	readInt32,
	readUInt32,
	readInt40,
	readUInt40,
	readInt48,
	readUInt48,
	readBigInt64,
	readBigUInt64,
	readBigInt64BE_Compatible,
	readBigUInt64BE_Compatible,
	readInt,
	readUInt,
	readFloatBE, readFloatLE,
	readDoubleBE, readDoubleLE,
	readBigUIntBE, readBigUIntLE,
	// write
	writeInt8,
	writeUInt8,
	writeInt16BE,
	writeUInt16BE,
	writeInt32BE,
	writeUInt32BE,
	writeInt48BE,
	writeUInt48BE,
	writeBigInt64BE,
	writeBigUInt64BE,
	writeIntBE,
	writeUIntBE,
	writeFloatBE, writeFloatLE,
	writeDoubleBE, writeDoubleLE,
	writeBigIntLE,
};