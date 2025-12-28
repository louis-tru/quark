/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright (c) 2015, Louis.chu
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of Louis.chu nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL Louis.chu BE LIABLE FOR ANY
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
		: _state(kNormal_UIState)
		, _setState(kNormal_UIState)
		, _havePseudoType(false)
		, _firstApply(true)
		, _host(host)
		, _parent(nullptr)
	{
		Qk_ASSERT(host);
		// sizeof(CStyleSheetsClass);
	}

	void CStyleSheetsClass::set(cArray<String> &name) {
		// Qk_DLog("set class: %s", name.join(" ").c_str());
		_async_call([](auto ctx, auto val) {
			Sp<Array<String>> valp(val.arg);
			ctx->_nameHash_rt.clear();
			for ( auto &j: **valp ) {
				auto s = j.trim();
				if (!s.isEmpty()) {
					auto hash = CSSCName(s).hashCode();
					if (hash != UndefinedHashCode) // // exclude undefined
						ctx->_nameHash_rt.add(hash);
				}
			}
			ctx->updateClass_rt();
		}, this, new Array<String>(name));
	}

	void CStyleSheetsClass::add(cString &name) {
		auto name_trimmed = name.trim();
		if (name_trimmed.isEmpty()) return;
		_async_call([](auto ctx, auto hash) {
			if (!ctx->_nameHash_rt.has(hash.arg)) {
				ctx->_nameHash_rt.add(hash.arg);
				ctx->updateClass_rt();
			}
		}, this, CSSCName(name_trimmed).hashCode());
	}

	void CStyleSheetsClass::remove(cString &name) {
		auto name_trimmed = name.trim();
		if (name_trimmed.isEmpty()) return;
		_async_call([](auto ctx, auto hash) {
			auto it = ctx->_nameHash_rt.find(hash.arg);
			if (it != ctx->_nameHash_rt.end()) {
				ctx->_nameHash_rt.erase(it);
				ctx->updateClass_rt();
			}
		}, this, CSSCName(name_trimmed).hashCode());
	}

	void CStyleSheetsClass::toggle(cString &name) {
		auto name_trimmed = name.trim();
		if (name_trimmed.isEmpty()) return;
		_async_call([](auto ctx, auto hash) {
			auto it = ctx->_nameHash_rt.find(hash.arg);
			if (it == ctx->_nameHash_rt.end()) {
				ctx->_nameHash_rt.add(hash.arg);
			} else {
				ctx->_nameHash_rt.erase(it);
			}
			ctx->updateClass_rt();
		}, this, CSSCName(name_trimmed).hashCode());
	}

	void CStyleSheetsClass::updateClass_rt() {
		_host->mark_layout(View::kClass_Change, true);
	}

	void CStyleSheetsClass::setState_rt(UIState state) {
		if ( _setState != state ) {
			_setState = state;
			if ( _havePseudoType ) {
				if (_setState == _state) {
					// Delete change state tags to optimize performance
					_host->unmark(View::kClass_State);
				} else {
					_host->mark_layout(View::kClass_State, true);
				}
			}
		}
	}

	bool CStyleSheetsClass::apply_rt(CStyleSheetsClass *parent, bool force) {
		auto lastHash = _stylesForHaveSubstylesHash_rt.hashCode();
		auto lastStylesHash = _stylesHash_rt.hashCode();
		// clear status
		_stylesHash_rt = Hash5381();
		_havePseudoType = false;
		_parent = parent;
		_state = _setState; // apply state

		auto clearStatus = [this]() {
			// clear transition first
			_host->window()->actionCenter()->removeCSSTransition_rt(_host);
			// reset styles for have substyles
			_stylesForHaveSubstyles_rt.clear();
			_stylesForHaveSubstylesHash_rt = Hash5381();
		};

		if (_nameHash_rt.length()) {
			Array<CStyleSheets*> styles;
			findSubstylesFromParent_rt(parent, &styles);
			// apply styles if changed
			if (lastStylesHash != _stylesHash_rt.hashCode() || force) {
				clearStatus();
				// apply styles
				for (auto ss: styles) {
					if (ss->_time && !_firstApply) {
						_host->window()->actionCenter()->addCSSTransition_rt(_host, ss);
					} else {
						ss->apply(_host, true);
					}
					if (ss->_substyles.length()) {
						_stylesForHaveSubstyles_rt.push(ss);
						_stylesForHaveSubstylesHash_rt.updateu64(uintptr_t(ss));
					}
				}
			}
		} else {
			clearStatus();
		}
		_firstApply = false; // mark first apply false

		// affects children CStyleSheetsClass
		return _stylesForHaveSubstylesHash_rt.hashCode() != lastHash;
	}

	void CStyleSheetsClass::findSubstylesFromParent_rt(CStyleSheetsClass *parent, Array<CStyleSheets*> *out) {
		auto find = [](CStyleSheetsClass *self, CStyleSheets *ss, Array<CStyleSheets*> *out) {
			for (auto &n: self->_nameHash_rt) {
				qk::CStyleSheets *sss;
				if (ss->_substyles.get(n.first, sss)) { // find substyle by class name hash
					self->findStyle_rt(sss, out);
				}
			}
		};
		if (parent) {
			Qk_ASSERT(parent->_stylesForHaveSubstyles_rt.length());
			// find child substyles from parent class
			/*{
				'.parent': {...},
				'.parent .child1': {...},
				'.parent .child2': {...},
			}
			<box class="parent">
				<box class="child1">test a<box>
				<box class="child2">test b<box>
			</box>
			*/
			findSubstylesFromParent_rt(parent->_parent, out); // find styles from parent
			for (auto ss: parent->_stylesForHaveSubstyles_rt) {
				find(this, ss, out); // find sub styles from parent
			}
		} else {
			find(this, RootStyleSheets::shared(), out); // find global root substyles
		}
	}

	// CSS Sample, Expanding form
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
	<box class="a">
		<box class="a_a">test a<box>
		<box class="a_a a_b">test b<box>
	</box>
	*/

	void CStyleSheetsClass::findStyle_rt(CStyleSheets *css, Array<CStyleSheets*> *out) {
		out->push(css);
		_stylesHash_rt.updateu64(uintptr_t(css));

		// find pseudo class
		if (css->_havePseudoType) {
			_havePseudoType = true;

			CStyleSheets *css_pse = nullptr;
			switch (_state) {
				case kNone_UIState: break;
				case kNormal_UIState: css_pse = css->_normal; break;
				case kHover_UIState: css_pse = css->_hover; break;
				case kActive_UIState: css_pse = css->_active; break;
			}
			if (css_pse)
				findStyle_rt(css_pse, out);
		}

		if (css->_extends.length()) { // find extend
			 // more extend, optimize search
			if (css->_extends.length() > _nameHash_rt.length()) {
				for (auto &i: _nameHash_rt) { // test right extend
					CStyleSheets *ss;
					if (css->_extends.get(i.first, ss)) { // test ok
						findStyle_rt(ss, out);
					}
				}
			} else { // less extend, optimize search
				for (auto &i: css->_extends) { // test right extend
					if (_nameHash_rt.has(i.first)) { // test ok
						findStyle_rt(i.second, out);
					}
				}
			}
		}
	}

}
