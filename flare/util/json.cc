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

#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include "./json.h"
#include "./error.h"
#include "./log.h"

namespace flare {

	typedef rapidjson::CrtAllocator MemoryPoolAllocator;
	typedef       rapidjson::GenericValue<rapidjson::UTF8<>, MemoryPoolAllocator> RValue;
	typedef const rapidjson::GenericValue<rapidjson::UTF8<>, MemoryPoolAllocator> CRValue;
	typedef       rapidjson::GenericDocument<rapidjson::UTF8<>, MemoryPoolAllocator> RDocument;
	//typedef const rapidjson::GenericDocument<rapidjson::UTF8<>, MemoryPoolAllocator> CRDocument;

	static MemoryPoolAllocator shareMemoryPoolAllocator;
	static const JSON NullValue;

	JSON::JSON() {
		new(this)RValue();
	}

	JSON::JSON(Type type) {
		new(this)RValue(rapidjson::Type(type));
	}

	JSON::JSON(bool b) {
		new(this)RValue(b);
	}

	//! Constructor for int value.
	JSON::JSON(int i) {
		new(this)RValue(i);
	}

	//! Constructor for unsigned value.
	JSON::JSON(uint32_t u) {
		new(this)RValue(u);
	}

	//! Constructor for int64_t_t value.
	JSON::JSON(int64_t i64) {
		new(this)RValue((int64_t)i64);
	}

	//! Constructor for uint64_t value.
	JSON::JSON(uint64_t u64) {
		new(this)RValue((uint64_t)u64);
	}

	//! Constructor for double value.
	JSON::JSON(double d) {
		new(this)RValue(d);
	}

	//! Constructor for CStringRef value.
	JSON::JSON(cString& str) {
		new(this)RValue(*str, str.length(), shareMemoryPoolAllocator);
	}

	JSON::JSON(JSON&& json) {
		new(this)RValue(std::move(*reinterpret_cast<RValue*>(&json))); // init
	}
	JSON& JSON::operator=(JSON&& json) {
		*reinterpret_cast<RValue*>(this) = std::move(*reinterpret_cast<RValue*>(&json));
		return *this;
	}

	JSON::JSON(JSON& json) {
	//  TLog(sizeof(RValue));
	//  TLog(sizeof(JSON));
		new(this)RValue(std::move(*reinterpret_cast<RValue*>(&json))); // init
	}

	JSON::~JSON(){
		reinterpret_cast<RValue*>(this)->
			~GenericValue<rapidjson::UTF8<>, MemoryPoolAllocator>();
	}

	JSON JSON::clone() const {
		RValue copy(*reinterpret_cast<CRValue*>(this), shareMemoryPoolAllocator);
		return *reinterpret_cast<JSON*>(&copy);
	}

	JSON& JSON::operator=(JSON& json) {
		*reinterpret_cast<RValue*>(this) = *reinterpret_cast<RValue*>(&json);
		return *this;
	}

	JSON& JSON::operator=(bool b) {
		reinterpret_cast<RValue*>(this)->SetBool(b);
		return *this;
	}

	JSON& JSON::operator=(int i) {
		reinterpret_cast<RValue*>(this)->SetInt(i);
		return *this;
	}

	JSON& JSON::operator=(uint32_t u) {
		reinterpret_cast<RValue*>(this)->SetUint(u);
		return *this;
	}

	JSON& JSON::operator=(int64_t i64) {
		reinterpret_cast<RValue*>(this)->SetInt64(i64);
		return *this;
	}

	JSON& JSON::operator=(uint64_t u64) {
		reinterpret_cast<RValue*>(this)->SetUint64(u64);
		return *this;
	}

	JSON& JSON::operator=(double d) {
		reinterpret_cast<RValue*>(this)->SetDouble(d);
		return *this;
	}

	JSON& JSON::operator=(cChar* str) {
		reinterpret_cast<RValue*>(this)->
			SetString(str, uint32_t(strlen(str)), shareMemoryPoolAllocator);
		return *this;
	}

