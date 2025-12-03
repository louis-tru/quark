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
 * LMDB database statistics.
 */
export interface Stat {
	psize: Uint;       // 页大小 (一般 4096)
	depth: Uint;       // B+Tree 深度
	branchPages: Uint;// 分支页数量
	leafPages: Uint;  // 叶子页数量
	overflowPages: Uint; // 溢出页数量
	entries: Uint;     // ★ 关键字段：总键值对数量
}

/*
 * Native LMDB binding.
 * 
 * This is the low-level interface exposed from C++.
 * All methods operate on raw strings or Uint8Array.
 * 
 * The high-level JSON wrapper (class LMDB below) overrides
 * get/set/scan to automatically encode/decode using JSONB.
 */
declare abstract class NativeLMDB {
	static Make(path: string, max_dbis?: Uint, map_size?: Uint): NativeLMDB;
	private constructor(path: string, max_dbis?: Uint, map_size?: Uint);
	readonly opened: boolean;
	open(): Int;
	close(): Int;
	flush(): Int;
	nextId(dbi: DBI): Int;
	dbi(table: string): DBI;
	dbiStat(dbi: DBI): Stat;
	get(dbi: DBI, key: string): string | null;
	getBuf(dbi: DBI, key: string): Uint8Array | null;
	set(dbi: DBI, key: string, value: string): Int;
	setBuf(dbi: DBI, key: string, value: Uint8Array): Int;
	remove(dbi: DBI, key: string): Int;
	has(dbi: DBI, key: string): boolean;
	fuzzExists(dbi: DBI, prefix: string): boolean;
	scanPrefix(dbi: DBI, prefix: string, limit?: Uint): [string, Uint8Array][];
	scanRange(dbi: DBI, start: string, end: string, limit?: Uint): [string, Uint8Array][];
	removePrefix(dbi: DBI): Int;
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
export class LMDB {
	private _lmdb: NativeLMDB; // Native LMDB instance

	/**
	 * Create an LMDB storage wrapper.
	 *
	 * Automatically calls NativeLMDB.Make(), so user code never touches
	 * the native factory directly.
	 *
	 * @param path       Filesystem directory used to store the LMDB environment.
	 * @param max_dbis   Maximum number of named sub-databases (DBI).
	 *                   Default = 64.  
	 *                   Each DBI corresponds to one logical “table”.
	 *
	 * @param map_size   Initial mmap size in bytes.
	 *                   Default = 10,485,760 bytes (~10 MB).  
	 *                   LMDB will grow automatically if needed (OS allowing),
	 *                   but a larger initial size can reduce remap operations.
	 *
	 * The constructor does NOT open the environment immediately — actual
	 * opening happens lazily on the first dbi/get/set/scan call.
	 */
	constructor(path: string, max_dbis?: Uint, map_size?: Uint) {
		this._lmdb = _lmdb.LMDB.Make(path, max_dbis, map_size);
	}

	/**
	 * Whether the LMDB environment is already opened.
	 */
	get opened(): boolean {
		return this._lmdb.opened;
	}

	/**
	 * Open the LMDB environment.
	 * @return 0 on success, non-zero on error.
	 */
	open(): Int {
		return this._lmdb.open();
	}

	/**
	 * Close the LMDB environment.
	 * @return 0 on success.
	 */
	close(): Int {
		return this._lmdb.close();
	}

	/**
	 * Flush any pending changes to disk.
	 * @return 0 on success.
	*/
	flush(): Int {
		return this._lmdb.flush();
	}

	/**
	 * Internal nextId counter for generating unique keys.
	 * @return Next unique integer ID, if < 0 indicates error.
	*/
	nextId(dbi: DBI): Int {
		return this._lmdb.nextId(dbi);
	}

	/**
	 * Open or create a named database table.
	 * There will be mutex locking when calling, so it is best to cache the returned DBI.
	 * 
	 * @param table - Logical table name.
	 * @return DBI (opaque binary identifier).
	 */
	dbi(table: string): DBI {
		return this._lmdb.dbi(table);
	}

	/**
	 * Get database statistics for a given DBI/table.
	 */
	dbiStat(dbi: DBI): Stat {
		return this._lmdb.dbiStat(dbi);
	}

	/**
	 * Read raw binary value.
	 * Suitable for JSONB, images, or binary assets.
	 */
	getBuf(dbi: DBI, key: string): Uint8Array | null {
		return this._lmdb.getBuf(dbi, key);
	}

	/**
	 * Remove a key.
	 */
	remove(dbi: DBI, key: string): Int {
		return this._lmdb.remove(dbi, key);
	}

	/**
	 * Check if a key exists.
	 */
	has(dbi: DBI, key: string): boolean {
		return this._lmdb.has(dbi, key);
	}

	/**
	 * Check whether any key with the given prefix exists.
	 */
	fuzzExists(dbi: DBI, prefix: string): boolean {
		return this._lmdb.fuzzExists(dbi, prefix);
	}

	/**
	 * Remove all keys under a prefix.
	 */
	removePrefix(dbi: DBI): Int {
		return this._lmdb.removePrefix(dbi);
	}

	/**
	 * Clear all keys in this DBI/table.
	 */
	clear(dbi: DBI): Int {
		return this._lmdb.clear(dbi);
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
		const v = this._lmdb.getBuf(dbi, key);
		return v != null ? jsonb.parse(v) : null;
		// const v = this._lmdb.get(dbi, key);
		// return v != null ? JSON.parse(v) : null;
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
		return this._lmdb.setBuf(dbi, key, buf);
	}

	/**
	 * Scan prefix and automatically decode JSONB values.
	 * 
	 * @param limit - Maximum number of entries to return. Default = 0 (no limit).
	 *
	 * @return Array of [key, parsedValue].
	 */
	scanPrefix<T = any>(dbi: DBI, prefix: string, limit?: Uint): [string, T][] {
		const list = this._lmdb.scanPrefix(dbi, prefix, limit||0);
		list.forEach((pair) => {
			pair[1] = jsonb.parse(pair[1]);
		});
		return list as [string, T][];
	}

	/**
	 * Scan key range and automatically decode JSONB values.
	 * 
	 * @param limit - Maximum number of entries to return. Default = 0 (no limit).
	 *
	 * @return Array of [key, parsedValue].
	 */
	scanRange<T = any>(dbi: DBI, start: string, end: string, limit?: Uint): [string, T][] {
		const list = this._lmdb.scanRange(dbi, start, end, limit||0);
		list.forEach((pair) => {
			pair[1] = jsonb.parse(pair[1]);
		});
		return list as [string, T][];
	}
}
