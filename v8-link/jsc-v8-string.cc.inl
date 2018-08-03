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


// --- S t r i n g ---

Local<String> String::NewFromUtf8(Isolate* isolate,
																	const char* data,
																	NewStringType type, int length) {
	if (length == 0) {
		return i::Cast<String>(reinterpret_cast<i::Isolate*>(isolate)->Empty());
	}
	else if (length > 0 && data[length] != '\0') {
		char* tmp = (char*)malloc(length + 1);
		memcpy(tmp, data, length);
		tmp[length] = '\0';
		auto r = JSValueMakeString(JSC_CTX(isolate), JSStringCreateWithUTF8CString(tmp));
		free(tmp);
		return i::Cast<String>(r);
	}
	else {
		auto r = JSValueMakeString(JSC_CTX(isolate), JSStringCreateWithUTF8CString(data));
		return i::Cast<String>(r);
	}
}

MaybeLocal<String> String::NewFromUtf8(Isolate* isolate,
																			 const char* data,
																			 v8::NewStringType type, int length) {
	return NewFromUtf8(isolate, (const char*)data, NewStringType(type), length);
}

Local<String> String::NewFromOneByte(Isolate* isolate,
																		 const uint8_t* data,
																		 NewStringType type, int length) {
	return NewFromUtf8(isolate, (const char*)data, NewStringType(type), length);
}

MaybeLocal<String> String::NewFromOneByte(Isolate* isolate,
																					const uint8_t* data,
																					v8::NewStringType type, int length) {
	return NewFromUtf8(isolate, (const char*)data, NewStringType(type), length);
}

Local<String> String::NewFromTwoByte(Isolate* isolate,
																		 const uint16_t* data,
																		 NewStringType type, int length) {
	if (length == -1) {
		length = 0;
		while (data[length]) {
			length++;
		}
	}
	auto r = JSValueMakeString(JSC_CTX(isolate), JSStringCreateWithCharacters(data, length));
	return i::Cast<String>(r);
}

MaybeLocal<String> String::NewFromTwoByte(Isolate* isolate,
																					const uint16_t* data,
																					v8::NewStringType type, int length) {
	return NewFromTwoByte(isolate, data, NewStringType(type), length);
}

Local<String> v8::String::Concat(Local<String> left, Local<String> right) {
	ENV();
	JSValueRef argv[2] = { i::Back(left), i::Back(right), };
	auto r = JSObjectCallAsFunction(ctx, isolate->stringConcat(), 0, 2, argv,
																	OK(Local<String>()));
	return i::Cast<String>(r);
}

MaybeLocal<String> v8::String::NewExternalTwoByte(
Isolate* isolate, v8::String::ExternalStringResource* resource) {
	auto s = NewFromTwoByte(isolate, resource->data(),
													v8::NewStringType::kNormal, int(resource->length()));
	resource->Dispose();
	return s;
}

Local<String> v8::String::NewExternal(
Isolate* isolate, v8::String::ExternalStringResource* resource) {
	auto s = NewFromTwoByte(isolate, resource->data(),
													v8::NewStringType::kNormal, int(resource->length()));
	resource->Dispose();
	return s.FromMaybe(Local<String>());
}

MaybeLocal<String> v8::String::NewExternalOneByte(
Isolate* isolate, v8::String::ExternalOneByteStringResource* resource) {
	auto s = NewFromOneByte(isolate,
													(const uint8_t*)resource->data(),
													v8::NewStringType::kNormal, int(resource->length()));
	resource->Dispose();
	return s;
}

Local<String> v8::String::NewExternal(
Isolate* isolate, v8::String::ExternalOneByteStringResource* resource) {
	return NewExternalOneByte(isolate, resource).FromMaybe(Local<String>());
}

bool v8::String::MakeExternal(v8::String::ExternalStringResource* resource) {
	return false;
}

bool v8::String::MakeExternal(v8::String::ExternalOneByteStringResource* resource) {
	return false;
}

bool v8::String::CanMakeExternal() {
	return false;
}

v8::String::Utf8Value::Utf8Value(v8::Local<v8::Value> obj)
: str_(NULL), length_(0) {
	ENV();
	i::JSCStringPtr s = JSValueToStringCopy(ctx, i::Back(obj), OK());
	size_t bufferSize = JSStringGetMaximumUTF8CStringSize(*s);
	str_ = (char*)malloc(bufferSize);
	length_ = (int)JSStringGetUTF8CString(*s, str_, bufferSize) - 1;
}

v8::String::Utf8Value::~Utf8Value() {
	free(str_);
}

v8::String::Value::Value(v8::Local<v8::Value> obj) : str_(NULL), length_(0) {
	ENV();
	i::JSCStringPtr s = JSValueToStringCopy(ctx, i::Back(obj), OK());
	length_ = (int)JSStringGetLength(*s);
	str_ = (uint16_t*)malloc(sizeof(uint16_t) * length_);
	memcpy(str_, JSStringGetCharactersPtr(*s), sizeof(uint16_t) * length_);
}

v8::String::Value::~Value() {
	free(str_);
}

int v8::String::Length() const {
	ENV();
	auto s = i::Back(this);
	auto l = JSObjectCallAsFunction(ctx, isolate->stringLength(), 0, 1, &s, OK(0));
	int r = JSValueToNumber(ctx, l, OK(0));
	return r;
}

bool v8::String::IsOneByte() const {
	return false;
}

bool v8::String::ContainsOnlyOneByte() const {
	return false;
}


