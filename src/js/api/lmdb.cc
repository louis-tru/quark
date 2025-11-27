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

#include "./types.h"
#include "../../util/lmdb.h"
#include "./ui.h"

namespace qk { namespace js {

	struct MixLMDB: MixObject {
		typedef LMDB Type;

		static JSObject* toArray(Worker* worker, Array<LMDB::Pair>& out) {
			JSObject* js_array = worker->newArray();
			for (int i = 0; i < out.length(); i++) {
				JSObject* js_pair = worker->newArray();
				js_pair->set(worker, 0, worker->newValue(out[i].first));
				// collapse to buffer
				js_pair->set(worker, 1, worker->newValue(out[i].second.collapse()));
				js_array->set(worker, i, js_pair);
			}
			return js_array;
		};

		static void binding(JSObject* exports, Worker* worker) {
			Js_Define_Class(LMDB, 0, {
				Js_Throw("Access forbidden.");
			});

			Js_Class_Static_Method(Make, {
				Js_Parse_Args(String, 0, "path = %s");
				Js_Parse_Args(uint32_t, 1, "max_dbis = %d", (64)); // default 64
				Js_Parse_Args(uint32_t, 2, "map_size = %d", (10485760)); // 10MB
				auto lmdb = LMDB::Make(arg0, arg1, arg2); // native lmdb
				auto mix = MixLMDB::mix(lmdb.get()); // mix object
				Js_Return(mix->handle());
			});

			Js_MixObject_Acce_Get(LMDB, bool, opened, opened);

			Js_Class_Method(open, {
				Js_Return(self->open());
			});

			Js_Class_Method(close, {
				Js_Return(self->close());
			});

			Js_Class_Method(flush, {
				Js_Return(self->flush());
			});

			Js_Class_Method(dbi, {
				Js_Parse_Args(String, 0, "name = %s");
				auto dbi = (NativePtr)self->dbi(arg0); // have to cast to NativePtr
				Js_Return( worker->types()->jsvalue(dbi) );
			});

			Js_Class_Method(get, {
				Js_Parse_Args(NativePtr, 0, "name = %s");
				Js_Parse_Args(String, 1, "key = %s");
				String out;
				if (self->get((LMDB::DBI*)arg0, arg1, &out) == 0) {
					Js_Return(out); // found
				} else {
					Js_Return_Null(); // not found
				}
			});

			Js_Class_Method(getBuf, {
				Js_Parse_Args(NativePtr, 0, "name = %s");
				Js_Parse_Args(String, 1, "key = %s");
				Buffer out;
				if (self->get_buf((LMDB::DBI*)arg0, arg1, &out) == 0) {
					Js_Return(out); // found
				} else {
					Js_Return_Null(); // not found
				}
			});

			Js_Class_Method(set, {
				Js_Parse_Args(NativePtr, 0, "name = %s");
				Js_Parse_Args(String, 1, "key = %s");
				Js_Parse_Args(String, 2, "val = %s");
				Js_Return(self->set((LMDB::DBI*)arg0, arg1, arg2));
			});

			Js_Class_Method(setBuf, {
				Js_Parse_Args(NativePtr, 0, "name = %s");
				Js_Parse_Args(String, 1, "key = %s");
				Js_Parse_Args(WeakBuffer, 2, "val = %s");
				Js_Return(self->set_buf((LMDB::DBI*)arg0, arg1, arg2.buffer()));
			});

			Js_Class_Method(remove, {
				Js_Parse_Args(NativePtr, 0, "name = %s");
				Js_Parse_Args(String, 1, "key = %s");
				Js_Return(self->remove((LMDB::DBI*)arg0, arg1));
			});

			Js_Class_Method(has, {
				Js_Parse_Args(NativePtr, 0, "name = %s");
				Js_Parse_Args(String, 1, "key = %s");
				Js_Return(self->has((LMDB::DBI*)arg0, arg1));
			});

			Js_Class_Method(fuzzExists, {
				Js_Parse_Args(NativePtr, 0, "name = %s");
				Js_Parse_Args(String, 1, "prefix = %s");
				Js_Return(self->fuzz_exists((LMDB::DBI*)arg0, arg1));
			});

			Js_Class_Method(scanPrefix, {
				Js_Parse_Args(NativePtr, 0, "name = %s");
				Js_Parse_Args(String, 1, "prefix = %s");
				Js_Parse_Args(uint32_t, 2, "limit = %d", (0));
				Array<LMDB::Pair> out;
				if (self->scan_prefix((LMDB::DBI*)arg0, arg1, &out, arg2) == 0) {
					Js_Return(toArray(worker, out));
				} else {
					Js_Return(worker->newArray()); // not found, return empty array
				}
			});

			Js_Class_Method(scanRange, {
				Js_Parse_Args(NativePtr, 0, "name = %s");
				Js_Parse_Args(String, 1, "start = %s");
				Js_Parse_Args(String, 2, "end = %s");
				Js_Parse_Args(uint32_t, 3, "limit = %d", (0));
				Array<LMDB::Pair> out;
				if (self->scan_range((LMDB::DBI*)arg0, arg1, arg2, &out, arg3) == 0) {
					Js_Return(toArray(worker, out));
				} else {
					Js_Return(worker->newArray()); // not found, return empty array
				}
			});

			Js_Class_Method(removePrefix, {
				Js_Parse_Args(NativePtr, 0, "name = %s");
				Js_Parse_Args(String, 1, "prefix = %s");
				Js_Return(self->remove_prefix((LMDB::DBI*)arg0, arg1));
			});

			Js_Class_Method(clear, {
				Js_Parse_Args(NativePtr, 0, "name = %s");
				Js_Return(self->clear((LMDB::DBI*)arg0));
			});

			cls->exports("LMDB", exports);
		}
	};

	Js_Module(_lmdb, MixLMDB);
} }
