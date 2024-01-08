/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright (c) 2015, blue.chu
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of blue.chu nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL blue.chu BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * ***** END LICENSE BLOCK ***** */

#include "./css.h"

namespace qk {

	CSSName::CSSName(cArray<String>& name)
		: _str(String('.').append(name.join(".")))
		, _hash(_str.hashCode())
	{
	}

	CSSName::CSSName(cString& name)
		: _str(name)
		, _hash(name.hashCode()) {
	}

	StyleSheets::StyleSheets(const CSSName& name, StyleSheets *parent, CSSType type)
		: _name(name)
		, _parent(parent)
		, _time(0)
		, _normal(nullptr)
		, _hover(nullptr)
		, _active(nullptr)
		, _havePseudoType(false)
		, _type( parent && parent->_type ? parent->_type: type )
	{
	}

	StyleSheets::~StyleSheets() {
		// TODO ...
	}

	StyleSheets* StyleSheets::find(const CSSName &name) {
		return nullptr;
	}

	void StyleSheets::apply(Layout* layout) {
		// TODO ...
	}

	template<typename T>
	void applyToLayout(Layout *layout, ViewPropName name, T value) {
		typedef void (Layout::*Call)(T);
		auto set = (Call)layout_prop_accessor(kView_ViewType, _name)->set;
		if (set) {
			(layout->*set)(_value);
		}
	}

	template<typename T>
	class Property: public StyleSheets::Property {
	public:
		Property(ViewPropName name, T value)
			: _name(name), _value(value) {}
		void apply(Layout *layout) override {
			applyToLayout(layout, _name, _value);
		}
	private:
		ViewPropName _name;
		T _value;
	};

	template<typename T>
	class Property<T*>: public StyleSheets::Property {
	public:
		typedef T* Type;
		Property(ViewPropName name, Type value)
			: _name(name), _value(value) 
		{
			static_assert(T::Traits::isReference, "");
			_value->retain();
		};
		~Property() {
			_value->release();
		}
		void apply(Layout *layout) override {
			applyToLayout(layout, _name, _value);
		}
	private:
		ViewPropName _name;
		Type _value;
	};

	void StyleSheets::setProps(uint32_t key, Property* prop) {
		auto it = _props.find(key);
		if (it != _props.end()) {
			delete it->value;
			it->value = prop;
		} else {
			_props.set(key, prop);
		}
	}

	void StyleSheets::set_time(uint64_t value) {
		_time = value;
	}

	#define _Fun(Enum, Type, Name) void StyleSheets::set_##Name(Type value) {\
		setProps(k##Enum##_ViewProp, new qk::Property<Type>(k##Enum##_ViewProp, value));\
	}

	Qk_View_Props(_Fun)

	#undef _Fun

}
