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

#include "./css.inl"
#include "../layout/view.h"

N_NAMESPACE_START

static Dict<String, CSSPseudoClass> pseudo_class_table([]() {
	Dict<String, CSSPseudoClass> r;
	r["normal"] = CSS_PSEUDO_CLASS_NORMAL;
	r["hover"] = CSS_PSEUDO_CLASS_HOVER;
	r["down"] = CSS_PSEUDO_CLASS_DOWN;
	return r;
}());

static Dict<int, String> pseudo_class_table2([]() {
	Dict<int, String> r;
	r[CSS_PSEUDO_CLASS_NORMAL] = ":normal";
	r[CSS_PSEUDO_CLASS_HOVER] = ":hover";
	r[CSS_PSEUDO_CLASS_DOWN] = ":down";
	return r;
}());

CSSName::CSSName(const Array<String>& classs)
	: _name(String('.').append(classs.join(".")))
	, _hash((uint32_t)_name.hash_code())
{
}
CSSName::CSSName(cString& n)
: _name(n)
, _hash((uint32_t)n.hash_code()) {
}

StyleSheets* StyleSheets::Inl::find1(uint32_t hash) {
	auto i = _children.find(hash);
	return i == _children.end() ? nullptr : i->value;
}

StyleSheets* StyleSheets::Inl::find2(const CSSName& name, CSSPseudoClass pseudo) {
	
	StyleSheets* ss = nullptr;
	
	auto it = _children.find(name.hash());
	
	if ( it == _children.end() ) {
		mark_classs_names(name);
		ss = new StyleSheets(name, this, CSS_PSEUDO_CLASS_NONE);
		_children[name.hash()] = ss;
	} else {
		ss = it->value;
	}
	
	if ( pseudo ) { // pseudo cls
		if ( ss->_pseudo ) { // illegal pseudo cls, 伪类样式表,不能再存在子的伪类样式表
			return nullptr;
		}
		StyleSheets** pseudo_cls = nullptr;
		switch ( pseudo ) {
			default: break;
			case CSS_PSEUDO_CLASS_NORMAL:
				pseudo_cls = &ss->_child_NORMAL; break;
			case CSS_PSEUDO_CLASS_HOVER:
				pseudo_cls = &ss->_child_HOVER; break;
			case CSS_PSEUDO_CLASS_DOWN:
				pseudo_cls = &ss->_child_DOWN; break;
		}
		
		if ( pseudo_cls ) {
			if ( !*pseudo_cls ) {
				ss->_is_support_pseudo = true;
				ss = *pseudo_cls = new StyleSheets(name, this, pseudo);
			} else {
				ss = *pseudo_cls;
			}
		} else {
			return nullptr;
		}
	}
	
	return ss;
}

KeyframeAction* StyleSheets::Inl::assignment(View* view, KeyframeAction* action, bool ignore_action) {
	N_ASSERT(view);
	
	if ( ! ignore_action && _time ) { // 创建动作
		
		if ( !action ) {
			action = new KeyframeAction();
			action->add(0); // add frame 0
			action->add(_time); // add frame 1
			view->action(action); // set action
			action->play(); // start play
		}
		
		Frame* frame = action->frame(1);
		
		for ( auto i : _property ) {
			i.value->assignment(frame);
		}
		
		frame->set_time(_time);
		
	} else { // 立即设置
		for ( auto i : _property ) {
			i.value->assignment(view);
		}
	}
	return action;
}

StyleSheets::StyleSheets(const CSSName& name, StyleSheets* parent, CSSPseudoClass pseudo)
	: _css_name(name)
	, _parent(parent)
	, _time(0)
	, _child_NORMAL(nullptr)
	, _child_HOVER(nullptr)
	, _child_DOWN(nullptr)
	, _is_support_pseudo(false)
	, _pseudo( parent ? parent->_pseudo : CSS_PSEUDO_CLASS_NONE )
{
	if ( pseudo ) { // pseudo cls
		N_ASSERT( !_pseudo ); // 父样式表为伪样式表,子样式表必须不能为伪样式表
		_pseudo = pseudo;
	}
}

StyleSheets::~StyleSheets() {
	for ( auto i : _children ) {
		Release(i.value);
	}
	for ( auto i : _property ) {
		delete i.value;
	}
	Release(_child_NORMAL); _child_NORMAL = nullptr;
	Release(_child_HOVER); _child_HOVER = nullptr;
	Release(_child_DOWN); _child_DOWN = nullptr;
}

// -----

#define fx_def_property(ENUM, TYPE, NAME) \
	void StyleSheets::set_##NAME(TYPE value) { \
	_inl_ss(this)->set_property_value<ENUM>(value); \
	}