	JSON& JSON::operator=(cString& str) {
		reinterpret_cast<RValue*>(this)->
			SetString(str.c_str(), str.length(), shareMemoryPoolAllocator);
		return *this;
	}

	bool JSON::operator==(const JSON& json) const {
		CRValue& b = *reinterpret_cast<CRValue*>(&json);
		return reinterpret_cast<CRValue*>(this)->operator==(b);
	}

	bool JSON::operator==(cChar* str) const {
		return reinterpret_cast<CRValue*>(this)->operator==(str);
	}

	bool JSON::operator==(cString& str) const {
		return reinterpret_cast<CRValue*>(this)->operator==(*str);
	}

	JSON& JSON::operator[](cJSON& key) {
		RValue* self = reinterpret_cast<RValue*>(this);
		CRValue& n = *reinterpret_cast<CRValue*>(&key);
		F_ASSERT(self->IsObject());
		F_ASSERT(n.IsString());
		RValue::MemberIterator member = self->FindMember(n);
		RValue* value = NULL;
		
		if (member != self->MemberEnd()) {
			value = &member->value;
		}
		else {
			// 创建一个空值
			self->AddMember(RValue(n.GetString(), n.GetStringLength(), shareMemoryPoolAllocator),
											RValue(),
											shareMemoryPoolAllocator);
			value = &self->FindMember(n)->value;
		}
		return *reinterpret_cast<JSON*>(value);
	}

	const JSON& JSON::operator[](cJSON& key) const {
		CRValue* self = reinterpret_cast<CRValue*>(this);
		CRValue& n = *reinterpret_cast<CRValue*>(&key);
		F_ASSERT(self->IsObject());
		F_ASSERT(n.IsString());
		RValue::ConstMemberIterator member = self->FindMember(n);
		
		if (member != self->MemberEnd()){
			return *reinterpret_cast<const JSON*>(&member->value);
		}
		return NullValue;
	}

	JSON& JSON::operator[](int index) {
		F_ASSERT(is_array());
		RValue* self = reinterpret_cast<RValue*>(this);
		F_ASSERT(self->IsArray());
		RValue* value = NULL;
		
		int size = self->Size();
		
		if (index < size) {
			value = &(*self)[index];
		}
		else {
			for ( ; size <= index; size++) {
				self->PushBack(RValue(), shareMemoryPoolAllocator);
			}
			auto it = self->End();
			value = --it;
		}
		return *reinterpret_cast<JSON*>(value);
	}

	JSON& JSON::operator[] (cChar* key) {
		RValue* self = reinterpret_cast<RValue*>(this);
		F_ASSERT(self->IsObject());
		RValue n(rapidjson::StringRef(key));
		RValue::MemberIterator member = self->FindMember(n);
		RValue* value = NULL;
		
		if (member != self->MemberEnd()){
			value = &member->value;
		}
		else {
			// 创建一个空值
			self->AddMember(RValue(key, n.GetStringLength(), shareMemoryPoolAllocator),
											RValue(),
											shareMemoryPoolAllocator);
			value = &self->FindMember(n)->value;
		}
		return *reinterpret_cast<JSON*>(value);
	}

	JSON& JSON::operator[] (cString& key) {
		return operator[](*key);
	}

	const JSON& JSON::operator[](int index) const {
		CRValue* self = reinterpret_cast<CRValue*>(this);
		F_ASSERT(self->IsArray());
		
		int size = self->Size();
		if(index < size){
			return *reinterpret_cast<const JSON*>(&(*self)[index]);
		}
		return NullValue;
	}

	const JSON& JSON::operator[](cChar* key) const {
		CRValue* self = reinterpret_cast<CRValue*>(this);
		F_ASSERT(self->IsObject());
		RValue n(rapidjson::StringRef(key));
		RValue::ConstMemberIterator member = self->FindMember(n);
		
		if (member != self->MemberEnd()) {
			return *reinterpret_cast<const JSON*>(&member->value);
		}
		return NullValue;
	}

