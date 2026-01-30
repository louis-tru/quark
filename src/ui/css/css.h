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

#ifndef __quark__css__css__
#define __quark__css__css__

#include "../../util/util.h"
#include "../../util/dict.h"
#include "../filter.h"
#include "./css_props.h"

/**
 *
 * This file defines the core data structures for Qk's
 * style system inspired by CSS.
 *
 * NOTE:
 * This is NOT a full CSS implementation.
 * Qk intentionally supports a constrained, class-driven subset
 * of CSS-like selectors, optimized for GUI workloads rather than
 * general-purpose web rendering.
 *
 * The system is split into three conceptual layers:
 *
 * 1. Selector Tree (Global, Shared)
 *    - Represented by CStyleSheets / RootStyleSheets
 *    - Stores selector structure and style properties
 *    - Shared across all windows and views
 *
 * 2. Runtime Selector State (Per-View)
 *    - Represented by CStyleSheetsClass
 *    - Resolves which selectors apply to a specific View
 *    - Manages propagation of selectors to descendant views
 *
 * 3. Style Property Execution
 *    - Represented by StyleSheets and Property
 *    - Applies resolved style properties to Views
 *
 * IMPORTANT DESIGN PRINCIPLES:
 *
 * - Selector matching is class-name driven (hash-based),
 *   NOT general CSS selector matching.
 * - Selector structure is represented as an explicit tree,
 *   NOT as flattened selector strings.
 * - Only a deliberate subset of CSS concepts is supported
 *   (e.g. class selectors, hierarchy, and limited pseudo states).
 * - Runtime resolution is incremental and propagation-aware,
 *   optimized for GUI view trees rather than DOM semantics.
 *
 * NON-GOALS:
 *
 * - Full CSS selector coverage (e.g. attribute selectors,
 *   structural selectors, pseudo-elements).
 * - Browser-level cascade, specificity, or invalidation models.
 */

namespace qk {
	class View;
	class View;
	class KeyframeAction;
	class Window;

	/**
	 * Represents pseudo-class states used during selector matching.
	 *
	 * NOTE:
	 * - UIState describes selector state, not View lifecycle state.
	 * - Used to resolve :normal / :hover / :active selectors.
	 */
	enum UIState: uint8_t {
		kNone_UIState = 0,
		kNormal_UIState, //!< pseudo :normal (default state, no explicit interaction)
		kHover_UIState,  //!< pseudo :hover  (pointer hovering)
		kActive_UIState, //!< pseudo :active (pressed / active interaction)
	};

	/**
	* CSS class name representation.
	* Each CSS class name is represented by:
	* - a string (for debugging / inspection)
	* - a precomputed 64-bit hash (for fast selector matching)
	*
	* NOTE:
	* Hash collisions are theoretically possible but extremely unlikely.
	* All selector matching is performed using the hash value.
	*/
	class Qk_EXPORT CSSCName {
	public:
		Qk_DEFINE_PROP_GET(uint64_t, hashCode, Const);
		Qk_DEFINE_PROP_GET(String, name, Const);

		CSSCName(cString& name);
	};

	typedef const CSSCName cCSSCName;

	/**
	* StyleSheets represents a concrete set of style properties
	* that can be applied to a View.
	*
	* IMPORTANT:
	* - StyleSheets does NOT represent a selector.
	* - Selector logic lives in CStyleSheets.
	* - StyleSheets only stores resolved style properties (CssProp -> Property).
	*
	* One CStyleSheets node *inherits* from StyleSheets and
	* attaches selector semantics on top of it.
	*/
	class StyleSheets: public Object {
		Qk_DISABLE_COPY(StyleSheets);
	public:
		class Property {
		public:
			// Destructor
			virtual ~Property() = default;

			// Apply without priority checks (RT / animation / action)
			virtual void apply(View *view) = 0; // @thread Rt

			// Apply with style priority resolution (CSS static)
			virtual void apply_with_priority(View *view) = 0; // @thread Rt

			// Fetch current value from the view (used for transitions).
			virtual void fetch(View *from) = 0; // @thread Rt

			// Interpolate between two properties during transition.
			virtual void transition(View *view, Property *to, float t) = 0; // @thread Rt

			// Clone property instance (RT only).
			virtual Property* copy() = 0; // @thread Rt
		};
		// define props
		#define _Fun(Enum, Type, Name, From) void set_##Name(Type value);
			Qk_Css_Props(_Fun)
		#undef _Fun

		Qk_DEFINE_ACCE_GET(cCurve&, curve, Const);

		StyleSheets();
		~StyleSheets();

		/**
		 * Number of properties defined in this StyleSheets instance
		*/
		inline uint32_t itemsCount() const {
			return _props.length();
		}

		/**
		 * Check if a specific property exists
		*/
		inline bool hasProperty(CssProp key) const {
			return _props.count(key);
		}

