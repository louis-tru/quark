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
	// --------------------------- S t y l e . S h e e t s ---------------------------

	typedef StyleSheets::Property Property;

	enum ViewPropFrom {
		kView,kBox,kFlex,kFlow,kImage,kTextOptions,kInput,kScrollLayoutBase,kTransform
	};

	CSSName::CSSName(cString& name)
		: _hashCode(name.hashCode())
		, _name(name)
	{}

	StyleSheets::StyleSheets() {
	}

	StyleSheets::~StyleSheets() {
		for (auto i: _props)
			delete i.value;
	}

	void StyleSheets::apply(Layout *layout) const {
		Qk_ASSERT(layout);
		if (_props.length()) {
			for ( auto i: _props ) {
				i.value->apply(layout);
			}
		}
	}

	void StyleSheets::apply(cSet<Layout*> &layout) const {
		if (_props.length()) {
			for (auto &j: layout) {
				for ( auto i: _props ) {
					i.value->apply(j.key);
				}
			}
		}
	}

	void StyleSheets::applyTransition(cSet<Layout*> &layout, float y, StyleSheets *to) const {
		if (_props.length()) {
			Qk_ASSERT(_props.length() == b->_props.length());
			auto a = _props.begin(), e = _props.end();
			auto b = to->_props.begin();
			while (a != e) {
				for (auto &j: layout) {
					a->value->transition(j.key, y, b->value);
				}
				a++; b++;
			}
		}
	}

	void StyleSheets::setProp(uint32_t key, Property *prop) {
		auto it = _props.find(key);
		if (it == _props.end()) {
			onMake(ViewProp(key), prop);
			_props.set(key, prop);
		} else {
			delete it->value;
			it->value = prop;
		}
	}

	void StyleSheets::onMake(ViewProp key, Property* prop) {
		// NOOP
	}

	bool StyleSheets::has_property(ViewProp name) const {
		return _props.count(name);
	}

	// ----------------------------------------------------------------------------------------------

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
	class PropertyImpl: public Property {
	public:
		PropertyImpl(ViewProp prop, T value): _prop(prop), _value(value){}
		void apply(Layout *layout) {
			ApplyProp<From>::template apply<T>(layout, _prop, _value);
		}
		Property* copy() {
			// TODO ...
		}
		void transition(Layout *layout, float y, Property *to) {
			// TODO ...
		}
	private:
		ViewProp _prop;
		T _value;
	};

	template<typename T, ViewPropFrom From>
	class PropertyImpl<T*, From>: public Property {
	public:
		typedef T* Type;
		PropertyImpl(ViewProp prop, Type value): _prop(prop), _value(value)
		{
			static_assert(T::Traits::isReference, "Property value must be a reference type");
			_value->retain();
		}
		~PropertyImpl() {
			_value->release();
		}
		void apply(Layout *layout) {
			ApplyProp<From>::template apply<Type>(layout, _prop, _value);
		}
		Property* copy() {
			// TODO ...
		}
		void transition(Layout *layout, float y, Property *to) {
			// TODO ...
		}
	private:
		ViewProp _prop;
		Type _value;
	};

	#define _Fun(Enum, Type, Name, From) void StyleSheets::set_##Name(Type value) {\
		setProp(k##Enum##_ViewProp, new qk::PropertyImpl<Type, k##From>(k##Enum##_ViewProp, value));\
	}
	Qk_View_Props(_Fun)

	#undef _Fun

	// -------------------------------------------------------------------------------------

	CStyleSheets::CStyleSheets(cCSSName &name, CStyleSheets *parent, CSSType type)
		: StyleSheets()
		, _name(name)
		, _parent(parent)
		, _normal(nullptr), _hover(nullptr), _active(nullptr)
		, _havePseudoType(false)
		, _haveSubstyles(false)
		, _type( parent && parent->_type ? parent->_type: type )
	{}

	CStyleSheets::~CStyleSheets() {
		for ( auto i : _substyles )
			Release(i.value);
		for ( auto i : _extends )
			Release(i.value);
		Release(_normal); _normal = nullptr;
		Release(_hover); _hover = nullptr;
		Release(_active); _active = nullptr;
	}

	cCStyleSheets* CStyleSheets::find(cCSSName &name) const {
		auto i = _substyles.find(name.hashCode());
		return i == _substyles.end() ? nullptr : i->value;
	}

	CStyleSheets* CStyleSheets::findAndMake(cCSSName &name, CSSType type, bool isExtend) {
		CStyleSheets *ss;
		CStyleSheetsDict &from = isExtend ? _extends: _substyles;

		if (!from.get(name.hashCode(), ss)) {
			ss = new CStyleSheets(name, isExtend ? _parent: this, kNone_CSSType);
			from[name.hashCode()] = ss;
			_haveSubstyles = _substyles.length();
		}
		if ( !type ) return ss; // no find pseudo type
		if ( ss->_type ) return nullptr; // illegal pseudo cls, 伪类样式表,不能存在子伪类样式表

		// find pseudo type
		CStyleSheets **ss_pseudo = nullptr;
		switch ( type ) { 
			case kNormal_CSSType: ss_pseudo = &ss->_normal; break;
			case kHover_CSSType: ss_pseudo = &ss->_hover; break;
			case kActive_CSSType: ss_pseudo = &ss->_active; break;
		}
		if ( !*ss_pseudo ) {
			ss->_havePseudoType = true;
			*ss_pseudo = new CStyleSheets(name, ss->parent(), type);
		}
		return *ss_pseudo;
	}

	// --------------------------- R o o t . S t y l e . S h e e t s ---------------------------

	static Dict<String, CSSType> Pseudo_type_keys({
		{"normal",kNormal_CSSType},{"hover",kHover_CSSType},{"active",kActive_CSSType}
	});

	RootStyleSheets::RootStyleSheets()
		: CStyleSheets(CSSName(String()), nullptr, kNone_CSSType)
	{}

	Array<CStyleSheets*> RootStyleSheets::search(cString &exp) {
		Array<CStyleSheets*> rv;

		auto searchItem = [this, &rv](cString &exp) {
			#define Qk_InvalidCss(e) { Qk_WARN("Invalid css name \"%s\"", *e); return; }
			CStyleSheets *ss = this;

			for ( auto &j : exp.split(' ') ) { // .div_cls.div_cls2 .aa.bb.cc
				bool isExt = false;
				auto e = j.trim();
				if ( e.isEmpty() ) continue;
				if ( e[0] != '.' ) Qk_InvalidCss(exp);

				for ( auto n: e.split('.') ) { // .div_cls.div_cls2
					if ( n.isEmpty() ) continue;
					auto type = kNone_CSSType;
					auto k = n.split(':'); // .div_cls:hover
					if (k.length() > 1) {
						// normal | hover | down
						if (!Pseudo_type_keys.get(k[1], type))
							Qk_InvalidCss(exp);
						n = k[0];
						if (n.isEmpty()) continue;
					}
					ss = ss->findAndMake(CSSName(n), type, isExt);
					if ( !ss ) Qk_InvalidCss(exp);
					isExt = true;
				}
			}
			if (ss != this)
				rv.push(ss);
			#undef Qk_InvalidCss
		};

		// .div_cls.div_cls2.kkk .aa.bb.cc, .div_cls.div_cls2.ddd:down .aa.bb.cc
		for ( auto &i : exp.split(',') )
			searchItem(i);

		Qk_ReturnLocal(rv);
	}

}
