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

#include "ftr/util/util.h"
#include "ftr/util/string.h"
#include <v8.h>
#include <libplatform/libplatform.h>
#include <sstream>
#include <istream>

using namespace v8;
namespace n = ftr;

MaybeLocal<v8::Value> run_script(Isolate* isolate, v8::Local<v8::String> source_string,
																 v8::Local<v8::String> name,
																 v8::Local<v8::Object> sandbox = v8::Local<v8::Object>()) {
	Local<Context> context = isolate->GetCurrentContext();
	ScriptOrigin origin(name);
	ScriptCompiler::Source source(source_string, ScriptOrigin(name));
	MaybeLocal<Value> result;
	
	if ( sandbox.IsEmpty() ) { // use default sandbox
		Local<Script> script;
		if ( ScriptCompiler::Compile(context, &source).ToLocal(&script) ) {
			result = script->Run(context);
		}
	} else {
		Local<Function> func;
		if (ScriptCompiler::CompileFunctionInContext(context, &source, 0, NULL, 1, &sandbox)
				.ToLocal(&func)
				) {
			result = func->Call(context, Undefined(isolate), 0, NULL);
		}
	}
	return result;
}

MaybeLocal<v8::Value> run_script(Isolate* isolate, n::cString& source, n::cString& name,
																 Local<Object> sandbox = Local<Object>()) {
	return run_script(isolate,
										String::NewFromUtf8(isolate, *source),
										String::NewFromUtf8(isolate, *name),
										sandbox);
}

void test_persistent(Isolate* isolate, Local<Context> ctx) {
	
	Local<Number> num = Number::New(isolate, 100);
	
	 // new
	Persistent<Number> p1(isolate, num);
	Persistent<Number> p2(isolate, num);
	
	 // copy
	Persistent<Number, CopyablePersistentTraits<Number>> p3(p1);
	Persistent<Number, CopyablePersistentTraits<Number>> p4 = p2;
	
	Local<Number> num1 = *reinterpret_cast<Local<Number>*>(&p1);
	Local<Number> num2 = *reinterpret_cast<Local<Number>*>(&p2);
	Local<Number> num3 = *reinterpret_cast<Local<Number>*>(&p3);
	Local<Number> num4 = *reinterpret_cast<Local<Number>*>(&p4);
	
	LOG(num1->Value());
	LOG(num2->Value());
	LOG(num3->Value());
	LOG(num4->Value());

	LOG("OK");
}

void test_template(Isolate* isolate, Local<Context> ctx) {
	auto global = ctx->Global();
	auto num_s = String::NewFromUtf8(isolate, "num");
	auto num = Integer::New(isolate, 100);
	// Function Template
	auto ft = FunctionTemplate::New(isolate, [](const FunctionCallbackInfo<Value>& info){
		LOG(info.Data()->NumberValue());
		LOG("OK.1");
	}, num);
	ft->PrototypeTemplate()->Set(isolate, "a", num);
	ft->Set(isolate, "num", num);
	
	// Object Template
	auto ot = ObjectTemplate::New(isolate, ft);
	//  ot->SetCallAsFunctionHandler([](const FunctionCallbackInfo<Value>& info){
	//    LOG(info.Data()->NumberValue());
	//    LOG("OK.2");
	//  }, num);
	auto o = ot->NewInstance();
	auto o3 = ot->NewInstance();
	LOG(o->GetPrototype()->StrictEquals(o3->GetPrototype()));
	LOG(o->StrictEquals(o3));
	LOG(o->IsFunction());
	if (o->IsFunction())
		o->CallAsFunction(ctx, Undefined(isolate), 0, 0).ToLocalChecked();
	auto c = o->Get(ctx, String::NewFromUtf8(isolate, "constructor")).ToLocalChecked();
	LOG(c->IsFunction());
	c.As<Object>()->CallAsFunction(ctx, Undefined(isolate), 0, 0).ToLocalChecked();
	LOG(o->Has(ctx, String::NewFromUtf8(isolate, "a")).ToChecked());
	LOG(o->HasOwnProperty(ctx, String::NewFromUtf8(isolate, "a")).ToChecked());
	LOG(o->Get(ctx, String::NewFromUtf8(isolate, "a")).ToLocalChecked()->NumberValue());
	
	// Object Template
	auto ot2 = ObjectTemplate::New(isolate);
	ot2->Set(isolate, "num", num);
	auto o2 = ot2->NewInstance();
	LOG(o2->IsFunction());
	auto c2 = o2->Get(ctx, String::NewFromUtf8(isolate, "constructor")).ToLocalChecked();
	LOG(c2->IsFunction());
	auto O = global->Get(ctx, String::NewFromUtf8(isolate, "Object")).ToLocalChecked();
	LOG(c2->StrictEquals(O));
	c2.As<Object>()->CallAsFunction(ctx, Undefined(isolate), 0, 0).ToLocalChecked();
	LOG(o2->HasOwnProperty(ctx, num_s).ToChecked());
	LOG(o2->HasOwnProperty(ctx, String::NewFromUtf8(isolate, "constructor")).ToChecked());
	
	// Function
	auto f = ft->GetFunction(ctx).ToLocalChecked();
	ft->Set(String::NewFromUtf8(isolate, "k"), num);
	ft->PrototypeTemplate()->Set(String::NewFromUtf8(isolate, "j"), num);
	auto f2 = ft->GetFunction(ctx).ToLocalChecked();
	LOG(f->IsFunction());
	LOG(f->StrictEquals(f2));
	LOG(f->Get(String::NewFromUtf8(isolate, "prototype"))->
			StrictEquals(f2->Get(String::NewFromUtf8(isolate, "prototype"))));
	auto f_i = f->CallAsConstructor(0, 0).As<Object>();
	auto f2_i = f2->CallAsConstructor(0, 0).As<Object>();
	LOG(f_i->Get(String::NewFromUtf8(isolate, "j"))->NumberValue());
	LOG(f2_i->Get(String::NewFromUtf8(isolate, "j"))->NumberValue());
	LOG(f->Get(String::NewFromUtf8(isolate, "k"))->NumberValue());
	LOG(f2->Get(String::NewFromUtf8(isolate, "k"))->NumberValue());
	LOG(f2->Has(ctx, num_s).ToChecked());
	LOG(f->Has(ctx, num_s).ToChecked());
	LOG(f->Get(ctx, num_s).ToLocalChecked()->NumberValue());
	
	// run script
	global->Set(String::NewFromUtf8(isolate, "test"), ft->GetFunction());
	run_script(isolate, "test()", "test.js");
}

void test_v8(int argc, char **argv) {
	
	v8::Platform* platform = v8::platform::CreateDefaultPlatform();
	v8::V8::InitializePlatform(platform);
	v8::V8::Initialize();
	
	Isolate::CreateParams params;
	params.array_buffer_allocator = v8::ArrayBuffer::Allocator::NewDefaultAllocator();
	Isolate* isolate = v8::Isolate::New(params);
	{
		Locker locker(isolate);
		isolate->Enter();
		{
			HandleScope scope(isolate);
			Local<Context> context = v8::Context::New(isolate);
			context->Enter();
			// ...
			test_template(isolate, context);
			test_persistent(isolate, context);
			context->Exit();
		}
		isolate->Exit();
	}
	isolate->Dispose();
	v8::V8::ShutdownPlatform();
	v8::V8::Dispose();
	delete platform;
}
