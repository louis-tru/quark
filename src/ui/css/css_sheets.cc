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
#include "../window.h"

namespace qk {

	static cDict<String, UIState> UIStateKeys({
		{"normal",kNormal_UIState},{"hover",kHover_UIState},{"active",kActive_UIState}
	});

	static cDict<uint32_t, String> UIStateNames({
		{kNormal_UIState, "normal"}, {kHover_UIState,  "hover"}, {kActive_UIState, "active"}
	});

	static String getUIStateName(UIState state) {
		String name;
		UIStateNames.get(state, name);
		return name;
	}

	/**
	* Represents a single CSS class name.
	*
	* This is a lightweight immutable value object containing:
	* - A precomputed 64-bit hash for fast selector matching
	* - The original string (for debugging / diagnostics)
	*
	* IMPORTANT:
	* - Selector matching ALWAYS uses hashCode.
	* - String name is never used in hot paths.
	* - CSSCName instances are cheap and can be created freely.
	*/
	CSSCName::CSSCName(cString& name)
		: _hashCode(name.hashCode())
		, _name(name)
	{}
	
	/**
	* Container for resolved style properties.
	*
	* IMPORTANT:
	* - This class does NOT represent selectors.
	* - It only stores CssProp -> Property mappings.
	* - Selector semantics are added by CStyleSheets.
	 */
	StyleSheets::StyleSheets() {
	}

	StyleSheets::~StyleSheets() {
		// Owns all Property instances
		for (auto i: _props)
			delete i.second;
	}

	/**
	* Apply all stored style properties to the given view,
	* resolving style priority for each property.
	* @param view Target view
	*/
	void StyleSheets::apply_with_priority(View *view) const {
		Qk_ASSERT_NE(view, nullptr);
		if (_props.length()) {
			for ( auto i: _props ) {
				i.second->apply_with_priority(view);
			}
		}
	}

	/**
	* Apply all stored style properties to the given view.
	*
	* @param view Target view
	* @param isRt Whether this call is on render thread
	 */
	void StyleSheets::apply(View *view, bool isRt) const {
		Qk_ASSERT_NE(view, nullptr);
		if (_props.length()) {
			if (isRt) {
				for ( auto i: _props ) {
					i.second->apply(view);
				}
			} else {
				auto win = getWindow();
				if (win) {
					win->pre_render().async_call([](auto self, auto arg) {
						for ( auto i: self->_props ) {
							i.second->apply(arg.arg);
						}
					}, const_cast<StyleSheets*>(this), view);
				}
			}
		}
	}

	/**
	* Fetch current property values from the view.
	*
	* Used as the starting point for CSS transitions.
	*
	* If called outside render thread, this method schedules
	* an async fetch on the render thread.
	*/
	void StyleSheets::fetch(View *view, bool isRt) {
		Qk_ASSERT_NE(view, nullptr);
		if (isRt) {
			for ( auto i: _props ) {
				i.second->fetch(view);
			}
		} else {
			auto win = getWindow();
			if (win) {
				win->pre_render().async_call([](auto self, auto arg) {
					for ( auto i: self->_props ) {
						i.second->fetch(arg.arg);
					}
				}, this, view);
			}
		}
	}

	/**
	* Interpolate between this StyleSheets and another one.
	*
	* This method assumes:
	* - Both StyleSheets contain the same set of properties
	* - Properties are iterated in the same order
	*/
	void StyleSheets::applyTransition(View* view, StyleSheets *to, float y) const {
		if (_props.length()) {
			Qk_ASSERT(_props.length() == to->_props.length());
			auto a = _props.begin(), e = _props.end();
			auto b = to->_props.begin();
			while (a != e) {
				a->second->transition(view, b->second, y);
				a++; b++;
			}
		}
	}

	// Return window used for async property updates.
	// Base implementation returns nullptr.
	Window* StyleSheets::getWindow() const {
		return nullptr;
	}

	// Override point for subclasses to react to property creation.
	void StyleSheets::onMake(CssProp key, Property* prop) {
		// NOOP
	}

	// --------------------------- C S t y l e . S h e e t s ---------------------------

