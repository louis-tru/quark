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

#include "./view.h"
#include "../../ui/css/css.h"
#include "../../ui/app.h"

namespace qk { namespace js {

	class WrapStyleSheets: public WrapObject {
	public:
		static void binding(JSObject* exports, Worker* worker) {
			// TODO ...
		}
	};

	class WrapCStyleSheets: public WrapObject {
	public:
		static void binding(JSObject* exports, Worker* worker) {
			// TODO ...
		}
	};

	class NativeCSS {
	public:
		static void createCSS(Worker *worker, JSArray *names, JSObject *arg) {
			auto rss = shared_app()->styleSheets();

			for ( uint32_t i = 0, len = names->length(); i < len; i++ ) {
				auto key = names->get(worker, i);
				auto val = arg->get(worker, key);
				if ( !val ) return; // js error
				if ( !val->isObject() ) {
					Js_Throw("Bad argument.");
				}
				auto arr = rss->search( key->toStringValue(worker, true) );

				if ( arr.length() ) {
					auto props = val->cast<JSObject>();
					auto names = props->getPropertyNames(worker);

					for ( uint32_t j = 0, len = names->length(); j < len; j++ ) {
						auto key = names->get(worker, j);
						auto val = props->get(worker, key);
						if ( !val ) return; // js error
						for ( auto ss : arr ) {
							auto that = WrapObject::wrap<CStyleSheets>(ss)->that();
							if ( !that->set(worker, key, val) ) {
								return; // js error
							}
						}
					}
				} // if (arr.length)
			} // for
		}

		static void binding(JSObject* exports, Worker* worker) {
			worker->bindingModule("_types");
			WrapStyleSheets::binding(exports, worker);

			Js_Set_Method("create", {
				Qk_Fatal_Assert(shared_app());

				if ( args.length() < 1 || !args[0]->isObject() || args[0]->isNull() ) {
					Js_Throw("Bad argument.");
				}
				Js_Handle_Scope();

				auto arg = args[0]->cast<JSObject>();
				auto names = arg->getPropertyNames(worker);
				if (names->length()) {
					shared_app()->lockAllRenderThreads(Cb([worker,names,arg](auto& e) {
						createCSS(worker, names, arg);
					}));
				}
			});
		}
	};

	Js_Set_Module(_css, NativeCSS);
}	}