	const JSON& JSON::operator[](cString& key) const {
		return operator[](*key);
	}

	bool JSON::is_member(cChar* key) const {
		return reinterpret_cast<CRValue*>(this)->HasMember(key);
	}

	bool JSON::is_member(cString& key) const {
		return reinterpret_cast<CRValue*>(this)->HasMember(*key);
	}

	bool JSON::is_null()   const { return reinterpret_cast<CRValue*>(this)->IsNull(); }
	bool JSON::is_false()  const { return reinterpret_cast<CRValue*>(this)->IsFalse(); }
	bool JSON::is_true()   const { return reinterpret_cast<CRValue*>(this)->IsTrue(); }
	bool JSON::is_bool()   const { return reinterpret_cast<CRValue*>(this)->IsBool(); }
	bool JSON::is_object() const { return reinterpret_cast<CRValue*>(this)->IsObject(); }
	bool JSON::is_array()  const { return reinterpret_cast<CRValue*>(this)->IsArray(); }
	bool JSON::is_number() const { return reinterpret_cast<CRValue*>(this)->IsNumber(); }
	bool JSON::is_int()    const { return reinterpret_cast<CRValue*>(this)->IsInt(); }
	bool JSON::is_uint32()   const { return reinterpret_cast<CRValue*>(this)->IsUint(); }
	bool JSON::is_int64()  const { return reinterpret_cast<CRValue*>(this)->IsInt64(); }
	bool JSON::is_uint64() const { return reinterpret_cast<CRValue*>(this)->IsUint64(); }
	bool JSON::is_double() const { return reinterpret_cast<CRValue*>(this)->IsDouble(); }
	bool JSON::is_string() const { return reinterpret_cast<CRValue*>(this)->IsString(); }
	bool JSON::to_bool()   const { return reinterpret_cast<CRValue*>(this)->GetBool(); }
	double JSON::to_double()   const { return reinterpret_cast<CRValue*>(this)->GetDouble(); }
	int JSON::to_int()         const { return reinterpret_cast<CRValue*>(this)->GetInt(); }
	int64_t JSON::to_int64()   const { return reinterpret_cast<CRValue*>(this)->GetInt64(); }
	String JSON::to_string()   const {
		return std::move(String(reinterpret_cast<CRValue*>(this)->GetString(), string_length()));
	}
	uint32_t JSON::to_uint32()       const { return reinterpret_cast<CRValue*>(this)->GetUint(); }
	uint64_t JSON::to_uint64()   const { return reinterpret_cast<CRValue*>(this)->GetUint64(); }
	uint32_t JSON::string_length() const { return reinterpret_cast<CRValue*>(this)->GetStringLength(); }
	uint32_t JSON::length()        const { return reinterpret_cast<CRValue*>(this)->Size(); }

	void JSON::clear() {
		if (is_array()) {
			reinterpret_cast<RValue*>(this)->Clear();
		}
		else if (is_object()) {
			reinterpret_cast<RValue*>(this)->RemoveAllMembers();
		}
	}

	void JSON::pop() {
		reinterpret_cast<RValue*>(this)->PopBack();
	}

	void JSON::remove(cChar* key) {
		RValue* self = reinterpret_cast<RValue*>(this);
		F_ASSERT(self->IsObject());
		RValue n(rapidjson::StringRef(key));
		RValue::MemberIterator member = self->FindMember(n);
		
		if (member != self->MemberEnd()) {
			self->RemoveMember(member);
		}
	}

	void JSON::remove(cString& key) {
		remove(*key);
	}

	void JSON::remove(JSON::IteratorConst it) {
		RValue* self = reinterpret_cast<RValue*>(this);
		auto it_ = reinterpret_cast<RValue::ConstMemberIterator*>(&it);
		self->EraseMember(*it_);
	}

