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

import util from './util';
import _buffer, {ERR_OUT_OF_RANGE,ERR_INVALID_ARG_TYPE} from './_buffer';

import type {Uint, Int, Int8, Uint8, Int16, Uint16, Float} from './defs';

const _codec = util.isQuark ? __binding__('_buffer') : // use native buffer for the quark
	// use _codec.ts for the qktool
	require('./_codec').default;

Object.assign(exports, {
	toString: _codec.toString,
	fromString: _codec.fromString,
	encodeUTF8Length: _codec.encodeUTF8Length,
});

// Get constructor on the Uint8Array base prototype.
const TypedArrayConstructor = (Uint8Array as any).prototype.__proto__.constructor;

/**
 * All encoding algorithms of string
*/
export type Encoding = 'binary'|'latin1'|'ascii'|'base64'|'hex'|'utf-8'|'utf8'|'utf-16'|'utf16'|'ucs4';

/**
 * Bytes type
 */
export type Bytes = Uint8ClampedArray | Uint8Array | Buffer;

/**
 * @class Buffer
 * This class represents a binary buffer.
 */
export class Buffer extends Uint8Array {
	/**
	 * Copy current buffer data to the target buffer.
	 * @param target The target buffer to copy data into.
	 * @param targetStart? The starting index in the target buffer, **Default** is 0.
	 * @param sourceStart? The starting index in the source buffer, **Default** is 0.
	 * @param sourceEnd? The ending index in the source buffer, **Default** is the length of the source buffer.
	 * @return The number of bytes copied.
	 * @throws {TypeError} If the target is not a valid typed array.
	 * @throws {RangeError} If the target start index is out of range.
	 * @throws {RangeError} If the source start or end index is out of range.
	 * @example
	 * ```ts
	 * const source = new Buffer([1, 2, 3, 4, 5]);
	 * const target = new Buffer(3);
	 * const bytesCopied = source.copy(target, 0, 1, 4);
	 * console.log(bytesCopied); // Outputs: 3
	 * console.log(target); // Outputs: Uint8Array(3) [2, 3, 4]
	 * ```
	*/
	copy(target: ArrayBufferView, targetStart?: Uint, sourceStart?: Uint, sourceEnd?: Uint): Uint {
		let source: Uint8Array = this;

		if (!isTypedArray(target))
			throw ERR_INVALID_ARG_TYPE(target, ['Buffer', 'ArrayBufferView'], 'target');

		if (targetStart === undefined) {
			targetStart = 0;
		} else {
			targetStart = Number(targetStart) || 0;
			if (targetStart < 0)
				throw ERR_OUT_OF_RANGE('targetStart', '>= 0', targetStart);
		}

		if (sourceStart === undefined) {
			sourceStart = 0;
		} else {
			sourceStart = Number(sourceStart) || 0;
			if (sourceStart < 0)
				throw ERR_OUT_OF_RANGE('sourceStart', '>= 0', sourceStart);
		}

		if (sourceEnd === undefined) {
			sourceEnd = source.byteLength;
		} else {
			sourceEnd = Number(sourceEnd) || 0;
			if (sourceEnd < 0)
				throw ERR_OUT_OF_RANGE('sourceEnd', '>= 0', sourceEnd);
		}

		if (sourceStart > source.byteLength) {
			throw ERR_OUT_OF_RANGE('sourceStart', `<= ${source.byteLength}`, sourceStart);
		}

		let targetLen = target.byteLength - targetStart;
		let sourceLen = sourceEnd - sourceStart;

		if (targetLen <= 0 || sourceLen <= 0)
			return 0;

		if (sourceLen > targetLen) {
			sourceEnd = sourceStart + targetLen;
			sourceLen = sourceEnd - sourceStart;
		}

		if (sourceStart !== 0 || sourceEnd !== source.byteLength)
			source = new Uint8Array(source.buffer, source.byteOffset + sourceStart, sourceLen);

		new Uint8Array(target.buffer, target.byteOffset).set(source, targetStart);

		return sourceLen;
	}

	/**
	 * slice the buffer data into a new buffer, shared memory with the original buffer.
	*/
	slice(start?: Int, end?: Uint): Buffer {
		return wrapUintArray(super.slice(start, end));
	}

