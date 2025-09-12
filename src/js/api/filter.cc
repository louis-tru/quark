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
#include "../../ui/filter.h"

namespace qk { namespace js {

	struct MixBoxFilter: MixObject {
		typedef BoxFilter Type;
		static void binding(JSObject* exports, Worker* worker) {
			Js_Define_Class(BoxFilter, 0, {
				Js_Throw("Forbidden access abstract");
			});

			Js_Class_Accessor_Get(type, {
				Js_Return(self->type());
			});

			Js_MixObject_Accessor(BoxFilter, BoxFilterPtr, next, next);

			cls->exports("BoxFilter", exports);
		}
	};

	struct MixFillImage: MixObject {
		typedef FillImage Type;
		static void binding(JSObject* exports, Worker* worker) {
			Js_Define_Class(FillImage, BoxFilter, {
				if (!args.length() || !args[0]->isString()) {
					Js_Throw("@constructor FillImage(cString& src, Init init = {})");
				}
				auto src = args[0]->toString(worker)->value(worker);
				if (args.length() > 1) {
					Js_Parse_Type(FillImageInit, args[1], "@constructor FillImage(src,Init init = %s)");
					New<MixFillImage>(args, new FillImage(src, out));
				} else {
					New<MixFillImage>(args, new FillImage(src));
				}
			});

			Js_MixObject_Accessor(FillImage, String, src, src);
			// Qk_DEFINE_VIEW_ACCESSOR(ImageSource*, source);
			Js_MixObject_Accessor(FillImage, FillSize, width, width);
			Js_MixObject_Accessor(FillImage, FillSize, height, height);
			Js_MixObject_Accessor(FillImage, FillPosition, x, x);
			Js_MixObject_Accessor(FillImage, FillPosition, y, y);
			Js_MixObject_Accessor(FillImage, Repeat, repeat, repeat);

			cls->exports("FillImage", exports);
		}
	};

	struct MixFillGradientRadial: MixObject {
		typedef FillGradientRadial Type;

		static bool parse(
			FunctionArgs args, Array<float> *pos, Array<Color4f> *colors, cChar* msg, cChar* msg2
		) {
			Js_Worker(args);
			if (args.length() < 2) {
				Js_Throw("@constructor %s(cArray<float>& pos,cArray<Color4f>& colors%s)", msg, msg2), false;
			}
			Js_Parse_Type(ArrayFloat, args[0],
				*String::format("@constructor %s(cArray<float>& pos = %s", msg, "%s")
			) false;
			*pos = std::move(out);
			{
				Js_Parse_Type(ArrayColor, args[1],
					*String::format("@constructor %s(pos,cArray<Color4f>& colors = %s", msg, "%s")
				) false;
				colors->reset(out.length());
				for (int i = 0; i < out.length(); i++) {
					colors->at(i) = out[i].to_color4f();
				}
			}
			return true;
		}

		static void binding(JSObject* exports, Worker* worker) {
			Js_Define_Class(FillGradientRadial, BoxFilter, {
				ArrayFloat pos;
				Array<Color4f> colors;
				if (parse(args, &pos, &colors, "FillGradientRadial", "")) {
					New<MixFillGradientRadial>(args, new FillGradientRadial(pos, colors));
				}
			});

			Js_Class_Accessor_Get(positions, {
				Js_Return( worker->types()->jsvalue(self->positions()) );
			});

			Js_Class_Accessor_Get(colors, {
				Js_Return( worker->types()->jsvalue(self->colors()) );
			});
		
			cls->exports("FillGradientRadial", exports);
		}
	};

	struct MixFillGradientLinear: MixObject {
		typedef FillGradientLinear Type;
		static void binding(JSObject* exports, Worker* worker) {
			Js_Define_Class(FillGradientLinear, FillGradientRadial, {
				ArrayFloat pos;
				Array<Color4f> colors;
				if (MixFillGradientRadial::parse(args, &pos, &colors, "FillGradientLinear", ",float angle")) {
					float angle = 0;
					if (args.length() > 2) {
						Js_Parse_Type(float, args[2],
							"@constructor FillGradientLinear(pos,colors,float angle = %s)"
						);
						angle = out;
					}
					New<MixFillGradientLinear>(args, new FillGradientLinear(pos, colors, angle));
				}
			});

			Js_MixObject_Accessor(FillGradientLinear, float, angle, angle);

			cls->exports("FillGradientLinear", exports);
		}
	};

	struct MixBoxShadow: MixObject {
		typedef BoxShadow Type;
		static void NewBoxShadow(Worker *worker, FunctionArgs args) {
			if (!args.length()) {
				Js_Throw("@constructor BoxShadow(Shadow value)");
			}
			Js_Parse_Type(Shadow, args[0], "@constructor BoxShadow(Shadow value = %s)");
			New<MixBoxShadow>(args, new BoxShadow(out));
		}
		static void binding(JSObject* exports, Worker* worker) {
			Js_Define_Class(BoxShadow, BoxFilter, {
				NewBoxShadow(worker, args);
			});
			Js_MixObject_Accessor(BoxShadow, Shadow, value, value);

			cls->exports("BoxShadow", exports);
		}
	};

	void binding_filter(JSObject* exports, Worker* worker) {
		MixBoxFilter::binding(exports, worker);
		MixFillImage::binding(exports, worker);
		MixFillGradientRadial::binding(exports, worker);
		MixFillGradientLinear::binding(exports, worker);
		MixBoxShadow::binding(exports, worker);
	}
} }
