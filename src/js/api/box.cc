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

#include "./view.h"
#include "../../ui/view/box.h"
#include "../../ui/view/flex.h"
#include "../../ui/view/flow.h"
#include "../../ui/view/float.h"
#include "../../ui/view/image.h"

namespace qk { namespace js {

	class WrapBox: public WrapViewObject {
	public:
		static void binding(JSObject* exports, Worker* worker) {
			Js_Define_Class(Box, View, {
				Js_NewView(Box);
			});

			Js_Set_Class_Accessor(clip, {
				Js_Self(Box);
				Js_Return( self->clip() );
			}, {
				Js_Self(Box);
				self->set_clip(val->toBooleanValue(worker));
			});

			Js_Set_Class_Accessor(width, {
				Js_Self(Box);
				Js_Return( worker->types()->newInstance(self->width()) );
			}, {
				Js_Parse_Type(BoxSize, val, "Box.width = %s");
				Js_Self(Box);
				self->set_width(out);
			});

			Js_Set_Class_Accessor(height, {
				Js_Self(Box);
				Js_Return( worker->types()->newInstance(self->height()) );
			}, {
				Js_Parse_Type(BoxSize, val, "Box.height = %s");
				Js_Self(Box);
				self->set_height(out);
			});

			Js_Set_Class_Accessor(widthLimit, {
				Js_Self(Box);
				Js_Return( worker->types()->newInstance(self->width_limit()) );
			}, {
				Js_Parse_Type(BoxSize, val, "Box.widthLimit = %s");
				Js_Self(Box);
				self->set_width_limit(out);
			});

			Js_Set_Class_Accessor(heightLimit, {
				Js_Self(Box);
				Js_Return( worker->types()->newInstance(self->height_limit()) );
			}, {
				Js_Parse_Type(BoxSize, val, "Box.heightLimit = %s");
				Js_Self(Box);
				self->set_height_limit(out);
			});

			Js_Set_Class_Accessor(marginTop, {
				Js_Self(Box);
				Js_Return( worker->types()->newInstance(self->margin_top()) );
			}, {
				Js_Parse_Type(float, val, "Box.marginTop = %s");
				Js_Self(Box);
				self->set_margin_top(out);
			});

			Js_Set_Class_Accessor(marginRight, {
				Js_Self(Box);
				Js_Return( worker->types()->newInstance(self->margin_right()) );
			}, {
				Js_Parse_Type(float, val, "Box.marginRight = %s");
				Js_Self(Box);
				self->set_margin_right(out);
			});

			Js_Set_Class_Accessor(marginBottom, {
				Js_Self(Box);
				Js_Return( worker->types()->newInstance(self->margin_bottom()) );
			}, {
				Js_Parse_Type(float, val, "Box.marginBottom = %s");
				Js_Self(Box);
				self->set_margin_bottom(out);
			});

			Js_Set_Class_Accessor(marginLeft, {
				Js_Self(Box);
				Js_Return( worker->types()->newInstance(self->margin_left()) );
			}, {
				Js_Parse_Type(float, val, "Box.marginLeft = %s");
				Js_Self(Box);
				self->set_margin_left(out);
			});

			Js_Set_Class_Accessor(paddingTop, {
				Js_Self(Box);
				Js_Return( worker->types()->newInstance(self->padding_top()) );
			}, {
				Js_Parse_Type(float, val, "Box.paddingTop = %s");
				Js_Self(Box);
				self->set_padding_top(out);
			});

			Js_Set_Class_Accessor(paddingRight, {
				Js_Self(Box);
				Js_Return( worker->types()->newInstance(self->padding_right()) );
			}, {
				Js_Parse_Type(float, val, "Box.paddingRight = %s");
				Js_Self(Box);
				self->set_padding_right(out);
			});

			Js_Set_Class_Accessor(paddingBottom, {
				Js_Self(Box);
				Js_Return( worker->types()->newInstance(self->padding_bottom()) );
			}, {
				Js_Parse_Type(float, val, "Box.paddingBottom = %s");
				Js_Self(Box);
				self->set_padding_bottom(out);
			});

			Js_Set_Class_Accessor(paddingLeft, {
				Js_Self(Box);
				Js_Return( worker->types()->newInstance(self->padding_left()) );
			}, {
				Js_Parse_Type(float, val, "Box.paddingLeft = %s");
				Js_Self(Box);
				self->set_padding_left(out);
			});

			Js_Set_Class_Accessor(borderRadiusLeftTop, {
				Js_Self(Box);
				Js_Return( worker->types()->newInstance(self->border_radius_left_top()) );
			}, {
				Js_Parse_Type(float, val, "Box.borderRadiusLeftTop = %s");
				Js_Self(Box);
				self->set_border_radius_left_top(out);
			});

			Js_Set_Class_Accessor(borderRadiusRightTop, {
				Js_Self(Box);
				Js_Return( worker->types()->newInstance(self->border_radius_right_top()) );
			}, {
				Js_Parse_Type(float, val, "Box.borderRadiusRightTop = %s");
				Js_Self(Box);
				self->set_border_radius_right_top(out);
			});

			Js_Set_Class_Accessor(borderRadiusRightBottom, {
				Js_Self(Box);
				Js_Return( worker->types()->newInstance(self->border_radius_right_bottom()) );
			}, {
				Js_Parse_Type(float, val, "Box.borderRadiusRightBottom = %s");
				Js_Self(Box);
				self->set_border_radius_right_bottom(out);
			});

			Js_Set_Class_Accessor(borderRadiusLeftBottom, {
				Js_Self(Box);
				Js_Return( worker->types()->newInstance(self->border_radius_left_bottom()) );
			}, {
				Js_Parse_Type(float, val, "Box.borderRadiusLeftBottom = %s");
				Js_Self(Box);
				self->set_border_radius_left_bottom(out);
			});

			Js_Set_Class_Accessor(borderColorTop, {
				Js_Self(Box);
				Js_Return( worker->types()->newInstance(self->border_color_top()) );
			}, {
				Js_Parse_Type(Color, val, "Box.borderColorTop = %s");
				Js_Self(Box);
				self->set_border_color_top(out);
			});

			Js_Set_Class_Accessor(borderColorRight, {
				Js_Self(Box);
				Js_Return( worker->types()->newInstance(self->border_color_right()) );
			}, {
				Js_Parse_Type(Color, val, "Box.borderColorRight = %s");
				Js_Self(Box);
				self->set_border_color_right(out);
			});

			Js_Set_Class_Accessor(borderColorBottom, {
				Js_Self(Box);
				Js_Return( worker->types()->newInstance(self->border_color_bottom()) );
			}, {
				Js_Parse_Type(Color, val, "Box.borderColorBottom = %s");
				Js_Self(Box);
				self->set_border_color_bottom(out);
			});

			Js_Set_Class_Accessor(borderColorLeft, {
				Js_Self(Box);
				Js_Return( worker->types()->newInstance(self->border_color_left()) );
			}, {
				Js_Parse_Type(Color, val, "Box.borderColorLeft = %s");
				Js_Self(Box);
				self->set_border_color_left(out);
			});

			Js_Set_Class_Accessor(borderWidthTop, {
				Js_Self(Box);
				Js_Return( worker->types()->newInstance(self->border_width_top()) );
			}, {
				Js_Parse_Type(float, val, "Box.borderWidthTop = %s");
				Js_Self(Box);
				self->set_border_width_top(out);
			});

			Js_Set_Class_Accessor(borderWidthRight, {
				Js_Self(Box);
				Js_Return( worker->types()->newInstance(self->border_width_right()) );
			}, {
				Js_Parse_Type(float, val, "Box.borderWidthRight = %s");
				Js_Self(Box);
				self->set_border_width_right(out);
			});

			Js_Set_Class_Accessor(borderWidthBottom, {
				Js_Self(Box);
				Js_Return( worker->types()->newInstance(self->border_width_bottom()) );
			}, {
				Js_Parse_Type(float, val, "Box.borderWidthBottom = %s");
				Js_Self(Box);
				self->set_border_width_bottom(out);
			});

			Js_Set_Class_Accessor(borderWidthLeft, {
				Js_Self(Box);
				Js_Return( worker->types()->newInstance(self->border_width_left()) );
			}, {
				Js_Parse_Type(float, val, "Box.borderWidthLeft = %s");
				Js_Self(Box);
				self->set_border_width_left(out);
			});

			Js_Set_Class_Accessor(backgroundColor, {
				Js_Self(Box);
				Js_Return( worker->types()->newInstance(self->background_color()) );
			}, {
				Js_Parse_Type(Color, val, "Box.backgroundColor = %s");
				Js_Self(Box);
				self->set_background_color(out);
			});

			Js_Set_Class_Accessor(background, {
				Js_Self(Box);
				Js_Return( worker->types()->newInstance(self->background()) );
			}, {
				Js_Parse_Type2(BoxFilter, BoxFilter*, val, "Box.background = %s");
				Js_Self(Box);
				self->set_background(out);
			});

			Js_Set_Class_Accessor(boxShadow, {
				Js_Self(Box);
				Js_Return( worker->types()->newInstance(self->box_shadow()) );
			}, {
				Js_Parse_Type2(BoxShadow, BoxShadow*, val, "Box.boxShadow = %s");
				Js_Self(Box);
				self->set_box_shadow(out);
			});

			Js_Set_Class_Accessor(weight, {
				Js_Self(Box);
				Js_Return( worker->types()->newInstance(self->weight()) );
			}, {
				Js_Parse_Type(float, val, "Box.weight = %s");
				Js_Self(Box);
				self->set_weight(out);
			});

			Js_Set_Class_Accessor(align, {
				Js_Self(Box);
				Js_Return( worker->types()->newInstance(self->align()) );
			}, {
				Js_Parse_Type(Align, val, "Box.align = %s");
				Js_Self(Box);
				self->set_align(out);
			});

			// -----------------------------------------------------------------------------
			// @safe Rt
			Js_Set_Class_Accessor_Get(contentSize, {
				Js_Self(Box);
				Js_Return( worker->types()->newInstance(self->content_size()) );
			});
			Js_Set_Class_Accessor_Get(clientSize, {
				Js_Self(Box);
				Js_Return( worker->types()->newInstance(self->client_size()) );
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

			Js_Set_Class_Accessor(direction, {
				Js_Self(Flex);
				Js_Return( worker->types()->newInstance(self->direction()) );
			}, {
				Js_Parse_Type(Direction, val, "Flex.direction = %s");
				Js_Self(Flex);
				self->set_direction(out);
			});

			Js_Set_Class_Accessor(itemsAlign, {
				Js_Self(Flex);
				Js_Return( worker->types()->newInstance(self->items_align()) );
			}, {
				Js_Parse_Type(ItemsAlign, val, "Flex.itemsAlign = %s");
				Js_Self(Flex);
				self->set_items_align(out);
			});

			Js_Set_Class_Accessor(crossAlign, {
				Js_Self(Flex);
				Js_Return( worker->types()->newInstance(self->cross_align()) );
			}, {
				Js_Parse_Type(CrossAlign, val, "Flex.crossAlign = %s");
				Js_Self(Flex);
				self->set_cross_align(out);
			});

			cls->exports("Flex", exports);
		}
	};

	class WrapFlow: public WrapViewObject {
	public:
		static void binding(JSObject* exports, Worker* worker) {
			Js_Define_Class(Flow, Flex, {
				Js_NewView(Flow);
			});

			Js_Set_Class_Accessor(wrap, {
				Js_Self(Flow);
				Js_Return( worker->types()->newInstance(self->wrap()) );
			}, {
				Js_Parse_Type2(Wrap, qk::Wrap, val, "Flow.wrap = %s");
				Js_Self(Flow);
				self->set_wrap(out);
			});

			Js_Set_Class_Accessor(wrapAlign, {
				Js_Self(Flow);
				Js_Return( worker->types()->newInstance(self->wrap_align()) );
			}, {
				Js_Parse_Type(WrapAlign, val, "Flow.wrapAlign = %s");
				Js_Self(Flow);
				self->set_wrap_align(out);
			});

			cls->exports("Flow", exports);
		}
	};

	class WrapFloat: public WrapViewObject {
	public:
		static void binding(JSObject* exports, Worker* worker) {
			Js_Define_Class(Float, Box, {
				Js_NewView(Float);
			});
			cls->exports("Float", exports);
		}
	};

	class WrapImage: public WrapViewObject {
	public:
		static void binding(JSObject* exports, Worker* worker) {
			Js_Define_Class(Image, Box, {
				Js_NewView(Image);
			});

			Js_Set_Class_Accessor(src, {
				Js_Self(Image);
				Js_Return( worker->types()->newInstance(self->src()) );
			}, {
				Js_Parse_Type(String, val, "Image.src = %s");
				Js_Self(Image);
				self->set_src(out);
			});

			// Qk_DEFINE_PROP_ACC(ImageSource*, source);

			cls->exports("Image", exports);
		}
	};

	void binding_box(JSObject* exports, Worker* worker) {
		WrapBox::binding(exports, worker);
		WrapFlex::binding(exports, worker);
		WrapFlow::binding(exports, worker);
		WrapFloat::binding(exports, worker);
		WrapImage::binding(exports, worker);
	}
} }
