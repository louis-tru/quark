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

#ifndef __qgr__utils__array__
#define __qgr__utils__array__

#include "qgr/utils/util.h"
#include "qgr/utils/container.h"
#include "qgr/utils/iterator.h"
#include "qgr/utils/error.h"
#include <initializer_list>

/**
 * @ns qgr
 */

XX_NS(qgr)

template<class T> class ArrayBuffer;
template<class T> class WeakArrayBuffer;

/**
 * @class Array
 */
template<class T, class Container>
class XX_EXPORT Array: public Object {
 private:
	struct Wrap { T _item; };
	
	struct IteratorData {
	 public:
		typedef T Value;
		const T& value() const;
		T& value();
		inline int index() const { return _index; }
	 private:
		IteratorData();
		IteratorData(Array* host, uint index);
		bool equals(const IteratorData& it) const;
		bool is_null() const;
		void prev();
		void next();
		Array* _host;
		int _index;
		friend class Array;
		friend class IteratorTemplateConst<IteratorData>;
		friend class IteratorTemplate<IteratorData>;
	};
	
	Array(T* data, uint length, uint capacity);

 public:
	typedef IteratorTemplateConst<IteratorData> IteratorConst;
	typedef IteratorTemplate<IteratorData> Iterator;
	
	Array(const Array&);
	Array(Array&&);
	Array(uint length = 0, uint capacity = 0);
	Array(const std::initializer_list<T>& list);
	virtual ~Array();
	Array& operator=(const Array&);
	Array& operator=(Array&&);
	const T& operator[](uint index) const;
	T& operator[](uint index);
	const T& item(uint index) const;
	T& item(uint index);
	T& set(uint index, const T& item);
	T& set(uint index, T&& item);
	uint push(const T& item);
	uint push(T&& item);
	uint push(const Array& arr);
	uint push(Array&& arr);
	uint pop();
	uint pop(uint count);
	Array slice(uint start);
	Array slice(uint start, uint end);
	
	/**
	 * @func write
	 * @arg src 
	 * @arg to {int=-1} 当前数组开始写入的位置,-1从结尾开始写入
	 * @arg size {int=-1} 需要写入项目数量,超过要写入数组的长度自动取写入数组长度,-1写入全部
	 * @arg form {int=0} 从要写入数组的form位置开始取数据
	 * @ret {uint} 返回写入数据量
	 */
	uint write(const Array& src, int to = -1, int size = -1, uint form = 0);
	uint write(const T* src, int to, uint size);
	void clear();
	String join(cString& sp) const;
	IteratorConst begin() const;
	IteratorConst end() const;
	Iterator begin();
	Iterator end();
	uint length() const;
	uint capacity() const;
	
 private:
	template<class S> friend class ArrayBuffer;
	template<class S> friend class WeakArrayBuffer;
	uint        _length;
	Container   _container;
};

XX_END

#include "qgr/utils/array.inl"

#endif