		/**
		 * Apply all stored style properties to the given view,
		 * resolving style priority for each property.
		 *
		 * This method performs CSS-style priority checks for every property:
		 * - If a property is already set by a higher-priority source,
		 *   the assignment will be skipped.
		 * - If applied successfully, the corresponding style flags
		 *   will be updated to record the priority source.
		 *
		 * This method is intended for static CSS style resolution only.
		 *
		 * @thread Rt
		 */
		void apply_with_priority(View *view) const;

		/**
		 * Apply all stored style properties to the given view
		 * without resolving or modifying style priority.
		 *
		 * This method directly writes property values to the view:
		 * - No style priority checks are performed
		 * - Style flags are not updated
		 *
		 * Typical use cases include:
		 * - CSS transitions
		 * - Animations / actions
		 * - Internal render-thread corrections
		 *
		 * @param view Target view to apply properties to
		 * @param isRt Indicates whether the call originates from the render thread
		 */
		void apply(View *view, bool isRt) const;

		/**
		 * Fetch current property values from a view.
		 *
		 * This method reads the current state of each property from the view
		 * and stores it internally. The fetched values are later used as
		 * the starting point for style transitions and animations.
		 *
		 * No style priority checks are involved.
		 *
		 * @param from Source view to read property values from
		 * @param isRt Indicates whether the call originates from the render thread
		 */
		void fetch(View *from, bool isRt);

		/**
		 * Get the window associated with this StyleSheets instance for asynchronous operations
		*/
		virtual Window* getWindow() const;

	private:
		void applyTransition(View* view, StyleSheets *to, float y) const; // @thread Rt;
		void set_frame_rt(uint32_t frame);
	protected:
		// Mapping from CssProp enum to runtime Property implementation.
		// This container stores only properties explicitly defined
		// in this StyleSheets instance.
		Dict<uint32_t, Property*> _props; // ViewProp => Property*

		// Hook called when a new property is created
		virtual void onMake(CssProp key, Property* prop);

		friend class KeyframeAction;
		friend class Sprite;
	};

	/**
	 * Cascading style sheets
	 *
	 * Selector representation:
	 *
	 * 1. Descendant selector (with space):
	 *    .a .b .c
	 *    → stored via `_substyles` chain
	 *
	 * 2. Continuous selector (no space):
	 *    .a.b.c
	 *    → stored via `_ext` chain
	 *
	 * 3. Pseudo selectors:
	 *    :normal / :hover / :active
	 *    → stored via `normal / hover / active`
	 *
	 * Each selector segment is stored as a node in a selector tree,
	 * not as a flattened string.
	 *
	 * IMPORTANT:
	 * - This class represents BOTH selector structure and style properties.
	 * - Selector matching is performed by traversing this tree,
	 *   NOT by string comparison.
	 */
	class Qk_EXPORT CStyleSheets: public StyleSheets {
		Qk_DISABLE_COPY(CStyleSheets);
	public:
		Qk_DEFINE_PROPERTY(uint32_t, time, Const); // css transition time span
		Qk_DEFINE_PROP_GET(CStyleSheets*, parent);
		// Parent selector node in the selector tree.
		// Used to represent nested selector context.

		Qk_DEFINE_PROP_GET(CStyleSheets*, normal); // pseudo :normal
		Qk_DEFINE_PROP_GET(CStyleSheets*, hover);  // pseudo :hover
		Qk_DEFINE_PROP_GET(CStyleSheets*, active); // pseudo :active
		Qk_DEFINE_PROP_GET(UIState, state, Const);
		Qk_DEFINE_PROP_GET(bool, havePseudoType, Const); // normal | hover | active
		Qk_DEFINE_PROP_GET(bool, haveSubstyles, Const);
		Qk_DEFINE_PROP_GET(bool, directChildOnly, Const); // whether this selector is for direct child only

		/**
		 * @constructor
		*/
		CStyleSheets(cCSSCName &name, CStyleSheets *parent, UIState state, bool directChildOnly);

		/**
		 * @destructor
		*/
		~CStyleSheets();

		/**
		* @method find children style sheets
		*/
		const CStyleSheets* find(cCSSCName &name) const;

	private:
		CStyleSheets* findAndMake(cCSSCName &name, UIState state, bool isExt, bool make, bool directChildOnly);

		typedef Dict<uint64_t, CStyleSheets*> CStyleSheetsDict;

		// Descendant selector (with space).
		// Example:
		//   .self .sub { ... }
		//
		// This represents a selector separated by whitespace.
		// Matching requires the target view to be a descendant
		// (not necessarily direct child) of the current selector context.
		CStyleSheetsDict _substyles;

		// Continuous (no-space) selector continuation.
		// Stores only the immediate next class in a chained selector.
		//
		// Example:
		//   .self.ext1.xxx.fdd { ... }
		//
		// Parsed structure:
		//   self
		//     └─ _ext["ext1"]
		//          └─ _ext["xxx"]
		//               └─ _ext["fdd"]
		//
		// NOTE:
		// - Only the first segment after the current selector is stored here.
		// - Deeper chained segments are stored recursively in the child's `_ext`.
		// - This is NOT a flattened list of all chained names.
		// - This structure allows correct matching of chained selectors
		//   without ambiguity.
		CStyleSheetsDict _ext;
		CSSCName _name;

