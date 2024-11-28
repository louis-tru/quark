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

#ifndef __quark__js__jsc__macros__
#define __quark__js__jsc__macros__

#define Js_Const_List(F) \
	F(__native__) \
	F(prototype) \
	F(set) \
	F(get) \
	F(configurable) \
	F(enumerable) \
	F(writable) \
	F(length) \
	F(name) \
	F(column) \
	F(line) \
	F(message) \
	F(stack) \
	F(_stack) \
	F(constructor) \
	F(parent) \
	F(data) \
	F(value) \
	F(result) \
	F(source) \
	F(flags) \
	F(undefined) \
	F(null) \
	F(object) \
	F(boolean) \
	F(string) \
	F(number) \
	F(size) \
	F(scriptId) \
	F(isWasm) \
	F(functionName) \
	F(scriptName) \
	F(sourceMapUrl) \
	F(programFunc) \
	F(isEval) \
	F(isConstructor) \
	F(prototype_instance) \
	F(function_instance) \
	F(instance_template) \
	F(prototype_template) \
	F(toString) \

#define Js_Worker_Data_List(F) \
	F(getPropertyNames) \
	F(getOwnPropertyNames) \
	F(getOwnPropertyNames2) \
	F(getOwnPropertyDescriptor) \
	F(getOwnPropertyDescriptors) \
	F(defineProperty) \
	F(defineProperties) \
	F(hasOwnProperty) \
	F(isSymbol) \
	F(promiseState) \
	F(promiseCatch) \
	F(promiseThen) \
	F(stringLength) \
	F(typeOf) \
	F(valueOf) \
	F(newRegExp) \
	F(mixFunctionScript) \
	F(mapAsArray) \
	F(setAsArray) \
	F(newPrivateValue) \
	F(symbolName) \
	F(NativeToString) \
	F(Function_prototype) \
	F(symbol_for_api) \
	F(private_for_api) \
	F(JSONStringify) \
	F(JSONParse) \
	F(mapSet) \
	F(mapGet) \
	F(mapClear) \
	F(mapHas) \
	F(mapDelete) \
	F(setClear) \
	F(setAdd) \
	F(setHas) \
	F(setDelete) \
	F(symbolFor) \
	F(stringConcat) \
	F(getProperty) \
	F(setProperty) \
	F(deleteProperty) \
	F(hasProperty) \

#define Js_Context_Data_List(F) \
	F(Object) \
	F(Function) \
	F(Number) \
	F(Boolean) \
	F(String) \
	F(Date) \
	F(RegExp) \
	F(Error) \
	F(RangeError) \
	F(ReferenceError) \
	F(SyntaxError) \
	F(TypeError) \
	F(Map) \
	F(Set) \
	F(WeakMap) \
	F(WeakSet) \
	F(Symbol) \
	F(Proxy) \
	F(Promise) \
	F(DataView) \
	/*F(SharedArrayBuffer)*/ \
	F(ArrayBuffer) \
	F(TypedArray) \
	F(Uint8Array) \
	F(Int8Array) \
	F(Uint16Array) \
	F(Int16Array) \
	F(Uint32Array) \
	F(Int32Array) \
	F(Float32Array) \
	F(Float64Array) \
	F(Uint8ClampedArray) \
	F(AsyncFunction) \
	F(SetIterator) \
	F(MapIterator) \

#endif