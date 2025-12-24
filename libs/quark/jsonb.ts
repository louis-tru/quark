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

import errno from './errno';
import utils from './util';
import _buffer from './_buffer';
import {Buffer,from} from './buffer';

const TypedArray = (<any>(Uint8Array.prototype)).__proto__.constructor;

type Bytes = ArrayLike<number>;

const // FLAGS
	F_EOF = 0,
	F_STRING = 1, // utf8 encoded
	F_BUFFER = 2,
	F_INT_8 = 3,
	F_UINT_8 = 4,
	F_INT_16 = 5,
	F_UINT_16 = 6,
	F_INT_32 = 7,
	F_UINT_32 = 8,
	F_INT_64 = 9,
	F_UINT_64 = 10,
	F_FLOAT_NUM_32 = 11,
	F_FLOAT_NUM_64 = 12,
	F_BIGINT = 13,
	F_BIGINT_NEGATIVE = 14,
	F_NULL = 15,
	F_TRUE = 16,
	F_FALSE = 17,
	F_DATE = 18,
	F_OBJECT = 19,
	F_ARRAY = 20,
	F_OBJECT_END = 21,
	F_ARRAY_END = 22,
	F_UNDEFAULT = 23,
	F_NAN = 24,
	F_INFINITY_MIN = 25,
	F_INFINITY_MAX = 26;

const BigInt = (globalThis as any).BigInt; // is support BigInt
if (BigInt) {
	var BIGINT_MIN_SAFE_INTEGER = BigInt(Number.MIN_SAFE_INTEGER);
	var BIGINT_MAX_SAFE_INTEGER = BigInt(Number.MAX_SAFE_INTEGER);
}

type Out = Bytes[];
type Setr = Set<any>;

function write_flag(flag: number, out: Out): number {
	out.push([flag]);
	return 1;
}

function write_buffer(data: Bytes, out: Out): number {
	/*
		0   - 253   : 1,	len|data...
		254 - 65535 : 3,	254|len|len|data...
		65536 -     : 9,	255|len|len|len|len|len|len|len|len|data...
	*/
	let dataLength = data.length;
	let secondByte = dataLength;
	let headerLength = 1;

	if (dataLength > 65535) { // 65536 - *
		headerLength += 8;
		secondByte = 255;
	} else if (dataLength > 253) { // 254 - 65535
		headerLength += 2;
		secondByte = 254;
	}
	// write header:
	let index = 0;
	let header = new Uint8Array(headerLength);
	header[index] = secondByte; index++; // secondByte

	// write data length header:
	switch (secondByte) {
		case 254:
			header[index] = dataLength >> 8; index++;
			header[index] = dataLength % 256; index++;
			break;
		case 255:
			let l = dataLength;
			for (let i = index + 7; i >= index; i--) {
				header[i] = l & 0xff;
				l >>= 8;
			}
			index += 8;
	}

	out.push(header);
	out.push(data);

	return headerLength + dataLength;
}

type Api = 'writeInt8'|'writeUInt8'|'writeInt16BE'|'writeUInt16BE'|'writeInt32BE'|'writeUInt32BE'|'writeDoubleBE'|'writeFloatBE'|'writeInt48BE';

function write_num(v: number, api: Api, len: number, out: Out) {
	let b = new Uint8Array(len);
	_buffer[api](b, v);
	// let b = new Buffer(len);
	// b[api](v);
	out.push(b);
	return len;
}

function isFloat32(offset: number) {
	// float32: S 08-     EEEEEEEE 23>                                  DDDDDDD DDDDDDDD DDDDDDDD
	// float64: S 11- EEE EEEEEEEE 52- DDDD DDDDDDDD DDDDDDDD DDDDDDDD DDDDDDDD DDDDDDDD DDDDDDDD
	return false;
}

function write_number(v: number, out: Out) {
	if (Number.isInteger(v)) { // Integer
		// Int8   意思是8位整数(8bit integer),    相当于 char       占1个字节   -128 ~ 127
		// Int16  意思是16位整数(16bit integer),  相当于 short      占2个字节   -32768 ~ 32767
		// Int32  意思是32位整数(32bit integer),  相当于 int        占4个字节   -2147483648 ~ 2147483647
		// Int64  意思是64位整数(64bit interger), 相当于 long long  占8个字节   -9223372036854775808 ~ 9223372036854775807
		if (v < 0) {
			if (v > -129) { // int8
				return write_flag(F_INT_8, out) + write_num(v, 'writeInt8', 1, out);
			} else if (v > -32769) { // int16
				return write_flag(F_INT_16, out) + write_num(v, 'writeInt16BE', 2, out);
			} else if (v > -2147483649) { // int32
				return write_flag(F_INT_32, out) + write_num(v, 'writeInt32BE', 4, out);
			} else { // int64, javascript use double float
				return write_flag(F_FLOAT_NUM_64, out) + write_num(v, 'writeDoubleBE', 8, out);
			}
		} else {
			if (v < 256) { // uint8 0xff + 1
				return write_flag(F_UINT_8, out) + write_num(v, 'writeUInt8', 1, out);
			} else if (v < 65536) { // uint16 0xffff + 1
				return write_flag(F_UINT_16, out) + write_num(v, 'writeUInt16BE', 2, out);
			} else if (v < 4294967296) { // uint32 0xffffffff + 1
				return write_flag(F_UINT_32, out) + write_num(v, 'writeUInt32BE', 4, out);
			} else { // uint64, javascript use double float
				return write_flag(F_FLOAT_NUM_64, out) + write_num(v, 'writeDoubleBE', 8, out);
			}
		}
	}
	if (isFloat32(v)) { // Float 32/64
		return write_flag(F_FLOAT_NUM_32, out) + write_num(v, 'writeFloatBE', 4, out);
	} else {
		return write_flag(F_FLOAT_NUM_64, out) + write_num(v, 'writeDoubleBE', 8, out);
	}
}

