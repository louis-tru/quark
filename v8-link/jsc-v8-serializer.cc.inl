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


// --- J S O N ---

Local<Value> JSON::Parse(Local<String> json_string) {
	RETURN_TO_LOCAL_UNCHECKED(Parse(Isolate::GetCurrent(), json_string), Value);
}

MaybeLocal<Value> JSON::Parse(Isolate* iso, Local<String> json_string) {
	ENV(iso);
	JSValueRef argv[1] = { i::Back(json_string) };
	auto r = JSObjectCallAsFunction(ctx, isolate->JSONParse(), 0, 1, argv,
																	OK(MaybeLocal<Value>()));
	return i::Cast(r);
}

MaybeLocal<Value> JSON::Parse(Local<Context> context, Local<String> json_string) {
	return Parse(context->GetIsolate(), json_string);
}

MaybeLocal<String> JSON::Stringify(Local<Context> context, Local<Object> json_object,
																	 Local<String> gap) {
	ENV(context->GetIsolate());
	JSValueRef argv[1] = { i::Back(json_object) };
	auto r = JSObjectCallAsFunction(ctx, isolate->JSONStringify(), 0, 1, argv,
																	OK(MaybeLocal<String>()));
	return i::Cast<String>(r);
}

// ***************** UNIMPLEMENTED *****************

// --- V a l u e   S e r i a l i z a t i o n ---

v8_ns(internal)

class ValueSerializer {
 public:
	ValueSerializer(Isolate* isolate, v8::ValueSerializer::Delegate* delegate)
	: m_isolate(isolate), m_delegate(delegate) {}
 private:
	v8::ValueSerializer::Delegate* m_delegate;
	Isolate* m_isolate;
};

class ValueDeserializer {
 public:
	ValueDeserializer(Isolate* isolate, const uint8_t* data, size_t size,
										v8::ValueDeserializer::Delegate* delegate)
	: m_isolate(isolate), m_data(data), m_size(size), m_delegate(delegate) {}
 private:
	Isolate* m_isolate;
	const uint8_t* m_data;
	size_t m_size;
	v8::ValueDeserializer::Delegate* m_delegate;
	bool has_aborted = false;
	bool supports_legacy_wire_format = false;
};

v8_ns_end

Maybe<bool> ValueSerializer::Delegate::WriteHostObject(Isolate* v8_isolate,
																											 Local<Object> object) {
	ENV(v8_isolate);
	isolate->ThrowException(isolate->NewError("DataCloneError"));
	return Nothing<bool>();
}

Maybe<uint32_t> ValueSerializer::Delegate::GetSharedArrayBufferId(
	Isolate* v8_isolate, Local<SharedArrayBuffer> shared_array_buffer) {
	// ENV(v8_isolate);
	// isolate->ThrowException(isolate->NewError("DataCloneError"));
	// return Nothing<uint32_t>();
	//
	// TODO..
	// SharedArrayBuffer Discarded
	UNIMPLEMENTED();
	return Nothing<uint32_t>();
}

Maybe<uint32_t> ValueSerializer::Delegate::GetWasmModuleTransferId(
	Isolate* v8_isolate, Local<WasmCompiledModule> module) {
	return Nothing<uint32_t>();
}

void* ValueSerializer::Delegate::ReallocateBufferMemory(void* old_buffer,
																												size_t size,
																												size_t* actual_size) {
	*actual_size = size;
	return realloc(old_buffer, size);
}

void ValueSerializer::Delegate::FreeBufferMemory(void* buffer) {
	return free(buffer);
}

struct ValueSerializer::PrivateData {
	explicit PrivateData(i::Isolate* i, v8::ValueSerializer::Delegate* delegate)
	: serializer(i, delegate) {}
	i::ValueSerializer serializer;
};

ValueSerializer::ValueSerializer(Isolate* isolate)
: ValueSerializer(isolate, nullptr) {}

ValueSerializer::ValueSerializer(Isolate* isolate, Delegate* delegate)
: private_(new PrivateData(ISOLATE(isolate), delegate)) {}

