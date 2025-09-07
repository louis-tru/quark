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

#ifndef __quark__css__css__
#define __quark__css__css__

#include "../../util/util.h"
#include "../../util/dict.h"
#include "../filter.h"
#include "../view_prop.h"

namespace qk {
	class View;
	class View;
	class KeyframeAction;
	class Window;

	enum CSSType {
		kNone_CSSType = 0,
		kNormal_CSSType, // css pseudo type
		kHover_CSSType,
		kActive_CSSType,
	};

	// css class name
	class Qk_EXPORT CSSCName {
	public:
		Qk_DEFINE_PROP_GET(uint64_t, hashCode, Const);
		Qk_DEFINE_PROP_GET(String, name, Const);
		CSSCName(cString& name);
	};

	typedef const CSSCName cCSSCName;

	class StyleSheets: public Object {
		Qk_HIDDEN_ALL_COPY(StyleSheets);
	public:
		class Property {
		public:
			virtual ~Property() = default;
			virtual void apply(View *view, bool isRt) = 0;
			virtual void fetch(View *view) = 0; // @thread Rt
			virtual void transition(View *view, Property *to, float t) = 0; // @thread Rt
			virtual Property* copy() = 0; // @thread Rt
		};
		// define props
		#define _Fun(Enum, Type, Name, From) void set_##Name(Type value);
			Qk_View_Props(_Fun)
		#undef _Fun

		Qk_DEFINE_ACCE_GET(cCurve&, curve, Const);

		StyleSheets();
		~StyleSheets();

		/**
		 * @method itemsCount() StyleSheets items count
		*/
		uint32_t itemsCount() const;

		/**
		 * @method has_property
		*/
		bool hasProperty(ViewProp key) const;

		/**
		* @method apply style to view
		*/
		void apply(View *view, bool isRt) const;

		/**
		 * @method fetch view props
		*/
		void fetch(View *view, bool isRt);

		/**
		 * @method getWindowForAsyncSet
		 * Use asynchronous calls when returning window objects
		*/
		virtual Window* getWindowForAsyncSet();

	private:
		void applyTransition(View* view, StyleSheets *to, float y) const; // @thread Rt
		void set_frame_Rt(uint32_t frame);
	protected:
		Dict<uint32_t, Property*> _props; // ViewProp => Property*

		virtual void onMake(ViewProp key, Property* prop); // make new prop

		friend class KeyframeAction;
		friend class Sprite;
	};

	/**
	 * Cascading style sheets
	 * 
	 * @class CStyleSheets
	*/
	class Qk_EXPORT CStyleSheets: public StyleSheets {
		Qk_HIDDEN_ALL_COPY(CStyleSheets);
	public:
		Qk_DEFINE_PROPERTY(uint32_t, time, Const); // css transition time
		Qk_DEFINE_PROP_GET(CStyleSheets*, parent);
		Qk_DEFINE_PROP_GET(CStyleSheets*, normal); // style sheets for pseudo type
		Qk_DEFINE_PROP_GET(CStyleSheets*, hover);
		Qk_DEFINE_PROP_GET(CStyleSheets*, active);
		Qk_DEFINE_PROP_GET(CSSType, type, Const);
		Qk_DEFINE_PROP_GET(bool, havePseudoType, Const); // normal | hover | active
		Qk_DEFINE_PROP_GET(bool, haveSubstyles, Const);

		/**
		 * @constructor
		*/
		CStyleSheets(cCSSCName &name, CStyleSheets *parent, CSSType type);

		/**
		 * @destructor
		*/
		~CStyleSheets();

		/**
		* @method find children style sheets
		*/
		const CStyleSheets* find(cCSSCName &name) const;

	private:
		CStyleSheets* findAndMake(cCSSCName &name, CSSType type, bool isExtend, bool make);

		typedef Dict<uint64_t, CStyleSheets*> CStyleSheetsDict;
		CStyleSheetsDict _substyles; // css name => .self .sub { width: 100 }
		CStyleSheetsDict _extends; // css name => .self.extend { width: 200 }
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
		 * 
		 *   ".div_cls.div_cls2:active .aa.bb.cc"
		 * 
		 * @method searchItem()
		*/
		CStyleSheets* searchItem(cString &exp, bool make = false);

		/**
		*  ".div_cls.div_cls2 .aa.bb.cc, .div_cls.div_cls2:active .aa.bb.cc"
		* 
		*
		* @method search()
		*/
		Array<CStyleSheets*> search(cString &exp, bool make = false);
		
		/**
		 * @method shared()
		 */
		static RootStyleSheets* shared();
	};

	class Qk_EXPORT CStyleSheetsClass: public Object {
		Qk_HIDDEN_ALL_COPY(CStyleSheetsClass);
		CSSType _status, _setStatus; //!< @thread Rt Current pseudo type application status
	public:
		Qk_DEFINE_PROP_GET(bool, havePseudoType, Const); //!< The current style sheet group supports pseudo types
		Qk_DEFINE_PROP_GET(bool, firstApply, Const); //!< Is this the first time applying a style sheet
		Qk_DEFINE_PROP_GET(View*, host); //!< apply style sheet target object
		Qk_DEFINE_PROP_GET(CStyleSheetsClass*, parent); //!< @thread Rt apply parent ssc

		CStyleSheetsClass(View *host);

		void set(cArray<String> &name); //!< Calling in the main loop
		void add(cString &name); //!< Calling in the main loop
		void remove(cString &name); //!< Calling in the main loop
		void toggle(cString &name); //!< Calling in the main loop

		/**
		 * @method haveSubstyles()
		 * @thread Rt
		*/
		inline bool haveSubstyles() const {
			return _substyles_Rt.length();
		}

	private:
		void updateClass_Rt();
		void setStatus_Rt(CSSType status);
		bool apply_Rt(CStyleSheetsClass *parent); // Return whether it affects sub styles
		void applyFrom_Rt(CStyleSheetsClass *ssc);
		void applyFindSubstyle_Rt(CStyleSheets *ss);
		void applyStyle_Rt(CStyleSheets *ss);

		Set<uint64_t> _nameHash_Rt; //!< class name hash
		Array<CStyleSheets*> _substyles_Rt; //!< apply to all current style sheets have substyle sheets
		Hash5381 _substylesHash_Rt; //!< hash for apply current have substyle sheets

		friend class View;
	};

	inline RootStyleSheets* shared_root_styleSheets() {
		return RootStyleSheets::shared();
	}
}
#endif
