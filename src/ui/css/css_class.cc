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

 #include "../ui.h"
#include "./css.h"
#include "../app.h"
#include "../action/action.h"
#include "../window.h"
#include "../view/view.h"

namespace qk {
	/*
	* This file implements the runtime CSS class resolution logic
	* for a single View.
	*
	* Key responsibilities:
	* - Maintain class name list (work thread)
	* - Synchronize hashed class names to render thread
	* - Resolve matching selectors from the global stylesheet tree
	* - Apply style properties and transitions
	* - Track selector propagation to descendant views
	*
	* IMPORTANT:
	* - Selector trees (CStyleSheets / RootStyleSheets) are global and shared.
	* - This class holds only per-View runtime state.
	* - All selector matching is hash-based and performed on render thread.
	*/

	#define _IfHost(self, ...) auto _host = self->_host.load(); if (!_host) return __VA_ARGS__
	#define _async_call _host->window()->pre_render().async_call

	// Hash value for the reserved "undefined" class name.
	// Used to ignore invalid or placeholder class tokens.
	static uint64_t UndefinedHashCode(CSSCName("undefined").hashCode());

	CStyleSheetsClass::CStyleSheetsClass(View *host)
		: _state(kNormal_UIState)     // currently applied pseudo state
		, _setState(kNormal_UIState)  // requested pseudo state (pending)
		, _havePseudoType(false)      // whether any matched selector has pseudo variants
		, _firstApply(true)           // first-time style application flag
		, _host(host)                 // owning view
		, _parent(nullptr)            // parent runtime style context
	{
		Qk_ASSERT(host);
	}

	inline Set<View*>& CStyleSheetsClass::viewsSet(uint64_t hash) {
		return _host.load()->window()->_viewsByClass[hash];
	}

	void CStyleSheetsClass::release() {
		auto _host = this->host();
		// Cleanup render thread mappings
		_async_call([](auto self, auto arg) {
			auto host = arg.arg;
			auto &viewsByClass = host->window()->_viewsByClass;
			for (auto &i: self->_nameHash_rt) {
				viewsByClass[i.first].erase(host); // remove class mappings
			}
			self->_parent = nullptr;
		}, this, _host);
		this->_host.store(nullptr); // remove host reference
		Object::release(); // continue release
	}

	void CStyleSheetsClass::destroy() {
		auto app = shared_app();
		if (app) {
			Inl_Application(app)->add_delay_task(Cb([](auto e, Object *self) {
				// To ensure safety and efficiency,
				// it should be Completely destroyed in RT (render thread)
				// However, since there is no window object available, delayed destruction is the only option.
				static_cast<CStyleSheetsClass*>(self)->Object::destroy();
			}, (Object*)this/* Avoid being re quoted */));
		} else {
			Object::destroy();
		}
	}

	void CStyleSheetsClass::set(cArray<String> &name) {
		_IfHost(this);
		// Collect unique hashed class names
		Sp<Set<uint64_t>> hashs = new Set<uint64_t>();

		// Update work-thread string set
		_names.clear();

		for (auto &j: name) {
			auto s = j.trim();
			if (!s.is_empty()) {
				auto hash = CSSCName(s).hashCode();
				// Filter out undefined and duplicate class names
				if (hash != UndefinedHashCode && hashs->add(hash)) {
					_names.add(s);
				}
			}
		}

		// Synchronize hashed class names to render thread
		_async_call([](auto self, auto val) {
			Sp<Set<uint64_t>> valp(val.arg); // take ownership
			_IfHost(self);
			auto &viewsByClass = _host->window()->_viewsByClass;
			for (auto &i: self->_nameHash_rt) {
				viewsByClass[i.first].erase(_host); // remove old class mappings
			}
			for (auto &i: *val.arg) {
				viewsByClass[i.first].add(_host); // add new class mappings
			}
			self->_nameHash_rt = std::move(*val.arg);
			self->updateClass_rt(); // mark class change
		}, this, hashs.collapse());
	}

	void CStyleSheetsClass::add(cString &name) {
		_IfHost(this);
		auto name_ = name.trim();
		if (name_.is_empty()) return;
		if (!_names.add(name_)) return; // already exists

		_async_call([](auto self, auto hash) {
			_IfHost(self);
			self->viewsSet(hash.arg).add(_host); // add new class mappings
			self->_nameHash_rt.add(hash.arg);
			self->updateClass_rt(); // trigger selector re-evaluation
		}, this, CSSCName(name_).hashCode());
	}

	void CStyleSheetsClass::remove(cString &name) {
		_IfHost(this);
		auto name_ = name.trim();
		if (name_.is_empty()) return;
		if (!_names.erase(name_)) return; // not present
		_async_call([](auto self, auto hash) {
			_IfHost(self);
			self->viewsSet(hash.arg).erase(_host); // remove class mappings
			self->_nameHash_rt.erase(hash.arg);
			self->updateClass_rt(); // trigger selector re-evaluation
		}, this, CSSCName(name_).hashCode());
	}

