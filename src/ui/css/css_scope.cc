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
#include "../app.h"
#include "../layout/layout.h"

namespace qk {

	static void push_all_scope(StyleSheetsScope *self, Layout *scope) {
		if ( scope ) {
			push_all_scope(self, scope->parent());
			self->pushScope(scope);
		}
	}

	StyleSheetsScope::StyleSheetsScope(Layout *scope) {
		auto root = shared_app()->styleSheets();
		auto wrap = _styleSheetsMap[root] = { root, 1 };
		_styleSheets.pushBack({ &wrap, 1 });
		push_all_scope(this, scope);
		Qk_DEBUG("use StyleSheetsScope");
	}

	void StyleSheetsScope::pushScope(Layout *scope) {
		Qk_ASSERT(scope);
		auto ssclass = scope->ssclass();
		if ( ssclass && ssclass->haveSubstyles() ) {
			for ( auto i : ssclass->substyleSheets() ) {
				auto _i = const_cast<StyleSheets*>(i);
				Scope::Wrap *wrap = nullptr;
				auto it = _styleSheetsMap.find(_i);
				if ( it == _styleSheetsMap.end() ) { // 添加
					wrap = &(_styleSheetsMap[_i] = { _i, 1 });
				} else {
					wrap = &it->value;
					wrap->ref++;
				}
				_styleSheets.pushBack({ wrap, wrap->ref });
			}
		}
		_scopes.pushBack(scope);
	}

	void StyleSheetsScope::popScope() {
		if ( _scopes.length() ) {
			auto ssclass = _scopes.back()->ssclass();
			if ( ssclass && ssclass->haveSubstyles() ) {
				int count = ssclass->substyleSheets().length();
				for ( int i = 0; i < count; i++ ) {
					Qk_ASSERT( _styleSheets.length() > 1 );
					Scope scope = _styleSheets.back();
					Qk_ASSERT( scope.wrap->ref == scope.ref );
					if ( scope.ref == 1 ) {
						_styleSheetsMap.erase(scope.wrap->sheets);
					} else {
						scope.wrap->ref--;
					}
					_styleSheets.popBack();
				}
			}
			_scopes.popBack();
		}
	}

}
