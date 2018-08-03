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


// --- S c r i p t s ---

ScriptCompiler::CachedData::CachedData(const uint8_t* data, int length,
																			 BufferPolicy buffer_policy)
: data(data)
, length(length)
, rejected(false)
, buffer_policy(buffer_policy) {}

ScriptCompiler::CachedData::~CachedData() {
	if (buffer_policy == BufferOwned) {
		delete[] data;
	}
}

bool ScriptCompiler::ExternalSourceStream::SetBookmark() { return false; }

void ScriptCompiler::ExternalSourceStream::ResetToBookmark() { UNREACHABLE(); }

ScriptCompiler::StreamedSource::StreamedSource(ExternalSourceStream* stream,
																							 Encoding encoding) : impl_(nullptr) {
	UNIMPLEMENTED();
}

ScriptCompiler::StreamedSource::~StreamedSource() {}

const ScriptCompiler::CachedData* ScriptCompiler::StreamedSource::GetCachedData() const {
	return nullptr;
}

// --- U n b o u n d   S c r i p t ---

v8_ns(internal)

class UnboundScript: public Wrap {
 public:
	UnboundScript(Isolate* iso, ScriptOrigin* origin, JSStringRef source_code)
		: Wrap(iso), m_source_code(source_code)
	{
		ENV(iso);
		DCHECK(source_code);
		if (!origin->ResourceName().IsEmpty()) {
			m_source_url = Back(origin->ResourceName());
			Reference(m_source_url);
		}
		if (!origin->SourceMapUrl().IsEmpty()) {
			m_source_mapping_URL = Back(origin->SourceMapUrl());
			Reference(m_source_mapping_URL);
		}
		if (!origin->ResourceLineOffset().IsEmpty()) {
			m_resource_line_offset = JSValueToNumber(ctx, i::Back(origin->ResourceLineOffset()),
																							 OK());
		}
		JSStringRetain(m_source_code);
	}
	
	virtual ~UnboundScript() {
		JSStringRelease(m_source_code);
	}
	
	int GetId() {
		return 0;
	}
	
	int GetLineNumber(int code_pos) {
		return 0;
	}
	
	Local<Value> GetScriptName() {
		return i::Cast(m_source_url);
	}
	
	Local<Value> GetSourceURL() {
		return i::Cast(m_source_url);
	}
	
	Local<Value> GetSourceMappingURL() {
		return i::Cast(m_source_mapping_URL);
	}
	
 protected:
	JSValueRef  m_source_url = nullptr;
	JSValueRef  m_source_mapping_URL = nullptr;
	JSStringRef m_source_code = nullptr;
	int m_resource_line_offset = 0;
	friend class v8::UnboundScript;
};

class Script: public UnboundScript {
 public:
	inline Script(Isolate* isolate,
								Context* bound_context,
								ScriptOrigin* origin, JSStringRef source_code)
	: UnboundScript(isolate, origin, source_code), m_bound_context(bound_context) {
		Reference(m_bound_context);
	}
	
	JSValueRef Run(Context* context) {
		if (!context) {
			context = m_bound_context;
		}
		DCHECK(context->IsEnter());
		auto isolate = ISOLATE(context->GetIsolate());
		auto ctx = context->jscc();
		JSValueRef ex = nullptr;
		std::string stdSourceURL = std::string("[") + Isolate::ToSTDString(isolate, m_source_url) + "]";
		i::JSCStringPtr sourceURL = JSStringCreateWithUTF8CString(stdSourceURL.c_str());
		
		int startingLineNumber  = 1 + m_resource_line_offset;
		auto r = JSEvaluateScript(ctx, m_source_code, nullptr, *sourceURL, startingLineNumber, OK(0));
		return r;
	}
	
	inline UnboundScript* GetUnboundScript() {
		return this;
	}
	
	static Script* Compile(Context* context, JSValueRef source, ScriptOrigin* origin) {
		ENV(context->GetIsolate());
		auto sourceCode = JSValueToStringCopy(ctx, source, OK(0));
		auto s = new Script(context->GetIsolate(), context, origin, sourceCode);
		return s;
	}
	
 private:
	Context* m_bound_context;
};

v8_ns_end

