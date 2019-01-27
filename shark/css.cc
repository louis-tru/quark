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

#include "css.h"
#include "view.h"

XX_NS(shark)

#include "css.cc.inl"

static Map<String, CSSPseudoClass> pseudo_class_table([]() {
	Map<String, CSSPseudoClass> r;
	r.set("normal", CSS_PSEUDO_CLASS_NORMAL);
	r.set("hover", CSS_PSEUDO_CLASS_HOVER);
	r.set("down", CSS_PSEUDO_CLASS_DOWN);
	return r;
}());

static Map<int, String> pseudo_class_table2([]() {
	Map<int, String> r;
	r.set(CSS_PSEUDO_CLASS_NORMAL, ":normal");
	r.set(CSS_PSEUDO_CLASS_HOVER, ":hover");
	r.set(CSS_PSEUDO_CLASS_DOWN, ":down");
	return r;
}());

CSSName::CSSName(const Array<String>& classs)
: m_name(String('.').push(classs.join('.')))
, m_hash(m_name.hash_code()) {
	
}
CSSName::CSSName(cString& n)
: m_name(n)
, m_hash(n.hash_code()) {
	
}

static void mark_classs_names(const CSSName& name);

/**
 * @class StyleSheets::Inl
 */
class StyleSheets::Inl: public StyleSheets {
public:
#define _inl_ss(self) static_cast<StyleSheets::Inl*>(self)
	
	/**
	 * @func set_property_value
	 */
	template<PropertyName Name, class T> inline void set_property_value(T value) {
		typedef CSSProperty<T, Name> Type;
		auto it = m_property.find(Name);
		if ( it.is_null() ) {
			Type* prop = new Type(value);
			m_property.set(Name, prop);
		} else {
			static_cast<Type*>(it.value())->set_value(value);
		}
	}
	
	template<PropertyName Name, class T> inline T get_property_value() {
		typedef CSSProperty<T, Name> Type;
		auto it = m_property.find(Name);
		if ( it.is_null() ) {
			Type* prop = new Type(T());
			m_property.set(Name, prop);
			return prop->value();
		} else {
			return static_cast<Type*>(it.value())->value();
		}
	}
	
	inline StyleSheets* find1(uint hash) {
		auto i = m_children.find(hash);
		return i.is_null() ? nullptr : i.value();
	}
	