	/**
	 * filter the buffer data into a new buffer.
	*/
	filter(cb: (value: Uint, index: Uint, array: this) => any, thisArg?: any): Buffer {
		return wrapUintArray(super.filter(
			cb as (value: Uint, index: Uint, array: Uint8Array) => any, thisArg)) as any;
	}

	/**
	 * map the buffer data into a new buffer.
	*/
	map(cb: (value: Uint, index: Uint, array: this) => Uint, thisArg?: any): Buffer {
		return wrapUintArray(super.map(
			cb as (value: Uint, index: Uint, array: Uint8Array) => Uint, thisArg));
	}

	/**
	 * subarray the buffer data into a new buffer, shared memory with the original buffer.
	*/
	subarray(begin?: Int, end?: Uint): Buffer {
		return wrapUintArray(super.subarray(begin, end));
	}

	/**
	 * JSON representation of the buffer.
	*/
	toJSON(): { type: 'Buffer'; data: Uint[] } {
		var data = new Array(this.length);
		this.forEach((i,j)=>data[j]=i);
		return { type: 'Buffer', data };
	}

	/**
	 * writing data to self buffer.
	*/
	write(source: ArrayBufferView, offset?: Uint): Uint {
		return new Buffer(source.buffer as ArrayBuffer, source.byteOffset, source.byteLength).copy(this, offset);
	}

	/**
	 * Compare this buffer with another buffer or Uint8Array.
	 * @param target The target buffer or Uint8Array to compare with.
	 * @param start? The starting index in the target buffer, **Default** is 0.
	 * @param end? The ending index in the target buffer, **Default** is the length of the target buffer.
	 * @param thisStart? The starting index in this buffer, **Default** is 0.
	 * @param thisEnd? The ending index in this buffer, **Default** is the length of this buffer.
	 * @return {Int} Returns 0 if the buffers are equal, a negative number if this buffer is less than the target, or a positive number if this buffer is greater than the target.
	 * @throws {TypeError} If the target is not a valid typed array.
	 * @throws {RangeError} If the start or end indices are out of range.
	 * @example
	 * ```ts
	 * const buf1 = new Buffer([1, 2, 3]);
	 * const buf2 = new Buffer([1, 2, 3, 4]);
	 * console.log(buf1.compare(buf2)); // Outputs: -1 (buf1 is less than buf2)
	 * console.log(buf2.compare(buf1)); // Outputs: 1 (buf2 is greater than buf1)
	 * console.log(buf1.compare(buf1)); // Outputs: 0 (buf1 is equal to itself)
	 * ```
	*/
	compare(target: Uint8Array, start?: Uint, end?: Uint, thisStart?: Uint, thisEnd?: Uint): Int {
		if (!isUint8Array(target)) {
			throw new TypeError(
				'The "target" argument must be one of type Buffer or Uint8Array. ' +
				'Received type ' + (typeof target)
			)
		}
	
		if (start === undefined) {
			start = 0
		}
		if (end === undefined) {
			end = target ? target.length : 0
		}
		if (thisStart === undefined) {
			thisStart = 0
		}
		if (thisEnd === undefined) {
			thisEnd = this.length
		}
	
		if (start < 0 || end > target.length || thisStart < 0 || thisEnd > this.length) {
			throw new RangeError('out of range index')
		}
	
		if (thisStart >= thisEnd && start >= end) {
			return 0
		}
		if (thisStart >= thisEnd) {
			return -1
		}
		if (start >= end) {
			return 1
		}
	
		start >>>= 0
		end >>>= 0
		thisStart >>>= 0
		thisEnd >>>= 0
	
		if (this as any === target) return 0
	
		var x = thisEnd - thisStart
		var y = end - start
		var len = Math.min(x, y)
	
		var thisCopy = this.slice(thisStart, thisEnd)
		var targetCopy = target.slice(start, end)
	
		for (var i = 0; i < len; ++i) {
			if (thisCopy[i] !== targetCopy[i]) {
				x = thisCopy[i]
				y = targetCopy[i]
				break
			}
		}
	
		if (x < y) return -1
		if (y < x) return 1
		return 0
	}

