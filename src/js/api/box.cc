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

#include "./ui.h"
#include "../../ui/view/box.h"
#include "../../ui/view/flex.h"
#include "../../ui/view/flow.h"
#include "../../ui/view/free.h"
#include "../../ui/view/image.h"

namespace qk { namespace js {
	typedef qk::Wrap Wrap;

	class MixBox: public MixViewObject {
	public:
		typedef Box Type;
		static void binding(JSObject* exports, Worker* worker) {
			Js_Define_Class(Box, View, { Js_NewView(Box); });

			Js_MixObject_Accessor(Box, bool, clip, clip);
			Js_MixObject_Accessor(Box, Align, align, align);
			Js_MixObject_Accessor(Box, BoxSize, width, width);
			Js_MixObject_Accessor(Box, BoxSize, height, height);
			Js_MixObject_Accessor(Box, BoxSize, min_width, minWidth);
			Js_MixObject_Accessor(Box, BoxSize, min_height, minHeight);
			Js_MixObject_Accessor(Box, BoxSize, max_width, maxWidth);
			Js_MixObject_Accessor(Box, BoxSize, max_height, maxHeight);
			Js_MixObject_Accessor(Box, ArrayFloat, margin, margin);
			Js_MixObject_Accessor(Box, float, margin_top, marginTop);
			Js_MixObject_Accessor(Box, float, margin_right, marginRight);
			Js_MixObject_Accessor(Box, float, margin_bottom, marginBottom);
			Js_MixObject_Accessor(Box, float, margin_left, marginLeft);
			Js_MixObject_Accessor(Box, ArrayFloat, padding, padding);
			Js_MixObject_Accessor(Box, float, padding_top, paddingTop);
			Js_MixObject_Accessor(Box, float, padding_right, paddingRight);
			Js_MixObject_Accessor(Box, float, padding_bottom, paddingBottom);
			Js_MixObject_Accessor(Box, float, padding_left, paddingLeft);
			Js_MixObject_Accessor(Box, ArrayFloat, border_radius, borderRadius);
			Js_MixObject_Accessor(Box, float, border_radius_left_top, borderRadiusLeftTop);
			Js_MixObject_Accessor(Box, float, border_radius_right_top, borderRadiusRightTop);
			Js_MixObject_Accessor(Box, float, border_radius_right_bottom, borderRadiusRightBottom);
			Js_MixObject_Accessor(Box, float, border_radius_left_bottom, borderRadiusLeftBottom);
			Js_MixObject_Accessor(Box, ArrayBorder, border, border); // border width
			Js_MixObject_Accessor(Box, Border, border_top, borderTop);
			Js_MixObject_Accessor(Box, Border, border_right, borderRight);
			Js_MixObject_Accessor(Box, Border, border_bottom, borderBottom);
			Js_MixObject_Accessor(Box, Border, border_left, borderLeft);
			Js_MixObject_Accessor(Box, ArrayFloat, border_width, borderWidth);
			Js_MixObject_Accessor(Box, ArrayColor, border_color, borderColor);
			Js_MixObject_Accessor(Box, float, border_width_top, borderWidthTop); // border width
			Js_MixObject_Accessor(Box, float, border_width_right, borderWidthRight);
			Js_MixObject_Accessor(Box, float, border_width_bottom, borderWidthBottom);
			Js_MixObject_Accessor(Box, float, border_width_left, borderWidthLeft);
			Js_MixObject_Accessor(Box, Color, border_color_top, borderColorTop); // border color
			Js_MixObject_Accessor(Box, Color, border_color_right, borderColorRight);
			Js_MixObject_Accessor(Box, Color, border_color_bottom, borderColorBottom);
			Js_MixObject_Accessor(Box, Color, border_color_left, borderColorLeft);
			Js_MixObject_Accessor(Box, Color, background_color, backgroundColor);
			Js_MixObject_Accessor(Box, BoxFilterPtr, background, background);
			Js_MixObject_Accessor(Box, BoxShadowPtr, box_shadow, boxShadow);
			Js_MixObject_Accessor(Box, Vec2, weight, weight);
			// -----------------------------------------------------------------------------
			// @thread Rt
			Js_Class_Accessor_Get(contentSize, {
				Js_Return( worker->types()->jsvalue(self->content_size()) );
			});
			// -----------------------------------------------------------------------------

			cls->exports("Box", exports);
		}
	};

	class MixFlex: public MixViewObject {
	public:
		typedef Flex Type;
		static void binding(JSObject* exports, Worker* worker) {
			Js_Define_Class(Flex, Box, {
				Js_NewView(Flex);
			});
			Js_MixObject_Accessor(Flex, Direction, direction, direction);
			Js_MixObject_Accessor(Flex, ItemsAlign, items_align, itemsAlign);
			Js_MixObject_Accessor(Flex, CrossAlign, cross_align, crossAlign);
			cls->exports("Flex", exports);
		}
	};

	class MixFlow: public MixViewObject {
	public:
		typedef Flow Type;
		static void binding(JSObject* exports, Worker* worker) {
			Js_Define_Class(Flow, Flex, {
				Js_NewView(Flow);
			});
			Js_MixObject_Accessor(Flow, Wrap, wrap, wrap);
			Js_MixObject_Accessor(Flow, WrapAlign, wrap_align, wrapAlign);
			cls->exports("Flow", exports);
		}
	};

	class MixFree: public MixViewObject {
	public:
		static void binding(JSObject* exports, Worker* worker) {
			Js_Define_Class(Free, Box, {
				Js_NewView(Free);
			});
			cls->exports("Free", exports);
		}
	};

	class MixImage: public MixViewObject {
	public:
		typedef Image Type;
		static void binding(JSObject* exports, Worker* worker) {
			Js_Define_Class(Image, Box, {
				Js_NewView(Image);
			});
			Js_MixObject_Accessor(Image, String, src, src);
			// Qk_DEFINE_ACCESSOR(ImageSource*, source);
			cls->exports("Image", exports);
		}
	};

	void binding_box(JSObject* exports, Worker* worker) {
		MixBox::binding(exports, worker);
		MixFlex::binding(exports, worker);
		MixFlow::binding(exports, worker);
		MixFree::binding(exports, worker);
		MixImage::binding(exports, worker);
	}
} }
