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

// --- F u n c t i o n ---

MaybeLocal<Function> Function::New(Local<Context> context,
																	 FunctionCallback callback, Local<Value> data,
																	 int length, ConstructorBehavior behavior) {
	auto ft = FunctionTemplate::New(context->GetIsolate(),
																	callback, data,
																	Local<Signature>(), length, behavior);
	return ft->GetFunction();
}

Local<Function> Function::New(Isolate* v8_isolate, FunctionCallback callback,
															Local<Value> data, int length) {
	auto ft = FunctionTemplate::New(v8_isolate, callback, data, Local<Signature>(), length);
	return ft->GetFunction();
}

Local<v8::Object> Function::NewInstance() const {
	ENV();
	auto r = JSObjectCallAsConstructor(ctx, i::Back<JSObjectRef>(this), 0, 0,
																		 OK(Local<Object>()));
	return i::Cast<Object>(r);
}

MaybeLocal<Object> Function::NewInstance(Local<Context> context, int argc,
																				 v8::Local<v8::Value> argv[]) const {
	ENV(context->GetIsolate());
	auto r = JSObjectCallAsConstructor(ctx, i::Back<JSObjectRef>(this),
																		 argc, reinterpret_cast<JSValueRef*>(argv),
																		 OK(MaybeLocal<Object>()));
	return i::Cast<Object>(r);
}

Local<v8::Object> Function::NewInstance(int argc, v8::Local<v8::Value> argv[]) const {
	ENV();
	auto r = JSObjectCallAsConstructor(ctx, i::Back<JSObjectRef>(this),
																		 argc, reinterpret_cast<JSValueRef*>(argv),
																		 OK(Local<Object>()));
	return i::Cast<Object>(r);
}

MaybeLocal<v8::Value> Function::Call(Local<Context> context,
																		 v8::Local<v8::Value> recv, int argc,
																		 v8::Local<v8::Value> argv[]) {
	return CallAsFunction(context, recv, argc, argv);
}

Local<v8::Value> Function::Call(v8::Local<v8::Value> recv, int argc,
																v8::Local<v8::Value> argv[]) {
	return CallAsFunction(recv, argc, argv);
}

void Function::SetName(v8::Local<v8::String> name) {
	ENV();
	auto key = JSValueMakeString(ctx, i::name_s);
	PropertyDescriptor desc(name);
	DefineProperty(CONTEXT(isolate), i::Cast<String>(key), desc).FromJust();
}

Local<Value> Function::GetName() const {
	ENV();
	auto r = JSObjectGetProperty(ctx, i::Back<JSObjectRef>(this), i::name_s,
															 OK(Local<Value>()));
	return i::Cast(r);
}

Local<Value> Function::GetInferredName() const {
	return GetName();
}

Local<Value> Function::GetDebugName() const {
	return GetName();
}

Local<Value> Function::GetDisplayName() const {
	return GetName();
}

ScriptOrigin Function::GetScriptOrigin() const {
	return ScriptOrigin(i::Cast(ISOLATE()->Undefined()));
}

const int Function::kLineOffsetNotFound = -1;

int Function::GetScriptLineNumber() const {
	return 0;
}

int Function::GetScriptColumnNumber() const {
	return 0;
}

bool Function::IsBuiltin() const {
	return false;
}

int Function::ScriptId() const {
	return 0;
}

Local<v8::Value> Function::GetBoundFunction() const {
	return i::Cast(ISOLATE()->Undefined());
}