	void JSON::remove(JSON::ArrayIteratorConst it) {
		RValue* self = reinterpret_cast<RValue*>(this);
		auto it_ = reinterpret_cast<RValue::ConstValueIterator*>(&it);
		self->Erase(*it_);
	}

	JSON::Iterator JSON::begin() {
		auto it = reinterpret_cast<RValue*>(this)->MemberBegin();
		auto it_ = reinterpret_cast<JSON::Iterator*>(&it);
		return *it_;
	}

	JSON::Iterator JSON::end() {
		auto it = reinterpret_cast<RValue*>(this)->MemberEnd();
		auto it_ = reinterpret_cast<JSON::Iterator*>(&it);
		return *it_;
	}

	JSON::IteratorConst JSON::begin() const {
		auto it = reinterpret_cast<CRValue*>(this)->MemberBegin();
		auto it_ = reinterpret_cast<JSON::IteratorConst*>(&it);
		return *it_;
	}

	JSON::IteratorConst JSON::end() const {
		auto it = reinterpret_cast<CRValue*>(this)->MemberEnd();
		auto it_ = reinterpret_cast<JSON::IteratorConst*>(&it);
		return *it_;
	}

	JSON::ArrayIterator JSON::begin_array() {
		return reinterpret_cast<ArrayIterator>(reinterpret_cast<RValue*>(this)->Begin());
	}

	JSON::ArrayIterator JSON::end_array() {
		return reinterpret_cast<ArrayIterator>(reinterpret_cast<RValue*>(this)->End());
	}

	JSON::ArrayIteratorConst JSON::begin_array() const {
		return reinterpret_cast<ArrayIteratorConst>(reinterpret_cast<CRValue*>(this)->Begin());
	}

	JSON::ArrayIteratorConst JSON::end_array() const {
		return reinterpret_cast<ArrayIteratorConst>(reinterpret_cast<CRValue*>(this)->End());
	}

	static JSON parse_for(cChar* json, int64_t len = 0xFFFFFFFFFFFFFFF) throw(Error) {
		RDocument doc(&shareMemoryPoolAllocator);
		doc.Parse(json, len);
		F_CHECK(!doc.HasParseError(),
							ERR_JSON_PARSE_ERROR,
							"json parse error, offset: %lu, code: %d\n%s, %p, %ld",
							doc.GetErrorOffset(), doc.GetParseError(), json, json, len);
		return *reinterpret_cast<JSON*>(&doc);
	}

	JSON JSON::parse(cString& json) throw(Error) {
		return parse_for(json.c_str(), json.length());
	}

	JSON JSON::parse(cBuffer& json_str) throw(Error) {
		return parse_for(json_str.val(), json_str.length());
	}

	String JSON::stringify(cJSON& json){
		rapidjson::StringBuffer buffer;
		rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
		reinterpret_cast<CRValue*>(&json)->Accept(writer);
		return std::move(String(buffer.GetString(), uint32_t(buffer.GetSize())));
	}

	JSON& JSON::ext(JSON& o, JSON& extd) {
		
		if (o.is_object() && extd.is_object()) {
			for (auto i = extd.begin(); i != extd.end(); i++) {
				o[i->name] = i->value;
			}
		}
		else {
			//  F_ASSERT(0, "This method is only applicable to \"Object\" type of JOSN");
			F_WARN("JSON", "%s", "This method is only applicable to \"Object\" type of JOSN");
		}
		return o;
	}

	/**
	* 创建一个Object类型的JSON数据
	* @return {JSON}
	*/
	JSON JSON::object(){
		return JSON(kObjectType);
	}

	/**
	* 创建一个Array类型的JSON数据
	* @return {JSON}
	*/
	JSON JSON::array(){
		return JSON(kArrayType);
	}

	/**
	* 创建一个null类型的JSON数据
	* @return {JSON}
	*/
	JSON JSON::null(){
		return JSON();
	}

}