	bool CStyleSheetsClass::toggle(cString &name) {
		_IfHost(this, false);
		auto name_ = name.trim();
		if (name_.is_empty()) return false;
		if (_names.erase(name_)) { // removed
			_async_call([](auto self, auto hash) {
				_IfHost(self);
				self->viewsSet(hash.arg).erase(_host); // remove class mappings
				self->_nameHash_rt.erase(hash.arg);
				self->updateClass_rt();
			}, this, CSSCName(name_).hashCode());
			return false;
		} else {
			_names.add(name_); // add name
			_async_call([](auto self, auto hash) {
				_IfHost(self);
				self->viewsSet(hash.arg).add(_host); // add new class mappings
				self->_nameHash_rt.add(hash.arg);
				self->updateClass_rt();
			}, this, CSSCName(name_).hashCode());
			return true;
		}
	}

	void CStyleSheetsClass::clear() {
		_IfHost(this);
		if (_names.length() == 0) return;
		_names.clear();
		_async_call([](auto self, auto arg) {
			_IfHost(self);
			auto &viewsByClass = _host->window()->_viewsByClass;
			for (auto &i: self->_nameHash_rt) {
				viewsByClass[i.first].erase(_host); // remove class mappings
			}
			self->_nameHash_rt.clear();
			self->updateClass_rt();
		}, this, 0);
	}

	void CStyleSheetsClass::updateClass_rt() {
		// Mark layout dirty due to class list change.
		// Actual selector resolution happens during render/layout phase.
		host()->mark_layout<true>(View::kClass_Change);
	}

	void CStyleSheetsClass::setState_rt(UIState state) {
		if (_setState != state) {
			_setState = state;

			// Only trigger state change if any matched selector
			// actually contains pseudo variants.
			if (_havePseudoType) {
				if (_setState == _state) {
					// State unchanged, remove dirty mark
					host()->unmark(View::kClass_State);
				} else {
					// State changed, mark for selector re-evaluation
					host()->mark_layout<true>(View::kClass_State);
				}
			}
		}
	}

	/**
	 * Mark views that may be affected by class / propagating style changes.
	 *
	 * This method uses the Window-level reverse index (_viewsByClass)
	 * to find candidate views that declare class names referenced by
	 * the current propagating styles.
	 *
	 * Only a pre-filter is performed here:
	 *  - No selector resolution
	 *  - No hierarchy traversal
	 *  - No style application
	 *
	 * The marked views will be re-evaluated later during the normal
	 * render/layout update pass.
	 *
	 * @thread Rt
	 */
	void CStyleSheetsClass::markViewsForClassChange_rt() {
		auto host = _host.load();
		auto &_viewsByClass = host->window()->_viewsByClass;

		if (host->_level == 0) return; // skip visible = false views

		for (auto ss: _propagatingStyles_rt) {
			Set<qk::View *> *views;
			if (_viewsByClass.get(ss->_name.hashCode(), views)) {
				for (auto &v: *views) {
					auto view = v.first;
					if (ss->_directChildOnly) {
						if (host == view->parent()) {
							view->mark_layout<true>(View::kClass_Change);
						}
					} else if (host->is_child_rt(view)) {
						// descendant selector: mark any matching view in subtree
						view->mark_layout<true>(View::kClass_Change);
					}
				}
			}
		}
	}

	void CStyleSheetsClass::apply_rt(CStyleSheetsClass *parent, bool alwaysApply, bool propagate) {
		auto host = _host.load();
		// Snapshot previous propagation state
		auto lastHash = _propagatingStylesHash_rt.hashCode();
		auto lastStylesHash = _stylesHash_rt.hashCode();

		// Reset runtime state for re-application
		_stylesHash_rt = Hash5381();
		_havePseudoType = false;
		_parent = parent;
		_state = _setState; // commit pseudo state

		auto clearStatus = [this, host](bool propagate) {
			// Clear all running CSS transitions for this view
			host->window()->actionCenter()->removeCSSTransition_rt(host);

			if (propagate) {
				// Mark descendant views as needing style re-evaluation
				markViewsForClassChange_rt();
			}
			// Reset propagation cache
			_propagatingStyles_rt.clear();
			_propagatingStylesHash_rt = Hash5381();
		};

		if (_nameHash_rt.length()) {
			Array<CStyleSheets*> styles;

			// Resolve matching selectors from parent propagation context
			findSubstylesFromParent_rt(parent, &styles);

			// Apply styles only if the resolved set changed
			if (lastStylesHash != _stylesHash_rt.hashCode() || alwaysApply) {
				clearStatus(propagate);

				for (auto ss: styles) {
					// Handle transition vs immediate apply
					if (ss->_time && !_firstApply) {
						if (ss->itemsCount()) // has properties to apply
							host->window()->actionCenter()->addCSSTransition_rt(host, ss);
					} else {
						ss->apply_with_priority(host);
					}

					// Cache styles that may propagate to children
					if (ss->_substyles.length()) {
						_propagatingStyles_rt.push(ss);
						_propagatingStylesHash_rt.updateu64(uintptr_t(ss));
					}
					// Propagate class changes to potentially affected views.
					// This only marks candidates via reverse lookup;
					// actual style resolution happens later.
					if (propagate) {
						markViewsForClassChange_rt();
					}
				}
			}
		} else {
			// No class names → clear all styles and propagation
			clearStatus(propagate);
		}
		_firstApply = false; // mark first apply false
	}

