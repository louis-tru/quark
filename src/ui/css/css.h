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

#include "../util/util.h"
#include "../util/string.h"
#include "../util/dict.h"
#include "../types.h"

namespace qk {
	// Cascading Style Sheets

	class Layout;

	class Qk_EXPORT CSSName {
	public:
		Qk_DEFINE_PROP_ACC_GET(String, value);
		Qk_DEFINE_PROP_ACC_GET(uint32_t, hash);
		CSSName(const Array<String>& name);
		CSSName(cString& name);
	};

	enum CSSType {
		kNONE_CSSType = 0,
		kNORMAL_CSSType,
		kHOVER_CSSType,
		kACTIVE_CSSType,
	};

	class Qk_EXPORT StyleSheets: public Object {
		Qk_HIDDEN_ALL_COPY(StyleSheets);
		Qk_DEFINE_INLINE_CLASS(Inl);
	public:
		class Property {
		public:
			virtual void apply(Layout* view) = 0;
		};

		// -------------------- set property --------------------

		// # define fx_def_property(ENUM, TYPE, NAME) void set_##NAME(TYPE value);
		// 	Qk_EACH_PROPERTY_TABLE(fx_def_property)
		// # undef fx_def_property

		// BackgroundPtr background();

		Qk_DEFINE_PROP_GET(CSSName, name);
		Qk_DEFINE_PROP(uint64_t, time);
		// inline void set_time(uint64_t value) { _time = value; }
		Qk_DEFINE_PROP_GET(StyleSheets*, parent, NoConst);
		Qk_DEFINE_PROP_GET(StyleSheets*, normal, NoConst);
		Qk_DEFINE_PROP_GET(StyleSheets*, hover, NoConst);
		Qk_DEFINE_PROP_GET(StyleSheets*, active, NoConst);
		Qk_DEFINE_PROP_GET(CSSType, type);
		Qk_DEFINE_PROP_GET(bool, isSupportPseudoType);

		StyleSheets(const CSSName& name, StyleSheets *parent, CSSType type);

		virtual ~StyleSheets();

		/**
		* @func find children
		*/
		StyleSheets* find(const CSSName& name);

		/**
		* @func has_child
		*/
		inline bool has_child() const { return _children.length(); }

		/**
		* @method apply()
		*/
		void apply(Layout* view);

	private:
		Dict<uint32_t, StyleSheets*> _children;
		// Dict<PropertyName, Property*> _props;
		Dict<uint32_t, Property*>    _props;
	};

	class Qk_EXPORT RootStyleSheets: public StyleSheets {
	public:
		RootStyleSheets();

		/**
		*  ".div_cls.div_cls2 .aa.bb.cc, .div_cls.div_cls2:down .aa.bb.cc"
		*
		* @method search()
		*/
		Array<StyleSheets*> search(cString& expression);

	private:
		Dict<uint32_t, int>              _all_css_names;
		Dict<uint32_t, Array<uint32_t>>  _css_query_group_cache;
	};

	class Qk_EXPORT StyleSheetsClass: public Object {
		Qk_HIDDEN_ALL_COPY(StyleSheetsClass);
		Qk_DEFINE_INLINE_CLASS(Inl);
	public:
		Qk_DEFINE_PROP(CSSType, status);

		StyleSheetsClass(Layout* host);

		virtual ~StyleSheetsClass();

		inline bool hasChild() const { return _childStyleSheets.length(); }
		inline const Array<String>& name() const { return _name; }
		inline const Array<StyleSheets*>& childStyleSheets() { return _childStyleSheets; }

		void set(const Array<String> &value);
		void add(cString &name);
		void remove(cString &name);
		void toggle(cString &name);
		void apply(StyleSheetsScope* scope);
		void apply(StyleSheetsScope *scope, bool* effect_child);

	private:
		Layout*         _host;
		Array<String>   _name;
		Array<uint32_t> _queryGroup;
		Array<StyleSheets*> _childStyleSheets; // 当前应用的样式表中拥有子样式表的表供后代视图查询
		bool            _isSupportPseudoType;  // 当前样式表选择器能够找到支持伪类的样式表
		bool            _onceApply;            // 是否为第一次应用样式表,在处理动作时如果为第一次忽略动作
	};

	class Qk_EXPORT StyleSheetsScope: public Object {
		Qk_HIDDEN_ALL_COPY(StyleSheetsScope);
	public:
		struct Scope {
			struct Wrap {
				StyleSheets* sheets; int ref;
			};
			Wrap* wrap;
			int   ref;
		};
		StyleSheetsScope(Layout* scope);
		void push_scope(Layout* scope);
		void pop_scope();
		inline Layout* bottom_scope() { return _scopes.length() ? _scopes.back() : nullptr; }
		inline const List<Scope>& style_sheets() { return _style_sheets; }
	private:
		typedef Dict<StyleSheets*, Scope::Wrap> StyleSheetsMap;
		List<Layout*>  _scopes;
		List<Scope>    _style_sheets;
		StyleSheetsMap _style_sheets_map;
	};

}
#endif