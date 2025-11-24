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
import jsonb from './jsonb';

const _lmdb = __binding__('_lmdb');

export type DBI = Uint8Array; //!< Database table Identifier

/**
 * Native LMDB binding.
 * 
 * This is the low-level interface exposed from C++.
 * All methods operate on raw strings or Uint8Array.
 * 
 * The high-level JSON wrapper (class LMDB below) overrides
 * get/set/scan to automatically encode/decode using JSONB.
 */
declare class NativeLMDB {

	constructor(path: string, max_dbis?: Uint, map_size?: Uint);

	/**
	 * Whether the LMDB environment is already opened.
	 */
	readonly opened: boolean;

	/**
	 * Open the LMDB environment.
	 * @return 0 on success, non-zero on error.
	 */
	open(): Int;

	/**
	 * Close the LMDB environment.
	 * @return 0 on success.
	 */
	close(): Int;

	/**
	 * Open or create a named database table.
	 * @param table - Logical table name.
	 * @return DBI (opaque binary identifier).
	 */
	dbi(table: string): DBI;

	/**
	 * Read a UTF-8 string value.
	 * Returned string is decoded from raw bytes.
	 */
	get(dbi: DBI, key: string): string | null;

	/**
	 * Read raw binary value.
	 * Suitable for JSONB, images, or binary assets.
	 */
	getBuf(dbi: DBI, key: string): Uint8Array | null;

	/**
	 * Store a UTF-8 string value.
	 */
	set(dbi: DBI, key: string, value: string): Int;

	/**
	 * Store a raw binary buffer.
	 */
	setBuf(dbi: DBI, key: string, value: Uint8Array): Int;

	/**
	 * Remove a key.
	 */
	remove(dbi: DBI, key: string): Int;

	/**
	 * Check if a key exists.
	 */
	has(dbi: DBI, key: string): boolean;

	/**
	 * Check whether any key with the given prefix exists.
	 */
	fuzzExists(dbi: DBI, prefix: string): boolean;

	/**
	 * Scan all key-value pairs under a prefix.
	 * Each item is [key, Uint8Array].
	 */
	scanPrefix(dbi: DBI, prefix: string, limit?: Uint): [string, Uint8Array][];

	/**
	 * Scan a key range [start, end).
	 */
	scanRange(dbi: DBI, start: string, end: string, limit?: Uint): [string, Uint8Array][];

	/**
	 * Remove all keys under a prefix.
	 */
	removePrefix(dbi: DBI): Int;

	/**
	 * Clear all keys in this DBI/table.
	 */
	clear(dbi: DBI): Int;
}

/**
 * High-level LMDB wrapper with automatic JSONB encoding.
 *
 * All values stored by this class are encoded using:
 *   jsonb.binaryify(value)
 *
 * All values retrieved are decoded using:
 *   jsonb.parse(buffer)
 *
 * This provides type-safe persistent storage:
 *   - numbers keep type
 *   - booleans keep type
 *   - arrays/objects keep structure
 *   - Date, Buffer, BigInt are preserved
 *
 * Calling setBuf() directly is disabled to avoid bypassing JSONB.
 */
export class LMDB extends (_lmdb.LMDB as typeof NativeLMDB) {

	/**
	 * Disable direct setBuf() in JSON mode.
	 * Users must call set() instead so the value is encoded through JSONB.
	 */
	setBuf(): Int {
		throw new Error('Not supported setBuf in LMDB JSON mode');
	}

	/**
	 * Get a JSON value from database.
	 *
	 * @template T - Expected return type.
	 * @param dbi - Table identifier.
	 * @param key - Key string.
	 * @return Parsed JSONB value or null.
	 */
	get<T = any>(dbi: DBI, key: string): T | null {
		const v = super.getBuf(dbi, key);
		return v != null ? jsonb.parse(v) : null;
	}

	/**
	 * Store a JSON value into database.
	 *
	 * The value is serialized as binary using JSONB
	 * and stored via setBuf().
	 *
	 * @template T - Type of the value.
	 * @return 0 on success.
	 */
	set<T = any>(dbi: DBI, key: string, value: T): Int {
		const buf = jsonb.binaryify(value);
		return super.setBuf(dbi, key, buf);
	}

	/**
	 * Scan prefix and automatically decode JSONB values.
	 *
	 * @return Array of [key, parsedValue].
	 */
	scanPrefix<T = any>(dbi: DBI, prefix: string, limit?: Uint): [string, T][] {
		const list = this.scanPrefix(dbi, prefix, limit);
		list.forEach((pair) => {
			pair[1] = jsonb.parse(pair[1]);
		});
		return list;
	}

	/**
	 * Scan key range and automatically decode JSONB values.
	 *
	 * @return Array of [key, parsedValue].
	 */
	scanRange<T = any>(dbi: DBI, start: string, end: string, limit?: Uint): [string, T][] {
		const list = this.scanRange(dbi, start, end, limit);
		list.forEach((pair) => {
			pair[1] = jsonb.parse(pair[1]);
		});
		return list;
	}
}