	/**
	 * Check if this buffer is equal to another buffer or Uint8Array.
	 * @param b The buffer or Uint8Array to compare with.
	 * @return {boolean} Returns true if the buffers are equal, false otherwise.
	 * @throws {TypeError} If the argument is not a valid Uint8Array.
	 * @example
	 * ```ts
	 * const buf1 = new Buffer([1, 2, 3]);
	 * const buf2 = new Buffer([1, 2, 3]);
	 * const buf3 = new Buffer([4, 5, 6]);
	 * console.log(buf1.equals(buf2)); // Outputs: true (buf1 is equal to buf2)
	 * console.log(buf1.equals(buf3)); // Outputs: false (buf1 is not equal to buf3)
	 * ```
	*/
	equals(b: Uint8Array): boolean {
		if (!isUint8Array(b)) throw new TypeError('Argument must be a Buffer')
		if (this === b) return true
		return compare(this, b) === 0;
	}

	/**
	 * Convert the buffer to a string using the specified encoding.
	 * @param encoding? The encoding to use, **Default** is 'utf8'.
	 * @param start? The starting index, **Default** is 0.
	 * @param end? The ending index, **Default** is the length of the buffer.
	 * @return {string} The string representation of the buffer.
	 * @example
	 * ```ts
	 * const buf = new Buffer([72, 101, 108, 108,111]);
	 * console.log(buf.toString('utf8')); // Outputs: Hello
	 * ```
	 */
	toString(encoding?: Encoding, start?: Uint, end?: Uint): string {
		return _codec.toString(this, encoding, start, end);
	}

