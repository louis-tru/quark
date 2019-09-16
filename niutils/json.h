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

#ifndef __ngui__utils__json__
#define __ngui__utils__json__

#include "niutils/string.h"
#include "niutils/handle.h"

/**
 * @ns ngui
 */

XX_NS(ngui)

class JSON;
typedef const JSON cJSON;

/**
 * @class JSON
 */
class XX_EXPORT JSON {
 public:
	struct Member;
	typedef NonObjectTraits Traits;
	
	template <typename T> class XX_EXPORT MemberIterator {
	 private:
		typedef std::iterator<std::random_access_iterator_tag, T> BaseType;
		template <typename> friend class MemberIterator;
	 public:
		
		//! Iterator type itself
		typedef MemberIterator Iterator;
		//! Constant iterator type
		typedef MemberIterator<const Member>  IteratorConst;
		//! Non-constant iterator type
		typedef MemberIterator<Member>        NonIteratorConst;
		
		//! Pointer to (const) GenericMember
		typedef typename BaseType::pointer         Pointer;
		//! Reference to (const) GenericMember
		typedef typename BaseType::reference       Reference;
		
		MemberIterator(const NonIteratorConst& it) : ptr_(it.ptr_) { }
		
		MemberIterator& operator++();    // ++i
		MemberIterator& operator--();    // --i
		MemberIterator  operator++(int); // i++
		MemberIterator  operator--(int); // i--
		bool operator==(IteratorConst that) const { return ptr_ == that.ptr_; }
		bool operator!=(IteratorConst that) const { return ptr_ != that.ptr_; }
		
		Reference operator*() const { return *ptr_; }
		Pointer   operator->() const { return ptr_; }
		//
	 private:
		MemberIterator();
		Pointer ptr_; //!< raw pointer
	};
	
 public:
	
	typedef MemberIterator<Member>        Iterator;
	typedef MemberIterator<const Member>  IteratorConst;
	typedef JSON*                         ArrayIterator;
	typedef const JSON*                   ArrayIteratorConst;
	
	//! Type of JSON value
	enum Type {
		kNullType = 0,      //!< null
		kFalseType = 1,     //!< false
		kTrueType = 2,      //!< true
		kObjectType = 3,    //!< object
		kArrayType = 4,     //!< array
		kStringType = 5,    //!< string
		kNumberType = 6     //!< number
	};
	
	JSON();
	JSON(Type type);
	
	JSON(bool b);
	
	//! Constructor for int value.
	JSON(int i);
	
	//! Constructor for unsigned value.
	JSON(uint u);
	
	//! Constructor for int64_t value.
	JSON(int64 i64);
	
	//! Constructor for uint64_t value.
	JSON(uint64 u64);
	
	//! Constructor for double value.
	JSON(double d);
	
	//! Constructor for cchar* value.
	JSON(cchar* str);
	
	//! Constructor for cString& value.
	JSON(cString& str);
	
	//! Constructor for cData& value.
	JSON(cBuffer& data);
	
 private:
	//! Copy constructor is not permitted.
	JSON(cJSON& json);
 public:
	
	JSON(JSON& json);
	JSON(JSON&& json);
	JSON& operator=(JSON&& json);
	~JSON();
	
	/**
	 * 克隆对像
	 */
	JSON clone() const;
	JSON& operator=(JSON& json);
	JSON& operator=(bool b);
	JSON& operator=(int i);
	JSON& operator=(uint u);
	JSON& operator=(int64 i64);
	JSON& operator=(uint64 u64);
	JSON& operator=(double d);
	JSON& operator=(cchar* str);
	JSON& operator=(cString& str);
	JSON& operator=(cBuffer& data);
	//
	bool operator==(cJSON& json) const;
	bool operator==(cchar* str) const;
	bool operator==(cString& str) const;
	inline bool operator!=(cJSON& json) const { return !this->operator==(json); }
	inline bool operator!=(cchar* str) const { return !this->operator==(str); }
	inline bool operator!=(cString& str) const { return !this->operator==(str); }
	
	JSON& operator[](cJSON& key);
	JSON& operator[](int index);
	JSON& operator[](cchar* key);
	JSON& operator[](cString& key);
	cJSON& operator[](cJSON& key) const;
	cJSON& operator[](int index) const;
	cJSON& operator[](cchar* key) const;
	cJSON& operator[](cString& key) const;
	
