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

#ifndef __quark__util__lmdb___
#define __quark__util__lmdb___

#include "./thread.h"
#include "./dict.h"

namespace qk {

	/**
	 * @class LMDB
	 *
	 * Lightweight C++ wrapper around the LMDB (Lightning Memory-Mapped Database)
	 * environment used by Quark for persistent key–value storage.
	 *
	 * Key Features:
	 * ---------------
	 * • Single-writer, multi-reader architecture (LMDB native design).  
	 * • Zero-copy reads via mmap — extremely high performance.  
	 * • Supports multiple logical "tables" via named DBI handles.  
	 * • Provides both string and binary (Buffer) read/write APIs.  
	 * • Prefix and range scanning with LMDB-ordered lexicographic keys.  
	 * • Safe lazy-opening of DBI handles; thread-safe creation via internal mutex.  
	 * • Automatically closes environment on destruction.
	 *
	 * Usage Model:
	 * ---------------
	 * 1. Construct LMDB(path).  
	 * 2. Call open() once, usually it is called automatically on first use.
	 * 3. Acquire DBI using dbi("tablename").  
	 * 4. Read/write using get/set or get_buf/set_buf.  
	 * 5. Optionally use scan_prefix / scan_range for grouped keys.  
	 * 6. close() if needed (or allow destructor to auto-close).
	 *
	 * Threading Notes:
	 * ---------------
	 * • LMDB itself is thread-safe for *readers*, but write operations require
	 *   internal serialized transactions — handled internally by this wrapper.
	 * • _mutex only protects: environment opening + DBI map construction.
	 *   It is *not* used for user-level read/write operations.
	 *
	 * Typical Use Cases:
	 * ---------------
	 * • Cookie storage  
	 * • Game save data  
	 * • Local caches  
	 * • Key-prefix structured data (e.g., session/namespace trees)
	 *
	 * This wrapper exposes a minimal, safe, and cross-platform LMDB interface
	 * suitable for both native and JavaScript binding layers.
	 */
	class Qk_EXPORT LMDB: public Reference {
	public:
		struct DBI;                     // LMDB database handle (per logical table)
		typedef qk::Pair<String, String> Pair;

		// Statistics for a database in the environment
		struct Stat {
			uint32_t	psize;			/**< Size of a database page.
													This is currently the same for all databases. */
			uint32_t	depth;			/**< Depth (height) of the B-tree */
			size_t		branch_pages;	/**< Number of internal (non-leaf) pages */
			size_t		leaf_pages;		/**< Number of leaf pages */
			size_t		overflow_pages;	/**< Number of overflow pages */
			size_t		entries;			/**< Number of data items */
		};

		/**
		 * @static Make
		 *
		 * Create or retrieve an LMDB environment instance for the given path.
		 *
		 * - Returned as `Sp<LMDB>` so lifetime is fully reference-counted.
		 * - If the same path is requested multiple times, the same LMDB instance
		 *   is returned (shared by all callers).
		 * - When all `Sp<LMDB>` references are released, the environment is
		 *   automatically closed and destroyed.
		 *
		 * @param path       Filesystem directory for the LMDB environment.
		 * @param max_dbis   Maximum number of named databases (default: 64).
		 * @param map_size   Initial mmap size in bytes (default: 10 MB).
		 *
		 * @return Shared pointer to the LMDB environment instance.
		 */
		static Sp<LMDB> Make(cString& path, uint32_t max_dbis = 64, uint32_t map_size = 10485760);

		/**
		 * Destructor.
		 * Automatically closes the LMDB environment on process exit.
		 */
		~LMDB(); // auto close on process exit

		/**
		 * @return true if the LMDB environment is already opened.
		 */
		bool opened() const;

		/**
		 * Open the LMDB environment.
		 * Must be called before performing any DB operations.
		 *
		 * @return MDB_SUCCESS on success, error code on failure.
		 */
		int open(); // open env

		/**
		 * Close the LMDB environment.
		 * All DBI handles become invalid after closing.
		 *
		 * @return MDB_SUCCESS on success.
		 */
		int close(); // close env

		/**
		 * Flush any pending changes to disk.
		 * @return MDB_SUCCESS on success.
		*/
		int flush();

		/**
		 * Generate a new unique identifier auto incremented in the database.
		 *
		 * Uses an internal atomic counter stored in the database.
		 *
		 * @param dbi   Database handle.
		 * @return New unique identifier as int64_t, if < 0 then error occurred.
		 */
		int64_t next_id(DBI* dbi);

		/**
		 * Retrieve or allocate a named LMDB database handle (DBI).
		 *
		 * Each logical “table” corresponds to a DBI.
		 * Calling dbi("users") twice returns the same handle.
		 *
		 * @param name  Database/table identifier.
		 * @return Pointer to DBI structure.
		 */
		DBI* dbi(cString& name); // allocate or get existing dbi

