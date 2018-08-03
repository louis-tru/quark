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

// --- A l l o c a t o r ---

v8::ArrayBuffer::Allocator* v8::ArrayBuffer::Allocator::NewDefaultAllocator() {
	return nullptr;
}

void* v8::ArrayBuffer::Allocator::Reserve(size_t length) { UNIMPLEMENTED(); }

void v8::ArrayBuffer::Allocator::Free(void* data, size_t length, AllocationMode mode) {
	switch (mode) {
		case AllocationMode::kNormal: {
			Free(data, length);
			return;
		}
		case AllocationMode::kReservation: {
			UNIMPLEMENTED();
			return;
		}
	}
}

void v8::ArrayBuffer::Allocator::SetProtection(void* data, size_t length,
																							 v8::ArrayBuffer::Allocator::Protection protection) {
	UNIMPLEMENTED();
}

// --- A r r a y   B y f f e r ---

class BufferPrivateData: public i::PrivateData {
 public:
	inline BufferPrivateData(i::Isolate* isolate, bool external)
	: PrivateData(isolate, nullptr)
	, m_external(external) {}
	
	inline bool IsExternal() const { return m_external; }
	inline void MakeExternal() { m_external = true; }
	
	static BufferPrivateData* Private(i::Isolate* isolate, JSObjectRef value) {
		DCHECK(!isolate->HasDestroy());
		BufferPrivateData* priv = nullptr;
		auto ctx = JSC_CTX(isolate);
		JSObjectRef proto = value;
		do {
			proto = (JSObjectRef)JSObjectGetPrototype(ctx, proto);
			DCHECK(proto);
			if (JSValueIsObject(ctx, proto)) {
				priv = (BufferPrivateData*)JSObjectGetPrivate(proto);
				if (priv && priv->Handle() == value) {
					return priv;
				}
			} else {
				break;
			}
		} while(true);
		return priv;
	}
	
	static void ArrayBytesDeallocator(void* bytes, void* priv) {
		auto p = (BufferPrivateData*)priv;
		DCHECK(p);
		if (!p->IsExternal()) {
			free(bytes);
		}
	}
	
 private:
	bool m_external;
};

Local<ArrayBuffer> v8::ArrayBuffer::New(Isolate* iso, size_t byte_length) {
	ENV(iso);
	UniquePtr<void, CPointerTraits> bytes = malloc(byte_length);
	UniquePtr<BufferPrivateData> priv = new BufferPrivateData(isolate, false);
	auto bf = JSObjectMakeArrayBufferWithBytesNoCopy
	(
	 ctx,
	 *bytes, byte_length,
	 BufferPrivateData::ArrayBytesDeallocator,
	 *priv,
	 OK(Local<ArrayBuffer>())
	);
	auto bf_proto = JSObjectMake(ctx, i::CInfo::ObjectClass, *priv);
	DCHECK(bf_proto);
	priv->SetHandleNotBindPrivate(bf);
	JSObjectSetPrototype(ctx, bf_proto, JSObjectGetPrototype(ctx, bf));
	JSObjectSetPrototype(ctx, bf, bf_proto);
	bytes.collapse();
	priv.collapse();
	return i::Cast<ArrayBuffer>(bf);
}

Local<ArrayBuffer> v8::ArrayBuffer::New(Isolate* iso, void* data,
																				size_t byte_length,
																				ArrayBufferCreationMode mode) {
	ENV(iso);
	if (data == nullptr) {
		byte_length = 0;
	}
	bool externalized = (mode == ArrayBufferCreationMode::kExternalized);
	UniquePtr<BufferPrivateData> priv = new BufferPrivateData(isolate, externalized);
	auto bf = JSObjectMakeArrayBufferWithBytesNoCopy
	(
	 ctx,
	 data, byte_length,
	 BufferPrivateData::ArrayBytesDeallocator,
	 *priv,
	 OK(Local<ArrayBuffer>())
	);
	auto bf_proto = JSObjectMake(ctx, i::CInfo::ObjectClass, *priv);
	DCHECK(bf_proto);
	priv->SetHandleNotBindPrivate(bf);
	JSObjectSetPrototype(ctx, bf_proto, JSObjectGetPrototype(ctx, bf));
	JSObjectSetPrototype(ctx, bf, bf_proto);
	priv.collapse();
	return i::Cast<ArrayBuffer>(bf);
}

bool v8::ArrayBuffer::IsExternal() const {
	auto priv = BufferPrivateData::Private(ISOLATE(), i::Back<JSObjectRef>(this));
	if (priv) {
		return priv->IsExternal();
	}
	return false;
}

