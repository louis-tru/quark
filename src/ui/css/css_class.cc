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

	StyleSheetsClass::StyleSheetsClass(Layout *host)
		: _host(host)
		, _havePseudoType(false)
		, _onceApply(true)
		, _status(kNormal_CSSType)
	{
		Qk_ASSERT(host);
	}

	void StyleSheetsClass::set(cArray<String> &name) {
		_name.clear();
		for ( auto &j: name )
			_name.add(CSSName(j));
		updateClass();
	}

	void StyleSheetsClass::add(cString &name) {
		CSSName cn(name);
		if (!_name.has(cn)) {
			_name.add(cn);
			updateClass();
		}
	}

	void StyleSheetsClass::remove(cString &name) {
		auto it = _name.find(CSSName(name));
		if (it != _name.end()) {
			_name.erase(it);
			updateClass();
		}
	}

	void StyleSheetsClass::toggle(cString &name) {
		CSSName cn(name);
		auto it = _name.find(cn);
		if (it == _name.end()) {
			_name.add(cn);
		} else {
			_name.erase(it);
		}
		updateClass();
	}

	void StyleSheetsClass::setStatus(CSSType status) {
		if ( _status != status ) {
			_status = status;
			if ( _havePseudoType ) {
				_host->mark_layout(Layout::kStyle_Class);
			}
		}
	}

	void StyleSheetsClass::updateClass() {
		_host->mark_layout(Layout::kStyle_Class);
	}

	void StyleSheetsClass::apply(StyleSheetsScope *scope, bool *out_effectChild)
	{
		// typedef StyleSheetsScope::Scope Scope;

		// Set<StyleSheets*> origin_child_style_sheets_set;

		// if ( out_effectChild ) {
		// 	for ( auto &i : _substyleSheets ) {
		// 		origin_child_style_sheets_set.add(i);
		// 	}
		// }
		// _substyleSheets.clear();
		// _havePseudoType = false;

		// Qk_DEBUG("StyleSheetsClass apply, query group count: %d, style sheets count: %d, '%s'",
		// 				_queryGroup.length(), scope->styleSheets().length(), _name.keys().join(' ').c_str());

		// if ( !_queryGroup.length() ) return;

		// cList<Scope>& style_sheets = scope->styleSheets();
		// Set<StyleSheets*> child_style_sheets_set;
		// // KeyframeAction *action = nullptr;

		// for ( auto& i : _queryGroup ) {
		// 	for ( auto& j : style_sheets ) {

		// 		Scope scope = j;
		// 		if ( scope.ref == scope.wrap->ref ) { // 引用数不相同表示不是最优先的,忽略

		// 			StyleSheets* ss = scope.wrap->sheets->findHash(i);
		// 			if ( ss ) {
		// 				// action = _inl_ss(ss)->assignment(_host, action, _once_apply);

		// 				if ( ss->haveSubstyles() && !child_style_sheets_set.count(ss) ) {
		// 					if ( out_effectChild ) {
		// 						if ( !origin_child_style_sheets_set.count(ss) ) {
		// 							*out_effectChild = true;
		// 						}
		// 					}
		// 					child_style_sheets_set.add(ss);
		// 					_substyleSheets.push(ss);
		// 				}

		// 				if ( ss->havePseudoType() ) {
		// 					_havePseudoType = true;
		// 					// _host->set_receive(true); // TODO...
		// 					switch ( _status ) {
		// 						default: ss = nullptr; break;
		// 						case kNormal_CSSType: ss = ss->normal(); break;
		// 						case kHover_CSSType:  ss = ss->hover(); break;
		// 						case kActive_CSSType: ss = ss->active(); break;
		// 					}
		// 				} else {
		// 					ss = nullptr;
		// 				}

		// 				if ( ss ) {
		// 					// action = _inl_ss(ss)->assignment(_host, action, _once_apply);

		// 					if ( ss->haveSubstyles() && !child_style_sheets_set.count(ss) ) {
		// 						if ( out_effectChild ) {
		// 							if ( !origin_child_style_sheets_set.count(ss) ) {
		// 								*out_effectChild = true;
		// 							}
		// 						}
		// 						child_style_sheets_set.add(ss);
		// 						_substyleSheets.push(ss);
		// 					}
		// 				}
		// 			} // end if ( scope.ref == scope.wrap->ref ) {
		// 		}
		// 	}
		// }

		// // if ( action ) {
		// 	// action->frame(0)->fetch(); // fetch 0 frame property
		// // }

		// _onceApply = false;

		// if ( out_effectChild ) {
		// 	if (child_style_sheets_set.length() != origin_child_style_sheets_set.length() ) {
		// 		*out_effectChild = true;
		// 	}
		// }
	}
}
