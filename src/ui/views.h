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

#ifndef __quark__ui__views__
#define __quark__ui__views__

#define Qk_DEFINE_VIEW_ACCE_GET(...) Qk_DEFINE_ACCE_GET(__VA_ARGS__)
#define Qk_DEFINE_VIEW_ACCESSOR(type,name,...) \
	Qk_DEFINE_VIEW_ACCE_GET(type,name,##__VA_ARGS__); void set_##name (type val,bool isRt=false);

#define Qk_DEFINE_VIEW_PROP_GET(...) Qk_DEFINE_PROP_GET(__VA_ARGS__)
#define Qk_DEFINE_VIEW_PROPERTY(type,name,...) \
	Qk_DEFINE_VIEW_PROP_GET(type,name,##__VA_ARGS__) void set_##name (type val,bool isRt=false)

#define Qk_DEFINE_VIEW_PROP_GET_Atomic(...) Qk_DEFINE_PROP_GET_Atomic(__VA_ARGS__)
#define Qk_DEFINE_VIEW_PROPERTY_Atomic(type,name,...) \
	Qk_DEFINE_VIEW_PROP_GET_Atomic(type,name,##__VA_ARGS__) void set_##name (type val,bool isRt=false)

#include "./types.h"

namespace qk {
	class Action;
	class TextInput;
	class TextLines;
	class TextConfig;
	class TextOptions;
	class Painter;
	class Window;
	class ScrollView;
	class MatrixView;
	class CStyleSheetsClass;
	class Button;
	class CssPropAccessor;
	class BoxFilter;
	class BoxShadow;
	typedef BoxFilter* BoxFilterPtr;
	typedef BoxShadow* BoxShadowPtr;
	typedef Array<float> ArrayFloat;
	typedef Array<Color> ArrayColor;
	typedef Array<BoxOrigin> ArrayOrigin;
	typedef Array<Border> ArrayBorder;

	enum ViewType {
		kView_ViewType, // view
		kSprite_ViewType, // sprite
		kSpine_ViewType, // spine
		kLabel_ViewType, // textOpts
		kBox_ViewType,  // box
		kFlex_ViewType, // box flex
		kFlow_ViewType, // box flow
		kFree_ViewType, // box
		kImage_ViewType, // box
		kVideo_ViewType, // box
		kInput_ViewType, // box textOpts input
		kTextarea_ViewType, // box textOpts input
		kScroll_ViewType, // box
		kText_ViewType, // box textOpts
		kButton_ViewType, // box
		kMatrix_ViewType, // box
		kRoot_ViewType, // box
		kEnum_Counts_ViewType,
	};
}

#endif
