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

#include "./css.h"

#ifndef __flare__css__1__
#define __flare__css__1__

namespace flare {

	typedef StyleSheets::Property Property;
	typedef PropertysAccessor::Accessor Accessor;
	typedef KeyframeAction::Frame Frame;

	/**
	* @class CSSProperty
	*/
	template<class T> class CSSProperty: public Property {
		public:
		typedef T (View::*Get)();
		typedef void (View::*Set)(T value);
		typedef void (TextFont::*Set2)(T value);
		typedef void (TextLayout::*Set3)(T value);
		
		CSSProperty(T value): _value(value) {}
		virtual ~CSSProperty() {}
		
		inline T value() const { return _value; }
		inline void set_value(T value) { _value = value; }
		
		inline void assignment(View* view, PropertyName name) {
			ASSERT(view);
			PropertysAccessor::Accessor accessor =
			PropertysAccessor::shared()->accessor(view->view_type(), name);
			if (accessor.set_accessor) {
				(view->*reinterpret_cast<Set>(accessor.set_accessor))(_value);
			}
		}
		protected:
		T _value;
	};

	/**
	* @class CSSProperty1
	*/
	template<class T, PropertyName Name> class CSSProperty1: public CSSProperty<T> {
		public:
		CSSProperty1(T value): CSSProperty<T>(value) {}
		virtual void assignment(View* view) {
			CSSProperty<T>::assignment(view, Name);
		}
		virtual void assignment(Frame* frame) {
			FX_UNREACHABLE();
		}
	};

	FX_DEFINE_INLINE_MEMBERS(StyleSheets, Inl) {
		public:
		#define _inl_ss(self) static_cast<StyleSheets::Inl*>(self)

		template<PropertyName Name, class T>
		inline void set_property_value(T value) {
			typedef CSSProperty1<T, Name> Type;
			auto it = _property.find(Name);
			if ( it == _property.end() ) {
				Type* prop = new Type(value);
				_property[Name] = prop;
			} else {
				static_cast<Type*>(it->value)->set_value(value);
			}
		}
		
		template<PropertyName Name, class T>
		inline T get_property_value() {
			typedef CSSProperty1<T, Name> Type;
			auto it = _property.find(Name);
			if ( it == _property.end() ) {
				Type* prop = new Type(T());
				_property[Name] = prop;
				return prop->value();
			} else {
				return static_cast<Type*>(it->value)->value();
			}
		}
		
		StyleSheets* find1(uint32_t hash);
		StyleSheets* find2(const CSSName& name, CSSPseudoClass pseudo);
		KeyframeAction* assignment(View* view, KeyframeAction* action, bool ignore_action);
	};

	FX_DEFINE_INLINE_MEMBERS(RootStyleSheets, Inl) {
		public:
		#define _inl_r(self) static_cast<RootStyleSheets::Inl*>(self)
		static Array<String>& sort( Array<String>& arr, uint32_t len );
		static bool verification_and_format(cString& name, CSSName& out, CSSPseudoClass& pseudo);
		void mark_classs_names(const CSSName& name);
		// ".div_cls.div_cls2 .aa.bb.cc"
		// ".div_cls.div_cls2:down .aa.bb.cc"
		StyleSheets* instance(cString& expression);
		Array<uint32_t>* get_css_find_group(uint32_t hash);
		void add_css_query_grpup(uint32_t hash, Array<uint32_t>& css_query_group);
		CSSName new_css_name1(cString& a);
		CSSName new_css_name2(cString& a, cString& b);
		CSSName new_css_name3(cString& a, cString& b, cString& c);
		Array<uint32_t> get_css_query_grpup(Array<String>& classs);
	};

	FX_DEFINE_INLINE_MEMBERS(StyleSheetsClass, Inl) {
		public:
		#define _inl_cvc(self) static_cast<StyleSheetsClass::Inl*>(self)
		void update_classs(Array<String>&& classs);
		void apply(StyleSheetsScope* scope, bool* effect_child, bool RETURN_EFFECT_CHILD);
	};

	static void mark_classs_names(const CSSName& name);

}
#endif
