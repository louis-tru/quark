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

	class JSONStringify {
	public:
		uint32_t _indent;
		Array<String>* _rv;
		Worker* worker;
		Persistent<JSSet> _set;

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
					_rv->push(key->toStringValue(worker));
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
			if (!arg) return false; // error
			bool rv = true;

			if(arg->isString()) {
				_rv->push('"');
				_rv->push( arg->toStringValue(worker) );
				_rv->push('"');
			}
			else if (arg->isFunction()) {
				_rv->push( "[Function]" );
			}
			else if (arg->isObject()) {
				JSObject* o = arg->as<JSObject>();
				if (o->has(worker, worker->strs()->toStringStyled())) {
					auto indent = worker->newInstance(_indent)->as();
					auto toStringStyled = o->get(worker, worker->strs()->toStringStyled());
					auto str = toStringStyled->as<JSFunction>()->call(worker, 1, &indent, o);
					if (!str)
						return false; // error
					_rv->push( str->toStringValue(worker) );
				}
				else if (arg->isUint8Array()) {
					rv = stringify_buffer(o->asBuffer(worker));
				}
				else if ( arg->isDate() ) {
					_rv->push( arg->toStringValue(worker) );
				}
				else {
					if ( _set->has(worker, o) ) {
						_rv->push( "[Circular]" );
						return true;
					}
					_set->add(worker, o);

					if (arg->isArray()) {
						rv = stringify_array(o->as<JSArray>());
					} else {
						rv = stringify_object(o);
					}
					return _set->Delete(worker, o);
				}
			}
			else if(arg->isInt32()) {
				_rv->push( String(arg->toInt32Value(worker).unsafe()) );
			}
			else if(arg->isNumber()) {
				_rv->push( String(arg->toNumberValue(worker).unsafe()) );
			}
			else if(arg->isBoolean()) {
				if (arg->toBooleanValue(worker)) {
					_rv->push("true");
				} else {
					_rv->push("false");
				}
			}
			else if(arg->isDate()) {
				_rv->push('"');
				auto f =
					arg->as<JSObject>()->get(worker, worker->strs()->toJSON())->as<JSFunction>();
				_rv->push( f->call(worker, 0, nullptr, arg)->toStringValue(worker) );
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

	class NativeConsole {
	public:
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
					rv.push( args[i]->toStringValue(worker) );
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
			auto f = console->get(worker, name)->as<JSFunction>();
			f->call(worker, argv.length(), *argv, console);
		}

		static void binding(JSObject* exports, Worker* worker, bool isOrigin) {
			if (isOrigin) {
				Js_Set_Property(_log, exports->getProperty(worker, "log"));
				Js_Set_Property(_warn, exports->getProperty(worker, "warn"));
				Js_Set_Property(_error, exports->getProperty(worker, "error"));
				Js_Set_Property(_clear, exports->getProperty(worker, "clear"));
				Js_Set_Method(log, {
					print_to(args, log_println);
					NativeConsole::call_origin(args, worker->strs()->_log());
				});
				Js_Set_Method(warn, {
					print_to(args, log_println_warn);
					NativeConsole::call_origin(args, worker->strs()->_warn());
				});
				Js_Set_Method(error, {
					print_to(args, log_println_error);
					NativeConsole::call_origin(args, worker->strs()->_error());
				});
				Js_Set_Method(clear, {
					log_fflush();
					NativeConsole::call_origin(args, worker->strs()->_clear());
				});
			} else {
				Js_Set_Method(log, { print_to(args, log_println); });
				Js_Set_Method(warn, { print_to(args, log_println_warn); });
				Js_Set_Method(error, { print_to(args, log_println_error); });
				Js_Set_Method(clear, { log_fflush(); });
				Js_Set_Method(debug, {});
				Js_Set_Method(info, {});
				Js_Set_Method(dir, {});
				Js_Set_Method(dirxml, {});
				Js_Set_Method(table, {});
				Js_Set_Method(trace, {});
				Js_Set_Method(group, {});
				Js_Set_Method(groupCollapsed, {});
				Js_Set_Method(groupEnd, {});
				Js_Set_Method(count, {});
				Js_Set_Method(assert, {});
				Js_Set_Method(markTimeline, {});
				Js_Set_Method(profile, {});
				Js_Set_Method(profileEnd, {});
				Js_Set_Method(timeline, {});
				Js_Set_Method(timelineEnd, {});
				Js_Set_Method(time, {});
				Js_Set_Method(timeEnd, {});
				Js_Set_Method(timeStamp, {});
			}
		}
	};

	class NativeTimer {
	public:
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
				timeout = Float64::max(args[1]->toNumberValue(worker).unsafe(), 0) * 1e3;
			}
			if (repeatArg) {
				if (args.length() > 2 && args[2]->isNumber()) {
					repeat = args[2]->toNumberValue(worker).unsafe();
				}
			}
			auto id = first_loop()->timer(get_callback_for_none(worker, args[0]), timeout, repeat);
			Js_Return(id);
		}

		static void clear_timer(Worker *worker, FunctionArgs args, cChar* name) {
			if (!args.length() || !args[0]->isUint32()) {
				Js_Throw(
					"@method %s(id)\n"
					"@param id {Number}\n", name
				);
			}
			first_loop()->timer_stop(args[0]->toUint32Value(worker).unsafe());
		}

		static void binding(JSObject* exports, Worker* worker) {
			Js_Set_Method(setTimer, {
				timer_(args, 0, "setTimer", true);
			});
			Js_Set_Method(setTimeout, {
				timer_(args, 0, "setTimeout", false);
			});
			Js_Set_Method(setInterval, {
				timer_(args, -1, "setInterval", false);
			});
			Js_Set_Method(setImmediate, {
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
			Js_Set_Method(clearTimer, {
				clear_timer(worker, args, "clearTimer");
			});
			Js_Set_Method(clearTimeout, {
				clear_timer(worker, args, "clearTimeout");
			});
			Js_Set_Method(clearInterval, {
				clear_timer(worker, args, "clearInterval");
			});
			Js_Set_Method(clearImmediate, {
				clear_timer(worker, args, "clearImmediate");
			});
		}
	};

	class WrapNativeObject: public WrapObject {
	public:
		static void binding(JSObject* exports, Worker* worker) {
			Js_Define_Class(Object, 0, {
				New<WrapNativeObject>(args, new Object());
			});
		}
	};

	struct Hash5381Object: Object {
		Hash5381 hash;
	};

	class WrapHash5381Object: public WrapObject {
	public:
		static void binding(JSObject* exports, Worker* worker) {
			Js_Define_Class(Hash5381Object, 0, {
				New<WrapHash5381Object>(args, new Hash5381Object());
			});

			Js_Set_Class_Method(hashCode, {
				Js_Self(Hash5381Object);
				Js_Return( self->hash.hashCode() );
			});

			Js_Set_Class_Method(update, {
				if (  args.length() < 1 ||
						!(args[0]->isString() || args[0]->isBuffer())
				) {
					Js_Throw("@method Hash5381.update(string|Buffer), Bad argument");
				}
				Js_Self(Hash5381Object);
				if ( args[0]->isString() ) { // string
					String2 str = args[0]->toStringValue2(worker);
					self->hash.update(*str, str.length());
				}
				else { // Buffer
					auto buff = args[0]->asBuffer(worker);
					self->hash.update(*buff, buff.length());
				}
			});

			Js_Set_Class_Method(digest, {
				Js_Self(Hash5381Object);
				Js_Return( self->hash.digest() );
			});

			Js_Set_Class_Method(clear, {
				Js_Self(Hash5381Object);
				self->hash = Hash5381();
			});

			cls->exports("Hash5381", exports);
		}
	};

	void WorkerInl::initGlobalAPIs() {
		auto str = newStringOneByte("console");
		auto console = global()->get(this, str)->as<JSObject>();
		auto isObject = console->isObject();
		if (!isObject) {
			global()->set(this, str, (console = newObject()));
		}
		_console.reset(this, console);
		NativeConsole::binding(console, this, isObject);
		NativeTimer::binding(global(), this);
	}

	class NativeInit {
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
			for (uint32_t i = 0; i < __quark_js_argc; i++) {
				argv->set(worker, i, worker->newInstance(String(__quark_js_argv[i])));
			}

			Js_Set_Property(argv, argv);

			Js_Set_Method(version, {
				Js_Return( qk::version() );
			});

			Js_Set_Method(platform, {
				Js_Return( qk::platform() );
			});

			Js_Set_Method(timeMonotonic, {
				Js_Return( time_monotonic() / 1e3 );
			});

			Js_Set_Method(hashCode, {
				if (!args.length())
					Js_Throw("Bad argument");
				Js_Return( get_hashCode(worker, args).hashCode() );
			});

			Js_Set_Method(hash, {
				if (!args.length()) {
					Js_Throw("Bad argument");
				}
				Js_Return( get_hashCode(worker, args).digest() );
			});

			Js_Set_Method(nextTick, {
				if (!args.length() || !args[0]->isFunction())
					Js_Throw("@method nextTick(cb,...args), cb must be a function");
				first_loop()->tick(get_callback_for_none(worker, args[0]));
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
				if ( args.length() > 3 && args[3]->isInt32() ) {
					id = args[3]->toInt32Value(worker).unsafe();
				}
				{ HandleScope scope(worker);
					WrapObject* wrap = WrapObject::wrap(args[0]);
					String name = args[1]->toStringValue(worker,true);
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
				if ( args.length() > 2 && args[2]->isInt32() ) {
					id = args[2]->toInt32Value(worker).unsafe();
				}
				{ HandleScope scope(worker);
					String name = args[1]->toStringValue(worker,true);
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
			});

			Js_Set_Method(runScript, {
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
					sandbox = args[2]->template as<JSObject>();
				}
				auto rv = worker->runScript(args[0]->template as<JSString>(), name, sandbox);
				if (rv) {
					Js_Return(scope.escape(rv));
				} // else js error
			});

			Js_Set_Method(exit, {
				int code = 0;
				if (args.length() && args[0]->isInt32()) {
					code = args[0]->toInt32Value(worker).unsafe();
				}
				thread_exit(code);
			});

			Js_Set_Method(runDebugger, {
				int port = 9229;
				String host_name = "127.0.0.1";
				String script_path = "";
				if (args.length())
					host_name = args[0]->toStringValue(worker);
				if (args.length() > 1 && args[1]->isInt32())
					port = args[1]->toInt32Value(worker).unsafe();
				if (args.length() > 2)
					script_path = args[2]->toStringValue(worker);
				runDebugger(worker, {false,port,host_name,script_path});
			});

			Js_Set_Method(stopDebugger, {
				stopDebugger(worker);
			});

			Js_Set_Method(debuggerBreakNextStatement, {
				debuggerBreakNextStatement(worker);
			});

			WrapNativeObject::binding(exports, worker);
			WrapHash5381Object::binding(exports, worker);
		}
	};

	Js_Set_Module(_init, NativeInit);
} }