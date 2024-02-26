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
	class Layout;
	class View;
	class KeyframeAction;

	enum CSSType {
		kNone_CSSType = 0,
		kNormal_CSSType, // css pseudo type
		kHover_CSSType,
		kActive_CSSType,
	};

	class Qk_EXPORT CSSName {
	public:
		Qk_DEFINE_PROP_GET(uint64_t, hashCode, Const);
		Qk_DEFINE_PROP_GET(String, name, Const);
		CSSName(cString& name);
	};

	typedef const CSSName cCSSName;

	class StyleSheets: public Object {
		Qk_HIDDEN_ALL_COPY(StyleSheets);
	public:
		class Property {
		public:
			virtual ~Property() = 0;
			virtual void apply(Layout *layout) = 0;
			virtual void transition(Layout *layout, float y, Property *to) = 0;
			virtual Property* copy() = 0;
		};
		// define props
		#define _Fun(Enum, Type, Name, From) void set_##Name(Type value);
			Qk_View_Props(_Fun)
		#undef _Fun

		StyleSheets();
		~StyleSheets();

		/**
		 * @method has_property
		*/
		bool has_property(ViewProp name) const;

		/**
		* @method apply style to layout
		*/
		void apply(Layout *layout) const;
		void apply(cSet<Layout*> &layout) const;

	private:
		void setProp(uint32_t, Property* prop);
		void applyTransition(cSet<Layout*> &layout, float y, StyleSheets *to) const;
	protected:
		Dict<uint32_t, Property*> _props; // ViewProp => Property*

		virtual void onMake(ViewProp key, Property* prop); // make new prop

		friend class KeyframeAction;
	};

	/**
	 * 
	 * Cascading style sheets
	 * 
	 * @class CStyleSheets
	*/
	class Qk_EXPORT CStyleSheets: public StyleSheets {
		Qk_HIDDEN_ALL_COPY(CStyleSheets);
	public:
		Qk_DEFINE_PROP_GET(CSSName, name, Const);
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
		CStyleSheets(cCSSName &name, CStyleSheets *parent, CSSType type);

		/**
		 * @destructor
		*/
		~CStyleSheets();

		/**
		* @method find children style sheets
		*/
		const CStyleSheets* find(cCSSName &name) const;

	private:
		CStyleSheets* findAndMake(cCSSName &name, CSSType type, bool isExtend);

		typedef Dict<uint64_t, CStyleSheets*> CStyleSheetsDict;
		CStyleSheetsDict _substyles; // css name => .self .sub { width: 100px }
		CStyleSheetsDict _extends; // css name => .self.extend { width: 100px }

		friend class RootStyleSheets;
		friend class CStyleSheetsClass;
	};

	typedef const CStyleSheets cCStyleSheets;

	class Qk_EXPORT RootStyleSheets: public CStyleSheets {
	public:
		RootStyleSheets();

		/**
		*  ".div_cls.div_cls2 .aa.bb.cc, .div_cls.div_cls2:down .aa.bb.cc"
		*
		* @method search()
		*/
		Array<CStyleSheets*> search(cString &exp);
	};

	class Qk_EXPORT CStyleSheetsClass {
		Qk_HIDDEN_ALL_COPY(CStyleSheetsClass);
	public:
		Qk_DEFINE_PROP_GET(bool, havePseudoType, Const); //!< The current style sheet group supports pseudo types
		Qk_DEFINE_PROP_GET(bool, firstApply, Const); //!< Is this the first time applying a style sheet
		Qk_DEFINE_PROP_GET(Layout*, host); //!< apply style sheet target object
		Qk_DEFINE_PROP_GET(CStyleSheetsClass*, parent); //!< apply parent ssc

		CStyleSheetsClass(Layout *host);
		void set(cArray<String> &name); //!< Calling in the main loop
		void add(cString &name); //!< Calling in the main loop
		void remove(cString &name); //!< Calling in the main loop
		void toggle(cString &name); //!< Calling in the main loop

		inline bool haveSubstyles() const {
			return _styles.length();
		}

	private:
		void updateClass_RT();
		void setStatus_RT(CSSType status);
		bool apply_RT(CStyleSheetsClass *parent); // Return whether it affects sub styles
		void applyFrom(CStyleSheetsClass *ssc);
		void applyFindSubstyle(CStyleSheets *ss);
		void applyStyle(CStyleSheets *ss);

		Set<uint64_t> _nameHash; //!< class name hash
		Array<CStyleSheets*> _styles; //!< apply to all current style sheets have substyle sheets
		Hash5381 _stylesHash; //!< hash for apply current have substyle sheets
		CSSType _status, _setStatus; //!< Current pseudo type application status

		friend class Layout;
		friend class View;
	};

}
#endif
