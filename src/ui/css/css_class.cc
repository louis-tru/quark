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

#include "../app.h"
#include "./css.h"
#include "../window.h"
#include "../layout/layout.h"

namespace qk {
	#define qk_async_call _host->window()->preRender().async_call

	StyleSheetsClass::StyleSheetsClass(Layout *host)
		: _havePseudoType(false)
		, _firstApply(true)
		, _host(host)
		, _parent(nullptr)
		, _status(kNormal_CSSType)
		, _setStatus(kNormal_CSSType)
	{
		Qk_ASSERT(host);
	}

	void StyleSheetsClass::set(cArray<String> &name) {
		qk_async_call([](auto ctx, auto val) {
			Sp<Array<String>> valp(val);
			ctx->_nameHash.clear();
			for ( auto &j: **valp )
				ctx->_nameHash.add(CSSName(j).hashCode());
			ctx->updateClass_RT();
		}, this, new Array<String>(name));
	}

	void StyleSheetsClass::add(cString &name) {
		qk_async_call([](auto ctx, auto hash) {
			if (!ctx->_nameHash.has(hash)) {
				ctx->_nameHash.add(hash);
				ctx->updateClass_RT();
			}
		}, this, CSSName(name).hashCode());
	}

	void StyleSheetsClass::remove(cString &name) {
		qk_async_call([](auto ctx, auto hash) {
			auto it = ctx->_nameHash.find(hash);
			if (it != ctx->_nameHash.end()) {
				ctx->_nameHash.erase(it);
				ctx->updateClass_RT();
			}
		}, this, CSSName(name).hashCode());
	}

	void StyleSheetsClass::toggle(cString &name) {
		qk_async_call([](auto ctx, auto hash) {
			auto it = ctx->_nameHash.find(hash);
			if (it == ctx->_nameHash.end()) {
				ctx->_nameHash.add(hash);
			} else {
				ctx->_nameHash.erase(it);
			}
			ctx->updateClass_RT();
		}, this, CSSName(name).hashCode());
	}

	void StyleSheetsClass::updateClass_RT() {
		_host->mark_layout(Layout::kStyle_Class);
		_status = kNone_CSSType;
	}

	void StyleSheetsClass::setStatus_RT(CSSType status) {
		if ( _setStatus != status ) {
			_setStatus = status;
			if ( _havePseudoType ) {
				_host->mark_layout(Layout::kStyle_Class);
			}
		}
	}

	bool StyleSheetsClass::apply_RT(StyleSheetsClass *parent) {
		if (_setStatus == _status) {
			return false;
		}
		auto hash = _stylesHash.hashCode();
		// reset env
		_status = _setStatus;
		_styles.clear();
		_stylesHash = Hash5381();
		_havePseudoType = false;
		_parent = parent;

		if (_nameHash.length()) {
			applyFrom(parent);
		}
		_firstApply = false;

		// affects children StyleSheetsClass
		return _stylesHash.hashCode() != hash;
	}

	void StyleSheetsClass::applyFrom(StyleSheetsClass *ssc) {
		if (ssc) {
			Qk_ASSERT(ssc->_styles.length());
			applyFrom(ssc->_parent);
			for (auto ss: ssc->_styles) {
				applyFindSubstyle(ss);
			}
		} else {
			applyFindSubstyle(shared_app()->styleSheets()); // apply global style
		}
	}

	void StyleSheetsClass::applyFindSubstyle(StyleSheets *ss) {
		for (auto &n: _nameHash) {
			qk::StyleSheets *sss;
			if (ss->_substyles.get(n.key, sss)) {
				applyStyle(sss);
			}
		}
	}

	// CSS Sample
	/**
	.a {
		width: 100;
		.a_a {
			height: 110;
			&:hover {
				height: 170;
			}
		}
		.a_b {
			color: #f00;
		}
		&:hover {
			width: 150;
			.a_a {
				height: 160;
				// ------------------ error ------------------
				&:hover {
					height: 171;
				}
				// ------------------
			}
			.a_b {
				color: #ff0;
			}
		}
	}
	<box ssclass="a">
		<box ssclass="a_a">test a<box>
		<box ssclass="a_a a_b">test b<box>
	</bod>
	*/

	void StyleSheetsClass::applyStyle(StyleSheets *ss) {
		ss->apply(_host);

		if (ss->_substyles.length()) {
			_stylesHash.updateu64(uint64_t(ss));
			_styles.push(ss);
		}

		// apply pseudo class
		if (ss->_havePseudoType) {
			_havePseudoType = true;

			StyleSheets *ss_pt = nullptr;
			switch (_status) {
				case kNormal_CSSType: ss_pt = ss->_normal; break;
				case kHover_CSSType: ss_pt = ss->_hover; break;
				case kActive_CSSType: ss_pt = ss->_active; break;
			}
			if (ss_pt) applyStyle(ss_pt);
		}

		if (ss->_extends.length()) { // apply extend
			for (auto &i: ss->_extends) { // test right extend
				if (_nameHash.has(i.key)) { // test ok
					applyStyle(i.value);
				}
			}
		}
	}

}