Local<Script> UnboundScript::BindToCurrentContext() {
	auto us = reinterpret_cast<i::UnboundScript*>(this);
	ENV(us->GetIsolate());
	ScriptOrigin origin(i::Cast(us->m_source_url),
											us->m_resource_line_offset == 0 ? Local<Integer>() :
												i::Cast<Integer>(JSValueMakeNumber(ctx, us->m_resource_line_offset)),
											Local<Integer>(),
											Local<Boolean>(),
											Local<Integer>(),
											i::Cast(us->m_source_mapping_URL));
	auto s = new i::Script(us->GetIsolate(),
												 us->GetIsolate()->GetContext(), &origin, us->m_source_code);
	return i::Cast<Script>(s);
}

int UnboundScript::GetId() {
	return reinterpret_cast<i::UnboundScript*>(this)->GetId();
}

int UnboundScript::GetLineNumber(int code_pos) {
	return reinterpret_cast<i::UnboundScript*>(this)->GetLineNumber(code_pos);
}

Local<Value> UnboundScript::GetScriptName() {
	return reinterpret_cast<i::UnboundScript*>(this)->GetScriptName();
}

Local<Value> UnboundScript::GetSourceURL() {
	return reinterpret_cast<i::UnboundScript*>(this)->GetSourceURL();
}

Local<Value> UnboundScript::GetSourceMappingURL() {
	return reinterpret_cast<i::UnboundScript*>(this)->GetSourceMappingURL();
}

// --- S c r i p t ---

MaybeLocal<Value> Script::Run(Local<Context> context) {
	auto r = reinterpret_cast<i::Script*>(this)->Run(reinterpret_cast<i::Context*>(*context));
	return i::Cast(r);
}

Local<Value> Script::Run() {
	auto context = Isolate::GetCurrent()->GetCurrentContext();
	RETURN_TO_LOCAL_UNCHECKED(Run(context), Value);
}

Local<UnboundScript> Script::GetUnboundScript() {
	auto r = reinterpret_cast<i::Script*>(this)->GetUnboundScript();
	return i::Cast<UnboundScript>(r);
}

MaybeLocal<Script> Script::Compile(Local<Context> context, Local<String> source,
																	 ScriptOrigin* origin) {
	auto r = i::Script::Compile(reinterpret_cast<i::Context*>(*context),
															i::Back(source), origin);
	return i::Cast<Script>(r);
}

Local<Script> Script::Compile(v8::Local<String> source,
															v8::ScriptOrigin* origin) {
	auto context = Isolate::GetCurrent()->GetCurrentContext();
	RETURN_TO_LOCAL_UNCHECKED(Compile(context, source, origin), Script);
}

Local<Script> Script::Compile(v8::Local<String> source,
															v8::Local<String> file_name) {
	auto context = Isolate::GetCurrent()->GetCurrentContext();
	ScriptOrigin origin(file_name);
	RETURN_TO_LOCAL_UNCHECKED(Compile(context, source, &origin), Script);
}

// --- S c r i p t   C o m p i l e r ---

MaybeLocal<UnboundScript> ScriptCompiler::CompileUnboundInternal(
	Isolate* v8_isolate, Source* source, CompileOptions options) {
	auto source_string = i::Back(source->source_string);
	v8::ScriptOrigin origin(source->resource_name);
	auto script = i::Script::Compile(ISOLATE(v8_isolate)->GetContext(), source_string, &origin);
	return i::Cast<UnboundScript>(script);
}

MaybeLocal<Function> ScriptCompiler::CompileFunctionInContext(
	Local<Context> v8_context, Source* source, size_t arguments_count,
	Local<String> arguments[], size_t context_extension_count,
	Local<Object> context_extensions[]) {
	MaybeLocal<Function> err;
	DCHECK(reinterpret_cast<i::Context*>(*v8_context)->IsEnter());
	ENV(v8_context->GetIsolate());
	
	JSValueRef argv[3] = { JSValueMakeNumber(ctx, arguments_count) };
	JSObjectRef function_arguments = JSObjectMakeArray(ctx, 1, argv, OK(err));
	for (int i = 0; i < arguments_count; i++) {
		JSObjectSetPropertyAtIndex(ctx, function_arguments, i, i::Back(arguments[i]), OK(err));
	}
	argv[0] = function_arguments;
	argv[1] = JSValueMakeNumber(ctx, context_extension_count);
	argv[2] = i::Back(source->source_string);
	auto js = JSObjectCallAsFunction(ctx, isolate->wrapFunctionScript(), 0, 3, argv, OK(err));
	i::JSCStringPtr sourceCode = JSValueToStringCopy(ctx, js, OK(err));
	std::string stdSourceURL = std::string("[") + *v8::String::Utf8Value(source->resource_name) + "]";
	i::JSCStringPtr sourceURL = JSStringCreateWithUTF8CString(stdSourceURL.c_str());
	
	int startingLineNumber  = 1;
	if (!source->resource_line_offset.IsEmpty()) {
		startingLineNumber += JSValueToNumber(ctx, i::Back(source->resource_line_offset), OK(err));
	}
	auto f = (JSObjectRef)JSEvaluateScript(ctx, *sourceCode, nullptr, *sourceURL,
																				 startingLineNumber, OK(err));
	DCHECK(JSValueIsObject(ctx, f));
	DCHECK(JSObjectIsFunction(ctx, f));
	// bind context and context_extensions
	if (context_extension_count) {
		size_t argc = context_extension_count;
		UniquePtr<JSValueRef, DefaultTraits> argv2 = new JSValueRef[argc];
		memcpy(*argv2, context_extensions, sizeof(JSValueRef) * argc);
		f = (JSObjectRef)JSObjectCallAsFunction(ctx, f, nullptr, argc, *argv2, OK(err));
		DCHECK(JSValueIsObject(ctx, f));
		DCHECK(JSObjectIsFunction(ctx, f));
	}
	return i::Cast<Function>(f);
}