	/**
	 * Read int8 from the buffer at the specified offset.
	*/
	readInt8(offset?: Uint): Int8 {
		return _buffer.readInt8(this, offset);
	}
	/**
	 * Read uint8 from the buffer at the specified offset.
	*/
	readUInt8(offset?: Uint): Uint8 {
		return _buffer.readUInt8(this, offset);
	}
	readInt16BE(offset?: Uint): Int16 {
		return _buffer.readInt16(this, false, offset);
	}
	readUInt16BE(offset?: Uint): Uint16 {
		return _buffer.readUInt16(this, false, offset);
	}
	readInt32BE(offset?: Uint): Int {
		return _buffer.readInt32(this, false, offset);
	}
	readUInt32BE(offset?: Uint): Uint {
		return _buffer.readUInt32(this, false, offset);
	}
	readInt40BE(offset?: Uint): number {
		return _buffer.readInt40(this, false, offset);
	}
	readUInt40BE(offset?: Uint): number {
		return _buffer.readUInt40(this, false, offset);
	}
	readInt48BE(offset?: Uint): number {
		return _buffer.readInt48(this, false, offset);
	}
	readUInt48BE(offset?: Uint): number {
		return _buffer.readUInt48(this, false, offset);
	}
	readBigInt64BE(offset?: Uint): bigint {
		return _buffer.readBigInt64(this, false, offset);
	}
	readBigUInt64BE(offset?: Uint): bigint {
		return _buffer.readBigUInt64(this, false, offset);
	}
	readIntBE(offset?: Uint, byteLength = 4): Int {
		return _buffer.readInt(this, false, offset, byteLength);
	}
	readUIntBE(offset?: Uint, byteLength = 4): Uint {
		return _buffer.readUInt(this, false, offset, byteLength);
	}
	readInt16LE(offset?: Uint): Int16 {
		return _buffer.readInt16(this, true, offset);
	}
	readUInt16LE(offset?: Uint): Uint16 {
		return _buffer.readUInt16(this, true, offset);
	}
	readInt32LE(offset?: Uint): Int {
		return _buffer.readInt32(this, true, offset);
	}
	readUInt32LE(offset?: Uint): Uint {
		return _buffer.readUInt32(this, true, offset);
	}
	readInt40LE(offset?: Uint): number {
		return _buffer.readInt40(this, true, offset);
	}
	readUInt40LE(offset?: Uint): number {
		return _buffer.readUInt40(this, true, offset);
	}
	readInt48LE(offset?: Uint): number {
		return _buffer.readInt48(this, true, offset);
	}
	readUInt48LE(offset?: Uint): number {
		return _buffer.readUInt48(this, true, offset);
	}
	readBigInt64LE(offset?: Uint): bigint {
		return _buffer.readBigInt64(this, true, offset);
	}
	readBigUInt64LE(offset?: Uint): bigint {
		return _buffer.readBigUInt64(this, true, offset);
	}
	readIntLE(offset?: Uint, byteLength = 4): Int {
		return _buffer.readInt(this, true, offset, byteLength);
	}
	readUIntLE(offset?: Uint, byteLength = 4): Uint {
		return _buffer.readUInt(this, true, offset, byteLength);
	}
	readFloatBE(offset?: Uint): Float {
		return _buffer.readFloatBE(this, offset);
	}
	readFloatLE(offset?: Uint): Float {
		return _buffer.readFloatLE(this, offset);
	}
	readDoubleBE(offset?: Uint): number {
		return _buffer.readDoubleBE(this, offset);
	}
	readDoubleLE(offset?: Uint): number {
		return _buffer.readDoubleLE(this, offset);
	}
	readBigUIntBE(offset?: Uint, end?: Uint): bigint {
		return _buffer.readBigUIntBE(this, offset, end);
	}
	readBigUIntLE(offset?: Uint, end?: Uint): bigint {
		return _buffer.readBigUIntLE(this, offset, end);
	}
	writeInt8(value: Int8, offset?: Uint): Uint {
		return _buffer.writeInt8(this, value, offset);
	}
	writeUInt8(value: Uint8, offset?: Uint): Uint {
		return _buffer.writeUInt8(this, value, offset);
	}
	writeInt16BE(value: Int16, offset?: Uint): Uint {
		return _buffer.writeInt16BE(this, value, offset);
	}
	writeUInt16BE(value: Uint16, offset?: Uint): Uint {
		return _buffer.writeUInt16BE(this, value, offset);
	}
	writeInt32BE(value: Int, offset?: Uint): Uint {
		return _buffer.writeInt32BE(this, value, offset);
	}
	writeUInt32BE(value: Uint, offset?: Uint): Uint {
		return _buffer.writeUInt32BE(this, value, offset);
	}
	writeInt48BE(value: number, offset?: Uint): Uint {
		return _buffer.writeInt48BE(this, value, offset);
	}
	writeUInt48BE(value: number, offset?: Uint): Uint {
		return _buffer.writeUInt48BE(this, value, offset);
	}
	writeBigInt64BE(value: bigint, offset?: Uint): Uint {
		return _buffer.writeBigInt64BE(this, value, offset);
	}
	writeBigUInt64BE(value: bigint, offset?: Uint): Uint {
		return _buffer.writeBigUInt64BE(this, value, offset);
	}
	writeIntBE(value: Int, offset?: Uint, byteLength?: Uint): Uint {
		return _buffer.writeIntBE(this, value, offset, byteLength);
	}
	writeUIntBE(value: Uint, offset?: Uint, byteLength?: Uint): Uint {
		return _buffer.writeUIntBE(this, value, offset, byteLength);
	}
	writeFloatBE(value: Float, offset?: Uint): Uint {
		return _buffer.writeFloatBE(this, value, offset);
	}
	writeFloatLE(value: Float, offset?: Uint): Uint {
		return _buffer.writeFloatLE(this, value, offset);
	}
	writeDoubleBE(value: number, offset?: Uint): Uint {
		return _buffer.writeDoubleBE(this, value, offset);
	}
	writeDoubleLE(value: number, offset?: Uint): Uint {
		return _buffer.writeDoubleLE(this, value, offset);
	}
	writeBigIntLE(bigint: bigint, offset?: Uint): Uint {
		let arr: number[] = [];
		let l = _buffer.writeBigIntLE(arr, bigint);
		this.set(arr, offset);
		return l;
	}
	// writeUInt16LE(value: number, offset: number, noAssert?: boolean): number; // write le
	// writeUInt32LE(value: number, offset: number, noAssert?: boolean): number;
	// writeInt16LE(value: number, offset: number, noAssert?: boolean): number;
	// writeInt32LE(value: number, offset: number, noAssert?: boolean): number;
}

/**
 * Zero buffer
*/
export const Zero = alloc(0);

/**
 * Alloc new buffer
*/
export function alloc(size: Uint, initFill?: Uint): Buffer {
	let buf = new Buffer(Number(size) || 0);
	if (initFill !== undefined)
		buf.fill(Number(initFill) || 0);
	return buf;
}

/**
 * Alloc unsafe buffer
*/
export function allocUnsafe(size: Uint): Buffer {
	return new Buffer(Number(size) || 0);
}