ValueSerializer::~ValueSerializer() { delete private_; }

void ValueSerializer::WriteHeader() {
}

void ValueSerializer::SetTreatArrayBufferViewsAsHostObjects(bool mode) {}

Maybe<bool> ValueSerializer::WriteValue(Local<Context> context,
																				Local<Value> value) {
	return Just(false);
}

std::vector<uint8_t> ValueSerializer::ReleaseBuffer() {
	return std::vector<uint8_t>();
}

std::pair<uint8_t*, size_t> ValueSerializer::Release() {
	return std::pair<uint8_t*, size_t>();
}

void ValueSerializer::TransferArrayBuffer(uint32_t transfer_id,
																					Local<ArrayBuffer> array_buffer) {
}

void ValueSerializer::TransferSharedArrayBuffer(
	uint32_t transfer_id, Local<SharedArrayBuffer> shared_array_buffer) {
	// Discarded SharedArrayBuffer
}

void ValueSerializer::WriteUint32(uint32_t value) {
}

void ValueSerializer::WriteUint64(uint64_t value) {
}

void ValueSerializer::WriteDouble(double value) {
}

void ValueSerializer::WriteRawBytes(const void* source, size_t length) {
}

// --- V a l u e   D e s e r i a l i z e r ---

MaybeLocal<Object> ValueDeserializer::Delegate::ReadHostObject(Isolate* v8_isolate) {
	ENV(v8_isolate);
	isolate->ThrowException(isolate->NewError("DataCloneDeserializationError"));
	return MaybeLocal<Object>();
}

MaybeLocal<WasmCompiledModule> ValueDeserializer::Delegate::GetWasmModuleFromId(
	Isolate* v8_isolate, uint32_t id) {
	ENV(v8_isolate);
	isolate->ThrowException(isolate->NewError("DataCloneDeserializationError"));
	return MaybeLocal<WasmCompiledModule>();
}

struct ValueDeserializer::PrivateData {
	PrivateData(i::Isolate* isolate, const uint8_t* data, size_t size, Delegate* delegate)
		 : deserializer(isolate, data, size, delegate) {}
	i::ValueDeserializer deserializer;
};

ValueDeserializer::ValueDeserializer(Isolate* isolate, const uint8_t* data,
																		 size_t size)
: ValueDeserializer(isolate, data, size, nullptr) {}

ValueDeserializer::ValueDeserializer(Isolate* isolate, const uint8_t* data,
																		 size_t size, Delegate* delegate): private_(nullptr) {
	private_ = new PrivateData(ISOLATE(isolate), data, size, delegate);
}

ValueDeserializer::~ValueDeserializer() { delete private_; }

Maybe<bool> ValueDeserializer::ReadHeader(Local<Context> context) {
	return Just(false);
}

void ValueDeserializer::SetSupportsLegacyWireFormat(bool supports_legacy_wire_format) {
}

void ValueDeserializer::SetExpectInlineWasm(bool expect_inline_wasm) {
}

uint32_t ValueDeserializer::GetWireFormatVersion() const {
	return 0;
}

MaybeLocal<Value> ValueDeserializer::ReadValue(Local<Context> context) {
	auto r = ISOLATE(context->GetIsolate())->Empty();
	return i::Cast(r);
}

void ValueDeserializer::TransferArrayBuffer(uint32_t transfer_id,
																						Local<ArrayBuffer> array_buffer) {
}

void ValueDeserializer::TransferSharedArrayBuffer(
	uint32_t transfer_id, Local<SharedArrayBuffer> shared_array_buffer) {
	// Discarded SharedArrayBuffer
}

bool ValueDeserializer::ReadUint32(uint32_t* value) {
	return false;
}

bool ValueDeserializer::ReadUint64(uint64_t* value) {
	return false;
}

bool ValueDeserializer::ReadDouble(double* value) {
	return false;
}

bool ValueDeserializer::ReadRawBytes(size_t length, const void** data) {
	return false;
}

