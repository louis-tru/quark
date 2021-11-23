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

#include "./css.h"

namespace flare {

	static void push_all_scope(StyleSheetsScope* self, View* scope) {
		if ( scope ) {
			push_all_scope(self, scope->parent());
			self->push_scope(scope);
		}
	}

	StyleSheetsScope::StyleSheetsScope(View* scope) {
		auto wrap = _style_sheets_map[root_styles()] = { root_styles(), 1 };
		_style_sheets.push_back({ &wrap, 1 });
		push_all_scope(this, scope);
		F_DEBUG("use StyleSheetsScope");
	}

	void StyleSheetsScope::push_scope(View* scope) {
		F_ASSERT(scope);
		StyleSheetsClass* classs = scope->classs();
		if ( classs && classs->has_child() ) {
			for ( auto& i : classs->child_style_sheets() ) {
				Scope::Wrap* wrap = nullptr;
				auto it = _style_sheets_map.find(i);
				if ( it == _style_sheets_map.end() ) { // 添加
					wrap = &(_style_sheets_map[i] = { i, 1 });
				} else {
					wrap = &it->value;
					wrap->ref++;
				}
				_style_sheets.push_back({ wrap, wrap->ref });
			}
		}
		_scopes.push_back(scope);
	}
	
	void StyleSheetsScope::pop_scope() {
		if ( _scopes.length() ) {
			StyleSheetsClass* classs = _scopes.back()->classs();
			if ( classs && classs->has_child() ) {
				int count = classs->child_style_sheets().length();
				for ( int i = 0; i < count; i++ ) {
					F_ASSERT( _style_sheets.length() > 1 );
					Scope scope = _style_sheets.back();
					F_ASSERT( scope.wrap->ref == scope.ref );
					if ( scope.ref == 1 ) {
						_style_sheets_map.erase(scope.wrap->sheets);
					} else {
						scope.wrap->ref--;
					}
					_style_sheets.pop_back();
				}
			}
			_scopes.pop_back();
		}
	}
}
