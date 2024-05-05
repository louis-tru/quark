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
#include "../../ui/view/transform.h"
#include "../../ui/view/root.h"

namespace qk { namespace js {

	class WrapTransform: public WrapViewObject {
	public:
		static void binding(JSObject* exports, Worker* worker) {
			Js_Define_Class(Transform, Box, {
				Js_NewView(Transform);
			});

			Js_Set_Class_Accessor(translate, {
				Js_Self(Transform);
				Js_Return( worker->types()->newInstance(self->translate()) );
			}, {
				Js_Parse_Type(Vec2, val, "Transform.translate = %s");
				Js_Self(Transform);
				self->set_translate(out);
			});

			Js_Set_Class_Accessor(scale, {
				Js_Self(Transform);
				Js_Return( worker->types()->newInstance(self->scale()) );
			}, {
				Js_Parse_Type(Vec2, val, "Transform.scale = %s");
				Js_Self(Transform);
				self->set_scale(out);
			});

			Js_Set_Class_Accessor(skew, {
				Js_Self(Transform);
				Js_Return( worker->types()->newInstance(self->skew()) );
			}, {
				Js_Parse_Type(Vec2, val, "Transform.skew = %s");
				Js_Self(Transform);
				self->set_skew(out);
			});

			Js_Set_Class_Accessor(rotateZ, {
				Js_Self(Transform);
				Js_Return( worker->types()->newInstance(self->rotate_z()) );
			}, {
				Js_Parse_Type(float, val, "Transform.rotateZ = %s");
				Js_Self(Transform);
				self->set_rotate_z(out);
			});

			Js_Set_Class_Accessor(originX, {
				Js_Self(Transform);
				Js_Return( worker->types()->newInstance(self->origin_x()) );
			}, {
				Js_Parse_Type(BoxOrigin, val, "Transform.originX = %s");
				Js_Self(Transform);
				self->set_origin_x(out);
			});

			Js_Set_Class_Accessor(originY, {
				Js_Self(Transform);
				Js_Return( worker->types()->newInstance(self->origin_y()) );
			}, {
				Js_Parse_Type(BoxOrigin, val, "Transform.originY = %s");
				Js_Self(Transform);
				self->set_origin_y(out);
			});

			Js_Set_Class_Accessor_Get(originValue, {
				Js_Self(Transform);
				Js_Return( worker->types()->newInstance(self->origin_value()) );
			});

			Js_Set_Class_Accessor(x, {
				Js_Self(Transform);
				Js_Return( worker->types()->newInstance(self->x()) );
			}, {
				Js_Parse_Type(float, val, "Transform.x = %s");
				Js_Self(Transform);
				self->set_x(out);
			});

			Js_Set_Class_Accessor(y, {
				Js_Self(Transform);
				Js_Return( worker->types()->newInstance(self->y()) );
			}, {
				Js_Parse_Type(float, val, "Transform.y = %s");
				Js_Self(Transform);
				self->set_y(out);
			});

			Js_Set_Class_Accessor(scaleX, {
				Js_Self(Transform);
				Js_Return( worker->types()->newInstance(self->scale_x()) );
			}, {
				Js_Parse_Type(float, val, "Transform.scaleX = %s");
				Js_Self(Transform);
				self->set_scale_x(out);
			});

			Js_Set_Class_Accessor(scaleY, {
				Js_Self(Transform);
				Js_Return( worker->types()->newInstance(self->scale_y()) );
			}, {
				Js_Parse_Type(float, val, "Transform.scaleY = %s");
				Js_Self(Transform);
				self->set_scale_y(out);
			});

			Js_Set_Class_Accessor(skewX, {
				Js_Self(Transform);
				Js_Return( worker->types()->newInstance(self->skew_x()) );
			}, {
				Js_Parse_Type(float, val, "Transform.skewX = %s");
				Js_Self(Transform);
				self->set_skew_x(out);
			});

			Js_Set_Class_Accessor(skewY, {
				Js_Self(Transform);
				Js_Return( worker->types()->newInstance(self->skew_y()) );
			}, {
				Js_Parse_Type(float, val, "Transform.skewY = %s");
				Js_Self(Transform);
				self->set_skew_y(out);
			});

			Js_Set_Class_Accessor_Get(matrix, {
				Js_Self(Transform);
				Js_Return( worker->types()->newInstance(self->matrix()) );
			});

			cls->exports("Transform", exports);
		}
	};

	class WrapRoot: public WrapViewObject {
	public:
		static void binding(JSObject* exports, Worker* worker) {
			Js_Define_Class(Root, Transform, {
				Js_NewView(Root);
			});
			cls->exports("Root", exports);
		}
	};

	void binding_transform(JSObject* exports, Worker* worker) {
		WrapTransform::binding(exports, worker);
		WrapRoot::binding(exports, worker);
	}
} }