/**
 * Tests whether it is a typed array
*/
export function isTypedArray(arr: any): boolean {
	return arr instanceof TypedArrayConstructor;
}

/**
 * Tests whether it is a Uint8Array
*/
export function isUint8Array(arr: any): arr is Uint8Array {
	return arr instanceof Uint8Array;
}

/**
 * Tests whether it is a buffer data type
*/
export function isBuffer(arr: any): arr is Buffer {
	return arr instanceof Buffer;
}

/**
 * Compare sizes of two buffers
*/
export function compare(a: Uint8Array, b: Uint8Array): Int {
	if (!isUint8Array(a) || !isUint8Array(b)) {
		throw new TypeError(
			'The "buf1", "buf2" arguments must be one of type Buffer or Uint8Array'
		)
	}

	if (a === b)
		return 0

	let x = a.length
	let y = b.length

	for (let i = 0, len = Math.min(x, y); i < len; ++i) {
		if (a[i] !== b[i]) {
			x = a[i]
			y = b[i]
			break
		}
	}

	if (x < y)
		return -1
	if (y < x)
		return 1
	return 0
}

type FromArg = string | ArrayBufferView | ArrayBufferLike | Iterable<number> | ArrayLike<number>;
type MapFn = (v: number, k: number) => number;

/**
 * Calculate the length of the string after encoding using the specified algorithm
*/
export declare function encodeUTF8Length(str: string): Uint;

/**
 * Decode data into a string using the specified algorithm
*/
export declare function toString(src: Uint8Array, encoding?: Encoding, start?: Uint, end?: Uint): string;

/**
 * Specify the algorithm to encode the string and return the encoded data
 * @param str The string to encode.
 * @param targetEn? Optional, the encoding algorithm to use, **Default** is 'utf8'.
 * @return A Uint8Array containing the encoded data.
 * @example
 * ```ts
 * const encoded = fromString("Hello, World!", "utf8");
 * console.log(encoded); // Outputs: Uint8Array(13) [72, 101, 108, 108, 111, 44, 32, 87, 111, 114, 108, 100, 33]
 * ```
*/
export declare function fromString(str: string, targetEn?: Encoding): Uint8Array;

/**
 * From string create a buffer
 * @param from a string to create a buffer from
 * @param encoding? Optional, this specifies the encoding to use.
 */
export declare function from(from: string, encoding?: Encoding): Buffer;

/**
 * From ArrayBufferView create a buffer
 * @param from a TypedArray or DataView to create a buffer from
*/
export declare function from(from: ArrayBufferView): Buffer;

/**
 * From ArrayBufferLike create a buffer
 * @param from an ArrayBuffer or SharedArrayBuffer to create a buffer from
 * @param byteOffset? Optional, the offset in bytes to start from, **Default** is 0.
 * @param length? Optional, the length in bytes of the buffer, **Default** is the remaining length of the ArrayBuffer.
 * @example
 * ```ts
 * const buffer = from(new ArrayBuffer(10), 0, 10);
 * console.log(buffer.byteLength); // Outputs: 10
 * ```
*/
export declare function from(from: ArrayBufferLike, byteOffset?: Uint, length?: Uint): Buffer;

/**
 * From Iterable or ArrayLike create a buffer
 * @param from an iterable or array-like object to create a buffer from
 * @param mapFn? Optional, a mapping function to apply to each element.
 * @example
 * ```ts
 * const buffer = from([1, 2, 3, 4], (v, k) => v * 2);
 * console.log(buffer); // Outputs: Uint8Array(4) [2, 4, 6, 8]
 * ```
*/
export declare function from(from: Iterable<Int> | ArrayLike<Int>, mapFn?: MapFn): Buffer;

// Helper function to wrap Uint8Array as Buffer
// This is necessary to ensure that Uint8Array methods are available on the Buffer instance.
function wrapUintArray(uint8Array: Uint8Array): Buffer {
	// Set the prototype to Buffer to allow Buffer methods
	(uint8Array as any).__proto__ = Buffer.prototype;
	return uint8Array as Buffer;
}

