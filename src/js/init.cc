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

#include "./js_.h"

namespace qk { namespace js {
	extern int    __quark_js_argc;
	extern char** __quark_js_argv;

	struct JSONStringify {
		uint32_t _indent;
		Array<String>* _rv;
		Worker* worker;
		Persistent<JSSet> _set;
		WeakBuffer wbuff;

		JSONStringify(Worker* worker) : _indent(0), _rv(nullptr), worker(worker) {
			_set.reset(worker, worker->newSet());
		}

		~JSONStringify() { _set.reset(); }

		void push_indent() {
			for (uint32_t i = 0; i < _indent; i++) _rv->push(' ');
		}

		bool stringify_object(JSObject* arg) {
			_rv->push('{');
			JSArray* names = arg->getPropertyNames(worker);
			if ( names->length() > 1 ) {
				_indent += 2;
				for (int i = 0, j = 0; i < names->length(); i++) {
					JSValue* key = names->get(worker, i);
					if (j > 0) _rv->push(','); // ,
					_rv->push('\n'); push_indent();
					//  _rv->push('"');
					_rv->push(key->toString(worker)->value(worker));
					//  _rv->push('"');
					_rv->push(':'); _rv->push(' ');
					bool rv = stringify( arg->get(worker, key) ); // value
					if ( ! rv ) return false;
					j++;
				}
				_indent -= 2;
				_rv->push('\n'); push_indent();
			} else {
				_rv->push(' ');
			}
			_rv->push('}');
			return true;
		}

		bool stringify_array(JSArray* arg) {
			_rv->push('[');
			if (arg->length() > 0) {
				_indent += 2;
				for (int i = 0; i < arg->length(); i++) {
					if (i > 0)
						_rv->push(',');
					_rv->push('\n');
					push_indent();
					stringify( arg->get(worker, i) ); // value
				}
				_indent -= 2;
				_rv->push('\n');
				push_indent();
			} else {
				_rv->push(' ');
			}
			_rv->push(']');
			return true;
		}

		bool stringify_buffer(WeakBuffer buf) {
			// ->cast<JSUint8Array>()->value(worker)
			_rv->push("<Buffer");
			cChar* hex = "0123456789abcdef";
			uint8_t* s = (uint8_t*)*buf;
			for (uint32_t i = 0; i < buf.length(); i++) {
				uint8_t ch = s[i];
				_rv->push(' ');
				_rv->push( hex[ch >> 4] );
				_rv->push( hex[ch & 15] );
				if (i > 50) {
					_rv->push(" ... "); break;
				}
			}
			_rv->push('>');
			return true;
		}

		bool stringify(JSValue* arg) {
			if (!arg)
				return false; // error
			bool rv = true;

			if(arg->isString()) {
				_rv->push('"');
				_rv->push( arg->toString(worker)->value(worker) );
				_rv->push('"');
			}
			else if (arg->isFunction()) {
				_rv->push( "[Function]" );
			}
			else if (arg->isObject()) {
				JSObject* o = arg->cast<JSObject>();
				if (o->has(worker, worker->strs()->toStringStyled())) {
					auto indent = worker->newValue(_indent)->cast();
					auto toStringStyled = o->get(worker, worker->strs()->toStringStyled());
					auto str = toStringStyled->cast<JSFunction>()->call(worker, 1, &indent, o);
					if (!str)
						return false; // error
					_rv->push( str->toString(worker)->value(worker) );
				}
				else if (arg->asBuffer(worker).to(wbuff)) {
					rv = stringify_buffer(wbuff);
				}
				else if ( arg->isDate() ) {
					_rv->push( arg->toString(worker)->value(worker) );
				}
				else {
					if ( _set->has(worker, o) ) {
						_rv->push( "[Circular]" );
						return true;
					}
					_set->add(worker, o);

					if (arg->isArray()) {
						rv = stringify_array(o->cast<JSArray>());
					} else {
						rv = stringify_object(o);
					}
					return _set->deleteFor(worker, o);
				}
			}
			else if(arg->isInt32()) {
				_rv->push( String(arg->toInt32(worker)->value()) );
			}
			else if(arg->isNumber()) {
				_rv->push( String(arg->cast<JSNumber>()->value()) );
			}
			else if(arg->isBoolean()) {
				if (arg->cast<JSBoolean>()->value()) {
					_rv->push("true");
				} else {
					_rv->push("false");
				}
			}
			else if(arg->isDate()) {
				_rv->push('"');
				auto f =
					arg->cast<JSObject>()->get(worker, worker->strs()->toJSON())->cast<JSFunction>();
				_rv->push( f->call(worker, 0, nullptr, arg)->toString(worker)->value(worker) );
				_rv->push('"');
			}
			else if(arg->isNull()) {
				_rv->push("null");
			}
			else if(arg->isUndefined()) {
				_rv->push("undefined");
			}
			return true;
		}

