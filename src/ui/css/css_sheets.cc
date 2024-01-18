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
#include "../layout/layout.h"
#include "../app.h"

namespace qk {

	enum ViewPropFrom {
		kView,kBox,kFlex,kFlow,kImage,kTextOptions,kInput,kScrollLayoutBase,kTransform
	};

	CSSName::CSSName(cString& name)
		: _hashCode(name.hashCode())
	{}

	StyleSheets::StyleSheets(cCSSName &name, StyleSheets *parent, CSSType type)
		: _name(name)
		, _parent(parent)
		, _time(0)
		, _normal(nullptr), _hover(nullptr), _active(nullptr)
		, _havePseudoType(false)
		, _type( parent && parent->_type ? parent->_type: type )
	{}

	StyleSheets::~StyleSheets() {
		for ( auto i : _substyles )
			Release(i.value);
		for ( auto i : _extends )
			Release(i.value);
		for (auto i: _props)
			delete i.value;
		Release(_normal); _normal = nullptr;
		Release(_hover); _hover = nullptr;
		Release(_active); _active = nullptr;
	}

	void StyleSheets::set_time(uint64_t value) {
		_time = value;
	}

	void StyleSheets::apply(Layout *layout) const {
		Qk_ASSERT(layout);
		if (_props.length()) {
			for ( auto i: _props ) {
				i.value->apply(layout);
			}
		}
	}

	void StyleSheets::setProp(uint32_t key, Property *prop) {
		auto it = _props.find(key);
		if (it != _props.end()) {
			delete it->value;
			it->value = prop;
		} else {
			_props.set(key, prop);
		}
	}

	template<ViewPropFrom From>
	struct ApplyProp {
		template<typename T>
		static void apply(Layout *layout, ViewProp prop, T value) {
			auto set = (void (Layout::*)(T))(layout->accessor() + prop)->set;
			if (set)
				(layout->*set)(value);
		}
	};

	template<> struct ApplyProp<kScrollLayoutBase> {
		template<typename T>
		static void apply(Layout *layout, ViewProp prop, T value) {
			auto set = (void (ScrollLayoutBase::*)(T))(layout->accessor() + prop)->set;
			if (set)
				(layout->asScrollLayoutBase()->*set)(value);
		}
	};

	template<> struct ApplyProp<kTextOptions> {
		template<typename T>
		static void apply(Layout *layout, ViewProp prop, T value) {
			auto set = (void (TextOptions::*)(T))(layout->accessor() + prop)->set;
			if (set)
				(layout->asTextOptions()->*set)(value);
		}
	};

	template<typename T, ViewPropFrom From>
	class Property: public StyleSheets::Property {
	public:
		Property(ViewProp prop, T value)
			: _prop(prop), _value(value) {}
		void apply(Layout *layout) override {
			ApplyProp<From>::template apply<T>(layout, _prop, _value);
		}
	private:
		ViewProp _prop;
		T _value;
	};

	template<typename T, ViewPropFrom From>
	class Property<T*, From>: public StyleSheets::Property {
	public:
		typedef T* Type;
		Property(ViewProp prop, Type value)
			: _prop(prop), _value(value) 
		{
			static_assert(T::Traits::isReference, "Property value must be a reference type");
			_value->retain();
		};
		~Property() {
			_value->release();
		}
		void apply(Layout *layout) override {
			ApplyProp<From>::template apply<Type>(layout, _prop, _value);
		}
	private:
		ViewProp _prop;
		Type _value;
	};

	#define _Fun(Enum, Type, Name, From) void StyleSheets::set_##Name(Type value) {\
		setProp(k##Enum##_ViewProp, new qk::Property<Type, k##From>(k##Enum##_ViewProp, value));\
	}
	Qk_View_Props(_Fun)

	#undef _Fun

	// ------------------- @private -------------------

	StyleSheets* StyleSheets::findHash(uint64_t hash) const {
		auto i = _substyles.find(hash);
		return i == _substyles.end() ? nullptr : i->value;
	}

	StyleSheets* StyleSheets::findAndMake(cCSSName &name, CSSType type) {
		StyleSheets *ss = nullptr;
		/*
			.a {
				.b {
					.c {
						height: 100px;
					}
				}
				.b_1 {
					.c {
						width: 100pt;
					}
					color: #aaa;
				}
			}
		*/
		auto it = _substyles.find(name.hashCode());
		if ( it == _substyles.end() ) {
			ss = new StyleSheets(name, this, kNone_CSSType);
			_substyles[name.hashCode()] = ss;
		} else {
			ss = it->value;
		}

		if ( !type ) return ss; // no find pseudo type
		if ( _type ) return nullptr; // illegal pseudo cls, 伪类样式表,不能存在子伪类样式表

		// find pseudo type
		StyleSheets **ss_pseudo = nullptr;
		switch ( type ) { 
			case kNormal_CSSType: ss_pseudo = &ss->_normal; break;
			case kHover_CSSType: ss_pseudo = &ss->_hover; break;
			case kActive_CSSType: ss_pseudo = &ss->_active; break;
		}

		if ( !*ss_pseudo ) {
			ss->_havePseudoType = true;
			*ss_pseudo = new StyleSheets(name, this, type);
		}
		return *ss_pseudo;
	}

}
