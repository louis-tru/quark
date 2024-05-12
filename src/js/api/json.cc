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
#include "../types.h"
#include "../../ui/view_prop.h"

namespace qk { namespace js {

	static cString Undefined("undefined");
	static cString Null("null");
	static cString True("true");
	static cString False("false");
	static cString Circular("[Circular]");
	static cString Newline('\n');
	static cString Comma(',');
	static cString Space(' ');
	static cString LBrack('[');
	static cString RBrack(']');
	static cString LBrace('{');
	static cString RBrace('}');
	static cString COLON(':');
	static cString ELLIPSIS(" ... ");
	static cString Quotes('"');
	static cString BufferPrefix("<Buffer");
	static cString GT(">");
	static cString FUNCTION("[Function]");
	static cString ARRAY("[Array]");
	static cString OBJECT("[Object]");

	class JSONStringify {
		uint32_t _indent;
		Array<String>* _rv;
		Worker* worker;
		Persistent<JSSet> _set;

		void push_indent() {
			for (uint32_t i = 0; i < _indent; i++) {
				_rv->push(Space);
			}
		}

		bool stringify_object(JSObject* arg) {
			_rv->push(LBrace);
			JSArray* names = arg->getPropertyNames(worker);
			if ( names->length() > 1 ) {
				_indent += 2;
				for (int i = 0, j = 0; i < names->length(); i++) {
					JSValue* key = names->get(worker, i);
					if (j > 0) _rv->push(Comma); // ,
					_rv->push(Newline); push_indent();
					//  _rv->push(Quotes);
					_rv->push(key->toStringValue(worker));
					//  _rv->push(Quotes);
					_rv->push(COLON); _rv->push(Space);
					bool rv = stringify( arg->get(worker, key), false ); // value
					if ( ! rv ) return false;
					j++;
				}
				_indent -= 2;
				_rv->push(Newline); push_indent();
			} else {
				_rv->push(Space);
			}
			_rv->push(RBrace);
			return true;
		}

		bool stringify_array(JSArray* arg) {
			_rv->push(LBrack);
			if (arg->length() > 0) {
				_indent += 2;
				for (int i = 0; i < arg->length(); i++) {
					if (i > 0)
						_rv->push(Comma);
					_rv->push(Newline);
					push_indent();
					stringify( arg->get(worker, i), false ); // value
				}
				_indent -= 2;
				_rv->push(Newline);
				push_indent();
			} else {
				_rv->push(Space);
			}
			_rv->push(RBrack);
			return true;
		}

		bool stringify_buffer(WeakBuffer buf) {
			_rv->push(BufferPrefix);
			cChar* hex = "0123456789abcdef";
			uint8_t* s = (uint8_t*)*buf;
			for (uint32_t i = 0; i < buf.length(); i++) {
				uint8_t ch = s[i];
				_rv->push(Space);
				_rv->push( hex[ch >> 4] );
				_rv->push( hex[ch & 15] );
				if (i > 50) {
					_rv->push(ELLIPSIS); break;
				}
			}
			_rv->push(GT);
			return true;
		}

		bool stringify_view(JSObject* arg) {
			_rv->push(LBrace);
			JSArray* names = arg->getPropertyNames(worker);
			if ( names->length() > 0 ) {
				_indent += 2;
				for (int i = 0, j = 0; i < names->length(); i++) {
					JSValue* key = names->get(worker, i);
					if (j > 0) _rv->push(Comma); // ,
					_rv->push(Newline); push_indent();
					//  _rv->push(Quotes);
					_rv->push(key->toStringValue(worker));
					//  _rv->push(Quotes);
					_rv->push(COLON); _rv->push(Space);
					bool rv = stringify( arg->get(worker, key), true ); // value
					if ( ! rv ) return false;
					j++;
				}
				_indent -= 2;
				_rv->push(Newline); push_indent();
			} else {
				_rv->push(Space);
			}
			_rv->push(RBrace);
			return true;
		}

		bool stringify(JSValue* arg, bool leaf) {
			if (!arg) { // error
				return false;
			}
			
			bool rv = true;
			
			if(arg->isString()) {
				_rv->push(Quotes);
				_rv->push( arg->toStringValue(worker) );
				_rv->push(Quotes);
			}
			else if (arg->isFunction()) {
				_rv->push( FUNCTION );
			}
			else if (arg->isObject()) {
				
				JSObject* o = arg->cast<JSObject>();
				if (arg->isUint8Array()) {
					rv = stringify_buffer(o->asBuffer(worker));
				}
				else if (worker->types() && worker->types()->isTypesBase(o)) {
					_rv->push(Quotes);
					_rv->push( o->toStringValue(worker) );
					_rv->push(Quotes);
				}
				else if (leaf) {
					if (arg->isArray()) {
						_rv->push(ARRAY);
					} else {
						_rv->push(OBJECT);
					}
				}
				else if (worker->instanceOf(arg, kView_ViewType)) {
					rv = stringify_view(o);
				}
				else if ( arg->isDate() ) {
					_rv->push( arg->toStringValue(worker) );
				}
				else {
					if ( _set->has(worker, o) ) {
						_rv->push( Circular );
						return true;
					}

					_set->add(worker, o);

					if (arg->isArray()) {
						rv = stringify_array(o->cast<JSArray>());
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
					_rv->push(True);
				} else {
					_rv->push(False);
				}
			}
			else if(arg->isDate()) {
				_rv->push(Quotes);
				auto f =
					arg->cast<JSObject>()->get(worker, worker->strs()->toJSON())->cast<JSFunction>();
				_rv->push( f->call(worker, 0, nullptr, arg)->toStringValue(worker) );
				_rv->push(Quotes);
			}
			else if(arg->isNull()) {
				_rv->push(Null);
			}
			else if(arg->isUndefined()) {
				_rv->push(Undefined);
			}
			return true;
		}
		
	public:
		JSONStringify(Worker* worker) : _indent(0), _rv(NULL), worker(worker) {
			_set.reset(worker, worker->newSet());
		}
		
		~JSONStringify() {
			_set.reset();
		}

		bool stringify_console_styled(JSValue* arg, Array<String>* out) {
			HandleScope scope(worker);
			_rv = out;
			return stringify(arg, false);
		}
	};

	bool stringifyConsoleStyled(Worker* worker, JSValue* arg, Array<String>* out) {
		return JSONStringify(worker).stringify_console_styled(arg, out);
	}

} }