		bool stringify_console_styled(JSValue* arg, Array<String>* out) {
			HandleScope scope(worker);
			_rv = out;
			return stringify(arg);
		}
	};

	struct NativeConsole {
		static void print_to(FunctionArgs args, void(*print)(cString&)) {
			Js_Worker(args);
			Array<String> rv;

			for (int i = 0; i < args.length(); i++) {
				if (i)
					rv.push(' ');
				if (args[i]->isObject()) {
					if (!JSONStringify(worker).stringify_console_styled(args[i], &rv))
						return; // error
				} else {
					rv.push( args[i]->toString(worker)->value(worker) );
				}
			}
			print(rv.join(String()));
		}

		static void call_origin(FunctionArgs args, JSValue* name) {
			Js_Worker(args);
			Array<JSValue*> argv(args.length());
			for (int i = 0; i < args.length(); i++) {
				argv[i] = args[i];
			}
			auto console = Qk_WorkerInl(worker)->console();
			auto f = console->get(worker, name)->cast<JSFunction>();
			f->call(worker, argv.length(), *argv, console);
		}

		static void binding(JSObject* exports, Worker* worker, bool isOrigin) {
			if (isOrigin) {
				Js_Property(_log, exports->get(worker, "log"));
				Js_Property(_warn, exports->get(worker, "warn"));
				Js_Property(_error, exports->get(worker, "error"));
				Js_Property(_clear, exports->get(worker, "clear"));
				Js_Method(log, {
					print_to(args, log_println);
					NativeConsole::call_origin(args, worker->strs()->_log());
				});
				Js_Method(warn, {
					print_to(args, log_println_warn);
					NativeConsole::call_origin(args, worker->strs()->_warn());
				});
				Js_Method(error, {
					print_to(args, log_println_error);
					NativeConsole::call_origin(args, worker->strs()->_error());
				});
				Js_Method(clear, {
					log_fflush();
					NativeConsole::call_origin(args, worker->strs()->_clear());
				});
			} else {
				Js_Method(log, { print_to(args, log_println); });
				Js_Method(warn, { print_to(args, log_println_warn); });
				Js_Method(error, { print_to(args, log_println_error); });
				Js_Method(clear, { log_fflush(); });
				Js_Method(debug, {});
				Js_Method(info, {});
				Js_Method(dir, {});
				Js_Method(dirxml, {});
				Js_Method(table, {});
				Js_Method(trace, {});
				Js_Method(group, {});
				Js_Method(groupCollapsed, {});
				Js_Method(groupEnd, {});
				Js_Method(count, {});
				Js_Method(assert, {});
				Js_Method(markTimeline, {});
				Js_Method(profile, {});
				Js_Method(profileEnd, {});
				Js_Method(timeline, {});
				Js_Method(timelineEnd, {});
				Js_Method(time, {});
				Js_Method(timeEnd, {});
				Js_Method(timeStamp, {});
			}
		}
	};

	struct NativeTimer {
		static void timer_(FunctionArgs args, int64_t repeat, cChar* name, bool repeatArg) {
			Js_Worker(args);
			if (!args.length() || !args[0]->isFunction()) {
				Js_Throw(
					"@method %s(cb,time%s)\n"
					"@param cb {Function}\n"
					"@param [time] {Number}\n"
					"%s"
					"@return {Number}\n",
					name,
					repeatArg ? "[,repeat]": "",
					repeatArg ? "@param [repeat] {Number}\n": ""
				);
			}

			uint64_t timeout = 0;
			if (args.length() > 1 && args[1]->isNumber()) {
				timeout = Float64::max(args[1]->cast<JSNumber>()->value(), 0) * 1e3;
			}
			if (repeatArg) {
				if (args.length() > 2 && args[2]->isNumber()) {
					repeat = args[2]->cast<JSNumber>()->value();
				}
			}
			auto id = first_loop()->timer(get_callback_for_none(worker, args[0]), timeout, repeat);
			Js_Return(id);
		}

