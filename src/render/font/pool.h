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

#ifndef __quark__font__pool__
#define __quark__font__pool__

#include "../../value.h"
#include "./font.h"

namespace quark {

	class Application;

	class Qk_EXPORT FontPool: public Object {
		Qk_HIDDEN_ALL_COPY(FontPool);
	public:
		FontPool(Application* host);
		virtual ~FontPool();
		// define ptops
		Qk_Define_Prop_Get(int32_t, count_families);
		Qk_Define_Prop_Get(Array<String>, familys);
		Qk_Define_Prop_Acc_Get(const Array<Typeface>&, second);
		Qk_Define_Prop_Acc_Get(const Typeface&, last);
		Qk_Define_Prop_Get(Application*, host);
		Qk_Define_Prop_Get(GlyphID, last_65533);
		// methods
		FFID getFFID(cString familys = String());
		FFID getFFID(const Array<String>& familys);
		Typeface match(cString& familyName, const FontStyle& style, bool useDefault = false);
		void register_from_data(cBuffer& buff);
		void register_from_file(cString& path);
	private:
		void initialize();
		void           *_impl;
		Array<Typeface> _second;
		Typeface        _last;
		Dict<String, Dict<FontStyle, Typeface>> _rtf;
		Dict<uint64_t, FFID> _FFIDs;
	};

}
#endif