	StyleSheets* find2(const CSSName& name, CSSPseudoClass pseudo) {
		
		StyleSheets* ss = nullptr;
		
		auto it = m_children.find(name.hash());
		
		if ( it.is_null() ) {
			mark_classs_names(name);
			ss = new StyleSheets(name, this, CSS_PSEUDO_CLASS_NONE);
			m_children.set(name.hash(), ss);
		} else {
			ss = it.value();
		}
		
		if ( pseudo ) { // pseudo cls
			if ( ss->m_pseudo ) { // illegal pseudo cls, 伪类样式表,不能再存在子的伪类样式表
				return nullptr;
			}
			StyleSheets** pseudo_cls = nullptr;
			switch ( pseudo ) {
				default: break;
				case CSS_PSEUDO_CLASS_NORMAL:
					pseudo_cls = &ss->m_child_NORMAL; break;
				case CSS_PSEUDO_CLASS_HOVER:
					pseudo_cls = &ss->m_child_HOVER; break;
				case CSS_PSEUDO_CLASS_DOWN:
					pseudo_cls = &ss->m_child_DOWN; break;
			}
			
			if ( pseudo_cls ) {
				if ( !*pseudo_cls ) {
					ss->m_is_support_pseudo = true;
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
	
	/**
	 * @func assignment
	 */
	KeyframeAction* assignment(View* view, KeyframeAction* action, bool ignore_action) {
		XX_ASSERT(view);
		
		if ( ! ignore_action && m_time ) { // 创建动作
			
			if ( !action ) {
				action = new KeyframeAction();
				action->add(0); // add frame 0
				action->add(m_time); // add frame 1
				view->action(action); // set action
				action->play(); // start play
			}
			
			Frame* frame = action->frame(1);
			
			for ( auto& i : m_property ) {
				i.value()->assignment(frame);
			}
			
			frame->set_time(m_time);
			
		} else { // 立即设置
			for ( auto& i : m_property ) {
				i.value()->assignment(view);
			}
		}
		return action;
	}
};

StyleSheets::StyleSheets(const CSSName& name, StyleSheets* parent, CSSPseudoClass pseudo)
: m_css_name(name)
, m_parent(parent)
, m_time(0)
, m_child_NORMAL(nullptr)
, m_child_HOVER(nullptr)
, m_child_DOWN(nullptr)
, m_is_support_pseudo(false)
, m_pseudo( parent ? parent->m_pseudo : CSS_PSEUDO_CLASS_NONE )
{
	if ( pseudo ) { // pseudo cls
		XX_ASSERT( !m_pseudo ); // 父样式表为伪样式表,子样式表必须不能为伪样式表
		m_pseudo = pseudo;
	}
}

StyleSheets::~StyleSheets() {
	for ( auto& i : m_children ) {
		Release(i.value());
	}
	for ( auto& i : m_property ) {
		delete i.value();
	}
	Release(m_child_NORMAL); m_child_NORMAL = nullptr;
	Release(m_child_HOVER); m_child_HOVER = nullptr;
	Release(m_child_DOWN); m_child_DOWN = nullptr;
}

// -----

#define xx_def_property(ENUM, TYPE, NAME) \
void StyleSheets::set_##NAME(TYPE value) { \
_inl_ss(this)->set_property_value<ENUM>(value); \
}
XX_EACH_PROPERTY_TABLE(xx_def_property)
#undef xx_def_accessor

template <> BackgroundPtr StyleSheets::Inl::
get_property_value<PROPERTY_BACKGROUND, BackgroundPtr>() {
	typedef CSSProperty<BackgroundPtr, PROPERTY_BACKGROUND> Type;
	auto it = m_property.find(PROPERTY_BACKGROUND);
	if (it.is_null()) {
		Type* prop = new Type(new BackgroundImage());
		m_property.set(PROPERTY_BACKGROUND, prop);
		return prop->value();
	} else {
		return static_cast<Type*>(it.value())->value();
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
	XX_ASSERT(view);
	for ( auto& i : m_property ) {
		i.value()->assignment(view);
	}
}

/**
 * @func assignment
 */
void StyleSheets::assignment(Frame* frame) {
	XX_ASSERT(frame);
	for ( auto& i : m_property ) {
		i.value()->assignment(frame);
	}
}

class RootStyleSheets::Inl: public RootStyleSheets {
public:
#define _inl_r(self) static_cast<RootStyleSheets::Inl*>(self)

	static Array<String>& sort( Array<String>& arr, uint len ) {
		
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
	
	static bool verification_and_format(cString& name, CSSName& out, CSSPseudoClass& pseudo) {
		
		if ( name[0] != '.' ) {
			return false;
		}
		
		int len = name.length();
		int i = name.index_of(':'); // "cls:hover"
		if ( i != -1 ) {
			auto it = pseudo_class_table.find(name.substr(i + 1)); // normal | hover | down
			if ( it.is_null() ) {
				return false;
			} else {
				pseudo = it.value();
			}
			len = i;
		}
		
		Array<String> arr = name.substring(1, len).split('.');
		if ( arr.length() > 1 ) { // ".cls.cls2"
			sort( arr, arr.length() );
		}
		out = CSSName(arr);
		
		return true;
	}
	
	void mark_classs_names(const CSSName& name) {
		m_all_css_names.set(name.hash(), 1);
		m_css_query_group_cache.clear();
	}
	
	// ".div_cls.div_cls2 .aa.bb.cc"
	// ".div_cls.div_cls2:down .aa.bb.cc"
	
	StyleSheets* instance(cString& expression) {
		StyleSheets* ss = this;
		
		for ( auto& i : expression.split(' ') ) {
			CSSName name((String()));
			CSSPseudoClass pseudo = CSS_PSEUDO_CLASS_NONE;
			
			if ( !verification_and_format(i.value().trim(), name, pseudo) ) {
				XX_ERR("Invalid css name \"%s\"", *expression);
				return nullptr;
			}
			
			XX_ASSERT( !name.value().is_empty() );
			
			ss = _inl_ss(ss)->find2(name, pseudo);
			
			if ( ! ss ) {
				XX_ERR("Invalid css name \"%s\"", *expression);
				return nullptr;
			}
		}
		XX_ASSERT( ss != this );
		
		return ss;
	}
	
	Array<uint>* get_css_find_group(uint hash) {
		auto it = m_css_query_group_cache.find(hash);
		if ( it.is_null() ) {
			return nullptr;
		} else {
			return &it.value();
		}
	}
	
	inline void add_css_query_grpup(uint hash, Array<uint>& css_query_group) {
		if ( m_all_css_names.has(hash) ) {
			css_query_group.push(hash);
		}
	}
	
	inline CSSName new_css_name1(cString& a) {
		return CSSName(String('.').push(a));
	}
	
	inline CSSName new_css_name2(cString& a, cString& b) {
		Array<String> r(2);
		r[0] = a;
		r[1] = b;
		sort(r, 2);
		return CSSName(r);
	}
	
	inline CSSName new_css_name3(cString& a, cString& b, cString& c) {
		Array<String> r(3);
		r[0] = a;
		r[1] = b;
		r[2] = c;
		sort(r, 3);
		return CSSName(r);
	}
	
	Array<uint> get_css_query_grpup(Array<String>& classs) {
		uint len = classs.length();
		
		if ( !len ) {
			return Array<uint>();
		}
		
		Array<uint> r;
		
		if ( len == 1 ) {
			add_css_query_grpup(new_css_name1(classs[0]).hash(), r);
			return r;
		}
		
		uint hash = CSSName(len > 4 ? sort(classs, 4).slice(0, 4): sort(classs, len)).hash();
		Array<uint>* group = get_css_find_group(hash);
		
		if ( group && len <= 4 ) {
			return *group;
		}
		
		switch ( len ) {
			case 2:
				add_css_query_grpup(new_css_name1(classs[0]).hash(), r);
				add_css_query_grpup(new_css_name1(classs[1]).hash(), r);
				add_css_query_grpup(hash, r);
				m_css_query_group_cache.set(hash, r);
				break;
			case 3:
				add_css_query_grpup(new_css_name1(classs[0]).hash(), r);
				add_css_query_grpup(new_css_name1(classs[1]).hash(), r);
				add_css_query_grpup(new_css_name1(classs[2]).hash(), r);
				add_css_query_grpup(new_css_name2(classs[0], classs[1]).hash(), r);
				add_css_query_grpup(new_css_name2(classs[0], classs[2]).hash(), r);
				add_css_query_grpup(new_css_name2(classs[1], classs[2]).hash(), r);
				add_css_query_grpup(hash, r);
				m_css_query_group_cache.set(hash, r);
				break;
			default:  // 4...
				if ( group ) { // len > 4
					for ( uint i = 4; i < len; i++ ) {
						add_css_query_grpup(CSSName(classs[i]).hash(), r);
					}
					r.push( *group );
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
				m_css_query_group_cache.set(hash, r);
				// 
				for ( uint i = 4; i < len; i++ ) { // len > 4
					add_css_query_grpup(CSSName(classs[i]).hash(), r);
				}
				break;
		}

		return r;
	}
	
};

static void mark_classs_names(const CSSName& name) {
	_inl_r(root_styles())->mark_classs_names(name);
}

/**
 * @class CSSViewClasss::Inl
 */
class CSSViewClasss::Inl: public CSSViewClasss {
public:
#define _inl_cvc(self) static_cast<CSSViewClasss::Inl*>(self)
	
	void update_classs(Array<String>&& classs) {
		m_classs = move(classs);
		m_query_group = _inl_r(root_styles())->get_css_query_grpup(m_classs);
		m_host->mark_pre(View::M_STYLE_CLASS);
	}
	
	template<bool RETURN_EFFECT_CHILD>
	void apply(StyleSheetsScope* scope, bool* effect_child) {
		
		typedef StyleSheetsScope::Scope Scope;
		
		Map<PrtKey<StyleSheets>, int> origin_child_style_sheets_map;
		
		if ( RETURN_EFFECT_CHILD ) {
			for ( auto& i : m_child_style_sheets ) {
				origin_child_style_sheets_map.set(i.value(), 1);
			}
		}
		
		m_child_style_sheets.clear();
		m_is_support_pseudo = false;
		
		XX_DEBUG("CSSViewClasss apply, query group count: %d, style sheets count: %d, '%s'",
						 m_query_group.length(), scope->style_sheets().length(), m_classs.join(' ').c());
		
		if ( m_query_group.length() ) {
			const List<Scope>& style_sheets = scope->style_sheets();
			Map<PrtKey<StyleSheets>, int> child_style_sheets_map;
			
			KeyframeAction* action = nullptr;
			
			for ( auto& i : m_query_group ) {
				for ( auto& j : style_sheets ) {
					
					Scope scope = j.value();
					if ( scope.ref == scope.wrap->ref ) { // 引用数不相同表示不是最优先的,忽略
						
						StyleSheets* ss = _inl_ss(scope.wrap->sheets)->find1(i.value());
						if ( ss ) {
							
							action = _inl_ss(ss)->assignment(m_host, action, m_once_apply);
							
							if ( ss->has_child() && !child_style_sheets_map.has(ss) ) {
								if ( RETURN_EFFECT_CHILD ) {
									if ( !origin_child_style_sheets_map.has(ss) ) {
										*effect_child = true;
									}
								}
								child_style_sheets_map.set(ss, 1);
								m_child_style_sheets.push(ss);
							}
							
							if ( ss->is_support_pseudo() ) {
								m_is_support_pseudo = true;
								// m_host->set_receive(true);
								switch ( m_multiple_status ) {
									default: ss = nullptr; break;
									case CSS_PSEUDO_CLASS_NORMAL: ss = ss->normal(); break;
									case CSS_PSEUDO_CLASS_HOVER:  ss = ss->hover(); break;
									case CSS_PSEUDO_CLASS_DOWN:   ss = ss->down(); break;
								}
							} else {
								ss = nullptr;
							}
							
							if ( ss ) {
								action = _inl_ss(ss)->assignment(m_host, action, m_once_apply);
								
								if ( ss->has_child() && !child_style_sheets_map.has(ss) ) {
									if ( RETURN_EFFECT_CHILD ) {
										if ( !origin_child_style_sheets_map.has(ss) ) {
											*effect_child = true;
										}
									}
									child_style_sheets_map.set(ss, 1);
									m_child_style_sheets.push(ss);
								}
							}
							//
						} // end if ( scope.ref == scope.wrap->ref ) {
					}
				}
			}
			
			if ( action ) {
				action->frame(0)->fetch(); // fetch 0 frame property
			}
			
			m_once_apply = false;
			
			if ( RETURN_EFFECT_CHILD ) {
				if (child_style_sheets_map.length() !=
						origin_child_style_sheets_map.length() ) {
					*effect_child = true;
				}
			}
		}
	}
	
};

/**
 * @constructor
 */
CSSViewClasss::CSSViewClasss(View* host)
: m_host(host)
, m_is_support_pseudo(false)
, m_once_apply(true)
, m_multiple_status(CSS_PSEUDO_CLASS_NORMAL) {
	XX_ASSERT(host);
}

/**
 * @destructor
 */
CSSViewClasss::~CSSViewClasss() {
}

/**
 * @func names
 */
void CSSViewClasss::name(const Array<String>& value) {
	Map<String, int> new_classs;
	for ( auto& j : value ) {
		new_classs.set(j.value(), 1);
	}
	_inl_cvc(this)->update_classs(new_classs.keys());
}

/**
 * @func add
 */
void CSSViewClasss::add(cString& names) {
	bool up = false;
	
	Map<String, int> new_classs;
	for ( auto& j : m_classs ) {
		new_classs.set(j.value(), 1);
	}
	for ( auto& i : names.split(' ') ) {
		String s = i.value().trim();
		if ( !s.is_empty() ) {
			if ( !new_classs.has(s) ) {
				new_classs.set(s, 1); up = true;
			}
		}
	}
	if ( up ) {
		_inl_cvc(this)->update_classs(new_classs.keys());
	}
}

/**
 * @func remove
 */
void CSSViewClasss::remove(cString& names) {
	bool up = false;
	
	Map<String, int> new_classs;
	for ( auto& j : m_classs ) {
		new_classs.set(j.value(), 1);
	}
	for ( auto& i : names.split(' ') ) {
		String s = i.value().trim();
		if ( !s.is_empty() ) {
			if ( new_classs.del(s) ) up = true;
		}
	}
	if ( up ) {
		_inl_cvc(this)->update_classs(new_classs.keys());
	}
}

/**
 * @func toggle
 */
void CSSViewClasss::toggle(cString& names) {
	bool up = false;
	
	Map<String, int> new_classs;
	for ( auto& j : m_classs ) {
		new_classs.set(j.value(), 1);
	}
	for ( auto& i : names.split(' ') ) {
		String s = i.value().trim();
		if ( !s.is_empty() ) {
			up = true;
			if ( !new_classs.del(s) ) { // no del
				new_classs.set(s, 1);
			}
		}
	}
	if ( up ) {
		_inl_cvc(this)->update_classs(new_classs.keys());
	}
}

/**
 * @func set_style_pseudo_status
 */
void CSSViewClasss::set_style_pseudo_status(CSSPseudoClass status) {
	if ( m_multiple_status != status ) {
		m_multiple_status = status;
		if ( m_is_support_pseudo ) {
			m_host->mark_pre(View::M_STYLE_CLASS);
		}
	}
}

void CSSViewClasss::apply(StyleSheetsScope* scope) {
	_inl_cvc(this)->apply<0>(scope, nullptr);
}

void CSSViewClasss::apply(StyleSheetsScope* scope, bool* effect_child) {
	XX_ASSERT(effect_child);
	_inl_cvc(this)->apply<1>(scope, effect_child);
}

static void push_all_scope(StyleSheetsScope* self, View* scope) {
	if ( scope ) {
		push_all_scope(self, scope->parent());
		self->push_scope(scope);
	}
}

StyleSheetsScope::StyleSheetsScope(View* scope) {
	m_style_sheets.push({
		&m_style_sheets_map.set(root_styles(), { root_styles(), 1 }),
		1
	});
	push_all_scope(this, scope);
	XX_DEBUG("use StyleSheetsScope");
}

void StyleSheetsScope::push_scope(View* scope) {
	XX_ASSERT(scope);
	CSSViewClasss* classs = scope->classs();
	if ( classs && classs->has_child() ) {
		for ( auto& i : classs->child_style_sheets() ) {
			Scope::Wrap* wrap = nullptr;
			auto it = m_style_sheets_map.find(i.value());
			if ( it.is_null() ) { // 添加
				wrap = &m_style_sheets_map.set(i.value(), { i.value(), 1 });
			} else {
				wrap = &it.value();
				wrap->ref++;
			}
			m_style_sheets.push({ wrap, wrap->ref });
		}
	}
	m_scopes.push(scope);
}
void StyleSheetsScope::pop_scope() {
	if ( m_scopes.length() ) {
		CSSViewClasss* classs = m_scopes.last()->classs();
		if ( classs && classs->has_child() ) {
			int count = classs->child_style_sheets().length();
			for ( int i = 0; i < count; i++ ) {
				XX_ASSERT( m_style_sheets.length() > 1 );
				Scope scope = m_style_sheets.last();
				XX_ASSERT( scope.wrap->ref == scope.ref );
				if ( scope.ref == 1 ) {
					m_style_sheets_map.del(scope.wrap->sheets);
				} else {
					scope.wrap->ref--;
				}
				m_style_sheets.pop();
			}
		}
		m_scopes.pop();
	}
}

static RootStyleSheets* root_style_sheets = nullptr;

RootStyleSheets::RootStyleSheets()
: StyleSheets(CSSName(""), nullptr, CSS_PSEUDO_CLASS_NONE) {
}

Array<StyleSheets*> RootStyleSheets::instances(cString& expression) { // TODO
	Array<StyleSheets*> rv;
	
	if ( expression.index_of(',') != -1 ) {
		for ( auto& i : expression.split(',') ) {
			StyleSheets* ss = _inl_r(this)->instance(i.value().trim());
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

XX_END
