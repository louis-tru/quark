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

// @private head

#ifndef __quark__js__jsc__macros__
#define __quark__js__jsc__macros__

#define Js_Const_Strings(F) \
	F(prototype) \
	F(constructor) \
	F(set) \
	F(get) \
	F(name) \
	F(Error) \
	F(line) \
	F(column) \
	F(scriptId) \
	F(functionName) \
	F(scriptName) \
	F(sourceURL) \
	F(isEval) \
	F(isConstructor) \
	F(isWasm) \
	F(stack) \
	F(_stack) \
	F(length) \
	F(errno) \
	F(message) \
	F(toString) \
	F(enumerable) \
	F(configurable) \

#define Js_Worker_Data_Each(F) \
	F(Object, global) \
	F(Error, global) \
	F(RangeError, global) \
	F(ReferenceError, global) \
	F(SyntaxError, global) \
	F(TypeError, global) \
	F(Function, global) \
	F(ArrayBuffer, global) \
	F(Uint8Array, global) \
	F(Set, global) \
	F(prototype, global_Set) \
	F(add, global_Set_prototype) \
	F(has, global_Set_prototype) \
	F(delete, global_Set_prototype) \
	F(Date, global) \
	F(prototype, global_Date) \
	F(valueOf, global_Date_prototype) \
	F(prototype, global_Object) \
	F(defineProperty, global_Object) \
	F(prototype, global_Function) \

#endif
