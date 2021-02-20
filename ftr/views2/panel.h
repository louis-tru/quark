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

#ifndef __ftr__panel__
#define __ftr__panel__

#include "./div.h"

namespace ftr {

	/**
	 * @class Panel
	 */
	class FX_EXPORT Panel: public Div {
		public:
		FX_DEFINE_GUI_VIEW(PANEL, Panel, panel)
		
		Panel();
		
		/**
		 * @func allow_leave get
		 */
		inline bool allow_leave() const { return _allow_leave; }
		
		/**
		 * @func set_allow_leave set
		 */
		inline void set_allow_leave(bool value) { _allow_leave = value; }
		
		/**
		 * @func allow_entry get
		 */
		inline bool allow_entry() const { return _allow_entry; }
		
		/**
		 * @func set_allow_entry set
		 */
		inline void set_allow_entry(bool value) { _allow_entry = value; }
		
		/**
		 * @func switch_time get
		 */
		inline uint64_t interval_time() const { return _interval_time; }
		
		/**
		 * @func set_switch_time set
		 */
		inline void set_interval_time(uint64_t value) { _interval_time = value; }
		
		/**
		 * @func enable_switch get
		 */
		inline bool enable_select() const { return _enable_select; }
		
		/**
		 * @func set_enable_switch set
		 */
		inline void set_enable_select(bool value) { _enable_select = value; }
		
		/**
		 * @func parent_panel
		 */
		Panel* parent_panel();
		
		/**
		 * @func is_activity
		 */
		bool is_activity() const;
		
		private:
		
		bool  _allow_leave;
		bool  _allow_entry;
		uint64_t _interval_time;
		bool  _enable_select;
		
		FX_DEFINE_INLINE_CLASS(Inl);
	};

}

#endif
