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

#include "./ui.h"
#include "../../render/font/pool.h"
#include "../../ui/app.h"

namespace qk { namespace js {

	struct WrapFontPool: WrapObject {

		static void getFontFamilysFromPool(FunctionArgs args, FontPool* pool) {
			Js_Worker(args);
			if (args.length()) {
				Js_Parse_Type(String, args[0], "@method FontPool.getFontFamilys(cString& familys = %s)");
				Js_Return( worker->types()->jsvalue(pool->getFontFamilys(out)) );
			} else {
				Js_Return( worker->types()->jsvalue(pool->getFontFamilys()) );
			}
		}

		static void getFamilysNameFromFFID(FunctionArgs args) {
			Js_Worker(args);
			if (!args.length()) {
				Js_Throw("@method _font.getFamilysName(FFID)");
			}
			Js_Parse_Type(FFID, args[0], "@method _font.getFamilysName(FFID)");
			Js_Return( out->familys().keys().join(',') );
		}

		static void binding(JSObject* exports, Worker* worker) {
			Js_Define_Class(FontPool, 0, { Js_Throw("Access forbidden."); });

			Js_Set_Class_Accessor_Get(countFamilies, {
				Js_Self(FontPool);
				Js_Return(self->countFamilies());
			});

			Js_Set_Class_Accessor_Get(defaultFamilyNames, {
				Js_Self(FontPool);
				Js_Return(self->defaultFamilyNames());
			});

			Js_Set_Class_Accessor_Get(defaultFontFamilys, {
				Js_Self(FontPool);
				Js_Return( worker->types()->jsvalue(self->defaultFontFamilys()) );
			});

			Js_Set_Class_Method(getFontFamilys, {
				Js_Self(FontPool);
				getFontFamilysFromPool(args, self);
			});

			Js_Set_Class_Method(addFontFamily, {
				WeakBuffer buff;
				if (!args.length() || !(buff = args[0]->asBuffer(worker)).length()) {
					Js_Throw("@method FontPool.addFontFamily(cBuffer& buff, cString& alias = String())");
				}
				Js_Self(FontPool);
				if (args.length() > 1) {
					self->addFontFamily(buff.buffer(), args[1]->toStringValue(worker));
				} else {
					self->addFontFamily(buff.buffer());
				}
			});

			Js_Set_Class_Method(getFamilyName, {
				if (!args.length() || !args[0]->isUint32()) {
					Js_Throw("@method FontPool.getFamilyName(int index)");
				}
				Js_Self(FontPool);
				Js_Return( self->getFamilyName(args[0]->toInt32Value(worker).unsafe()) );
			});

			Js_Set_Method(getFontFamilys, {
				if ( !checkApp(worker) ) return;
				getFontFamilysFromPool(args, shared_app()->fontPool());
			});

			Js_Set_Method(getFamilysName, {
				getFamilysNameFromFFID(args);
			});

			cls->exports("FontPool", exports);
		}
	};

	Js_Set_Module(_font, WrapFontPool);
} }