		/**
		 * Retrieve statistics for a given DBI.
		 * @param dbi   Database handle.
		 * @return Stat structure with database statistics.
		*/
		Stat dbi_stat(DBI* dbi);

		/**
		 * Read a raw binary value from the database.
		 *
		 * @param dbi   Database handle.
		 * @param key   Key string.
		 * @param out   Output buffer.
		 * @return MDB_SUCCESS if found, otherwise error code.
		 */
		int get_buf(DBI* dbi, cString& key, Buffer* out);

		/**
		 * Store a raw binary value into the database.
		 *
		 * @param dbi   Database handle.
		 * @param key   Key string.
		 * @param val   Binary data.
		 * @return MDB_SUCCESS on success.
		 */
		int set_buf(DBI* dbi, cString& key, cArray<char>& val);

		/**
		 * Read a UTF-8 string value from the database.
		 *
		 * @param dbi   Database handle.
		 * @param key   Key string.
		 * @param out   Output string.
		 * @return MDB_SUCCESS if found, otherwise error code.
		 */
		int get(DBI* dbi, cString& key, String* out);

		/**
		 * Store a UTF-8 string value into the database.
		 *
		 * @param dbi   Database handle.
		 * @param key   Key string.
		 * @param val   String value.
		 * @return MDB_SUCCESS on success.
		 */
		int set(DBI* dbi, cString& key, cString& val);

		/**
		 * Remove a specific key from the database.
		 *
		 * @param dbi   Database handle.
		 * @param key   Key string.
		 * @return MDB_SUCCESS on success.
		 */
		int remove(DBI* dbi, cString& key);

		/**
		 * Check if a specific key exists.
		 *
		 * @return true if key exists.
		 */
		bool has(DBI* dbi, cString& key);

		/**
		 * Check if any key beginning with a prefix exists.
		 *
		 * This uses range-based prefix lookup and does not scan the full DB.
		 *
		 * @return true if any matching key exists.
		 */
		bool fuzz_exists(DBI* dbi, cString& prefix);

		/**
		 * Scan all key-value pairs with a given prefix.
		 *
		 * Keys must follow LMDB lexicographic ordering to ensure grouping.
		 *
		 * @param prefix   Prefix string (not including the terminating '@').
		 * @param out      Output list of (key,value) pairs.
		 * @param limit    Optional limit; 0 means unlimited.
		 *
		 * @return MDB_SUCCESS on success.
		 */
		int scan_prefix(DBI* dbi, cString& prefix, Array<Pair>* out, uint32_t limit = 0);

		/**
		 * Scan a key range [start, end).
		 *
		 * Typically used for prefix + suffix range scanning.
		 *
		 * @param start   Starting key (inclusive).
		 * @param end     Ending key (exclusive).
		 * @param out     Output list of (key,value) pairs.
		 * @param limit   Optional limit; 0 means unlimited.
		 */
		int scan_range(DBI* dbi, cString& start, cString& end, Array<Pair>* out, uint32_t limit = 0);

		/**
		 * Remove all keys starting with a specific prefix.
		 *
		 * Implemented using cursor iteration + range boundaries.
		 */
		int remove_prefix(DBI* dbi, cString& prefix);

		/**
		 * Clear all entries in this database table (DBI).
		 *
		 * @return MDB_SUCCESS on success.
		 */
		int clear(DBI* dbi);

		/**
		 * Global shared LMDB instance.
		 * Typically used for system-level storage (cookies, caches, etc.)
		 */
		static LMDB* shared(); // get shared lmdb instance

	private:

		/**
		 * @private
		 * @constructor
		 * Create an LMDB environment.
		 *
		 * @param path       Filesystem path to the LMDB environment directory.
		 * @param max_dbis   Maximum number of named databases
		 * @param map_size   Initial mmap size in bytes
		 */
		LMDB(cString& path, uint32_t max_dbis, uint32_t map_size);

		/**
		 * Ensure that the given DBI is opened in the LMDB environment.
		 *
		 * Automatically called by all public operations.
		 */
		int opendbi(DBI *dbi);

	private:
		void* _env;                     // LMDB environment handle
		DBI* _next_id_dbi;              // Internal DBI for next_id counter
		Dict<String, DBI*> _dbis;       // Map of opened DBI handles
		String _path;                   // Filesystem path to LMDB environment
		Mutex _mutex;                   // Guards DBI map & env open sequence
		uint32_t _max_dbis;             // Maximum allowed DBI count
		uint32_t _map_size;             // Initial mmap size
	};

	typedef LMDB::DBI* LMDB_DBIPtr;

} // namespace qk

#endif // __quark__util__lmdb__