N_EACH_PROPERTY_TABLE(fx_def_property)
#undef fx_def_accessor

template <> BackgroundPtr StyleSheets::Inl::
get_property_value<PROPERTY_BACKGROUND, BackgroundPtr>() {
	typedef CSSProperty1<BackgroundPtr, PROPERTY_BACKGROUND> Type;
	auto it = _property.find(PROPERTY_BACKGROUND);
	if (it == _property.end()) {
		Type* prop = new Type(new BackgroundImage());
		_property[PROPERTY_BACKGROUND] = prop;
		return prop->value();
	} else {
		return static_cast<Type*>(it->value)->value();
	}
}

/**
* @func background()
*/
BackgroundPtr StyleSheets::background() {
	return _inl_ss(this)->get_property_value<PROPERTY_BACKGROUND, BackgroundPtr>();
}

// -----

StyleSheets* StyleSheets::find(const CSSName& name) {
	return _inl_ss(this)->find1(name.hash());
}

/**
* @func assignment
*/
void StyleSheets::assignment(View* view) {
	N_ASSERT(view);
	for ( auto i : _property ) {
		i.value->assignment(view);
	}
}

/**
* @func assignment
*/
void StyleSheets::assignment(Frame* frame) {
	N_ASSERT(frame);
	for ( auto i : _property ) {
		i.value->assignment(frame);
	}
}

Array<String>& RootStyleSheets::Inl::sort( Array<String>& arr, uint32_t len ) {
	for ( int i = len - 1; i > 0; i-- ) {
		for ( int j = 0; j < i; j++ ) {
			if ( arr[j] > arr[j+1] ) {
				String tmp = arr[j+1];
				arr[j+1] = arr[j];
				arr[j] = tmp;
			}
		}
	}
	return arr;
}

bool RootStyleSheets::Inl::verification_and_format(cString& name, CSSName& out, CSSPseudoClass& pseudo) {
	
	if ( name[0] != '.' ) {
		return false;
	}
	
	int len = name.length();
	int i = name.index_of(':'); // "cls:hover"
	if ( i != -1 ) {
		auto it = pseudo_class_table.find(name.substr(i + 1)); // normal | hover | down
		if ( it == pseudo_class_table.end() ) {
			return false;
		} else {
			pseudo = it->value;
		}
		len = i;
	}
	
	Array<String> arr = name.substring(1, len).split('.');
	if ( arr.size() > 1 ) { // ".cls.cls2"
		sort( arr, (uint32_t)arr.size() );
	}
	out = CSSName(arr);
	
	return true;
}

void RootStyleSheets::Inl::mark_classs_names(const CSSName& name) {
	_all_css_names[name.hash()] = 1;
	_css_query_group_cache.clear();
}

// ".div_cls.div_cls2 .aa.bb.cc"
// ".div_cls.div_cls2:down .aa.bb.cc"

StyleSheets* RootStyleSheets::Inl::instance(cString& expression) {
	StyleSheets* ss = this;
	
	for ( auto i : expression.split(' ') ) {
		CSSName name((String()));
		CSSPseudoClass pseudo = CSS_PSEUDO_CLASS_NONE;
		
		if ( !verification_and_format(i.trim(), name, pseudo) ) {
			N_ERR("Invalid css name \"%s\"", *expression);
			return nullptr;
		}
		
		N_ASSERT( !name.value().is_empty() );
		
		ss = _inl_ss(ss)->find2(name, pseudo);
		
		if ( ! ss ) {
			N_ERR("Invalid css name \"%s\"", *expression);
			return nullptr;
		}
	}
	N_ASSERT( ss != this );
	
	return ss;
}

Array<uint32_t>* RootStyleSheets::Inl::get_css_find_group(uint32_t hash) {
	auto it = _css_query_group_cache.find(hash);
	if ( it == _css_query_group_cache.end() ) {
		return nullptr;
	} else {
		return &it->value;
	}
}

void RootStyleSheets::Inl::add_css_query_grpup(uint32_t hash, Array<uint32_t>& css_query_group) {
	if ( _all_css_names.count(hash) ) {
		css_query_group.push(hash);
	}
}

CSSName RootStyleSheets::Inl::new_css_name1(cString& a) {
	return CSSName(String('.').append(a));
}

CSSName RootStyleSheets::Inl::new_css_name2(cString& a, cString& b) {
	Array<String> r(2);
	r[0] = a;
	r[1] = b;
	sort(r, 2);
	return CSSName(r);
}

CSSName RootStyleSheets::Inl::new_css_name3(cString& a, cString& b, cString& c) {
	Array<String> r(3);
	r[0] = a;
	r[1] = b;
	r[2] = c;
	sort(r, 3);
	return CSSName(r);
}