	// Represents a node in the selector tree.
	//
	// Each node corresponds to ONE selector segment (class + optional pseudo).
	CStyleSheets::CStyleSheets(cCSSCName &name, CStyleSheets *parent, UIState state, bool directChildOnly)
		: StyleSheets()
		, _time(0)                      // transition duration
		, _parent(parent)               // parent selector node
		, _normal(nullptr)
		, _hover(nullptr)
		, _active(nullptr)
		, _havePseudoType(false)        // whether this node has pseudo variants
		, _haveSubstyles(false)         // whether this node has descendant selectors
		, _state(parent && parent->_state ? parent->_state : state)
		, _name(name)
		, _directChildOnly(directChildOnly)
	{}

	CStyleSheets::~CStyleSheets() {
		// Release descendant selector nodes
		for ( auto i : _substyles )
			Release(i.second);

		// Release continuous selector nodes
		for ( auto i : _ext )
			Release(i.second);

		// Release pseudo selector variants
		Releasep(_normal);
		Releasep(_hover);
		Releasep(_active);
	}

	void CStyleSheets::set_time(uint32_t val) {
		_time = val;
	}

	/**
	* Find descendant selector by class name.
	*
	* NOTE:
	* - Only searches `_substyles` (space-separated selectors).
	* - Does NOT search `_next` (continuous selectors).
	*/
	cCStyleSheets* CStyleSheets::find(cCSSCName &name) const {
		auto i = _substyles.find(name.hashCode());
		return i == _substyles.end() ? nullptr : i->second;
	}
	

	/**
	* Find or create a selector node.
	*
	* @param name   Selector class name
	* @param state  Pseudo state (normal / hover / active)
	* @param isExt  Whether this segment is a continuous selector (no space)
	* @param make   Whether to create node if not found
	* @param directChildOnly Whether this selector is for direct child only
	*
	* Selector examples:
	*   ".a .b"      → isExt = false (use _substyles)
	*   ".a.b"       → isExt = true  (use _ext)
	*   ".a:hover"   → state != kNone_UIState
	*/
	CStyleSheets* CStyleSheets::findAndMake(cCSSCName &name, UIState state, bool isExt, bool make, bool directChildOnly) {
		if (isExt)
			directChildOnly = false; // continuous selector cannot be direct-child-only
		CStyleSheets *ss;
		CStyleSheetsDict &from = isExt ? _ext : _substyles;

		// Find or create base selector node
		if (!from.get(name.hashCode(), ss)) {
			if (!make)
				return nullptr;

			if (_state && isExt) {
				Qk_Warn("Invalid selector: pseudo state must be the last suffix; cannot chain \".%s\" after :%s",
					*name.name(), getUIStateName(_state).c_str());
				return nullptr;
			}

			// For continuous selectors (.a.b),
			// parent remains the same selector context.
			// For descendant selectors (.a .b),
			// parent becomes the current node.
			ss = new CStyleSheets(name, isExt ? _parent : this, kNone_UIState, directChildOnly);
			from[name.hashCode()] = ss;

			_haveSubstyles = _substyles.length();
		}

		if (directChildOnly != ss->_directChildOnly) {
			Qk_Warn("Selector conflict: \"%s\" cannot be used as both a "
				"descendant selector and a direct-child selector", *name.name());
			return nullptr;
		}

		// No pseudo selector requested
		if (!state)
			return ss;

		// Illegal: pseudo selector cannot have sub-pseudo selectors
		if (ss->_state) {
			Qk_Warn("Invalid selector: pseudo selector \"%s\" cannot have sub-pseudo selectors",
				*name.name());
			return nullptr;
		}

		// Resolve pseudo selector
		CStyleSheets **ss_pseudo = nullptr;
		switch (state) {
			case kNone_UIState: break;
			case kNormal_UIState: ss_pseudo = &ss->_normal; break;
			case kHover_UIState:  ss_pseudo = &ss->_hover;  break;
			case kActive_UIState: ss_pseudo = &ss->_active; break;
		}

		if (!*ss_pseudo) {
			if (!make)
				return nullptr;

			ss->_havePseudoType = true;
			*ss_pseudo = new CStyleSheets(name, ss->parent(), state, false);
		}
		return *ss_pseudo;
	}