function write_bigint(v: bigint, out: Out) {
	if (v < BIGINT_MAX_SAFE_INTEGER && v > BIGINT_MIN_SAFE_INTEGER) {
		return write_number(Number(v), out);
	}
	if (v < 0) { // 
		write_flag(F_BIGINT_NEGATIVE, out);
		v = -v;
	} else {
		write_flag(F_BIGINT, out);
	}
	let bytes: number[] = [];
	_buffer.writeBigIntLE(bytes, v);

	return 1 + write_buffer(bytes.reverse(), out);
}

function write_array(o: any[], out: Out, set: Setr) {
	if (set.has(o))
		return write_flag(F_NULL, out);
	set.add(o);
	let l = 0;
	for (let val of o) {
		l += serialize(val, out, set);
	}
	set.delete(o);
	return l;
}

function write_object(o: any, out: Out, set: Setr) {
	if (set.has(o))
		return write_flag(F_NULL, out);
	set.add(o);
	let l = 0;
	if (o.toJSON) {
		l = serialize(o.toJSON(), out, set);
	} else {
		l += write_flag(F_OBJECT, out);	
		for (let key in o) {
			l += serialize(key, out, set);
			l += serialize(o[key], out, set);
		}
		l += write_flag(F_OBJECT_END, out);
	}
	set.delete(o);
	return l;
}

function serialize(o: any, out: Out, set: Setr): number {
	switch (typeof o) {
		case 'string':
			return write_flag(F_STRING, out) + write_buffer(from(o), out);
		case 'number':
			if (Number.isNaN(o)) {
				return write_flag(F_NAN, out);
			} else if (o === Infinity) {
				return write_flag(F_INFINITY_MAX, out);
			} else if (o === -Infinity) {
				return write_flag(F_INFINITY_MIN, out);
			} else {
				return write_number(o, out);
			}
		case 'boolean':
			return write_flag(o ? F_TRUE: F_FALSE, out);
		case 'bigint':
			return write_bigint(o, out);
		case 'object':
			if (!o) {
				return write_flag(F_NULL, out);
			} else if (Array.isArray(o)) {
				return write_flag(F_ARRAY, out) + write_array(o, out, set) + write_flag(F_ARRAY_END, out);
			} else if (o instanceof Uint8Array) {
				return write_flag(F_BUFFER, out) + write_buffer(o, out);
			} else if (o instanceof TypedArray) {
				return write_flag(F_BUFFER, out) + write_buffer(new Uint8Array(o.buffer, o.byteOffset, o.byteLength), out);
			} else if (o instanceof ArrayBuffer) {
				return write_flag(F_BUFFER, out) + write_buffer(new Uint8Array(o), out);
			} else if (o instanceof Date) {
				return write_flag(F_DATE, out) + write_num(o.valueOf(), 'writeInt48BE', 6, out);
			} else {
				return write_object(o, out, set);
			}
		case 'undefined':
			return write_flag(F_UNDEFAULT, out);
		default: // default use string
			return write_flag(F_STRING, out) + write_buffer(from(String(o)), out);
	}
}

function binaryify(o: any): Buffer {
	let output: Out = [];
	let byteLen = serialize(o, output, new Set<string>());
	let offset = 0;
	let rev = new Uint8Array(byteLen);
	for (let bytes of output) {
		rev.set(bytes, offset);
		offset += bytes.length;
	}
	return from(rev);
}

// parse binary:

class Binary {
	d: Buffer;
	index: number;
	get value() {
		return this.d[this.index];
	}
	get length() {
		return this.d.length;
	}
	constructor(buf: Uint8Array) {
		this.d = from(buf);
		this.index = 0;
	}
	next() {
		let v = this.d[this.index];
		this.index++;
		return v;
	}
	has(flag: number) {
		return this.value == flag;
	}
	isEOF() {
		return this.index >= this.d.length;
	}
}

function assert(cond: any) {
	utils.assert(cond, errno.ERR_UNABLE_PARSE_JSONB);
}