// ============ Utf-8 编码的范围 ==============
//  1 | 0000 0000 - 0000 007F |                                              0xxxxxxx
//  2 | 0000 0080 - 0000 07FF |                                     110xxxxx 10xxxxxx
//  3 | 0000 0800 - 0000 FFFF |                            1110xxxx 10xxxxxx 10xxxxxx
//  4 | 0001 0000 - 0010 FFFF |                   11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
//  5 | 0020 0000 - 03FF FFFF |          111110xx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
//  6 | 0400 0000 - 7FFF FFFF | 1111110x 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
// ===========================================
// 单个unicode转换成utf8的长度
static int encoding_utf8_char_length(uint32_t unicode) {
	if (unicode < 0x7F + 1) {               // 单字节编码
		return 1;
	}
	else {
		if (unicode < 0x7FF + 1) {            // 两字节编码
			return 2;
		}
		else if (unicode < 0xFFFF + 1) {      // 三字节编码
			return 3;
		}
		else if (unicode < 0x10FFFF + 1) {    // 四字节编码
			return 4;
		}
		else if (unicode < 0x3FFFFFF + 1) {   // 五字节编码
			if (unicode > 0x200000 - 1) {
				return 5;
			}
			else { // 这个区间没有编码
				return 0;
			}
		}
		else {                                //六字节编码
			return 6;
		}
		return 0;
	}
}

// 单个unicode转换到utf-8编码
static uint encoding_unicode_to_utf8_char(uint unicode, char* s) {
	uint rev = 1;
	
	if (unicode < 0x7F + 1) {             // 单字节编码
		*s = unicode;
	}
	else {
		int length;
		if (unicode < 0x7FF + 1) {            // 两字节编码
			rev = 2;
			*s = 0b11000000;
		}
		else if (unicode < 0xFFFF + 1) {      // 三字节编码
			rev = 3;
			*s = 0b11100000;
		}
		else if (unicode < 0x10FFFF + 1) {    // 四字节编码
			rev = 4;
			*s = 0b11110000;
		}
		else if (unicode < 0x3FFFFFF + 1) {   // 五字节编码
			if (unicode > 0x200000 - 1) {
				rev = 5;
				*s = 0b11111000;
			}
			else { // 这个区间没有编码
				return 0;
			}
		}
		else {                               //六字节编码
			rev = 6;
			*s = 0b11111100;
		}
		for (int i = rev - 1; i > 0; i--) {
			s[i] = 0b10000000 | (unicode & 0b00111111);
			unicode >>= 6;
		}
		s[0] |= unicode;
	}
	return rev;
}

static uint32_t encoding_utf8_str_length(const uint16_t* source, size_t len) {
	uint32_t rev = 0;
	const uint16_t* end = source + len;
	while (source < end) {
		rev += encoding_utf8_char_length(*source);
		source++;
	}
	return rev;
}

int v8::String::Utf8Length() const {
	ENV();
	i::JSCStringPtr s = JSValueToStringCopy(ctx, i::Back(this), OK(0));
	int len = encoding_utf8_str_length(JSStringGetCharactersPtr(*s), JSStringGetLength(*s));
	return len;
}

int v8::String::WriteUtf8(char* buffer, int capacity, int* nchars_ref, int options) const {
	ENV();
	i::JSCStringPtr s = JSValueToStringCopy(ctx, i::Back(this), OK(0));
	size_t len = JSStringGetLength(*s);
	const JSChar* ch = JSStringGetCharactersPtr(*s);
	
	if (capacity == -1)
		capacity = std::numeric_limits<int>::max();
	if (nchars_ref)
		*nchars_ref = Length();
	
	int size = 0;
	char out[8];
	for (int i = 0; i < len; i++) {
		uint count = encoding_unicode_to_utf8_char(ch[i], out);
		if (capacity >= count) {
			if (count == 1) {
				buffer[size] = out[0];
			} else {
				memcpy(buffer + size, out, count);
			}
			size += count;
			capacity -= count;
		} else { // Lack of space
			break;
		}
	}
	if (capacity > 0)
		buffer[size] = '\0';
	
	return size;
}

template<class T>
static int StringWrite(const String* self, T* buffer, int start, int length, int options) {
	ENV();
	i::JSCStringPtr s = JSValueToStringCopy(ctx, i::Back(self), OK(0));
	int len = (int)JSStringGetLength(*s);
	auto ch = JSStringGetCharactersPtr(*s);
	int end = length == -1 ? len : MIN(length + start, len);
	
	if (start >= 0 && start < len) {
		len = end - start;
		do {
			*buffer = ch[start];
			start++; buffer++;
		} while(start < end);
		return len;
	} else {
		return 0;
	}
}

int v8::String::WriteOneByte(uint8_t* buffer, int start, int length, int options) const {
	return StringWrite(this, buffer, start, length, options);
}

int String::Write(uint16_t* buffer, int start, int length, int options) const {
	return StringWrite(this, buffer, start, length, options);
}

bool v8::String::IsExternal() const {
	return false;
}

bool String::IsExternalOneByte() const {
	return false;
}

void v8::String::VerifyExternalStringResource(v8::String::ExternalStringResource* value) const {
}

void v8::String::VerifyExternalStringResourceBase(
	v8::String::ExternalStringResourceBase* value,
	Encoding encoding) const {
}

const v8::String::ExternalOneByteStringResource*
	v8::String::GetExternalOneByteStringResource() const {
}