		static void clear_timer(Worker *worker, FunctionArgs args, cChar* name) {
			uint32_t id;
			if (!args.length() || !args[0]->asUint32(worker).to(id)) {
				Js_Throw(
					"@method %s(id)\n"
					"@param id {Number}\n", name
				);
			}
			first_loop()->timer_stop(id);
		}

		static void binding(JSObject* exports, Worker* worker) {
			Js_Method(setTimer, {
				timer_(args, 0, "setTimer", true);
			});
			Js_Method(setTimeout, {
				timer_(args, 0, "setTimeout", false);
			});
			Js_Method(setInterval, {
				timer_(args, -1, "setInterval", false);
			});
			Js_Method(setImmediate, {
				if (!args.length() || !args[0]->isFunction()) {
					Js_Throw(
						"@method setImmediate(cb)\n"
						"@param cb {Function}\n"
						"@return {Number}\n"
					);
				}
				auto id = first_loop()->timer(get_callback_for_none(worker, args[0]), 0, 0);
				Js_Return(id);
			});
			Js_Method(clearTimer, {
				clear_timer(worker, args, "clearTimer");
			});
			Js_Method(clearTimeout, {
				clear_timer(worker, args, "clearTimeout");
			});
			Js_Method(clearInterval, {
				clear_timer(worker, args, "clearInterval");
			});
			Js_Method(clearImmediate, {
				clear_timer(worker, args, "clearImmediate");
			});
		}
	};

	struct MixNativeObject: MixObject {
		static void binding(JSObject* exports, Worker* worker) {
			Js_Define_Class(Object, 0, {
				New<MixNativeObject>(args, new Object());
			});
		}
	};

	struct Hash5381Object: Object {
		Hash5381 hash;
	};

	struct MixHash5381Object: MixObject {
		static void binding(JSObject* exports, Worker* worker) {
			Js_Define_Class(Hash5381Object, 0, {
				New<MixHash5381Object>(args, new Hash5381Object());
			});

			Js_Class_Method(hashCode, {
				Js_Self(Hash5381Object);
				Js_Return( self->hash.hashCode() );
			});

			Js_Class_Method(update, {
				if (  args.length() < 1 ||
						!(args[0]->isString() || args[0]->isBuffer())
				) {
					Js_Throw("@method Hash5381.update(string|Buffer), Bad argument");
				}
				Js_Self(Hash5381Object);
				if ( args[0]->isString() ) { // string
					String2 str = args[0]->toString(worker)->value2(worker);
					self->hash.update(*str, str.length());
				}
				else { // Buffer
					WeakBuffer buff;
					args[0]->asBuffer(worker).to(buff);
					self->hash.update(*buff, buff.length());
				}
			});

			Js_Class_Method(digest, {
				Js_Self(Hash5381Object);
				Js_Return( self->hash.digest() );
			});

			Js_Class_Method(clear, {
				Js_Self(Hash5381Object);
				self->hash = Hash5381();
			});

			cls->exports("Hash5381", exports);
		}
	};

	void WorkerInl::initGlobalAPIs() {
		auto str = newStringOneByte("console");
		auto console = global()->get(this, str)->cast<JSObject>();
		auto isObject = console->isObject();
		if (!isObject) {
			global()->set(this, str, (console = newObject()));
		}
		_console.reset(this, console);
		NativeConsole::binding(console, this, isObject);
		NativeTimer::binding(global(), this);
	}

	struct NativeInit {
		static Hash5381 get_hashCode(Worker *worker, FunctionArgs args) {
			Hash5381 hash;
			String2 str = args[0]->toString(worker)->value2(worker);
			hash.update(*str, str.length());
			return hash;
		}

