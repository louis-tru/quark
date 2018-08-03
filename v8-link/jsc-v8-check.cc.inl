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


// --- C h e c k ---

void External::CheckCast(v8::Value* that) {
	Utils::ApiCheck(that->IsExternal(), "v8::External::Cast",
									"Could not convert to external");
}

void v8::Object::CheckCast(Value* that) {
	Utils::ApiCheck(that->IsObject(), "v8::Object::Cast",
									"Could not convert to object");
}

void v8::Function::CheckCast(Value* that) {
	Utils::ApiCheck(that->IsFunction(), "v8::Function::Cast",
									"Could not convert to function");
}

void v8::Boolean::CheckCast(v8::Value* that) {
	Utils::ApiCheck(that->IsBoolean(), "v8::Boolean::Cast",
									"Could not convert to boolean");
}

void v8::Name::CheckCast(v8::Value* that) {
	Utils::ApiCheck(that->IsName(), "v8::Name::Cast", "Could not convert to name");
}

void v8::String::CheckCast(v8::Value* that) {
	Utils::ApiCheck(that->IsString(), "v8::String::Cast",
									"Could not convert to string");
}

void v8::Symbol::CheckCast(v8::Value* that) {
	Utils::ApiCheck(that->IsSymbol(), "v8::Symbol::Cast",
									"Could not convert to symbol");
}

void v8::Number::CheckCast(v8::Value* that) {
	Utils::ApiCheck(that->IsNumber(),
									"v8::Number::Cast()",
									"Could not convert to number");
}

void v8::Integer::CheckCast(v8::Value* that) {
	Utils::ApiCheck(that->IsNumber(), "v8::Integer::Cast",
									"Could not convert to number");
}

void v8::Int32::CheckCast(v8::Value* that) {
	Utils::ApiCheck(that->IsInt32(), "v8::Int32::Cast",
									"Could not convert to 32-bit signed integer");
}

void v8::Uint32::CheckCast(v8::Value* that) {
	Utils::ApiCheck(that->IsUint32(), "v8::Uint32::Cast",
									"Could not convert to 32-bit unsigned integer");
}

void v8::Array::CheckCast(Value* that) {
	Utils::ApiCheck(that->IsArray(), "v8::Array::Cast",
									"Could not convert to array");
}

void v8::Map::CheckCast(Value* that) {
		Utils::ApiCheck(that->IsMap(), "v8::Map::Cast", "Could not convert to Map");
}

void v8::Set::CheckCast(Value* that) {
	Utils::ApiCheck(that->IsSet(), "v8_Set_Cast", "Could not convert to Set");
}

void v8::Promise::CheckCast(Value* that) {
	Utils::ApiCheck(that->IsPromise(), "v8::Promise::Cast",
					 "Could not convert to promise");
}

void v8::Promise::Resolver::CheckCast(Value* that) {
	Utils::ApiCheck(that->IsPromise(), "v8::Promise::Resolver::Cast",
					 "Could not convert to promise resolver");
}

void v8::Proxy::CheckCast(Value* that) {
	Utils::ApiCheck(that->IsProxy(), "v8::Proxy::Cast",
					 "Could not convert to proxy");
}

void v8::WasmCompiledModule::CheckCast(Value* that) {
	Utils::ApiCheck(that->IsWebAssemblyCompiledModule(),
									"v8::WasmCompiledModule::Cast",
									"Could not convert to wasm compiled module");
}

void v8::ArrayBuffer::CheckCast(Value* that) {
	Utils::ApiCheck(that->IsArrayBuffer(),
									"v8::ArrayBuffer::Cast()", "Could not convert to ArrayBuffer");
}

void v8::ArrayBufferView::CheckCast(Value* that) {
	Utils::ApiCheck(that->IsArrayBufferView(),
									"v8::ArrayBufferView::Cast()",
									"Could not convert to ArrayBufferView");
}

void v8::TypedArray::CheckCast(Value* that) {
	Utils::ApiCheck(that->IsTypedArray(),
									"v8::TypedArray::Cast()",
									"Could not convert to TypedArray");
}

#define CHECK_TYPED_ARRAY_CAST(Type, typeName, TYPE, ctype, size)             \
	void v8::Type##Array::CheckCast(Value* that) {                              \
		ENV();                                                                    \
		bool ok = JSValueIsInstanceOfConstructor(ctx,                             \
								i::Back(that), isolate->Type##Array(), &ex);                  \
		Utils::ApiCheck(!ok || ex,                                                \
				"v8::" #Type "Array::Cast()", "Could not convert to " #Type "Array"); \
}

TYPED_ARRAYS(CHECK_TYPED_ARRAY_CAST)

#undef CHECK_TYPED_ARRAY_CAST


void v8::DataView::CheckCast(Value* that) {
	Utils::ApiCheck(that->IsDataView(),
									"v8::DataView::Cast()",
									"Could not convert to DataView");
}

void v8::SharedArrayBuffer::CheckCast(Value* that) {
	Utils::ApiCheck(that->IsSharedArrayBuffer(),
									"v8::SharedArrayBuffer::Cast()",
									"Could not convert to SharedArrayBuffer");
}

void v8::Date::CheckCast(v8::Value* that) {
	Utils::ApiCheck(that->IsDate(),
									"v8::Date::Cast()",
									"Could not convert to date");
}

void v8::StringObject::CheckCast(v8::Value* that) {
	Utils::ApiCheck(that->IsString(),
									"v8::StringObject::Cast()",
									"Could not convert to StringObject");
}

void v8::SymbolObject::CheckCast(v8::Value* that) {
	Utils::ApiCheck(that->IsSymbolObject(),
									"v8::SymbolObject::Cast()",
									"Could not convert to SymbolObject");
}

void v8::NumberObject::CheckCast(v8::Value* that) {
	Utils::ApiCheck(that->IsNumberObject(),
									"v8::NumberObject::Cast()",
									"Could not convert to NumberObject");
}

void v8::BooleanObject::CheckCast(v8::Value* that) {
	Utils::ApiCheck(that->IsBooleanObject(),
									"v8::BooleanObject::Cast()",
									"Could not convert to BooleanObject");
}

void v8::RegExp::CheckCast(v8::Value* that) {
	Utils::ApiCheck(that->IsRegExp(),
									"v8::RegExp::Cast()",
									"Could not convert to regular expression");
}
