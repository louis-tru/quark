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
#include "../property.h"
#include "../value.h"
#include "../action/action.h"
#include "../action/keyframe.h"
#include "../util/dict.h"

Qk_NAMESPACE_START

class Qk_EXPORT CSSName {
public:
	CSSName(const Array<String>& classs);
	CSSName(cString& name);
	inline String value() const { return _name; }
	inline uint32_t hash() const { return _hash; }
private:
	String   _name;
	uint32_t _hash;
};

enum CSSPseudoClass { // pseudo class
	CSS_PSEUDO_CLASS_NONE = 0,
	CSS_PSEUDO_CLASS_NORMAL,
	CSS_PSEUDO_CLASS_HOVER,
	CSS_PSEUDO_CLASS_DOWN,
};

/**
* @class StyleSheets
*/
class Qk_EXPORT StyleSheets: public Object {
	Qk_HIDDEN_ALL_COPY(StyleSheets);
protected:
	StyleSheets(const CSSName& name, StyleSheets* parent, CSSPseudoClass pseudo);
	
	/**
	* @destructor
	*/
	virtual ~StyleSheets();
	
public:
	typedef KeyframeAction::Frame Frame;
	
	class Qk_EXPORT Property {
	public:
		virtual ~Property() = default;
		virtual void assignment(View* view) = 0;
		virtual void assignment(Frame* frame) = 0;
	};
	
	// -------------------- set property --------------------
	
	# define fx_def_property(ENUM, TYPE, NAME) void set_##NAME(TYPE value);
		Qk_EACH_PROPERTY_TABLE(fx_def_property)
	# undef fx_def_property
	
	/**
	* @func background()
	*/
	BackgroundPtr background();
	
	/**
	* @func time
	*/
	inline uint64_t time() const { return _time; }
	
	/**
	* @func set_time
	*/
	inline void set_time(uint64_t value) { _time = value; }
	
	/**
	* @func name
	*/
	inline String name() const { return _css_name.value(); }
	
	/**
	* @func hash
	*/
	inline uint32_t hash() const { return _css_name.hash(); }
	
	/**
	* @func parent
	*/
	inline StyleSheets* parent() { return _parent; }
	
	/**
	* @func normal
	*/
	inline StyleSheets* normal() { return _child_NORMAL; }
	
	/**
	* @func normal
	*/
	inline StyleSheets* hover() { return _child_HOVER; }
	
	/**
	* @func normal
	*/
	inline StyleSheets* down() { return _child_DOWN; }
	
	/**
	* @func find children
	*/
	StyleSheets* find(const CSSName& name);
	
	/**
	* @func has_child
	*/
	inline bool has_child() const { return _children.length(); }
	
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
	inline bool is_support_pseudo() const { return _is_support_pseudo; }
	
	/**
	* @func pseudo
	*/
	inline CSSPseudoClass pseudo() const { return _pseudo; }

private:
	CSSName                       _css_name;
	StyleSheets*                  _parent;
	Dict<uint32_t, StyleSheets*>     _children;
	// Dict<PropertyName, Property*>    _property;
	Dict<uint32_t, Property*>    _property;
	uint64_t       _time;
	StyleSheets*   _child_NORMAL;
	StyleSheets*   _child_HOVER;
	StyleSheets*   _child_DOWN;
	bool           _is_support_pseudo; // _NORMAL | _HOVER | _DOWN
	CSSPseudoClass _pseudo;
	
	Qk_DEFINE_INLINE_CLASS(Inl);
	friend class CSSManager;
};

/**
* @class RootStyleSheets
*/
class Qk_EXPORT RootStyleSheets: public StyleSheets {
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
	Dict<uint32_t, int>                    _all_css_names;
	Dict<uint32_t, Array<uint32_t>>  _css_query_group_cache;

	Qk_DEFINE_INLINE_CLASS(Inl);
};

/**
* @class StyleSheetsClass
*/
class Qk_EXPORT StyleSheetsClass: public Object {
	Qk_HIDDEN_ALL_COPY(StyleSheetsClass);
public:
	StyleSheetsClass(View* host);
	
	/**
	* @destructor
	*/
	virtual ~StyleSheetsClass();
	
	/**
	* @func name
	*/
	inline const Array<String>& name() const {
		return _classs;
	}
	
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
	inline bool has_child() const {
		return _child_style_sheets.size();
	}
	
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
		return _child_style_sheets;
	}
	
private:
	View*           _host;
	Array<String>   _classs;
	Array<uint32_t> _query_group;
	Array<StyleSheets*> _child_style_sheets; // 当前应用的样式表中拥有子样式表的表供后代视图查询
	bool            _is_support_pseudo;      // 当前样式表选择器能够找到支持伪类的样式表
	bool            _once_apply;             // 是否为第一次应用样式表,在处理动作时如果为第一次忽略动作
	CSSPseudoClass  _multiple_status;
	
	Qk_DEFINE_INLINE_CLASS(Inl);
};

/**
* @class StyleSheetsScope
*/
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
	StyleSheetsScope(View* scope);
	void push_scope(View* scope);
	void pop_scope();
	inline View* bottom_scope() { return _scopes.length() ? _scopes.back() : nullptr; }
	inline const List<Scope>& style_sheets() { return _style_sheets; }
private:
	typedef Dict<StyleSheets*, Scope::Wrap> StyleSheetsMap;
	List<View*>   _scopes;
	List<Scope>   _style_sheets;
	StyleSheetsMap     _style_sheets_map;
};

Qk_INLINE RootStyleSheets* root_styles() { 
	return RootStyleSheets::shared(); 
}

Qk_NAMESPACE_END
#endif