		static void binding(JSObject* exports, Worker* worker) {
			Qk_ASSERT(__quark_js_argv);
			auto argv = worker->newArray();
			for (uint32_t i = 0; i < __quark_js_argc; i++) {
				argv->set(worker, i, worker->newValue(String(__quark_js_argv[i])));
			}

			Js_Property(argv, argv);

			Js_Method(version, {
				Js_Return( qk::version() );
			});

			Js_Method(platform, {
				Js_Return( qk::platform() );
			});

			Js_Method(timeMonotonic, {
				Js_Return( time_monotonic() / 1e3 );
			});

			Js_Method(hashCode, {
				if (!args.length())
					Js_Throw("Bad argument");
				Js_Return( get_hashCode(worker, args).hashCode() );
			});

			Js_Method(hash, {
				if (!args.length()) {
					Js_Throw("Bad argument");
				}
				Js_Return( get_hashCode(worker, args).digest() );
			});

			Js_Method(nextTick, {
				if (!args.length() || !args[0]->isFunction())
					Js_Throw("@method nextTick(cb,...args), cb must be a function");
				first_loop()->tick(get_callback_for_none(worker, args[0]));
			});

			Js_Method(addNativeEventListener, {
				if ( args.length() < 3 || !args[0]->isObject() ||
						!args[1]->isString() || !args[2]->isFunction()) {
					Js_Throw("Bad argument");
				}
				if ( ! MixObject::mix(args[0]) ) {
					Js_Throw("Bad argument");
				}
				int id = 0;
				if ( args.length() > 3 && args[3]->isInt32() ) {
					id = args[3]->toInt32(worker)->value();
				}
				{ HandleScope scope(worker);
					MixObject* mix = MixObject::mix(args[0]);
					String name = args[1]->toString(worker)->value(worker);
					String func = String("_on").append(name).append("Native").append(String(id));
					bool ok = mix->addEventListener(name, func, id);
					if (ok) {
						mix->handle()->set(worker, worker->newStringOneByte(func), args[2]);
					}
					Js_ReturnBool(ok);
				}
			});

			Js_Method(removeNativeEventListener, {
				if ( args.length() < 2 || !args[0]->isObject() || !args[1]->isString()) {
					Js_Throw("Bad argument");
				}
				if ( ! MixObject::mix(args[0]) ) {
					Js_Throw("Bad argument");
				}
				int id = 0;
				if ( args.length() > 2 && args[2]->isInt32() ) {
					id = args[2]->toInt32(worker)->value();
				}
				{ HandleScope scope(worker);
					String name = args[1]->toString(worker)->value(worker);
					MixObject* mix = MixObject::mix(args[0]);
					bool ok = mix->removeEventListener(name, id);
					if ( ok ) {
						String func = String("_on").append(name).append("Native").append(String(id));
						mix->handle()->deleteFor(worker, worker->newStringOneByte(func) );
					}
					Js_ReturnBool(ok);
				}
			});

			Js_Method(garbageCollection, {
				args.worker()->garbageCollection();
			});

			Js_Method(runScript, {
				if (!args.length() || ! args[0]->isString()) {
					Js_Throw("Bad argument");
				}
				EscapableHandleScope scope(worker);
				JSString* name;
				JSObject* sandbox = nullptr;
				if (args.length() > 1) {
					name = args[1]->toString(worker);
				} else {
					name = worker->newStringOneByte("[eval]");
				}
				if (args.length() > 2 && args[2]->isObject()) {
					sandbox = args[2]->template cast<JSObject>();
				}
				auto rv = worker->runScript(args[0]->template cast<JSString>(), name, sandbox);
				if (rv) {
					Js_Return(scope.escape(rv));
				} // else js error
			});

			Js_Method(exit, {
				int code = 0;
				if (args.length() && args[0]->isInt32()) {
					code = args[0]->toInt32(worker)->value();
				}
				thread_exit(code);
			});

			Js_Method(runDebugger, {
				int port = 9229;
				String host_name = "127.0.0.1";
				String script_path = "";
				if (args.length())
					host_name = args[0]->toString(worker)->value(worker);
				if (args.length() > 1 && args[1]->isInt32())
					port = args[1]->toInt32(worker)->value();
				if (args.length() > 2)
					script_path = args[2]->toString(worker)->value(worker);
				runDebugger(worker, {false,port,host_name,script_path});
			});

			Js_Method(stopDebugger, {
				stopDebugger(worker);
			});

			Js_Method(debuggerBreakNextStatement, {
				debuggerBreakNextStatement(worker);
			});

			MixNativeObject::binding(exports, worker);
			MixHash5381Object::binding(exports, worker);
		}
	};

	Js_Module(_init, NativeInit);
} }
