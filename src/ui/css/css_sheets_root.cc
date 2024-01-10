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

namespace qk {

	static Dict<String, CSSType> pseudo_type_keys({
		{"normal",kNormal_CSSType},{"normal",kHover_CSSType},{"active",kActive_CSSType}
	});

	static Array<String>& sortString( Array<String> &arr, uint32_t len ) {
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

	static bool verifyCssName(cString &name, CSSName &out, CSSType &type) {
		if ( name[0] != '.' ) {
			return false;
		}

		int len = name.length();
		int i = name.indexOf(':'); // "cls:hover"
		if ( i != -1 ) {
			auto it = pseudo_type_keys.find(name.substr(i + 1)); // normal | hover | down
			if ( it == pseudo_type_keys.end() ) {
				return false;
			} else {
				type = it->value;
			}
			len = i;
		}

		auto arr = name.substring(1, len).split('.');
		if ( arr.length() > 1 ) { // ".cls.cls2"
			sortString( arr, arr.length() );
		}
		out = CSSName(arr);

		return true;
	}

	// --------------

	RootStyleSheets::RootStyleSheets()
		: StyleSheets(CSSName(""), nullptr, kNone_CSSType)
	{}

	Array<StyleSheets*> RootStyleSheets::search(cString& exp) {
		Array<StyleSheets*> rv;

		if ( exp.indexOf(',') != -1 ) {
			for ( auto& i : exp.split(',') ) {
				auto ss = find(i.trim());
				if ( ss ) {
					rv.push(ss);
				}
			}
		} else {
			auto ss = find(exp.trim());
			if ( ss ) {
				rv.push(ss);
			}
		}

		Qk_ReturnLocal(rv);
	}

	StyleSheets* RootStyleSheets::find(cString& exp) {
		StyleSheets* ss = this;

		for ( auto i : exp.split(' ') ) {
			CSSName name((String()));
			CSSType type = kNone_CSSType;

			if ( !verifyCssName(i.trim(), name, type) ) {
				Qk_WARN("Invalid css name \"%s\"", *exp);
				return nullptr;
			}

			Qk_ASSERT( !name.str().isEmpty() );

			ss = ss->findFrom(name, type);

			if ( ! ss ) {
				Qk_WARN("Invalid css name \"%s\"", *exp);
				return nullptr;
			}
		}
		Qk_ASSERT( ss != this );

		return ss;
	}

	static CSSName newCSSName1(cString &className) {
		return CSSName(String('.').append(className));
	}

	static CSSName newCSSname2(cString& a, cString& b) {
		Array<String> r{a,b};
		return CSSName(sortString(r, 2));
	}

	static CSSName newCSSName3(cString& a, cString& b, cString& c) {
		Array<String> r{a,b,c};
		return CSSName(sortString(r, 3));
	}

	Array<uint32_t> RootStyleSheets::getCssQueryGrpup(Array<String> &className) {
		uint32_t len = className.length();

		if ( !len ) {
			return Array<uint32_t>();
		}
		Array<uint32_t> r;

		auto addGrpup = [this, &r](uint32_t hash) {
			if ( _allClassNames.count(hash) ) {
				r.push(hash);
			}
		};

		auto getCssFindGroup = [this](uint32_t hash) -> Array<uint32_t>* {
			auto it = _cssQueryGroupCache.find(hash);
			if ( it == _cssQueryGroupCache.end() ) {
				return nullptr;
			} else {
				return &it->value;
			}
		};

		if ( len == 1 ) {
			addGrpup(newCSSName1(className[0]).hash());
			return r;
		}

		CSSName cssName(len > 4 ? sortString(className, 4).slice(0, 4): sortString(className, len));
		auto hash = cssName.hash();
		Array<uint32_t>* group = getCssFindGroup(hash);

		if ( group && len <= 4 ) {
			return *group;
		}

		switch ( len ) {
			case 2:
				addGrpup(newCSSName1(className[0]).hash());
				addGrpup(newCSSName1(className[1]).hash());
				addGrpup(hash);
				_cssQueryGroupCache[hash] = r;
				break;
			case 3:
				addGrpup(newCSSName1(className[0]).hash());
				addGrpup(newCSSName1(className[1]).hash());
				addGrpup(newCSSName1(className[2]).hash());
				addGrpup(newCSSname2(className[0], className[1]).hash());
				addGrpup(newCSSname2(className[0], className[2]).hash());
				addGrpup(newCSSname2(className[1], className[2]).hash());
				addGrpup(hash);
				_cssQueryGroupCache[hash] = r;
				break;
			default: // 4 ...
				if ( group ) { // len > 4
					for ( uint32_t i = 4; i < len; i++ ) {
						addGrpup(CSSName(className[i]).hash());
					}
					r.write(**group, group->length());
					return r;
				}
				addGrpup(newCSSName1(className[0]).hash());
				addGrpup(newCSSName1(className[1]).hash());
				addGrpup(newCSSName1(className[2]).hash());
				addGrpup(newCSSName1(className[3]).hash());
				addGrpup(newCSSname2(className[0], className[1]).hash());
				addGrpup(newCSSname2(className[0], className[2]).hash());
				addGrpup(newCSSname2(className[0], className[3]).hash());
				addGrpup(newCSSname2(className[1], className[2]).hash());
				addGrpup(newCSSname2(className[1], className[3]).hash());
				addGrpup(newCSSname2(className[2], className[3]).hash());
				addGrpup(newCSSName3(className[0], className[1], className[2]).hash());
				addGrpup(newCSSName3(className[0], className[1], className[3]).hash());
				addGrpup(newCSSName3(className[1], className[2], className[3]).hash());
				addGrpup(newCSSName3(className[0], className[2], className[3]).hash());
				addGrpup(hash);
				_cssQueryGroupCache[hash] = r;
				for ( uint32_t i = 4; i < len; i++ ) { // len > 4
					addGrpup(CSSName(className[i]).hash());
				}
				break;
		}

		return r;
	}

	void RootStyleSheets::markClassName(cCSSName &name) {
		_allClassNames.add(name.hash());
		_cssQueryGroupCache.clear();
	}

}
