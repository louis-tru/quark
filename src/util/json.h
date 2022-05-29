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

#ifndef __noug__util__json__
#define __noug__util__json__

#include "./handle.h"
#include "./string.h"
#include "./error.h"
#include "./iterator.h"

namespace noug {

	class JSON;

	typedef const JSON cJSON;

	/**
	* @class JSON
	*/
	class N_EXPORT JSON {
	public:
		typedef NonObjectTraits Traits;
		struct Member;

		typedef SimpleIterator<Member,       Member>  Iterator;
		typedef SimpleIterator<const Member, Member>  IteratorConst;
		typedef       JSON*                           ArrayIterator;
		typedef const JSON*                           ArrayIteratorConst;
		
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
		JSON(uint32_t u);
		
		//! Constructor for int64_t value.
		JSON(int64_t i64);
		
		//! Constructor for uint64_t value.
		JSON(uint64_t u64);
		
		//! Constructor for double value.
		JSON(double d);
		
		//! Constructor for cString& value.
		JSON(cString& str);
	
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
		JSON& operator=(uint32_t u);
		JSON& operator=(int64_t i64);
		JSON& operator=(uint64_t u64);
		JSON& operator=(double d);
		JSON& operator=(cChar* str);
		JSON& operator=(cString& str);
		//
		bool operator==(cJSON& json) const;
		bool operator==(cChar* str) const;
		bool operator==(cString& str) const;
		bool operator!=(cJSON& json) const { return !this->operator==(json); }
		bool operator!=(cChar* str) const { return !this->operator==(str); }
		bool operator!=(cString& str) const { return !this->operator==(str); }
		
		JSON& operator[](cJSON& key);
		JSON& operator[](int index);
		JSON& operator[](cChar* key);
		JSON& operator[](cString& key);
		cJSON& operator[](cJSON& key) const;
		cJSON& operator[](int index) const;
		cJSON& operator[](cChar* key) const;
		cJSON& operator[](cString& key) const;
		
		bool is_member(cChar* key) const;
		bool is_member(cString& key) const;
		bool is_null()   const;
		bool is_false()  const;
		bool is_true()   const;
		bool is_bool()   const;
		bool is_object() const;
		bool is_array()  const;
		bool is_number() const;
		bool is_int()    const;
		bool is_uint32() const;
		bool is_int64()  const;
		bool is_uint64() const;
		bool is_double() const;
		bool is_string() const;
		
		bool to_bool()   const;
		int to_int()     const;
		int64_t to_int64() const;
		uint32_t to_uint32() const;
		uint64_t to_uint64() const;
		double to_double()   const;
		String to_string()   const;

		uint32_t string_length() const;
		
		/**
		* 数组长度
		*/
		uint32_t length() const;
		void pop();
		void clear();
		
		void remove(cChar* key);
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
		* @param  {cString&} json_str
		* @return {JSON}
		*/
		static JSON parse(cString& json_str) throw(Error);
		static JSON parse(cBuffer& json_str) throw(Error);
		
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

		//! Copy constructor is not permitted.
		JSON(cJSON& json);
		
		// 12 bytes in 32-bit mode, 16 bytes in 64-bit mode
		union Data {
			// 12 bytes in 32-bit mode, 16 bytes in 64-bit mode
			struct InlString {
				cChar* str;
				uint32_t length;
				uint32_t hashcode;  //!< reserved
			} s;
			// 8 bytes
			union Number {
				struct I { Char padding[4]; int i; } i;
				int64_t i64;
				uint64_t u64;
				double d;
			} n;
			// 12 bytes in 32-bit mode, 16 bytes in 64-bit mode
			struct Object {
				Member* members;
				uint32_t size;
				uint32_t capacity;
			} o;
			// 12 bytes in 32-bit mode, 16 bytes in 64-bit mode
			// Array a;
			struct Array {
				JSON* elements;
				uint32_t size;
				uint32_t capacity;
			} a;
		} _data;
		
		uint32_t _flags;
	};

	struct JSON::Member {
		JSON name;     //!< name of member (must be a string)
		JSON value;    //!< value of member.
	};

}
#endif
