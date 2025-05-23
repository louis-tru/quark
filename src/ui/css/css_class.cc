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
#include "../action/action.h"
#include "../window.h"
#include "../view/view.h"

namespace qk {
	#define _async_call _host->window()->preRender().async_call

	static uint64_t UndefinedHashCode(CSSCName("undefined").hashCode());

	CStyleSheetsClass::CStyleSheetsClass(View *host)
		: _status(kNormal_CSSType)
		, _setStatus(kNormal_CSSType)
		, _havePseudoType(false)
		, _firstApply(true)
		, _host(host)
		, _parent(nullptr)
	{
		Qk_ASSERT(host);
	}

	CStyleSheetsClass::~CStyleSheetsClass() {
		Qk_ASSERT_EQ(_host, nullptr);
	}

	void CStyleSheetsClass::release() {
		auto center = _host->window()->actionCenter();
		if (center) {
			_host->preRender().async_call([](auto self, auto arg) {
				self->removeCSSTransition_Rt(arg.arg);
			}, center, _host);
			_host = nullptr;
		}
		Object::release();
	}

	void CStyleSheetsClass::set(cArray<String> &name) {
		_async_call([](auto ctx, auto val) {
			Sp<Array<String>> valp(val.arg);
			ctx->_nameHash_Rt.clear();
			for ( auto &j: **valp ) {
				auto s = j.trim();
				if (!s.isEmpty()) {
					auto hash = CSSCName(s).hashCode();
					if (hash != UndefinedHashCode) // // exclude undefined
						ctx->_nameHash_Rt.add(hash);
				}
			}
			ctx->updateClass_Rt();
		}, this, new Array<String>(name));
	}

	void CStyleSheetsClass::add(cString &name) {
		_async_call([](auto ctx, auto hash) {
			if (!ctx->_nameHash_Rt.has(hash.arg)) {
				ctx->_nameHash_Rt.add(hash.arg);
				ctx->updateClass_Rt();
			}
		}, this, CSSCName(name.trim()).hashCode());
	}

	void CStyleSheetsClass::remove(cString &name) {
		_async_call([](auto ctx, auto hash) {
			auto it = ctx->_nameHash_Rt.find(hash.arg);
			if (it != ctx->_nameHash_Rt.end()) {
				ctx->_nameHash_Rt.erase(it);
				ctx->updateClass_Rt();
			}
		}, this, CSSCName(name.trim()).hashCode());
	}

	void CStyleSheetsClass::toggle(cString &name) {
		_async_call([](auto ctx, auto hash) {
			auto it = ctx->_nameHash_Rt.find(hash.arg);
			if (it == ctx->_nameHash_Rt.end()) {
				ctx->_nameHash_Rt.add(hash.arg);
			} else {
				ctx->_nameHash_Rt.erase(it);
			}
			ctx->updateClass_Rt();
		}, this, CSSCName(name.trim()).hashCode());
	}

	void CStyleSheetsClass::updateClass_Rt() {
		_host->mark_layout(View::kStyle_Class, true);
		// _status = kNone_CSSType; // force apply update
	}

	void CStyleSheetsClass::setStatus_Rt(CSSType status) {
		if ( _setStatus != status ) {
			_setStatus = status;
			if ( _havePseudoType ) {
				_host->mark_layout(View::kStyle_Class, true);
			}
		}
	}

	bool CStyleSheetsClass::apply_Rt(CStyleSheetsClass *parent) {
		// if (_setStatus == _status) return false;
		auto hash = _substylesHash_Rt.hashCode();
		// reset env
		_status = _setStatus;
		_substyles_Rt.clear();
		_substylesHash_Rt = Hash5381();
		_havePseudoType = false;
		_parent = parent;
		_host->window()->actionCenter()->removeCSSTransition_Rt(_host);

		if (_nameHash_Rt.length()) {
			applyFrom_Rt(parent);
		}
		// _status = _setStatus; // May have been modified, reset it
		_firstApply = false;

		// affects children CStyleSheetsClass
		return _substylesHash_Rt.hashCode() != hash;
	}

	void CStyleSheetsClass::applyFrom_Rt(CStyleSheetsClass *ssc) {
		if (ssc) {
			Qk_ASSERT(ssc->_substyles_Rt.length());
			applyFrom_Rt(ssc->_parent);
			for (auto ss: ssc->_substyles_Rt) {
				applyFindSubstyle_Rt(ss);
			}
		} else {
			applyFindSubstyle_Rt(RootStyleSheets::shared()); // apply global style
		}
	}

	void CStyleSheetsClass::applyFindSubstyle_Rt(CStyleSheets *ss) {
		for (auto &n: _nameHash_Rt) {
			qk::CStyleSheets *sss;
			if (ss->_substyles.get(n.key, sss)) {
				applyStyle_Rt(sss);
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

	void CStyleSheetsClass::applyStyle_Rt(CStyleSheets *css) {
		if (css->_time && !_firstApply) {
			_host->window()->actionCenter()->addCSSTransition_Rt(_host, css);
		} else {
			css->apply(_host, true);
		}

		if (css->_substyles.length()) {
			_substylesHash_Rt.updateu64(uint64_t(css));
			_substyles_Rt.push(css);
		}

		// apply pseudo class
		if (css->_havePseudoType) {
			_havePseudoType = true;

			CStyleSheets *css_pt = nullptr;
			switch (_status) {
				case kNone_CSSType: break;
				case kNormal_CSSType: css_pt = css->_normal; break;
				case kHover_CSSType: css_pt = css->_hover; break;
				case kActive_CSSType: css_pt = css->_active; break;
			}
			if (css_pt) applyStyle_Rt(css_pt);
		}

		if (css->_extends.length()) { // apply extend
			for (auto &i: css->_extends) { // test right extend
				if (_nameHash_Rt.has(i.key)) { // test ok
					applyStyle_Rt(i.value);
				}
			}
		}
	}

}
