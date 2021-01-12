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

template<class T, class Container>
ArrayBuffer<T, Container>::ArrayBuffer(ArrayBuffer& arr): ArrayBuffer(std::move(arr))
{}

template<class T, class Container>
ArrayBuffer<T, Container>::ArrayBuffer(ArrayBuffer&& arr) : _length(0), _container(0)
{
	T* t = *arr._container;
	_container.operator=(std::move(arr._container));
	if ( t == *_container ) {
		_length = arr._length;
		arr._length = 0;
	}
}

template<class T, class Container>
ArrayBuffer<T, Container>::ArrayBuffer(const std::initializer_list<T>& list)
: _length((uint32_t)list.size()), _container((uint32_t)list.size()) {
	T* begin = *_container;
	for (auto& i : list) {
		new(begin) T(std::move(i)); // 调用默认构造
		begin++;
	}
}

template<class T, class Container>
ArrayBuffer<T, Container>::ArrayBuffer(T* data, uint32_t length, uint32_t capacity, bool is_readonly)
: _length(length), _container(FX_MAX(capacity, length), data, is_readonly)
{}

template<class T, class Container>
ArrayBuffer<T, Container>::ArrayBuffer(uint32_t length, uint32_t capacity)
: _length(length), _container(FX_MAX(length, capacity))
{
	if (_length) {
		T* begin = *_container;
		T* end = begin + _length;
		while (begin < end) {
			new(begin) T(); // 调用默认构造
			begin++;
		}
	}
}

template<class T, class Container>
ArrayBuffer<T, Container>::~ArrayBuffer() {
	clear();
}

template<class T, class Container>
ArrayBuffer<T, Container>& ArrayBuffer<T, Container>::operator=(ArrayBuffer& arr) {
	return operator=(std::move(arr));
}

template<class T, class Container>
ArrayBuffer<T, Container>& ArrayBuffer<T, Container>::operator=(ArrayBuffer&& arr) {
	if ( &arr._container == &_container ) return *this;
	clear();
	T* t = *arr._container;
	_container.operator=(std::move(arr._container));
	if ( t == *_container ) {
		_length = arr._length;
		arr._length = 0;
	}
	return *this;
}

template<class T, class Container>
const T& ArrayBuffer<T, Container>::operator[](uint32_t index) const {
	ASSERT(index < _length, "ArrayBuffer access violation.");
	return (*_container)[index];
}

template<class T, class Container>
T& ArrayBuffer<T, Container>::operator[](uint32_t index) {
	ASSERT(index < _length, "ArrayBuffer access violation.");
	return (*_container)[index];
}

template<class T, class Container>
uint32_t ArrayBuffer<T, Container>::push(const T& item) {
	_length++;
	_container.realloc(_length);
	new((*_container) + _length - 1) T(item);
	return _length;
}

template<class T, class Container>
uint32_t ArrayBuffer<T, Container>::push(T&& item) {
	_length++;
	_container.realloc(_length);
	new((*_container) + _length - 1) T(std::move(item));
	return _length;
}

template<class T, class Container>
uint32_t ArrayBuffer<T, Container>::concat(ArrayBuffer&& arr) {
	if (arr._length) {
		_length += arr._length;
		_container.realloc(_length);
		
		const T* source = *arr._container;
		T* end = (*_container) + _length;
		T* begin = end - arr._length;
		
		while (begin < end) {
			new(begin) T(std::move(*source)); // 调用移动构造
			source++; begin++;
		}
	}
	return _length;
}

template<class T, class Container>
inline ArrayBuffer<T, Container> ArrayBuffer<T, Container>::slice(uint32_t start) const {
	return slice(start, _length);
}

template<class T, class Container>
ArrayBuffer<T, Container> ArrayBuffer<T, Container>::slice(uint32_t start, uint32_t end) const {
	end = FX_MIN(end, _length);
	if (start < end) {
		ArrayBuffer arr;
		arr._length = end - start;
		arr._container.realloc(arr._length);
		T* tar = *arr._container;
		T* e = tar + arr._length;
		const T* src = *_container + start;
		while (tar < e) {
			new(tar) T(*src);
			tar++; src++;
		}
		return arr;
	}
	return ArrayBuffer();
}

template<class T, class Container>
uint32_t ArrayBuffer<T, Container>::write(const ArrayBuffer& arr, int to, int size, uint32_t form) {
	int s = FX_MIN(arr._length - form, size < 0 ? arr._length : size);
	if (s > 0) {
		return write((*arr._container) + form, to, s);
	}
	return 0;
}

/**
 * @func write
 */
template<class T, class Container>
uint32_t ArrayBuffer<T, Container>::write(const T* src, int to, uint32_t size) {
	if (size) {
		if ( to == -1 ) to = _length;
		uint32_t old_len = _length;
		uint32_t end = to + size;
		_length = FX_MAX(end, _length);
		_container.realloc(_length);
		T* tar = (*_container) + to;
		
		for (int i = to; i < end; i++) {
			if (i < old_len) {
				reinterpret_cast<Sham*>(tar)->~Sham(); // 先释放原对像
			}
			new(tar) T(*src);
			tar++; src++;
		}
	}
	return size;
}

template<class T, class Container>
uint32_t ArrayBuffer<T, Container>::pop(uint32_t count) {
	int j = FX_MAX(_length - count, 0);
	if (_length > j) {
		do {
			_length--;
			reinterpret_cast<Sham*>((*_container) + _length)->~Sham(); // 释放
		} while (_length > j);
		
		_container.realloc(_length);
	}
	return _length;
}

template<class T, class Container> void ArrayBuffer<T, Container>::clear() {
	if (_length) {
		T* item = *_container;
		T* end = item + _length;
		while (item < end) {
			reinterpret_cast<Sham*>(item)->~Sham(); // 释放
			item++;
		}
		_length = 0;
	}
	_container.free();
}

#define FX_DEF_ARRAY_SPECIAL(T, Container) \
	template<>                              ArrayBuffer<T, Container<T>>::ArrayBuffer(uint32_t length, uint32_t capacity); \
	template<>                              ArrayBuffer<T, Container<T>>::ArrayBuffer(const std::initializer_list<T>& list); \
	template<> uint32_t                     ArrayBuffer<T, Container<T>>::concat(ArrayBuffer&& arr); \
	template<> ArrayBuffer<T, Container<T>> ArrayBuffer<T, Container<T>>::slice(uint32_t start, uint32_t end) const; \
	template<> uint32_t                     ArrayBuffer<T, Container<T>>::write(const T* src, int to, uint32_t size);\
	template<> uint32_t                     ArrayBuffer<T, Container<T>>::pop(uint32_t count);  \
	template<> void                         ArrayBuffer<T, Container<T>>::clear()

FX_DEF_ARRAY_SPECIAL(char, Container);
FX_DEF_ARRAY_SPECIAL(unsigned char, Container);
FX_DEF_ARRAY_SPECIAL(int16_t, Container);
FX_DEF_ARRAY_SPECIAL(uint16_t, Container);
FX_DEF_ARRAY_SPECIAL(int, Container);
FX_DEF_ARRAY_SPECIAL(uint32_t, Container);
FX_DEF_ARRAY_SPECIAL(int64_t, Container);
FX_DEF_ARRAY_SPECIAL(uint64_t, Container);
FX_DEF_ARRAY_SPECIAL(float, Container);
FX_DEF_ARRAY_SPECIAL(double, Container);
FX_DEF_ARRAY_SPECIAL(bool, Container);
