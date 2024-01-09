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

	StyleSheetsClass::StyleSheetsClass(Layout* host)
		: _host(host)
		, _havePseudoType(false)
		, _onceApply(true)
		, _status(kNormal_CSSType)
	{
		Qk_ASSERT(host);
	}

	void StyleSheetsClass::set_value(cArray<String> &value) {
		Dict<String, int> set;
		for ( auto& j : value ) {
			set[j] = 1;
		}
		updateClass(set.keys());
	}

	void StyleSheetsClass::add(cString &name) {
		bool up = false;

		Dict<String, int> set;
		for ( auto& j : _value ) {
			set[j] = 1;
		}
		for ( auto& i : name.split(' ') ) {
			String s = i.trim();
			if ( !s.isEmpty() ) {
				if ( !set.count(s) ) {
					set[s] = 1;
					up = true;
				}
			}
		}
		if ( up ) {
			updateClass(set.keys());
		}
	}

	void StyleSheetsClass::remove(cString &name) {
		bool up = false;

		Dict<String, int> set;
		for ( auto& j : _value ) {
			set[j] = 1;
		}
		for ( auto& i : name.split(' ') ) {
			auto s = i.trim();
			if ( !s.isEmpty() ) {
				if ( set.erase(s) ) up = true;
			}
		}
		if ( up ) {
			updateClass(set.keys());
		}
	}

	void StyleSheetsClass::toggle(cString &name) {
		bool up = false;

		Dict<String, int> set;
		for ( auto& j : _value ) {
			set[j] = 1;
		}
		for ( auto& i : name.split(' ') ) {
			auto s = i.trim();
			if ( !s.isEmpty() ) {
				up = true;
				if ( !set.erase(s) ) { // no del
					set[s] = 1;
				}
			}
		}
		if ( up ) {
			updateClass(set.keys());
		}
	}

	void StyleSheetsClass::set_status(CSSType status) {
		if ( _status != status ) {
			_status = status;
			if ( _havePseudoType ) {
				// _host->mark_pre(Layout::M_STYLE_CLASS); // TODO ...
			}
		}
	}

	void StyleSheetsClass::updateClass(Array<String> &&value) {
		_value = std::move(value);
		auto root = shared_app()->styleSheets();
		_queryGroup = root->getCssQueryGrpup(_value);
		// _host->mark_pre(View::M_STYLE_CLASS); // TODO ...
	}

	void StyleSheetsClass::apply(StyleSheetsScope *scope, bool *out_effect_child)
	{
		typedef StyleSheetsScope::Scope Scope;

		Dict<StyleSheets*, int> origin_child_style_sheets_map;

		if ( out_effect_child ) {
			for ( auto& i : _substyleSheets ) {
				origin_child_style_sheets_map[i] = 1;
			}
		}
		_substyleSheets.clear();
		_havePseudoType = false;

		Qk_DEBUG("StyleSheetsClass apply, query group count: %d, style sheets count: %d, '%s'",
						_queryGroup.length(), scope->styleSheets().length(), _value.join(' ').c_str());

		if ( _queryGroup.length() ) {
			const List<Scope>& style_sheets = scope->styleSheets();
			Dict<StyleSheets*, int> child_style_sheets_map;
			// KeyframeAction *action = nullptr;

			for ( auto& i : _queryGroup ) {
				for ( auto& j : style_sheets ) {

					Scope scope = j;
					if ( scope.ref == scope.wrap->ref ) { // 引用数不相同表示不是最优先的,忽略

						StyleSheets* ss = scope.wrap->sheets->findHash(i);
						if ( ss ) {
							// action = _inl_ss(ss)->assignment(_host, action, _once_apply);

							if ( ss->haveSubstyles() && !child_style_sheets_map.count(ss) ) {
								if ( out_effect_child ) {
									if ( !origin_child_style_sheets_map.count(ss) ) {
										*out_effect_child = true;
									}
								}
								child_style_sheets_map[ss] = 1;
								_substyleSheets.push(ss);
							}

							if ( ss->havePseudoType() ) {
								_havePseudoType = true;
								// _host->set_receive(true); // TODO...
								switch ( _status ) {
									default: ss = nullptr; break;
									case kNormal_CSSType: ss = ss->normal(); break;
									case kHover_CSSType:  ss = ss->hover(); break;
									case kActive_CSSType:   ss = ss->active(); break;
								}
							} else {
								ss = nullptr;
							}

							if ( ss ) {
								// action = _inl_ss(ss)->assignment(_host, action, _once_apply);

								if ( ss->haveSubstyles() && !child_style_sheets_map.count(ss) ) {
									if ( out_effect_child ) {
										if ( !origin_child_style_sheets_map.count(ss) ) {
											*out_effect_child = true;
										}
									}
									child_style_sheets_map[ss] = 1;
									_substyleSheets.push(ss);
								}
							}
						} // end if ( scope.ref == scope.wrap->ref ) {
					}
				}
			}

			// if ( action ) {
				// action->frame(0)->fetch(); // fetch 0 frame property
			// }

			_onceApply = false;

			if ( out_effect_child ) {
				if (child_style_sheets_map.length() != origin_child_style_sheets_map.length() ) {
					*out_effect_child = true;
				}
			}
		}
	}

}
