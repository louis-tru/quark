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

#ifndef __ftr__css__
#define __ftr__css__

namespace ftr {

	/**
	* @class StyleSheets
	*/
	class FX_EXPORT StyleSheets: public Object {
		FX_HIDDEN_ALL_COPY(StyleSheets);
		protected:

		StyleSheets(const CSSName& name, StyleSheets* parent, CSSPseudoClass pseudo);
		
		/**
		* @destructor
		*/
		virtual ~StyleSheets();
		
		public:
		typedef KeyframeAction::Frame Frame;
		
		class FX_EXPORT Property {
			public:
			virtual ~Property() = default;
			virtual void assignment(View* view) = 0;
			virtual void assignment(Frame* frame) = 0;
		};
		
		// -------------------- set property --------------------
		
# define fx_def_property(ENUM, TYPE, NAME) void set_##NAME(TYPE value);
		FX_EACH_PROPERTY_TABLE(fx_def_property)
# undef fx_def_property
		
		/**
		* @func background()
		*/
		BackgroundPtr background();
		
		/**
		* @func time
		*/
		inline uint64 time() const { return _time; }
		
		/**
		* @func set_time
		*/
		inline void set_time(uint64 value) { _time = value; }
		
		/**
		* @func name
		*/
		inline String name() const { return _css_name.value(); }
		
		/**
		* @func hash
		*/
		inline uint hash() const { return _css_name.hash(); }
		
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
		Map<uint, StyleSheets*>       _children;
		Map<PropertyName, Property*>  _property;
		uint64         _time;
		StyleSheets*   _child_NORMAL;
		StyleSheets*   _child_HOVER;
		StyleSheets*   _child_DOWN;
		bool           _is_support_pseudo; // _NORMAL | _HOVER | _DOWN
		CSSPseudoClass _pseudo;
		
		FX_DEFINE_INLINE_CLASS(Inl);
		friend class CSSManager;
	};

	/**
	* @class RootStyleSheets
	*/
	class FX_EXPORT RootStyleSheets: public StyleSheets {
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
		Map<uint, int>          _all_css_names;
		Map<uint, Array<uint>>  _css_query_group_cache;

		FX_DEFINE_INLINE_CLASS(Inl);
	};

	FX_INLINE RootStyleSheets* root_styles() { 
		return RootStyleSheets::shared(); 
	}

}
#endif