	// --------------------------- R o o t . S t y l e . S h e e t s ---------------------------

	/**
	* Return global shared selector tree root.
	*
	* NOTE:
	* - Selector tree is shared across all windows.
	* - Modifications require locking render threads.
	 */
	RootStyleSheets* RootStyleSheets::shared() {
		static RootStyleSheets *instance = new RootStyleSheets();
		return instance;
	}

	// Initialize root selector tree and built-in styles.
	RootStyleSheets::RootStyleSheets()
		: CStyleSheets(CSSCName(String()), nullptr, kNone_UIState, false)
	{
		// Built-in button styles
		auto _button_Normal = searchItem(".qk_button:normal", true);
		auto _button_Hover = searchItem(".qk_button:hover", true);
		auto _button_Active = searchItem(".qk_button:active", true);
		_button_Normal->set_time(180);
		_button_Normal->set_color({255,255,255,255});
		_button_Hover->set_time(80);
		_button_Hover->set_color({255,255,255,uint8_t(255*0.7)});
		_button_Active->set_time(50);
		_button_Active->set_color({255,255,255,uint8_t(255*0.35)});
	}

	/**
	 * Parse a selector expression and return the corresponding selector node.
	 *
	 * Supported syntax:
	 * - Descendant selectors (space-separated):
	 *     ".a .b .c"
	 * - Direct child selectors:
	 *     ".a > .b > .c"
	 * - Continuous class selectors (no spaces):
	 *     ".a.b.c"
	 * - Pseudo states (must appear as the final suffix):
	 *     ":normal", ":hover", ":active"
	 *
	 * Example:
	 *     ".div_cls.div_cls2:active .aa.bb.cc"
	 *
	 * Notes:
	 * - Only class selectors are supported.
	 * - Pseudo states cannot be chained or followed by additional class selectors.
	 * - Selector semantics are resolved during parsing and construction.
	 */
	CStyleSheets* RootStyleSheets::searchItem(cString &exp, bool make) {
		CStyleSheets *ss = this;
		bool directChildOnly = false;

		auto invalid = [](cString& e) {
			Qk_Warn("Invalid css name \"%s\"", *e);
			return (CStyleSheets*)nullptr;
		};

		// Split by '>' → direct child selectors
		for ( auto &i : exp.split('>') ) { // direct child only
			auto part = i.trim();
			if ( part.is_empty() ) continue;

			// Split by spaces → descendant selectors
			for ( auto &j : part.split(' ') ) { // .div_cls.div_cls2 .aa.bb.cc
				bool isExt = false; // is extend selector
				auto e = j.trim();
				if ( e.is_empty() ) continue;
				if ( e[0] != '.' ) return invalid(exp);

				// Split by '.' → continuous selectors
				for ( auto &n: e.split('.') ) { // .div_cls.div_cls2
					if ( n.is_empty() ) continue;
					auto state = kNone_UIState;
					// Handle pseudo selector
					auto k = n.split(':'); // .div_cls:hover
					if (k.length() > 1) {
						// normal | hover | active
						if (!UIStateKeys.get(k[1], state))
							return invalid(exp);
						n = k[0];
						if (n.is_empty()) continue;
					}
					ss = ss->findAndMake(CSSCName(n), state, isExt, make, directChildOnly);
					if ( !ss ) {
						if (make)
							return invalid(exp);
						return nullptr;
					}
					isExt = true;
				}
				directChildOnly = false; // reset for next part
			}
			directChildOnly = true; // next part is direct child only
		}
		return ss == this ? nullptr: ss;
	}

	Array<CStyleSheets*> RootStyleSheets::search(cString &exp, bool make) {
		Array<CStyleSheets*> rv;
		// .div_cls.div_cls2.kkk .aa.bb.cc, .div_cls.div_cls2.ddd:active .aa.bb.cc
		for ( auto &i : exp.split(',') ) {
			auto item = searchItem(i, make);
			if (item)
				rv.push(item);
		}
		Qk_ReturnLocal(rv);
	}

}