		friend class RootStyleSheets;
		friend class CStyleSheetsClass;
	};

	typedef const CStyleSheets cCStyleSheets;

	/**
	 * 
	 * When creating new and updating style sheets in an application, 
	 * it is necessary to lock all window rendering threads `Application::lockAllRenderThreads()`,
	 * Because the style sheets in the application are shared by all windows.
	 * @class RootStyleSheets
	*/
	class Qk_EXPORT RootStyleSheets: public CStyleSheets {
	public:
		RootStyleSheets();

		/**
		 * Search or create selector from string expression
		 * @example searchItem(".div_cls.div_cls2:active .aa.bb.cc");
		*/
		CStyleSheets* searchItem(cString &exp, bool make = false);

		/**
		*  ".div_cls.div_cls2 .aa.bb.cc, .div_cls.div_cls2:active .aa.bb.cc"
		* 
		*
		* Search multiple selectors (comma-separated)
		*/
		Array<CStyleSheets*> search(cString &exp, bool make = false);
		
		/**
		 * get shared RootStyleSheets instance
		 */
		static RootStyleSheets* shared();
	};

	/**
	* CStyleSheetsClass represents the runtime CSS class state
	* of a specific View.
	*
	* It is responsible for:
	* - Managing the View's class list
	* - Resolving matching selectors from the global stylesheet tree
	* - Applying style properties to the View
	* - Tracking selector propagation to descendant views
	*
	* IMPORTANT:
	* - This is a per-View runtime object.
	* - It does NOT own the stylesheet data.
	* - All selector trees are shared globally (RootStyleSheets).
	*/
	class Qk_EXPORT CStyleSheetsClass: public Object {
		Qk_DISABLE_COPY(CStyleSheetsClass);
		UIState _state, _setState; //!< @thread Rt Current pseudo type application status
	public:
		CStyleSheetsClass(View *host);

		Qk_DEFINE_PROP_GET(bool, havePseudoType, Const); //!< The current style sheet group supports pseudo types
		Qk_DEFINE_PROP_GET(bool, firstApply, Const); //!< Is this the first time applying a style sheet
		Qk_DEFINE_PROP_GET_Atomic(View*, host); //!< apply style sheet target object
		Qk_DEFINE_PROP_GET(CStyleSheetsClass*, parent); //!< @thread Rt apply parent ssc

		void set(cArray<String> &name); //!< Calling in the work loop
		void add(cString &name); //!< Calling in the work loop
		void remove(cString &name); //!< Calling in the work loop
		bool toggle(cString &name); //!< Calling in the work loop
		void clear(); //!< Calling in the work loop

		/**
		 * has class name
		*/
		bool has(cString &name) const { return _names.has(name); }

		/**
		 * class name set for work loop
		*/
		inline cSet<String>& names() const { return _names; }

		/**
		 * @method haveSubstyles()
		 * @thread Rt
		*/
		inline bool haveSubstyles() const {
			return _propagatingStyles_rt.length();
		}

		/* @override */
		void release() override;
		void destroy() override;

	private:
		void updateClass_rt();
		void setState_rt(UIState state);

		// Apply styles and return whether propagation changed
		// Return whether it affects sub styles
		void apply_rt(CStyleSheetsClass *parent, bool alwaysApply, bool propagate);

		void findSubstylesFromParent_rt(CStyleSheetsClass *parent, Array<CStyleSheets*> *out);
		void findStyle_rt(CStyleSheets *ss, Array<CStyleSheets*> *out);

		// Accessor for the set of views associated with a specific class hash
		inline Set<View*>& viewsSet(uint64_t hash);

		// Mark descendant views as needing style re-evaluation
		void markViewsForClassChange_rt();

		Set<String> _names; //!< class name set for work loop (main thread)

		// Hashed class names for render thread usage.
		// Used for fast selector matching during style resolution.
		Set<uint64_t> _nameHash_rt; //!< class name hash, For .a .b .c

		// Stylesheet roots applied to this View that may propagate
		// selector matching to descendant views.
		//
		// These stylesheets:
		// - Are already applied to the current View
		// - Contain descendant or chained selectors
		// - Must be considered when resolving styles for child views
		//
		// NOTE:
		// - Only stylesheet ROOT nodes are cached here.
		// - Substyle containers are NOT stored directly.
		// - This structure is critical for incremental and selective
		//   style propagation.
		Vector<CStyleSheets*> _propagatingStyles_rt;

		// Hash of the propagating stylesheet roots.
		// Used to detect changes in propagation context
		// and to short-circuit unnecessary descendant resolution.
		Hash5381 _propagatingStylesHash_rt;

		// Hash of all styles applied to this View.
		// Used to detect whether style application actually changed.
		Hash5381 _stylesHash_rt;

		friend class View;
	};

	/**
	 * Convenience accessor for the shared RootStyleSheets instance
	 */
	inline RootStyleSheets* shared_root_styleSheets() {
		return RootStyleSheets::shared();
	}
}
#endif
