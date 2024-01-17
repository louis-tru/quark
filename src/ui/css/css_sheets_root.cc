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
		{"normal",kNormal_CSSType},{"hover",kHover_CSSType},{"active",kActive_CSSType}
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
		if ( name.isEmpty() || name[0] != '.' )
			return false;

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
		out = CSSName(sortString( arr, arr.length() ));

		return true;
	}

	static CSSName newCSSname2(cString& a, cString& b) {
		Array<String> arr{a,b};
		return CSSName(sortString(arr, 2));
	}

	static CSSName newCSSName3(cString& a, cString& b, cString& c) {
		Array<String> arr{a,b,c};
		return CSSName(sortString({arr}, 3));
	}

	// --------------

	RootStyleSheets::RootStyleSheets()
		: StyleSheets(CSSName(""), nullptr, kNone_CSSType)
	{}

	Array<StyleSheets*> RootStyleSheets::search(cString &exp) {
		Array<StyleSheets*> rv;

		auto searchItem = [](StyleSheets* self, cString &exp) -> StyleSheets* {
			StyleSheets *ss = self;
			CSSName name((String()));
			// ".div_cls.div_cls2 .aa.bb.cc"
			// ".div_cls.div_cls2:down .aa.bb.cc"
			for ( auto i : exp.split(' ') ) {
				CSSType type = kNone_CSSType;
				if ( !verifyCssName(i.trim(), name, type) ) {
					Qk_WARN("Invalid css name \"%s\"", *exp); return nullptr;
				}
				Qk_ASSERT( name.hash() != 5381 ); // is empty
				ss = ss->findAndMake(name, type);
				if ( ! ss ) {
					Qk_WARN("Invalid css name \"%s\"", *exp); return nullptr;
				}
			}
			Qk_ASSERT( ss != self );
			return ss;
		};

		// .div_cls.div_cls2 .aa.bb.cc, .div_cls.div_cls2:down .aa.bb.cc
		for ( auto& i : exp.split(',') ) {
			auto ss = searchItem(this, i.trim());
			if ( ss ) {
				rv.push(ss);
			}
		}
		Qk_ReturnLocal(rv);
	}

	Array<uint64_t> RootStyleSheets::getCssQueryGrpup(Array<String> &className) {
		uint32_t len = className.length();
		if ( !len ) {
			return Array<uint64_t>();
		}
		Array<uint64_t> r;

		auto add = [this, &r](CSSName name) {
			if ( _allClassNames.count(name.hash()) ) {
				r.push(name.hash());
			}
		};

		if ( len == 1 ) {
			add(CSSName(className[0]));
			return r;
		}

		CSSName part(len > 4 ? sortString(className, 4).slice(0, 4): sortString(className, len));
		cArray<uint64_t> *group = nullptr;

		if ( _cssQueryGroupCache.get(part.hash(), group) && len <= 4 ) {
			return *group;
		}

		switch ( len ) {
			case 2:
				add(CSSName(className[0]));
				add(CSSName(className[1]));
				add(part);
				_cssQueryGroupCache[part.hash()] = r;
				break;
			case 3:
				add(CSSName(className[0]));
				add(CSSName(className[1]));
				add(CSSName(className[2]));
				add(newCSSname2(className[0], className[1]));
				add(newCSSname2(className[0], className[2]));
				add(newCSSname2(className[1], className[2]));
				add(part);
				_cssQueryGroupCache[part.hash()] = r;
				break;
			default: // 4 ...
				if ( group ) { // len > 4
					for ( uint32_t i = 4; i < len; i++ ) {
						add(CSSName(className[i]));
					}
					r.write(**group, group->length());
					return r;
				}
				add(CSSName(className[0]));
				add(CSSName(className[1]));
				add(CSSName(className[2]));
				add(CSSName(className[3]));
				add(newCSSname2(className[0], className[1]));
				add(newCSSname2(className[0], className[2]));
				add(newCSSname2(className[0], className[3]));
				add(newCSSname2(className[1], className[2]));
				add(newCSSname2(className[1], className[3]));
				add(newCSSname2(className[2], className[3]));
				add(newCSSName3(className[0], className[1], className[2]));
				add(newCSSName3(className[0], className[1], className[3]));
				add(newCSSName3(className[1], className[2], className[3]));
				add(newCSSName3(className[0], className[2], className[3]));
				add(part);
				_cssQueryGroupCache[part.hash()] = r;

				for ( uint32_t i = 4; i < len; i++ ) { // len > 4
					add(CSSName(className[i]));
				}
				break;
		}

		return r;
	}

	void RootStyleSheets::markClassName(CSSName name) {
		_allClassNames.add(name.hash());
		_cssQueryGroupCache.clear();
	}

}