v8::ArrayBuffer::Contents v8::ArrayBuffer::Externalize() {
	Contents contents;
	auto priv = BufferPrivateData::Private(ISOLATE(), i::Back<JSObjectRef>(this));
	if (priv) {
		ENV(priv->GetIsolate());
		auto len = JSObjectGetArrayBufferByteLength(ctx, i::Back<JSObjectRef>(this), OK(contents));
		auto ptr = JSObjectGetArrayBufferBytesPtr(ctx, i::Back<JSObjectRef>(this), OK(contents));
		priv->MakeExternal();
		contents.data_ = ptr;
		contents.byte_length_ = len;
	}
	return contents;
}

v8::ArrayBuffer::Contents v8::ArrayBuffer::GetContents() {
	ENV();
	Contents contents;
	auto len = JSObjectGetArrayBufferByteLength(ctx, i::Back<JSObjectRef>(this), OK(contents));
	auto ptr = JSObjectGetArrayBufferBytesPtr(ctx, i::Back<JSObjectRef>(this), OK(contents));
	contents.data_ = ptr;
	contents.byte_length_ = len;
	return contents;
}

bool v8::ArrayBuffer::IsNeuterable() const {
	// UNIMPLEMENTED();
	return false;
}

void v8::ArrayBuffer::Neuter() {
	// UNIMPLEMENTED();
}

size_t v8::ArrayBuffer::ByteLength() const {
	ENV();
	auto r = JSObjectGetArrayBufferByteLength(ctx, i::Back<JSObjectRef>(this), OK(0));
	return r;
}

Local<ArrayBuffer> v8::ArrayBufferView::Buffer() {
	ENV();
	auto r = JSObjectGetTypedArrayBuffer(ctx, i::Back<JSObjectRef>(this),
																			 OK(Local<ArrayBuffer>()));
	return i::Cast<ArrayBuffer>(r);
}

size_t v8::ArrayBufferView::CopyContents(void* dest, size_t byte_length) {
	ENV();
	auto ptr = JSObjectGetTypedArrayBytesPtr(ctx, i::Back<JSObjectRef>(this), OK(0));
	auto len = JSObjectGetTypedArrayByteLength(ctx, i::Back<JSObjectRef>(this), OK(0));
	byte_length = MIN(len, byte_length);
	memcpy(dest, ptr, byte_length);
	return byte_length;
}

bool v8::ArrayBufferView::HasBuffer() const {
	return true;
}

size_t v8::ArrayBufferView::ByteOffset() {
	ENV();
	auto offset = JSObjectGetTypedArrayByteOffset(ctx, i::Back<JSObjectRef>(this), OK(0));
	return offset;
}

size_t v8::ArrayBufferView::ByteLength() {
	ENV();
	auto len = JSObjectGetTypedArrayByteLength(ctx, i::Back<JSObjectRef>(this), OK(0));
	return len;
}

size_t v8::TypedArray::Length() {
	ENV();
	auto len = JSObjectGetTypedArrayLength(ctx, i::Back<JSObjectRef>(this), OK(0));
	return len;
}

template<class TypedArray, JSTypedArrayType Type, class ArrayBuffer>
static Local<TypedArray> TypedArrayNew(Local<ArrayBuffer> array_buffer,
																			 size_t byte_offset, size_t length) {
	ENV();
	auto r = JSObjectMakeTypedArrayWithArrayBufferAndOffset
	(
	 ctx, Type,
	 i::Back<JSObjectRef>(array_buffer), 
	 byte_offset, length,
	 OK(Local<TypedArray>())
	);
	return i::Cast<TypedArray>(r);
}

//Local<Uint8Array> Uint8Array::New(Local<ArrayBuffer> array_buffer,
//                                    size_t byte_offset, size_t length) {
//  ENV();
//  Local<Uint8Array> err;
//  JSValueRef argv[3] = {
//    i::Back(array_buffer),
//    JSValueMakeNumber(ctx, byte_offset),
//    JSValueMakeNumber(ctx, length),
//  };
//  auto r = JSObjectCallAsConstructor(ctx, isolate->Uint8Array(), 3, argv, OK(err));
//  //size_t len = JSObjectGetTypedArrayLength(ctx, r, OK(err));
//  //size_t len2 = JSObjectGetTypedArrayByteLength(ctx, r, OK(err));
//  return i::Cast<Uint8Array>(r);
//}

#define TYPED_ARRAY_NEW(Type, typeName, TYPE, ctype, size)  \
Local<Type##Array> Type##Array::New(Local<ArrayBuffer> array_buffer, \
																		size_t byte_offset, size_t length) { \
	return TypedArrayNew<Type##Array,\
						kJSTypedArrayType##Type##Array>(array_buffer, byte_offset, length); \
} \
Local<Type##Array> Type##Array::New(Local<SharedArrayBuffer> array_buffer, \
																		size_t byte_offset, size_t length) { \
	return TypedArrayNew<Type##Array, \
						kJSTypedArrayType##Type##Array>(array_buffer, byte_offset, length); \
}

