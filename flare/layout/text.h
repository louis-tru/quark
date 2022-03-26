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

#ifndef __flare__layout__text__
#define __flare__layout__text__

#include "./box.h"

F_NAMESPACE_START

class F_EXPORT Text: public Box {
	F_Define_View(Text);
public:
// TODO ...
private:
// TODO ...
};

class DefaultTextSettings: public Object {
public:
	DefaultTextSettings();
	// define props
	F_DEFINE_PROP(TextColor, text_background_color);
	F_DEFINE_PROP(TextColor, text_color);
	F_DEFINE_PROP(TextSize, text_size);
	F_DEFINE_PROP(TextWeight, text_weight);
	F_DEFINE_PROP(TextStyle, text_style);
	F_DEFINE_PROP(TextFamily, text_family);
	F_DEFINE_PROP(TextShadow, text_shadow);
	F_DEFINE_PROP(TextLineHeight, text_line_height);
	F_DEFINE_PROP(TextDecoration, text_decoration);
	F_DEFINE_PROP(TextOverflow, text_overflow);
	F_DEFINE_PROP(TextWhiteSpace, text_white_space);
};

F_NAMESPACE_END
#endif