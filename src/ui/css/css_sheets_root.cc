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

	static bool verifyCssName(cString &name, Array<CSSName> *out, CSSType &type) {
		if ( name[0] != '.' )
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

		//auto arr = name.substring(1, len).split('.');
		auto arr = name.split('.');

		// out = CSSName(sortString( arr, arr.length() ));

		return true;
	}

	// --------------

	RootStyleSheets::RootStyleSheets()
		: StyleSheets(CSSName(""), nullptr, kNone_CSSType)
	{}

	Array<StyleSheets*> RootStyleSheets::search(cString &exp) {
		Array<StyleSheets*> rv;

		auto searchItem = [this, &rv](cString &exp) {
			#define Qk_InvalidCss(e) Qk_WARN("Invalid css name \"%s\"", *e); return;
			StyleSheets *ss = this;

			for ( auto &j : exp.split(' ') ) { // .div_cls.div_cls2 .aa.bb.cc
				auto e = j.trim();
				if ( !e.isEmpty() ) {
					if ( e[0] != '.' ) {
						Qk_InvalidCss(exp);
					}

					for ( auto n: e.substr(1).split('.') ) { // .div_cls.div_cls2
						auto type = kNone_CSSType;
						auto k = n.split(':'); // .div_cls:hover
						if (k.length() > 1) {
							auto it = pseudo_type_keys.find(k[1]); // normal | hover | down
							if (it != pseudo_type_keys.end()) {
								type = it->value;
								n = k[0];
							}
						}
					}

					if ( !verifyCssName(e, name, type) ) {
						Qk_InvalidCss(exp);
					}
					Qk_ASSERT( name.hash() != 5381 ); // is empty
					ss = ss->findAndMake(name, type);
					if ( ! ss ) {
						Qk_InvalidCss(exp);
					}
				}
			}
			if (ss != this)
				rv.push(ss);
		};

		// .div_cls.div_cls2.kkk .aa.bb.cc, .div_cls.div_cls2.ddd:down .aa.bb.cc
		for ( auto &i : exp.split(',') )
			searchItem(i);

		Qk_ReturnLocal(rv);
	}

}