MaybeLocal<UnboundScript> ScriptCompiler::CompileUnboundScript(
	Isolate* v8_isolate, Source* source, CompileOptions options) {
	Utils::ApiCheck(!source->GetResourceOptions().IsModule(),
									"v8::ScriptCompiler::CompileUnboundScript",
									"v8::ScriptCompiler::CompileModule must be used to compile modules");
	return CompileUnboundInternal(v8_isolate, source, options);
}

Local<UnboundScript> ScriptCompiler::CompileUnbound(Isolate* v8_isolate,
																										Source* source,
																										CompileOptions options) {
	Utils::ApiCheck(!source->GetResourceOptions().IsModule(),
									"v8::ScriptCompiler::CompileUnbound",
									"v8::ScriptCompiler::CompileModule must be used to compile modules");
	RETURN_TO_LOCAL_UNCHECKED(CompileUnboundInternal(v8_isolate, source, options),
														UnboundScript);
}

MaybeLocal<Script> ScriptCompiler::Compile(Local<Context> context,
																					 Source* source,
																					 CompileOptions options) {
	Utils::ApiCheck(!source->GetResourceOptions().IsModule(), "v8::ScriptCompiler::Compile",
									"v8::ScriptCompiler::CompileModule must be used to compile modules");
	auto isolate = context->GetIsolate();
	auto maybe = CompileUnboundInternal(isolate, source, options);
	Local<UnboundScript> result;
	if (!maybe.ToLocal(&result)) return MaybeLocal<Script>();
	v8::Context::Scope scope(context);
	return result->BindToCurrentContext();
}

Local<Script> ScriptCompiler::Compile(Isolate* v8_isolate,
																			Source* source,
																			CompileOptions options) {
	auto context = v8_isolate->GetCurrentContext();
	RETURN_TO_LOCAL_UNCHECKED(Compile(context, source, options), Script);
}

Local<Function> ScriptCompiler::CompileFunctionInContext(
	Isolate* v8_isolate, Source* source, Local<Context> v8_context,
	size_t arguments_count, Local<String> arguments[],
	size_t context_extension_count, Local<Object> context_extensions[]) {
	RETURN_TO_LOCAL_UNCHECKED(
	 CompileFunctionInContext(v8_context, source, arguments_count, arguments,
														context_extension_count, context_extensions), Function
	);
}

// --- UNIMPLEMENTED ---

ScriptCompiler::ScriptStreamingTask* ScriptCompiler::StartStreamingScript(
	Isolate* v8_isolate, StreamedSource* source, CompileOptions options) {
	UNIMPLEMENTED();
}

MaybeLocal<Script> ScriptCompiler::Compile(Local<Context> context,
																					 StreamedSource* v8_source,
																					 Local<String> full_source_string,
																					 const ScriptOrigin& origin) {
	UNIMPLEMENTED();
}

Local<Script> ScriptCompiler::Compile(Isolate* v8_isolate,
																			StreamedSource* v8_source,
																			Local<String> full_source_string,
																			const ScriptOrigin& origin) {
	auto context = v8_isolate->GetCurrentContext();
	RETURN_TO_LOCAL_UNCHECKED(Compile(context, v8_source, full_source_string, origin), Script);
}

uint32_t ScriptCompiler::CachedDataVersionTag() {
	return 0;
}

MaybeLocal<Module> ScriptCompiler::CompileModule(Isolate* isolate, Source* source) {
	UNIMPLEMENTED();
}
