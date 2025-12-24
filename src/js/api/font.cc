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

#include "./ui.h"
#include "../../render/font/pool.h"
#include "../../ui/app.h"

namespace qk { namespace js {

	struct MixFontPool: MixObject {
		typedef FontPool Type;

		static void getFontFamiliesFromPool(FunctionArgs args, FontPool* pool) {
			Js_Worker(args);
			if (args.length()) {
				Js_Parse_Type(String, args[0], "@method FontPool.getFontFamilies(cString& families = %s)");
				Js_Return( worker->types()->jsvalue(pool->getFontFamilies(out)) );
			} else {
				Js_Return( worker->types()->jsvalue(pool->getFontFamilies()) );
			}
		}

		static void getFamiliesNameFromFFID(FunctionArgs args) {
			Js_Worker(args);
			if (!args.length()) {
				Js_Throw("@method _font.getFamiliesName(FFID)");
			}
			Js_Parse_Type(FFID, args[0], "@method _font.getFamiliesName(FFID)");
			Js_Return( out->families().join(',') );
		}

		static void binding(JSObject* exports, Worker* worker) {
			Js_Define_Class(FontPool, 0, {
				Js_Throw("Access forbidden.");
			});

			Js_Class_Accessor_Get(countFamilies, {
				Js_Return(self->countFamilies());
			});

			Js_Class_Accessor_Get(defaultFamilyNames, {
				Js_Return(self->defaultFamilyNames());
			});

			Js_Class_Accessor_Get(defaultFontFamilies, {
				Js_Return( worker->types()->jsvalue(self->defaultFontFamilies()) );
			});

			Js_Class_Method(getFontFamilies, {
				getFontFamiliesFromPool(args, self);
			});

			Js_Class_Method(addFontFamily, {
				WeakBuffer buff;
				if (!args.length() || !args[0]->asBuffer(worker).to(buff) ) {
					Js_Throw("@method FontPool.addFontFamily(cBuffer& buff, cString& alias = String())");
				}
				String name;
				if (args.length() > 1) {
					name = self->addFontFamily(buff.buffer(), args[1]->toString(worker)->value(worker));
				} else {
					name = self->addFontFamily(buff.buffer());
				}
				Js_Return( name );
			});

			Js_Class_Method(getFamilyName, {
				int idx;
				if (!args.length() || !args[0]->asInt32(worker).to(idx)) {
					Js_Throw("@method FontPool.getFamilyName(int index)");
				}
				Js_Return( self->getFamilyName(idx) );
			});

			Js_Method(getFontFamilies, {
				// if ( !checkApp(worker) ) return;
				getFontFamiliesFromPool(args, shared_fontPool());
			});

			Js_Method(getFamiliesName, {
				getFamiliesNameFromFFID(args);
			});

			cls->exports("FontPool", exports);
		}
	};

	Js_Module(_font, MixFontPool);
} }
