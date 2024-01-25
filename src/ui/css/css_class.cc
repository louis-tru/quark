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
		: _haveSubstyles(false)
		, _havePseudoType(false)
		, _firstApply(true)
		, _status(kNormal_CSSType)
		, _host(host)
		, _parent(nullptr)
	{
		Qk_ASSERT(host);
	}

	void StyleSheetsClass::set(cArray<String> &name) {
		qk_async_call([](auto ctx, auto val) {
			Sp<Array<String>> valp(val);
			ctx->_name.clear();
			for ( auto &j: *valp )
				ctx->_name.add(CSSName(j).hashCode());
			ctx->updateClass();
		}, this, new Array<String>(name));
	}

	void StyleSheetsClass::add(cString &name) {
		qk_async_call([](auto ctx, auto hash) {
			if (!_nameHash.has(hash)) {
				_nameHash.add(hash);
				updateClass();
			}
		}, this, CSSName(name).hashCode());
	}

	void StyleSheetsClass::remove(cString &name) {
		qk_async_call([](auto ctx, auto hash) {
			auto it = _nameHash.find(hash);
			if (it != _nameHash.end()) {
				_nameHash.erase(it);
				updateClass();
			}
		}, this, CSSName(name).hashCode());
	}

	void StyleSheetsClass::toggle(cString &name) {
		qk_async_call([](auto ctx, auto hash) {
			auto it = _nameHash.find(hash);
			if (it == _nameHash.end()) {
				_nameHash.add(hash);
			} else {
				_nameHash.erase(it);
			}
			updateClass();
		}, this, CSSName(name).hashCode());
	}

	void StyleSheetsClass::updateClass() {
		_host->mark_layout(Layout::kStyle_Class);
	}

	void StyleSheetsClass::setStatus_RT(CSSType status) {
		if ( _status != status ) {
			_status = status;
			if ( _havePseudoType ) {
				_host->mark_layout(Layout::kStyle_Class);
			}
		}
	}

	void StyleSheetsClass::apply_RT(StyleSheetsClass *parent) {
		auto styles(std::move(_styles));
		_havePseudoType = false;
		_parent = parent;
		if (_nameHash.length())
			applyFrom(parent);
		_firstApply = false;
	}

	void StyleSheetsClass::applyFrom(StyleSheetsClass *ssc) {
		if (ssc) {
			Qk_ASSERT(ssc->_haveSubstyles);
			applyFrom(ssc->_parent);
			if (ssc->_styles.length()) {
				for (auto &it: ssc->_styles) {
					if (it.key->_substyles.length())
						applyFindSubstyle(it.key);
				}
			}
		} else {
			applyFindSubstyle(shared_app()->styleSheets()); // apply global style
		}
	}

	void StyleSheetsClass::applyFindSubstyle(StyleSheets *ss) {
		for (auto &n: _nameHash) {
			auto it = ss->_substyles.find(n.key);
			if (it != ss->_substyles.end()) {
				applyStyle(it->value);
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
		_styles.add(ss);

		if (ss->_substyles.length()) {
			_haveSubstyles = true;
		}

		// apply pseudo class
		if (ss->_havePseudoType) {
			switch (_status) {
				case kNormal_CSSType:
					if (ss->_normal) applyStyle(ss->_normal); break;
				case kHover_CSSType:
					if (ss->_hover) applyStyle(ss->_hover); break;
				case kActive_CSSType:
					if (ss->_active) applyStyle(ss->_active); break;
			}
			_havePseudoType = true;
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