	void CStyleSheetsClass::findSubstylesFromParent_rt(CStyleSheetsClass *parent, Array<CStyleSheets*> *out) {
		// Match descendant selectors against current class set
		auto find = [](CStyleSheetsClass *self, CStyleSheets *ss, Array<CStyleSheets*> *out) {
			for (auto &n: self->_nameHash_rt) {
				CStyleSheets *css;
				// Match descendant selector: ".parent .child"
				if (ss->_substyles.get(n.first, css)) { // find substyle by class name hash
					self->findStyle_rt(css, out);
				}
			}
		};
		if (parent) {
			Qk_ASSERT(parent->_propagatingStyles_rt.length());
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
			// Recursively resolve descendant selectors from ancestor propagation contexts
			// (".ancestor .child")
			findSubstylesFromParent_rt(parent->_parent, out); // find styles from parent

			// Then match current level against parent's propagating styles
			// to find direct child selectors.
			// e.g.: ".parent > .child"
			//
			auto isDirectChild = parent->host() == _host.load()->parent();

			// Match current level against parent's propagating styles
			for (auto ss: parent->_propagatingStyles_rt) {
				if (ss->_directChildOnly) {
					if (!isDirectChild)	continue; // skip non-direct child
				}
				find(this, ss, out); // find sub styles from parent
			}
		} else {
			// Root-level selector matching
			find(this, RootStyleSheets::shared(), out); // find global root substyles
		}
	}

	// CSS Sample, Expanding form
	/**
	.a {
		width: 100;
	}
	.a .a_a {
		height: 110;
	}
	.a .a_a:hover {
		height: 170;
	}
	.a .a_b {
		color: #f00;
	}
	.a:hover {
		width: 150;
	}
	.a:hover .a_a {
		height: 160;
	}
	// ------------------ error ------------------
	.a:hover .a_a:hover {
		height: 171;
	}
	// ------------------
	.a:hover .a_b {
		color: #ff0;
	}
	<box class="a">
		<box class="a_a">test a<box>
		<box class="a_a a_b">test b<box>
	</box>
	*/

	void CStyleSheetsClass::findStyle_rt(CStyleSheets *css, Array<CStyleSheets*> *out) {
		// Record matched selector node
		out->push(css);
		_stylesHash_rt.updateu64(uintptr_t(css));

		// find pseudo class
		if (css->_havePseudoType) {
			_havePseudoType = true;

			switch (_state) {
				case kNone_UIState: break;
				case kNormal_UIState:
					if (css->_normal) findStyle_rt(css->_normal, out);
					break;
				case kHover_UIState:
					if (css->_hover) findStyle_rt(css->_hover, out);
					break;
				case kActive_UIState:
					if (css->_active) findStyle_rt(css->_active, out);
					break;
			}
		}

		// Match continuous (no-space) selector extension.
		// `_ext` only stores the immediate next selector segment.
		//
		// Example:
		//   .self.ext1.xxx.fdd
		//
		// At `.self` level:
		//   _ext contains only "ext1"
		//
		// Deeper segments ("xxx", "fdd") are matched recursively
		// through subsequent `findStyle_rt` calls.
		//
		// Therefore, this lookup checks only the next chained class,
		// not the entire selector chain.
		if (css->_ext.length()) { // find extend
			 // more extend, optimize search
			if (css->_ext.length() > _nameHash_rt.length()) {
				for (auto &i: _nameHash_rt) { // test next extend
					CStyleSheets *ss;
					if (css->_ext.get(i.first, ss)) { // test ok
						findStyle_rt(ss, out);
					}
				}
			} else { // less extend, optimize search
				for (auto &i: css->_ext) { // test next extend
					if (_nameHash_rt.has(i.first)) { // test ok
						findStyle_rt(i.second, out);
					}
				}
			}
		}
	}

}
