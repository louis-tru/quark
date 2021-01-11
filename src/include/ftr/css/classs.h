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

#ifndef __ftr__styles__
#define __ftr__styles__

#include "ftr/util/util.h"
#include "ftr/util/map.h"
#include "ftr/util/string.h"
#include "ftr/util/list.h"
#include "ftr/property.h"
#include "ftr/value.h"
#include "ftr/action.h"

namespace ftr {

	/**
	* @class CSSViewClasss
	*/
	class FX_EXPORT CSSViewClasss: public Object {
		FX_HIDDEN_ALL_COPY(CSSViewClasss);
		public:
		CSSViewClasss(View* host);
		
		/**
		* @destructor
		*/
		virtual ~CSSViewClasss();
		
		/**
		* @func name
		*/
		inline const Array<String>& name() const {
			return m_classs;
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
			return m_child_style_sheets.length();
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
		
		FX_DEFINE_INLINE_CLASS(Inl);
	};

}

#endif