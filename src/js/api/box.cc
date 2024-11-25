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

	class WrapBox: public WrapViewObject {
	public:
		static void binding(JSObject* exports, Worker* worker) {
			Js_Define_Class(Box, View, { Js_NewView(Box); });

			Js_WrapObject_Accessor(Box, bool, clip, clip);
			Js_WrapObject_Accessor(Box, Align, align, align);
			Js_WrapObject_Accessor(Box, BoxSize, width, width);
			Js_WrapObject_Accessor(Box, BoxSize, height, height);
			Js_WrapObject_Accessor(Box, BoxSize, min_width, minWidth);
			Js_WrapObject_Accessor(Box, BoxSize, min_height, minHeight);
			Js_WrapObject_Accessor(Box, BoxSize, max_width, maxWidth);
			Js_WrapObject_Accessor(Box, BoxSize, max_height, maxHeight);
			Js_WrapObject_Accessor(Box, ArrayFloat, margin, margin);
			Js_WrapObject_Accessor(Box, float, margin_top, marginTop);
			Js_WrapObject_Accessor(Box, float, margin_right, marginRight);
			Js_WrapObject_Accessor(Box, float, margin_bottom, marginBottom);
			Js_WrapObject_Accessor(Box, float, margin_left, marginLeft);
			Js_WrapObject_Accessor(Box, ArrayFloat, padding, padding);
			Js_WrapObject_Accessor(Box, float, padding_top, paddingTop);
			Js_WrapObject_Accessor(Box, float, padding_right, paddingRight);
			Js_WrapObject_Accessor(Box, float, padding_bottom, paddingBottom);
			Js_WrapObject_Accessor(Box, float, padding_left, paddingLeft);
			Js_WrapObject_Accessor(Box, ArrayFloat, border_radius, borderRadius);
			Js_WrapObject_Accessor(Box, float, border_radius_left_top, borderRadiusLeftTop);
			Js_WrapObject_Accessor(Box, float, border_radius_right_top, borderRadiusRightTop);
			Js_WrapObject_Accessor(Box, float, border_radius_right_bottom, borderRadiusRightBottom);
			Js_WrapObject_Accessor(Box, float, border_radius_left_bottom, borderRadiusLeftBottom);
			Js_WrapObject_Accessor(Box, ArrayBorder, border, border); // border width
			Js_WrapObject_Accessor(Box, BoxBorder, border_top, borderTop);
			Js_WrapObject_Accessor(Box, BoxBorder, border_right, borderRight);
			Js_WrapObject_Accessor(Box, BoxBorder, border_bottom, borderBottom);
			Js_WrapObject_Accessor(Box, BoxBorder, border_left, borderLeft);
			Js_WrapObject_Accessor(Box, ArrayFloat, border_width, borderWidth);
			Js_WrapObject_Accessor(Box, ArrayColor, border_color, borderColor);
			Js_WrapObject_Accessor(Box, float, border_width_top, borderWidthTop); // border width
			Js_WrapObject_Accessor(Box, float, border_width_right, borderWidthRight);
			Js_WrapObject_Accessor(Box, float, border_width_bottom, borderWidthBottom);
			Js_WrapObject_Accessor(Box, float, border_width_left, borderWidthLeft);
			Js_WrapObject_Accessor(Box, Color, border_color_top, borderColorTop); // border color
			Js_WrapObject_Accessor(Box, Color, border_color_right, borderColorRight);
			Js_WrapObject_Accessor(Box, Color, border_color_bottom, borderColorBottom);
			Js_WrapObject_Accessor(Box, Color, border_color_left, borderColorLeft);
			Js_WrapObject_Accessor(Box, Color, background_color, backgroundColor);
			Js_WrapObject_Accessor(Box, BoxFilterPtr, background, background);
			Js_WrapObject_Accessor(Box, BoxShadowPtr, box_shadow, boxShadow);
			Js_WrapObject_Accessor(Box, float, weight, weight);
			// -----------------------------------------------------------------------------
			// @safe Rt
			Js_Class_Accessor_Get(wrapX, {
				Js_Self(Box);
				Js_Return( worker->types()->jsvalue(self->wrap_x()) );
			});
			Js_Class_Accessor_Get(wrapY, {
				Js_Self(Box);
				Js_Return( worker->types()->jsvalue(self->wrap_y()) );
			});
			Js_Class_Accessor_Get(contentSize, {
				Js_Self(Box);
				Js_Return( worker->types()->jsvalue(self->content_size()) );
			});
			Js_Class_Accessor_Get(clientSize, {
				Js_Self(Box);
				Js_Return( worker->types()->jsvalue(self->client_size()) );
			});
			// -----------------------------------------------------------------------------

			cls->exports("Box", exports);
		}
	};

	class WrapFlex: public WrapViewObject {
	public:
		static void binding(JSObject* exports, Worker* worker) {
			Js_Define_Class(Flex, Box, {
				Js_NewView(Flex);
			});
			Js_WrapObject_Accessor(Flex, Direction, direction, direction);
			Js_WrapObject_Accessor(Flex, ItemsAlign, items_align, itemsAlign);
			Js_WrapObject_Accessor(Flex, CrossAlign, cross_align, crossAlign);
			cls->exports("Flex", exports);
		}
	};

	class WrapFlow: public WrapViewObject {
	public:
		static void binding(JSObject* exports, Worker* worker) {
			Js_Define_Class(Flow, Flex, {
				Js_NewView(Flow);
			});
			Js_WrapObject_Accessor(Flow, Wrap, wrap, wrap);
			Js_WrapObject_Accessor(Flow, WrapAlign, wrap_align, wrapAlign);
			cls->exports("Flow", exports);
		}
	};

	class WrapFree: public WrapViewObject {
	public:
		static void binding(JSObject* exports, Worker* worker) {
			Js_Define_Class(Free, Box, {
				Js_NewView(Free);
			});
			cls->exports("Free", exports);
		}
	};

	class WrapImage: public WrapViewObject {
	public:
		static void binding(JSObject* exports, Worker* worker) {
			Js_Define_Class(Image, Box, {
				Js_NewView(Image);
			});
			Js_WrapObject_Accessor(Image, String, src, src);
			// Qk_DEFINE_ACCE(ImageSource*, source);
			cls->exports("Image", exports);
		}
	};

	void binding_box(JSObject* exports, Worker* worker) {
		WrapBox::binding(exports, worker);
		WrapFlex::binding(exports, worker);
		WrapFlow::binding(exports, worker);
		WrapFree::binding(exports, worker);
		WrapImage::binding(exports, worker);
	}
} }
