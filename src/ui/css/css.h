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

	enum CSSType {
		kNone_CSSType = 0,
		kNormal_CSSType, // css pseudo type
		kHover_CSSType,
		kActive_CSSType,
	};

	class Qk_EXPORT CSSName {
	public:
		Qk_DEFINE_PROP_GET(uint64_t, hash);
		CSSName(cString& name);
		CSSName(cArray<String>& name);
	};

	/**
	 * 
	 * Cascading style sheets
	 * 
	 * @class StyleSheets
	*/
	class Qk_EXPORT StyleSheets: public Object {
		Qk_HIDDEN_ALL_COPY(StyleSheets);
	public:
		class Property {
		public:
			virtual ~Property() = default;
			virtual void apply(Layout *layout) = 0;
		};

		// define props
		#define _Fun(Enum, Type, Name, From) void set_##Name(Type value);
			Qk_View_Props(_Fun)
		#undef _Fun

		Qk_DEFINE_PROP(uint64_t, time);
		Qk_DEFINE_PROP_GET(CSSName, name);
		Qk_DEFINE_PROP_GET(StyleSheets*, parent, NoConst);
		Qk_DEFINE_PROP_GET(StyleSheets*, normal, NoConst); // style sheets for pseudo type
		Qk_DEFINE_PROP_GET(StyleSheets*, hover,  NoConst);
		Qk_DEFINE_PROP_GET(StyleSheets*, active, NoConst);
		Qk_DEFINE_PROP_GET(CSSType, type);
		Qk_DEFINE_PROP_GET(bool, havePseudoType); // normal | hover | active

		/**
		 * @constructor
		*/
		StyleSheets(CSSName name, StyleSheets *parent, CSSType type);

		/**
		 * @destructor
		*/
		virtual ~StyleSheets();

		/**
		* @method haveSubstyles
		*/
		inline bool haveSubstyles() const { return _substyles.length(); }

		/**
		* @method find children style sheets
		*/
		inline const StyleSheets* find(CSSName name) const {
			return findHash(name.hash());
		}

		/**
		* @method apply style to layout
		*/
		void apply(Layout *layout) const;

	private:
		StyleSheets* findHash(uint64_t hash) const;
		StyleSheets* findAndMake(CSSName name, CSSType type);
		void setProp(uint32_t, Property* prop);
		Dict<uint64_t, StyleSheets*> _substyles;
		Dict<uint32_t, Property*> _props; // ViewProperty => Property*

		friend class RootStyleSheets;
		friend class StyleSheetsClass;
	};

	typedef const StyleSheets cStyleSheets;

	class Qk_EXPORT RootStyleSheets: public StyleSheets {
	public:
		RootStyleSheets();

		/**
		*  ".div_cls.div_cls2 .aa.bb.cc, .div_cls.div_cls2:down .aa.bb.cc"
		*
		* @method search()
		*/
		Array<StyleSheets*> search(cString& exp);

	private:
		Array<uint64_t> getCssQueryGrpup(Array<String> &className);
		void markClassName(CSSName className);

		Set<uint64_t>                    _allClassNames;
		Dict<uint64_t, Array<uint64_t>>  _cssQueryGroupCache;

		friend class StyleSheets;
		friend class StyleSheetsClass;
	};

	class Qk_EXPORT StyleSheetsScope {
		Qk_HIDDEN_ALL_COPY(StyleSheetsScope);
	public:
		struct Scope {
			struct Wrap {
				StyleSheets *sheets; int ref;
			} *wrap;
			int ref;
		};
		StyleSheetsScope(Layout *scope);
		void pushScope(Layout *scope);
		void popScope();
		inline Layout* bottomScope() {
			return _scopes.length() ? _scopes.back() : nullptr;
		}
		inline cList<Scope>& styleSheets() {
			return _styleSheets;
		}
	private:
		typedef Dict<StyleSheets*, Scope::Wrap> StyleSheetsMap;
		List<Layout*>  _scopes;
		List<Scope>    _styleSheets;
		StyleSheetsMap _styleSheetsMap;
	};

	class Qk_EXPORT StyleSheetsClass {
		Qk_HIDDEN_ALL_COPY(StyleSheetsClass);
	public:
		Qk_DEFINE_PROP_GET(CSSType, status); // 当前伪类应用状态
		Qk_DEFINE_PROP_GET(bool, havePseudoType); // 当前样式表选择器能够找到支持伪类的样式表
		Qk_DEFINE_PROP_GET(bool, onceApply); // 是否为第一次应用样式表,在处理动作时如果为第一次忽略动作
		StyleSheetsClass(Layout *host);
		inline cSet<String>& name() const { return _name; }
		inline bool haveSubstyles() const { return _substyleSheets.length(); }
		inline cArray<cStyleSheets*>& substyleSheets() const {
			return *(cArray<cStyleSheets*>*)&_substyleSheets;
		}
		void set(cArray<String> &name);
		void add(cString &name);
		void remove(cString &name);
		void toggle(cString &name);

	private:
		void set_status(CSSType status);
		void apply(StyleSheetsScope *scope, bool *out_effectChild = nullptr);
		void updateClass();
		Layout         *_host;
		Set<String>     _name;
		Array<uint64_t> _queryGroup;
		Array<StyleSheets*> _substyleSheets; // 当前应用的样式表中拥有子样式表的表供后代视图查询
		friend class Layout;
	};

}
#endif
