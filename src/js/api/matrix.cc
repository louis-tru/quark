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
#include "../../ui/view/matrix.h"
#include "../../ui/view/root.h"

namespace qk { namespace js {

	void inheritMatrixView(JSClass* cls, Worker* worker) {
		Js_UIObject_Accessor(MatrixView, Vec2, translate, translate);
		Js_UIObject_Accessor(MatrixView, Vec2, scale, scale);
		Js_UIObject_Accessor(MatrixView, Vec2, skew, skew);
		Js_UIObject_Accessor(MatrixView, ArrayOrigin, origin, origin);
		Js_UIObject_Accessor(MatrixView, BoxOrigin, origin_x, originX);
		Js_UIObject_Accessor(MatrixView, BoxOrigin, origin_y, originY);
		Js_UIObject_Accessor(MatrixView, float, x, x);
		Js_UIObject_Accessor(MatrixView, float, y, y);
		Js_UIObject_Accessor(MatrixView, float, scale_x, scaleX);
		Js_UIObject_Accessor(MatrixView, float, scale_y, scaleY);
		Js_UIObject_Accessor(MatrixView, float, skew_x, skewX);
		Js_UIObject_Accessor(MatrixView, float, skew_y, skewY);
		Js_UIObject_Accessor(MatrixView, float, rotate_z, rotateZ);

		Js_Class_Accessor_Get(originValue, {
			Js_UISelf(MatrixView);
			Js_Return( worker->types()->jsvalue(self->origin_value()) );
		});

		Js_Class_Accessor_Get(matrix, {
			Js_UISelf(MatrixView);
			Js_Return( worker->types()->jsvalue(self->matrix()) );
		});
	};

	class MixMatrix: public MixViewObject {
	public:
		static void binding(JSObject* exports, Worker* worker) {
			Js_Define_Class(Matrix, Box, {
				Js_NewView(Matrix);
			});
			inheritMatrixView(cls, worker);

			cls->exports("Matrix", exports);
		}
	};

	class MixRoot: public MixViewObject {
	public:
		static void binding(JSObject* exports, Worker* worker) {
#if 0
			Js_Define_Class(Root, Matrix, {
				Js_Throw("Forbidden access");
			});
#else
			static_assert(sizeof(MixObject)==sizeof(MixRoot), "Derived mix class pairs cannot declare data members");
			auto cls = worker->newClass("Root",(typeid(Root).hash_code()),([](auto args){
				auto worker=args.worker();
				return worker->throwError(worker->newError(("Forbidden access")));
			}),
			([](auto o){
				new(o) MixRoot();
			}), (typeid(Matrix).hash_code()));
#endif
			cls->exports("Root", exports);
		}
	};

	void binding_matrix(JSObject* exports, Worker* worker) {
		MixMatrix::binding(exports, worker);
		MixRoot::binding(exports, worker);
	}
} }
