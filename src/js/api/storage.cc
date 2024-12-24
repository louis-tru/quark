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

#include "../js_.h"
#include "../../util/storage.h"

namespace qk { namespace js {

	struct MixStorage {
		static void binding(JSObject* exports, Worker* worker) {

			Js_Method(get, {
				if (args.length() < 1) {
					Js_Throw(
						"@method get(key)\n"
						"@param key {String}\n"
						"@return {String}\n"
					);
				}
				auto key = args[0]->toString(worker)->value(worker);
				Js_Return( storage_get(key) );
			});

			Js_Method(set, {
				if (args.length() < 2) {
					Js_Throw(
						"@method set(key)\n"
						"@param key {String}\n"
						"@param value {String}\n"
					);
				}
				storage_set( args[0]->toString(worker)->value(worker), args[1]->toString(worker)->value(worker) );
			});

			Js_Method(remove, {
				if (args.length() < 1) {
					Js_Throw(
						"@method del(key)\n"
						"@param key {String}\n"
					);
				}
				storage_remove( args[0]->toString(worker)->value(worker) );
			});

			Js_Method(clear, {
				storage_clear();
			});
		}
	};

	Js_Module(_storage, MixStorage);
} }
