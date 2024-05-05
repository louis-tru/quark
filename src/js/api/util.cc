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

#include "./cb.h"

namespace qk { namespace js {

	extern Array<Char*>* __quark_js_argv;
	extern int           __quark_js_have_debug;

	class WrapNativeObject: public WrapObject {
	public:
		static void binding(JSObject* exports, Worker* worker) {
			Js_Define_Class(Object, 0, {
				New<WrapNativeObject>(args, new Object());
			});
		}
	};

	class WrapHash5381: public WrapObject {
	public:
		static void constructor(FunctionArgs args) {
			New<WrapHash5381>(args, new Hash5381());
		}

		static void binding(JSObject* exports, Worker* worker) {
			Js_Define_Class(Hash5381, 0, {
				New<WrapHash5381>(args, new Hash5381());
			});

			Js_Set_Method(hashCode, {
				Js_Self(Hash5381);
				Js_Return( self->hashCode() );
			});

			Js_Set_Method(update, {
				if (  args.length() < 1 ||
						!(args[0]->isString() || args[0]->isBuffer())
				) {
					Js_Throw("Bad argument");
				}
				Js_Self(Hash5381);
				
				if ( args[0]->isString() ) { // string
					String2 str = args[0]->toStringValue2(worker);
					self->update(*str, str.length());
				}
				else { // Buffer
					auto buff = args[0]->asBuffer(worker);
					self->update(*buff, buff.length());
				}
			});

			Js_Set_Method(digest, {
				Js_Self(Hash5381);
				Js_Return( self->digest() );
			});

			Js_Set_Method(clear, {
				Js_Self(Hash5381);
				*self = Hash5381();
			});

			cls->exports("Hash5381", exports);
		}
	};

	struct WrapUtil {
	public:
		static Hash5381 get_hashCode(Worker *worker, FunctionArgs args) {
			Hash5381 hash;
			String2 str = args[0]->toStringValue2(worker);
			hash.update(*str, str.length());
			return hash;
		}

		static void binding(JSObject* exports, Worker* worker) {

			Qk_ASSERT(__quark_js_argv);
			auto argv = worker->newArray();
			for (uint32_t i = 0; i < __quark_js_argv->length(); i++)
				argv->set(worker, i, worker->newInstance(__quark_js_argv->at(i)));

			Js_Set_Property(argv, argv);
			Js_Set_Property(debug, !!__quark_js_have_debug);

			Js_Set_Method(hashCode, {
				if (!args.length() || ! args[0]->isString())
					Js_Throw("Bad argument");
				Js_Return( get_hashCode(worker, args).hashCode() );
			});
			
			Js_Set_Method(hash, {
				if (!args.length() || ! args[0]->isString()) {
					Js_Throw("Bad argument");
				}
				Js_Return( get_hashCode(worker, args).digest() );
			});
			
			Js_Set_Method(version, {
				Js_Return( qk::version() );
			});
			
			Js_Set_Method(addNativeEventListener, {
				if ( args.length() < 3 || !args[0]->isObject() ||
						!args[1]->isString() || !args[2]->isFunction()) {
					Js_Throw("Bad argument");
				}
				if ( ! WrapObject::wrap(args[0]) ) {
					Js_Throw("Bad argument");
				}
				int id = 0;
				if ( args.length() > 3 && args[3]->isNumber() ) {
					id = args[3]->toNumberValue(worker);
				}
				{ HandleScope scope(worker);
					WrapObject* wrap = WrapObject::wrap(args[0]);
					String name = args[1]->toStringValue(worker,1);
					String func = String("_on").append(name).append("Native").append(String(id));
					bool ok = wrap->addEventListener(name, func, id);
					if (ok) {
						wrap->set(worker->newStringOneByte(func), args[2]);
					}
					Js_Return(ok);
				}
			});
			
			Js_Set_Method(removeNativeEventListener, {
				if ( args.length() < 2 || !args[0]->isObject() || !args[1]->isString()) {
					Js_Throw("Bad argument");
				}
				if ( ! WrapObject::wrap(args[0]) ) {
					Js_Throw("Bad argument");
				}
				int id = 0;
				if ( args.length() > 2 && args[2]->isNumber() ) {
					id = args[2]->toNumberValue(worker);
				}
				{ HandleScope scope(worker);
					String name = args[1]->toStringValue(worker,1);
					WrapObject* wrap = WrapObject::wrap(args[0]);
					bool ok = wrap->removeEventListener(name, id);
					if ( ok ) {
						String func = String("_on").append(name).append("Native").append(String(id));
						wrap->Delete( worker->newStringOneByte(func) );
					}
					Js_Return(ok);
				}
			});

			Js_Set_Method(garbageCollection, {
				args.worker()->garbageCollection();
				#if Qk_MEMORY_TRACE_MARK
					Array<Object*> objs = Object::mark_objects();
					Object** objs2 = &objs[0];
					Qk_LOG("All unrelease heap objects count: %d", objs.size());
				#endif
			});
			
			Js_Set_Method(runScript, {
				if (!args.length() || ! args[0]->isString()) {
					Js_Throw("Bad argument");
				}
				Js_Handle_Scope();
				JSString* name;
				JSObject* sandbox;
				if (args.length() > 1) {
					name = args[1]->toString(worker);
				} else {
					name = worker->newStringOneByte("[eval]");
				}
				if (args.length() > 2 && args[2]->isObject()) {
					sandbox = args[2]->cast<JSObject>();
				}
				auto rv = worker->runScript(args[0]->cast<JSString>(), name, sandbox);
				if (rv) {
					Js_Return( rv );
				} // else js error
			});

			Js_Set_Method(exit, {
				int code = 0;
				if (args.length() && args[0]->isInt32()) {
					code = args[0]->toInt32Value(worker);
				}
				thread_try_abort_and_exit(code);
			});

			WrapNativeObject::binding(exports, worker);
			WrapHash5381::binding(exports, worker);
		}
	};

	Js_Set_Module(_util, WrapUtil);
}}