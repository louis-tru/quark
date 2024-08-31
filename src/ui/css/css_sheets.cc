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
#include "../window.h"

namespace qk {

	CSSCName::CSSCName(cString& name)
		: _hashCode(name.hashCode())
		, _name(name)
	{}

	StyleSheets::StyleSheets() {
	}

	StyleSheets::~StyleSheets() {
		for (auto i: _props)
			delete i.value;
	}

	void StyleSheets::apply(View *view, bool isRt) const {
		Qk_Assert(view);
		if (_props.length()) {
			for ( auto i: _props ) {
				i.value->apply(view, isRt);
			}
		}
	}

	void StyleSheets::fetch(View *view, bool isRt) {
		Qk_Assert(view);
		if (_props.length()) {
			if (isRt) {
				for ( auto i: _props ) {
					i.value->fetch(view);
				}
			} else {
				auto win = getWindowForAsyncSet();
				if (win) {
					win->preRender().async_call([](auto self, auto arg) {
						for ( auto i: self->_props ) {
							i.value->fetch(arg.arg);
						}
					}, this, view);
				}
			}
		}
	}

	void StyleSheets::applyTransition(View* view, StyleSheets *to, float y) const {
		if (_props.length()) {
			Qk_ASSERT(_props.length() == to->_props.length());
			auto a = _props.begin(), e = _props.end();
			auto b = to->_props.begin();
			while (a != e) {
				a->value->transition(view, b->value, y);
				a++; b++;
			}
		}
	}

	Window* StyleSheets::getWindowForAsyncSet() {
		return nullptr;
	}

	void StyleSheets::onMake(ViewProp key, Property* prop) {
		// NOOP
	}

	uint32_t StyleSheets::itemsCount() const {
		return _props.length();
	}

	bool StyleSheets::hasProperty(ViewProp name) const {
		return _props.count(name);
	}

	// --------------------------- C S t y l e . S h e e t s ---------------------------

	CStyleSheets::CStyleSheets(cCSSCName &name, CStyleSheets *parent, CSSType type)
		: StyleSheets()
		, _time(0)
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

	void CStyleSheets::set_time(uint32_t val) {
		_time = val;
	}

	cCStyleSheets* CStyleSheets::find(cCSSCName &name) const {
		auto i = _substyles.find(name.hashCode());
		return i == _substyles.end() ? nullptr : i->value;
	}

	CStyleSheets* CStyleSheets::findAndMake(cCSSCName &name, CSSType type, bool isExtend, bool make) {
		CStyleSheets *ss;
		CStyleSheetsDict &from = isExtend ? _extends: _substyles;

		if (!from.get(name.hashCode(), ss)) {
			if (!make)
				return nullptr;
			ss = new CStyleSheets(name, isExtend ? _parent: this, kNone_CSSType);
			from[name.hashCode()] = ss;
			_haveSubstyles = _substyles.length();
		}
		if ( !type ) return ss; // no find pseudo type
		if ( ss->_type ) return nullptr; // illegal pseudo cls, 伪类样式表,不能存在子伪类样式表

		// find pseudo type
		CStyleSheets **ss_pseudo = nullptr;
		switch ( type ) {
			case kNone_CSSType: break;
			case kNormal_CSSType: ss_pseudo = &ss->_normal; break;
			case kHover_CSSType: ss_pseudo = &ss->_hover; break;
			case kActive_CSSType: ss_pseudo = &ss->_active; break;
		}
		if ( !*ss_pseudo ) {
			if (!make)
				return nullptr;
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
		: CStyleSheets(CSSCName(String()), nullptr, kNone_CSSType)
	{
		auto _button_Normal = searchItem(".qk_button:normal", true);
		auto _button_Hover = searchItem(".qk_button:hover", true);
		auto _button_Active = searchItem(".qk_button:active", true);
		_button_Normal->set_time(180);
		_button_Normal->set_opacity(1);
		_button_Hover->set_time(80);
		_button_Hover->set_opacity(0.7);
		_button_Active->set_time(50);
		_button_Active->set_opacity(0.35);
	}

	CStyleSheets* RootStyleSheets::searchItem(cString &exp, bool make) {
		#define Qk_InvalidCss(e) { Qk_WARN("Invalid css name \"%s\"", *e); return nullptr; }
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
					// normal | hover | active
					if (!Pseudo_type_keys.get(k[1], type))
						Qk_InvalidCss(exp);
					n = k[0];
					if (n.isEmpty()) continue;
				}
				ss = ss->findAndMake(CSSCName(n), type, isExt, make);
				if ( !ss ) {
					if (make)
						Qk_InvalidCss(exp);
					return nullptr;
				}
				isExt = true;
			}
		}
		return ss == this ? nullptr: ss;
		#undef Qk_InvalidCss
	}

	Array<CStyleSheets*> RootStyleSheets::search(cString &exp, bool make) {
		Array<CStyleSheets*> rv;
		// .div_cls.div_cls2.kkk .aa.bb.cc, .div_cls.div_cls2.ddd:active .aa.bb.cc
		for ( auto &i : exp.split(',') ) {
			auto item = searchItem(i, make);
			if (item)
				rv.push(item);
		}
		Qk_ReturnLocal(rv);
	}

}