Array<uint32_t> RootStyleSheets::Inl::get_css_query_grpup(Array<String>& classs) {
	uint32_t len = (uint32_t)classs.size();
	
	if ( !len ) {
		return Array<uint32_t>();
	}
	
	Array<uint32_t> r;
	
	if ( len == 1 ) {
		add_css_query_grpup(new_css_name1(classs[0]).hash(), r);
		return r;
	}
	
	uint32_t hash = CSSName(len > 4 ? sort(classs, 4).slice(0, 4): sort(classs, len)).hash();
	Array<uint32_t>* group = get_css_find_group(hash);
	
	if ( group && len <= 4 ) {
		return *group;
	}
	
	switch ( len ) {
		case 2:
			add_css_query_grpup(new_css_name1(classs[0]).hash(), r);
			add_css_query_grpup(new_css_name1(classs[1]).hash(), r);
			add_css_query_grpup(hash, r);
			_css_query_group_cache[hash] = r;
			break;
		case 3:
			add_css_query_grpup(new_css_name1(classs[0]).hash(), r);
			add_css_query_grpup(new_css_name1(classs[1]).hash(), r);
			add_css_query_grpup(new_css_name1(classs[2]).hash(), r);
			add_css_query_grpup(new_css_name2(classs[0], classs[1]).hash(), r);
			add_css_query_grpup(new_css_name2(classs[0], classs[2]).hash(), r);
			add_css_query_grpup(new_css_name2(classs[1], classs[2]).hash(), r);
			add_css_query_grpup(hash, r);
			_css_query_group_cache[hash] = r;
			break;
		default:  // 4...
			if ( group ) { // len > 4
				for ( uint32_t i = 4; i < len; i++ ) {
					add_css_query_grpup(CSSName(classs[i]).hash(), r);
				}
				r.write(*group);
				return r;
			}
			add_css_query_grpup(new_css_name1(classs[0]).hash(), r);
			add_css_query_grpup(new_css_name1(classs[1]).hash(), r);
			add_css_query_grpup(new_css_name1(classs[2]).hash(), r);
			add_css_query_grpup(new_css_name1(classs[3]).hash(), r);
			add_css_query_grpup(new_css_name2(classs[0], classs[1]).hash(), r);
			add_css_query_grpup(new_css_name2(classs[0], classs[2]).hash(), r);
			add_css_query_grpup(new_css_name2(classs[0], classs[3]).hash(), r);
			add_css_query_grpup(new_css_name2(classs[1], classs[2]).hash(), r);
			add_css_query_grpup(new_css_name2(classs[1], classs[3]).hash(), r);
			add_css_query_grpup(new_css_name2(classs[2], classs[3]).hash(), r);
			add_css_query_grpup(new_css_name3(classs[0], classs[1], classs[2]).hash(), r);
			add_css_query_grpup(new_css_name3(classs[0], classs[1], classs[3]).hash(), r);
			add_css_query_grpup(new_css_name3(classs[1], classs[2], classs[3]).hash(), r);
			add_css_query_grpup(new_css_name3(classs[0], classs[2], classs[3]).hash(), r);
			add_css_query_grpup(hash, r);
			_css_query_group_cache[hash] = r;
			// 
			for ( uint32_t i = 4; i < len; i++ ) { // len > 4
				add_css_query_grpup(CSSName(classs[i]).hash(), r);
			}
			break;
	}

	return r;
}

void mark_classs_names(const CSSName& name) {
	_inl_r(root_styles())->mark_classs_names(name);
}

static RootStyleSheets* root_style_sheets = nullptr;

RootStyleSheets::RootStyleSheets()
: StyleSheets(CSSName(""), nullptr, CSS_PSEUDO_CLASS_NONE) {
}

Array<StyleSheets*> RootStyleSheets::instances(cString& expression) { // TODO
	Array<StyleSheets*> rv;
	
	if ( expression.index_of(',') != -1 ) {
		for ( auto& i : expression.split(',') ) {
			StyleSheets* ss = _inl_r(this)->instance(i.trim());
			if ( ss ) {
				rv.push(ss);
			}
		}
	} else {
		StyleSheets* ss = _inl_r(this)->instance(expression.trim());
		if ( ss ) {
			rv.push(ss);
		}
	}
	
	return rv;
}

/**
* @func shared
*/
RootStyleSheets* RootStyleSheets::shared() {
	if ( !root_style_sheets ) {
		root_style_sheets = new RootStyleSheets();
	}
	return root_style_sheets;
}

N_NAMESPACE_END