// Implementation of the `from` function
exports.from = function from(
	from: FromArg, arg1?: Encoding|Uint|MapFn, arg2?: Uint|object
): Buffer {
	if (typeof from === 'string') { // step 0: check for string
		let encoding: Encoding = typeof arg1 == 'string' ? arg1 : 'utf8';
		return wrapUintArray(_codec.fromString(from, encoding));
	}
	else if (from instanceof TypedArrayConstructor && (from as any).buffer) { // step 1: check for TypedArray
		let bf = from as Uint8Array;
		return new Buffer(bf.buffer as ArrayBuffer, bf.byteOffset, bf.byteLength);
	}
	else if (from instanceof ArrayBuffer || (globalThis.SharedArrayBuffer && from instanceof SharedArrayBuffer)) {
		return new Buffer(from as ArrayBuffer, Number(arg1) || 0, Number(arg2) || from.byteLength);
	}
	else if (from instanceof DataView) { // and like step 1 TypedArray
		return new Buffer(from.buffer as ArrayBuffer, from.byteOffset, from.byteLength);
	}
	else if (arg1) {
		return wrapUintArray(Uint8Array.from(from as any, arg1 as any, arg2));
	}
	else {
		return new Buffer(from as any);
	}
}

/**
 * Calculate the byte length of a string when encoded with the specified encoding.
 * @param arg The string whose byte length is to be calculated.
 * @param encoding? Optional, the encoding to use. If not provided, 'utf8' is assumed.
 * @return The byte length of the encoded string.
 * @example
 * ```ts
 * const length = byteLength("Hello, World!", "utf8");
 * console.log(length); // Outputs: 13
 * ```
*/
export function byteLength(
	arg: FromArg, encoding?: Encoding): Uint
{
	if (typeof arg !== 'string') {
		if ('byteLength' in arg
			/*arg as BinaryLike */
			/*arg instanceof ArrayIBuffer || arg instanceof TypedArray*/) {
			return arg.byteLength;
		}
		throw ERR_INVALID_ARG_TYPE(arg, 
				['string', 'Buffer', 'ArrayBuffer', 'SharedArrayBuffer', 'DataView'], 'string');
	}
	if (encoding == 'utf8' || encoding == 'utf-8') {
		return _codec.encodeUTF8Length(arg);
	} else if (encoding == 'hex') {
		util.assert(arg.length % 2 === 0, `encoding error, ${encoding}`);
		return arg.length / 2;
	} else if (encoding == 'base64') {
		util.assert(arg.length % 4 === 0, `encoding error, ${encoding}`);
		if (arg.substring(arg.length - 1) == '=') {
			if (arg.substring(arg.length - 2) == '=')
				return arg.length / 4 * 3 - 2;
			else
				return arg.length / 4 * 3 - 1;
		} else {
			return arg.length / 4 * 3;
		}
	} else if (encoding == 'latin1' || encoding == 'binary') {
		return arg.length;
	} else if (encoding == 'ascii') {
		return arg.length;
	} else {
		return _codec.encodeUTF8Length(arg);
	}
}

/**
 * Concat multiple buffers into a new buffer
 * @param list An array of byte arrays to concatenate.
 * @param length? Optional, the total length of the new buffer. If not provided,
 * it will be calculated based on the lengths of the byte arrays in the list.
 * @return A new Buffer containing the concatenated data.
 * @example
 * ```ts
 * const buffer1 = new Buffer([1, 2, 3]);
 * const buffer2 = new Buffer([4, 5, 6]);
 * const concatenated = concat([buffer1, buffer2]);
 * console.log(concatenated); // Outputs: Uint8Array(6) [1, 2, 3, 4, 5, 6]
 * ```
 * @throws {TypeError} If the provided list is not an array-like object or if
 * any of the elements in the list are not of type `Uint8Array` or `Buffer`.
 * @throws {RangeError} If the specified length is negative.
 * @throws {Error} If the concatenation fails for any reason.
*/
export function concat(list: ArrayLike<Int>[], length?: Uint): Buffer {
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

/**
 * @default
*/
export default {
	byteLength, //!< {byteLength}
	isBuffer, //!< {isBuffer}
	isTypedArray, //!< {isTypedArray}
	isUint8Array, //!< {isUint8Array}
	from: exports.from as typeof from, //!< {from}
	allocUnsafe, //!< {allocUnsafe}
	alloc, //!< {alloc}
	concat, //!< {concat}
	Zero, //!< {Zero}
	compare, //!< {compare}
};