	bool is_member(cchar* key) const;
	bool is_member(cString& key) const;
	bool is_null()   const;
	bool is_false()  const;
	bool is_true()   const;
	bool is_bool()   const;
	bool is_object() const;
	bool is_array()  const;
	bool is_number() const;
	bool is_int()    const;
	bool is_uint()   const;
	bool is_int64()  const;
	bool is_uint64() const;
	bool is_double() const;
	bool is_string() const;
	
	bool to_bool()   const;
	int to_int()     const;
	int64 to_int64() const;
	uint to_uint()   const;
	uint64 to_uint64()   const;
	double to_double()   const;
	cchar* to_cstring()  const;
	String to_string()   const;
	//
	uint string_length() const;
	
	/**
	 * 数组长度
	 */
	uint length() const;
	void pop();
	void clear();
	
	void remove(cchar* key);
	void remove(cString& key);
	void remove(IteratorConst it);
	void remove(ArrayIteratorConst it);
	
	Iterator begin();
	Iterator end();
	IteratorConst begin() const;
	IteratorConst end() const;
	ArrayIterator begin_array();
	ArrayIterator end_array();
	ArrayIteratorConst begin_array() const;
	ArrayIteratorConst end_array() const;
	
	/**
	 * 把JSON格式的字符串转换为Json对像
	 * @param  {Data} data
	 * @return {JSON}
	 */
	static JSON parse(cBuffer& data) throw(Error);
	
	/**
	 * 把JSON格式的字符串转换为Json对像
	 * @param  {cString&} json_str
	 * @return {JSON}
	 */
	static JSON parse(cString& json_str) throw(Error);
	
	/**
	 * 把JSON格式的字符串转换为Json对像
	 * @param  {cchar*} json_str
	 * @return {JSON}
	 */
	static JSON parse(cchar* json_str) throw(Error);
	
	/**
	 * 把Json对像转换为字符串
	 * @param  {JSON} json
	 * @return {String}
	 */
	static String stringify(cJSON& json);
	
	/**
	 * 扩展
	 * @param {JSON&} o
	 * @param {cJSON&} extd
	 * @return {JSON&}
	 */
	static JSON& ext(JSON& o, JSON& extd);
	
	/**
	 * 创建一个Object类型的JSON数据
	 * @return {JSON}
	 */
	static JSON object();
	
	/**
	 * 创建一个Array类型的JSON数据
	 * @return {JSON}
	 */
	static JSON array();
	
	/**
	 * 创建一个null类型的JSON数据
	 * @return {JSON}
	 */
	static JSON null();
	
 private:
	
	// 12 bytes in 32-bit mode, 16 bytes in 64-bit mode
	union Data {
		// 12 bytes in 32-bit mode, 16 bytes in 64-bit mode
		struct InlString {
			cchar* str;
			uint length;
			uint hashcode;  //!< reserved
		} s;
		// 8 bytes
		union Number {
			struct I { char padding[4]; int i; } i;
			int64 i64;
			uint64 u64;
			double d;
		} n;
		// 12 bytes in 32-bit mode, 16 bytes in 64-bit mode
		struct Object {
			Member* members;
			uint size;
			uint capacity;
		} o;
		// 12 bytes in 32-bit mode, 16 bytes in 64-bit mode
		// Array a;
		struct Array {
			JSON* elements;
			uint size;
			uint capacity;
		} a;
	} m_data;
	
	uint m_flags;
};

struct JSON::Member {
	JSON name;     //!< name of member (must be a string)
	JSON value;    //!< value of member.
};

//! @name stepping
//@{
template<typename T>
JSON::MemberIterator<T>& JSON::MemberIterator<T>::operator++() { ++ptr_; return *this; }
template<typename T>
JSON::MemberIterator<T>& JSON::MemberIterator<T>::operator--() { --ptr_; return *this; }
template<typename T>
JSON::MemberIterator<T>  JSON::MemberIterator<T>::operator++(int) {
	MemberIterator old(*this); ++ptr_; return old;
}
template<typename T>
JSON::MemberIterator<T>  JSON::MemberIterator<T>::operator--(int) {
	MemberIterator old(*this); --ptr_; return old;
}
//@}

XX_END
#endif
