/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright (c) 2015, xuewen.chu
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of xuewen.chu nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL xuewen.chu BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * ***** END LICENSE BLOCK ***** */

#include "./_json.h"
#include "../str.h"
#include "../_view.h"
#include "../../views2/view.h"

/**
 * @ns ftr::js
 */

JS_BEGIN

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

/**
 * @class JSON
 */
class InlJSON {
	uint32_t _indent;
	StringBuilder* _rv;
	Worker* worker;
	Persistent<JSSet> _set;
	
	void push_indent() {
		for (uint32_t i = 0; i < _indent; i++) {
			_rv->push(Space);
		}
	}
	
	bool stringify_object(Local<JSObject> arg) {
		_rv->push(LBrace);
		Local<JSArray> names = arg->GetPropertyNames(worker);
		if ( names->Length(worker) > 1 ) {
			_indent += 2;
			for (int i = 0, j = 0; i < names->Length(worker); i++) {
				Local<JSValue> key = names->Get(worker, i);
				if (j > 0) _rv->push(Comma); // ,
				_rv->push(Newline); push_indent();
				//  _rv->push(Quotes);
				_rv->push(key->ToStringValue(worker));
				//  _rv->push(Quotes);
				_rv->push(COLON); _rv->push(Space);
				bool rv = stringify( arg->Get(worker, key), false ); // value
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
	
	bool stringify_array(Local<JSArray> arg) {
		_rv->push(LBrack);
		if (arg->Length(worker) > 0) {
			_indent += 2;
			for (int i = 0; i < arg->Length(worker); i++) {
				if (i > 0)
					_rv->push(Comma);
				_rv->push(Newline);
				push_indent();
				stringify( arg->Get(worker, i), false ); // value
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
		uint8_t* s = (uint8_t*)buf.value();
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
	
	bool stringify_view(Local<JSObject> arg) {
		_rv->push(LBrace);
		Local<JSArray> names = arg->GetPropertyNames(worker);
		if ( names->Length(worker) > 0 ) {
			_indent += 2;
			for (int i = 0, j = 0; i < names->Length(worker); i++) {
				Local<JSValue> key = names->Get(worker, i);
				if (j > 0) _rv->push(Comma); // ,
				_rv->push(Newline); push_indent();
				//  _rv->push(Quotes);
				_rv->push(key->ToStringValue(worker));
				//  _rv->push(Quotes);
				_rv->push(COLON); _rv->push(Space);
				bool rv = stringify( arg->Get(worker, key), true ); // value
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
	
	bool stringify(Local<JSValue> arg, bool leaf) {
		if (arg.IsEmpty()) { // error
			return false;
		}
		
		bool rv = true;
		
		if(arg->IsString(worker)) {
			_rv->push(Quotes);
			_rv->push( arg->ToStringValue(worker) );
			_rv->push(Quotes);
		}
		else if (arg->IsFunction(worker)) {
			_rv->push( FUNCTION );
		}
		else if (arg->IsObject(worker)) {
			
			Local<JSObject> o = arg.To<JSObject>();
			if (arg->IsUint8Array(worker)) {
				rv = stringify_buffer(o->AsBuffer(worker));
			}
			else if (worker->values() && worker->values()->isBase(arg)) {
				_rv->push(Quotes);
				_rv->push( o->ToStringValue(worker) );
				_rv->push(Quotes);
			}
			else if (leaf) {
				if (arg->IsArray(worker)) {
					_rv->push(ARRAY);
				} else {
					_rv->push(OBJECT);
				}
			}
			else if (worker->hasInstance(arg, View::VIEW)) {
				rv = stringify_view(o);
			}
			else if ( arg->IsDate(worker) ) {
				_rv->push( arg->ToStringValue(worker) );
			}
			else {
				if ( _set.local()->Has(worker, o).FromMaybe(true) ) {
					_rv->push( Circular ); return true;
				}

				if ( _set.local()->Add(worker, o).IsEmpty() ) return false;
				
				if (arg->IsArray(worker)) {
					rv = stringify_array(o.To<JSArray>());
				} else {
					rv = stringify_object(o);
				}

				return _set.local()->Delete(worker, o).FromMaybe(false);
			}
		}
		else if(arg->IsInt32(worker)) {
			_rv->push( String(arg->ToInt32Value(worker)) );
		}
		else if(arg->IsNumber(worker)) {
			_rv->push( String(arg->ToNumberValue(worker)) );
		}
		else if(arg->IsBoolean(worker)) {
			if (arg->ToBooleanValue(worker)) {
				_rv->push(True);
			} else {
				_rv->push(False);
			}
		}
		else if(arg->IsDate(worker)) {
			_rv->push(Quotes);
			Local<JSFunction> f =
				arg.To<JSObject>()->Get(worker, worker->strs()->toJSON()).To<JSFunction>();
			_rv->push( f->Call(worker, 0, NULL, arg)->ToStringValue(worker) );
			_rv->push(Quotes);
		}
		else if(arg->IsNull(worker)) {
			_rv->push(Null);
		}
		else if(arg->IsUndefined(worker)) {
			_rv->push(Undefined);
		}
		return true;
	}
	
	public:
	InlJSON(Worker* worker) : _indent(0), _rv(NULL), worker(worker) {
		_set.Reset(worker, worker->NewSet());
	}
	
	~InlJSON() {
		_set.Reset();
	}

	bool stringify_console_styled(Local<JSValue> arg, StringBuilder* out) {
		HandleScope scope(worker);
		_rv = out;
		return stringify(arg, false);
	}
};

bool JSON::stringify_console_styled(Worker* worker, Local<JSValue> arg, StringBuilder* out) {
	return InlJSON(worker).stringify_console_styled(arg, out);
}

JS_END
