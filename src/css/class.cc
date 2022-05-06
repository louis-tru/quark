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

#include "./_css.h"

F_NAMESPACE_START

void StyleSheetsClass::Inl::update_classs(Array<String>&& classs) {
	_classs = std::move(classs);
	_query_group = _inl_r(root_styles())->get_css_query_grpup(_classs);
	_host->mark_pre(View::M_STYLE_CLASS);
}

void StyleSheetsClass::Inl::apply(
	StyleSheetsScope* scope, bool* effect_child, bool RETURN_EFFECT_CHILD)
{
	typedef StyleSheetsScope::Scope Scope;
	
	Dict<StyleSheets*, int> origin_child_style_sheets_map;
	
	if ( RETURN_EFFECT_CHILD ) {
		for ( auto& i : _child_style_sheets ) {
			origin_child_style_sheets_map[i] = 1;
		}
	}
	
	_child_style_sheets.clear();
	_is_support_pseudo = false;
	
	F_DEBUG("StyleSheetsClass apply, query group count: %d, style sheets count: %d, '%s'",
					_query_group.length(), scope->style_sheets().length(), _classs.join(' ').c_str());
	
	if ( _query_group.length() ) {
		const List<Scope>& style_sheets = scope->style_sheets();
		Dict<StyleSheets*, int> child_style_sheets_map;
		
		KeyframeAction* action = nullptr;
		
		for ( auto& i : _query_group ) {
			for ( auto& j : style_sheets ) {
				
				Scope scope = j;
				if ( scope.ref == scope.wrap->ref ) { // 引用数不相同表示不是最优先的,忽略
					
					StyleSheets* ss = _inl_ss(scope.wrap->sheets)->find1(i);
					if ( ss ) {
						
						action = _inl_ss(ss)->assignment(_host, action, _once_apply);
						
						if ( ss->has_child() && !child_style_sheets_map.count(ss) ) {
							if ( RETURN_EFFECT_CHILD ) {
								if ( !origin_child_style_sheets_map.count(ss) ) {
									*effect_child = true;
								}
							}
							child_style_sheets_map[ss] = 1;
							_child_style_sheets.push(ss);
						}
						
						if ( ss->is_support_pseudo() ) {
							_is_support_pseudo = true;
							// _host->set_receive(true);
							switch ( _multiple_status ) {
								default: ss = nullptr; break;
								case CSS_PSEUDO_CLASS_NORMAL: ss = ss->normal(); break;
								case CSS_PSEUDO_CLASS_HOVER:  ss = ss->hover(); break;
								case CSS_PSEUDO_CLASS_DOWN:   ss = ss->down(); break;
							}
						} else {
							ss = nullptr;
						}
						
						if ( ss ) {
							action = _inl_ss(ss)->assignment(_host, action, _once_apply);
							
							if ( ss->has_child() && !child_style_sheets_map.count(ss) ) {
								if ( RETURN_EFFECT_CHILD ) {
									if ( !origin_child_style_sheets_map.count(ss) ) {
										*effect_child = true;
									}
								}
								child_style_sheets_map[ss] = 1;
								_child_style_sheets.push(ss);
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
		
		_once_apply = false;
		
		if ( RETURN_EFFECT_CHILD ) {
			if (child_style_sheets_map.length() !=
					origin_child_style_sheets_map.length() ) {
				*effect_child = true;
			}
		}
	}
}

/**
* @constructor
*/
StyleSheetsClass::StyleSheetsClass(View* host)
: _host(host)
, _is_support_pseudo(false)
, _once_apply(true)
, _multiple_status(CSS_PSEUDO_CLASS_NORMAL) {
	F_ASSERT(host);
}

/**
* @destructor
*/
StyleSheetsClass::~StyleSheetsClass() {
}

/**
* @func names
*/
void StyleSheetsClass::name(const Array<String>& value) {
	Dict<String, int> new_classs;
	for ( auto& j : value ) {
		new_classs[j] = 1;
	}
	// TODO ...
	// _inl_cvc(this)->update_classs(new_classs.keys());
}

/**
* @func add
*/
void StyleSheetsClass::add(cString& names) {
	bool up = false;
	
	Dict<String, int> new_classs;
	for ( auto& j : _classs ) {
		new_classs[j] = 1;
	}
	for ( auto& i : names.split(' ') ) {
		String s = i.trim();
		if ( !s.is_empty() ) {
			if ( !new_classs.count(s) ) {
				new_classs[s] = 1;
				up = true;
			}
		}
	}
	if ( up ) {
		// TODO ...
		//_inl_cvc(this)->update_classs(new_classs.keys());
	}
}

/**
* @func remove
*/
void StyleSheetsClass::remove(cString& names) {
	bool up = false;
	
	Dict<String, int> new_classs;
	for ( auto& j : _classs ) {
		new_classs[j] = 1;
	}
	for ( auto& i : names.split(' ') ) {
		auto s = i.trim();
		if ( !s.is_empty() ) {
			if ( new_classs.erase(s) ) up = true;
		}
	}
	if ( up ) {
		// TODO ...
		// _inl_cvc(this)->update_classs(new_classs.keys());
	}
}

/**
* @func toggle
*/
void StyleSheetsClass::toggle(cString& names) {
	bool up = false;
	
	Dict<String, int> new_classs;
	for ( auto& j : _classs ) {
		new_classs[j] = 1;
	}
	for ( auto& i : names.split(' ') ) {
		auto s = i.trim();
		if ( !s.is_empty() ) {
			up = true;
			if ( !new_classs.erase(s) ) { // no del
				new_classs[s] = 1;
			}
		}
	}
	if ( up ) {
		// TODO ...
		// _inl_cvc(this)->update_classs(new_classs.keys());
	}
}

/**
* @func set_style_pseudo_status
*/
void StyleSheetsClass::set_style_pseudo_status(CSSPseudoClass status) {
	if ( _multiple_status != status ) {
		_multiple_status = status;
		if ( _is_support_pseudo ) {
			_host->mark_pre(View::M_STYLE_CLASS);
		}
	}
}

void StyleSheetsClass::apply(StyleSheetsScope* scope) {
	_inl_cvc(this)->apply(scope, nullptr, 0);
}

void StyleSheetsClass::apply(StyleSheetsScope* scope, bool* effect_child) {
	F_ASSERT(effect_child);
	_inl_cvc(this)->apply(scope, effect_child, 1);
}


F_NAMESPACE_END