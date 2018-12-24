/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright (c) 2015, xuewen.chu
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of xuewen.chu nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL xuewen.chu BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * ***** END LICENSE BLOCK ***** */

#ifndef __ngui__css__
#define __ngui__css__

#include "utils/util.h"
#include "utils/map.h"
#include "utils/string.h"
#include "utils/list.h"
#include "property.h"
#include "value.h"
#include "action.h"

XX_NS(ngui)

class StyleSheets;
class CSSViewClasss;
class StyleSheetsScope;
class CSSManager;

enum CSSPseudoClass { // pseudo class
	CSS_PSEUDO_CLASS_NONE = 0,
	CSS_PSEUDO_CLASS_NORMAL,
	CSS_PSEUDO_CLASS_HOVER,
	CSS_PSEUDO_CLASS_DOWN,
};

class XX_EXPORT CSSName {
 public:
	CSSName(const Array<String>& classs);
	CSSName(cString& name);
	inline String value() const { return m_name; }
	inline uint hash() const { return m_hash; }
 private:
	String m_name;
	uint   m_hash;
};

/**
 * @class StyleSheets
 */
class XX_EXPORT StyleSheets: public Object {
	XX_HIDDEN_ALL_COPY(StyleSheets);
 protected:
	
	StyleSheets(const CSSName& name, StyleSheets* parent, CSSPseudoClass pseudo);
	
	/**
	 * @destructor
	 */
	virtual ~StyleSheets();
	
 public:
	typedef KeyframeAction::Frame Frame;
	
	class XX_EXPORT Property {
	public:
		virtual ~Property() = default;
		virtual void assignment(View* view) = 0;
		virtual void assignment(Frame* frame) = 0;
	};
	
	// -------------------- set property --------------------
	
# define xx_def_property(ENUM, TYPE, NAME) void set_##NAME(TYPE value);
	XX_EACH_PROPERTY_TABLE(xx_def_property)
# undef xx_def_property
	
	/**
	 * @func background()
	 */
	BackgroundPtr background();
	
	/**
	 * @func time
	 */
	inline uint64 time() const { return m_time; }
	
	/**
	 * @func set_time
	 */
	inline void set_time(uint64 value) { m_time = value; }
	
	/**
	 * @func name
	 */
	inline String name() const { return m_css_name.value(); }
	
	/**
	 * @func hash
	 */
	inline uint hash() const { return m_css_name.hash(); }
	
	/**
	 * @func parent
	 */
	inline StyleSheets* parent() { return m_parent; }
	
	/**
	 * @func normal
	 */
	inline StyleSheets* normal() { return m_child_NORMAL; }
	
	/**
	 * @func normal
	 */
	inline StyleSheets* hover() { return m_child_HOVER; }
	
	/**
	 * @func normal
	 */
	inline StyleSheets* down() { return m_child_DOWN; }
	
	/**
	 * @func find children
	 */
	StyleSheets* find(const CSSName& name);
	
	/**
	 * @func has_child
	 */
	inline bool has_child() const { return m_children.length(); }
	
	/**
	 * @func assignment
	 */
	void assignment(View* view);
	
	/**
	 * @func assignment
	 */
	void assignment(Frame* frame);
	
	/**
	 * @func is_support_pseudo support multiple pseudo status
	 */
	inline bool is_support_pseudo() const { return m_is_support_pseudo; }
	
	/**
	 * @func pseudo
	 */
	inline CSSPseudoClass pseudo() const { return m_pseudo; }
	
 private:
	CSSName                       m_css_name;
	StyleSheets*                  m_parent;
	Map<uint, StyleSheets*>       m_children;
	Map<PropertyName, Property*>  m_property;
	uint64         m_time;
	StyleSheets*   m_child_NORMAL;
	StyleSheets*   m_child_HOVER;
	StyleSheets*   m_child_DOWN;
	bool           m_is_support_pseudo; // m_NORMAL | m_HOVER | m_DOWN
	CSSPseudoClass m_pseudo;
	
	XX_DEFINE_INLINE_CLASS(Inl);
	friend class CSSManager;
};

/**
 * @class CSSViewClasss
 */
class XX_EXPORT CSSViewClasss: public Object {
	XX_HIDDEN_ALL_COPY(CSSViewClasss);
 public:
	CSSViewClasss(View* host);
	
	/**
	 * @destructor
	 */
	virtual ~CSSViewClasss();
	
	/**
	 * @func name
	 */
	inline const Array<String>& name() const { return m_classs; }
	
	/**
	 * @func name
	 */
	void name(const Array<String>& value);
	
	/**
	 * @func add
	 */
	void add(cString& name);
	
	/**
	 * @func remove
	 */
	void remove(cString& name);
	
	/**
	 * @func toggle
	 */
	void toggle(cString& name);
	
	/**
	 * @func has_child
	 */
	inline bool has_child() const { return m_child_style_sheets.length(); }
	
	/**
	 * @func set_style_pseudo_status
	 */
	void set_style_pseudo_status(CSSPseudoClass status);
	
	/**
	 * @func apply
	 */
	void apply(StyleSheetsScope* scope);
	
	/**
	 * @func apply
	 */
	void apply(StyleSheetsScope* scope, bool* effect_child);
	
	/**
	 * @func child_style_sheets current child style sheets
	 */
	inline const Array<StyleSheets*>& child_style_sheets() {
		return m_child_style_sheets;
	}
	
 private:
	
	View*           m_host;
	Array<String>   m_classs;
	Array<uint>     m_query_group;
	Array<StyleSheets*> m_child_style_sheets; // 当前应用的样式表中拥有子样式表的表供后代视图查询
	bool            m_is_support_pseudo;      // 当前样式表选择器能够找到支持伪类的样式表
	bool            m_once_apply;             // 是否为第一次应用样式表,在处理动作时如果为第一次忽略动作
	CSSPseudoClass  m_multiple_status;
	
	XX_DEFINE_INLINE_CLASS(Inl);
};

/**
 * @class StyleSheetsScope
 */
class XX_EXPORT StyleSheetsScope: public Object {
	XX_HIDDEN_ALL_COPY(StyleSheetsScope);
 public:
	struct XX_EXPORT Scope {
		struct Wrap {
			StyleSheets* sheets; int ref;
		};
		Wrap* wrap;
		int   ref;
	};
	StyleSheetsScope(View* scope);
	void push_scope(View* scope);
	void pop_scope();
	inline View* bottom_scope() {
		return m_scopes.length() ? m_scopes.last() : nullptr;
	}
	inline const List<Scope>& style_sheets() { return m_style_sheets; }
 private:
	typedef Map<PrtKey<StyleSheets>, Scope::Wrap> StyleSheetsMap;
	List<View*>   m_scopes;
	List<Scope>   m_style_sheets;
	StyleSheetsMap  m_style_sheets_map;
};

/**
 * @class RootStyleSheets
 */
class XX_EXPORT RootStyleSheets: public StyleSheets {
 public:
	
	RootStyleSheets();
	
	/**
	 *  ".div_cls.div_cls2 .aa.bb.cc, .div_cls.div_cls2:down .aa.bb.cc"
	 *
	 * @func instances
	 */
	Array<StyleSheets*> instances(cString& expression);
	
	/**
	 * @func shared
	 */
	static RootStyleSheets* shared();
	
 private:
	
	Map<uint, int>  m_all_css_names;
	Map<uint, Array<uint>>  m_css_query_group_cache;

	XX_DEFINE_INLINE_CLASS(Inl);
};

XX_INLINE RootStyleSheets* root_styles() { 
	return RootStyleSheets::shared(); 
}

XX_END

#endif