#define TYPED_ARRAY_NEW2(Type, typeName, TYPE, ctype, size) \
Local<Type##Array> Type##Array::New(Local<ArrayBuffer> array_buffer,  \
																	 size_t byte_offset, size_t length) {  \
	ENV(); \
	Local<Type##Array> err; \
	JSValueRef argv[3] = { \
		i::Back(array_buffer), \
		JSValueMakeNumber(ctx, byte_offset), \
		JSValueMakeNumber(ctx, length), \
	}; \
	auto r = JSObjectCallAsConstructor(ctx, isolate->Type##Array(), 3, argv, OK(err)); \
	return i::Cast<Type##Array>(r); \
} \
Local<Type##Array> Type##Array::New(Local<SharedArrayBuffer> array_buffer, \
																	size_t byte_offset, size_t length) { \
	ENV(); \
	Local<Type##Array> err; \
	JSValueRef argv[3] = { \
		i::Back(array_buffer), \
		JSValueMakeNumber(ctx, byte_offset), \
		JSValueMakeNumber(ctx, length), \
	}; \
	auto r = JSObjectCallAsConstructor(ctx, isolate->Type##Array(), 3, argv, OK(err)); \
	return i::Cast<Type##Array>(r); \
}

TYPED_ARRAYS(TYPED_ARRAY_NEW2)

#undef TYPED_ARRAY_NEW

Local<DataView> DataView::New(Local<ArrayBuffer> array_buffer,
															size_t byte_offset, size_t byte_length) {
	// ENV();
	// JSValueRef argv[3] = {
	//   i::Back(array_buffer),
	//   JSValueMakeNumber(ctx, byte_offset),
	//   JSValueMakeNumber(ctx, byte_length),
	// };
	// auto r = JSObjectCallAsConstructor(ctx, isolate->DataView(), 3, argv, OK(Local<DataView>()));
	// return i::Cast<DataView>(r);
	//
	// TODO..
	UNIMPLEMENTED();
	return Local<DataView>();
}

Local<DataView> DataView::New(Local<SharedArrayBuffer> shared_array_buffer,
															size_t byte_offset, size_t byte_length) {
	// ENV();
	// JSValueRef argv[3] = {
	//   i::Back(shared_array_buffer),
	//   JSValueMakeNumber(ctx, byte_offset),
	//   JSValueMakeNumber(ctx, byte_length),
	// };
	// auto r = JSObjectCallAsConstructor(ctx, isolate->DataView(), 3, argv, OK(Local<DataView>()));
	// return i::Cast<DataView>(r);
	//
	// TODO..
	// SharedArrayBuffer Discarded
	UNIMPLEMENTED();
	return Local<DataView>();
}

bool v8::SharedArrayBuffer::IsExternal() const {
	// return false;
	//
	// TODO..
	// SharedArrayBuffer Discarded
	UNIMPLEMENTED();
	return false;
}

v8::SharedArrayBuffer::Contents v8::SharedArrayBuffer::Externalize() {
	// return Contents();
	//
	// TODO..
	// SharedArrayBuffer Discarded
	UNIMPLEMENTED();
	return Contents();
}

v8::SharedArrayBuffer::Contents v8::SharedArrayBuffer::GetContents() {
	// ENV();
	// Contents contents;
	// auto len = JSObjectGetArrayBufferByteLength(ctx, i::Back<JSObjectRef>(this), OK(contents));
	// auto ptr = JSObjectGetArrayBufferBytesPtr(ctx, i::Back<JSObjectRef>(this), OK(contents));
	// contents.data_ = ptr;
	// contents.byte_length_ = len;
	// return contents;
	//
	// TODO..
	// SharedArrayBuffer Discarded
	UNIMPLEMENTED();
	return Contents();
}

size_t v8::SharedArrayBuffer::ByteLength() const {
	//ENV();
	//auto len = JSObjectGetTypedArrayByteLength(ctx, i::Back<JSObjectRef>(this), OK(0));
	// return len;
	//
	// TODO..
	// SharedArrayBuffer Discarded
	UNIMPLEMENTED();
	return 0;
}

Local<SharedArrayBuffer> v8::SharedArrayBuffer::New(Isolate* iso, size_t byte_length) {
	// ENV(iso);
	// if (JSValueIsUndefined(ctx, isolate->SharedArrayBuffer())) {
	//   return Local<SharedArrayBuffer>();
	// }
	// JSValueRef argv[1] = {
	//   JSValueMakeNumber(ctx, byte_length),
	// };
	// auto r = JSObjectCallAsConstructor(ctx, isolate->SharedArrayBuffer(), 1, argv,
	//                                    OK(Local<SharedArrayBuffer>()));
	// return i::Cast<SharedArrayBuffer>(r);
	//
	// TODO..
	// SharedArrayBuffer Discarded
	UNIMPLEMENTED();
	return Local<SharedArrayBuffer>();
}

Local<SharedArrayBuffer> v8::SharedArrayBuffer::New(
	Isolate* iso, void* data, size_t byte_length, ArrayBufferCreationMode mode) {
	UNIMPLEMENTED();
}