function read_object(bin: Binary) {
	let rev: Dict = {};
	do {
		if (bin.has(F_OBJECT_END)) {
			bin.next(); break;
		} else {
			let key = read_next(bin);
			rev[key] = read_next(bin);
		}
	} while(true);
	return rev;
}

function read_array(bin: Binary): any[] {
	let rev: any[] = [];
	do {
		if (bin.has(F_ARRAY_END)) {
			bin.next();
			break;
		} else {
			rev.push(read_next(bin));
		}
	} while(true);
	return rev;
}

function read_buffer(bin: Binary): Buffer {
	/*
		0   - 253   : 1,	len|data...
		254 - 65536 : 3,	254|len|len|data...
		65537 -     : 9,	255|len|len|len|len|len|len|len|len|data...
	*/
	let dataLen = bin.next(), end;
	if (dataLen < 254) { // 0 - 253 byte length
		end = bin.index + dataLen;
	} else if (dataLen < 255) { // 254 - 65535 byte length
		assert(bin.length > bin.index + 2);
		dataLen = (bin.next() << 8) | bin.next();
		end = bin.index + dataLen;
	} else { // 65536 - byte length
		assert(bin.length > bin.index + 8);
		dataLen = 0;
		for (let i = 0; i < 8; i++) {
			dataLen *= 256;
			dataLen |= bin.next();
		}
		end = bin.index + dataLen;
	}
	assert(bin.length >= end);
	let d = bin.d.slice(bin.index, end);
	bin.index = end;
	return d;
}

function read_bigint(bin: Binary): bigint | number {
	assert(bin.length > bin.index + 8);
	let bytes = read_buffer(bin);
	if (BigInt) {
		return _buffer.readBigUIntBE(bytes, 0, bytes.length);
	} else { // not support bigint
		console.log('Not support bigint');
		let num = 0;
		for (let byte of bytes) {
			num *= 256;
			num += byte;
		}
		return num;
	}
}

function read_next(bin: Binary): any {
	let flag = bin.next();
	let offset = bin.index;
	switch (flag) {
		case F_STRING:
			return read_buffer(bin).toString('utf8');
		case F_BUFFER:
			return read_buffer(bin);
		case F_INT_8:
			bin.index += 1;
			return _buffer.readInt8(bin.d, offset);
		case F_UINT_8:
			bin.index += 1;
			return _buffer.readUInt8(bin.d, offset);
		case F_INT_16:
			bin.index += 2;
			return _buffer.readInt16(bin.d, false, offset);
		case F_UINT_16:
			bin.index += 2;
			return _buffer.readUInt16(bin.d, false, offset);
		case F_INT_32:
			bin.index += 4;
			return _buffer.readInt32(bin.d, false, offset);
		case F_UINT_32:
			bin.index += 4;
			return _buffer.readUInt32(bin.d, false, offset);
		case F_INT_64:
			bin.index += 8;
			_buffer.readBigInt64BE_Compatible(bin.d, offset);
		case F_UINT_64:
			bin.index += 8;
			_buffer.readBigUInt64BE_Compatible(bin.d, offset);
		case F_FLOAT_NUM_32:
			bin.index += 4;
			return _buffer.readFloatBE(bin.d, offset);
		case F_FLOAT_NUM_64:
			bin.index += 8;
			return _buffer.readDoubleBE(bin.d, offset);
		case F_BIGINT:
			return read_bigint(bin);
		case F_BIGINT_NEGATIVE:
			return -read_bigint(bin);
		case F_TRUE:
			return true;
		case F_FALSE:
			return false;
		case F_DATE:
			bin.index += 6;
			return new Date(_buffer.readUInt48(bin.d, false, offset));
		case F_OBJECT:
			return read_object(bin);
		case F_ARRAY:
			return read_array(bin);
		case F_NULL:
			return null;
		case F_UNDEFAULT:
			return undefined;
		case F_NAN:
			return NaN;
		case F_INFINITY_MIN:
			return -Infinity;
		case F_INFINITY_MAX:
			return Infinity;
		default:
			assert(0);
	}
}

function parse(buf: Uint8Array) {
	return read_next(new Binary(buf));
}

/**
 * @default
*/
export default {
	/**
	 * @method binaryify(obj):Buffer
	 * Convert a JSON object to binary format.
	 * @param obj:any - The object to convert.
	 * @return {Buffer} - The binary representation of the object.
	 * @example
	 * const jsonb = require('quark/jsonb');
	 * const binaryData = jsonb.binaryify({ key: 'value' });
	 * console.log(binaryData); // Outputs the binary data.
	 * @throws {Error} - If the object cannot be serialized.
	*/
	binaryify,

	/**
	 * @method parse(buf):any
	 * Parse a binary buffer back to a JSON object.
	 * @param buf:Uint8Array - The binary data to parse.
	 * @return {any} - The parsed JSON object.
	 * @example
	 * const jsonb = require('quark/jsonb');
	 * const jsonData = jsonb.parse(binaryData);
	 * console.log(jsonData); // Outputs the original JSON object.
	 * @throws {Error} - If the buffer cannot be parsed.
	*/
	parse,
};
