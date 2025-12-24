/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright (c) 2015, Louis.chu
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of Louis.chu nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL Louis.chu BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * ***** END LICENSE BLOCK ***** */

#include "./types.h"
#include "../../render/path.h"

namespace qk { namespace js {

	struct MixPath: MixObject {
		typedef Path Type;

		static void binding(JSObject* exports, Worker* worker) {
			Js_Define_Class(Path, 0, {
				if (args.length()) {
					Js_Parse_Type(Vec2, args[0], "Path(move?)");
					New<MixPath>(args, new Path(out));
				} else {
					New<MixPath>(args, new Path());
				}
			});
			Js_Class_Accessor_Get(ptsLen, {
				Js_Return( self->ptsLen() );
			});
			Js_Class_Accessor_Get(verbsLen, {
				Js_Return( self->verbsLen() );
			});
			Js_Class_Accessor_Get(isNormalized, {
				Js_Return( self->isNormalized() );
			});
			Js_Class_Accessor_Get(isSealed, {
				Js_Return( self->isSealed() );
			});
			Js_Class_Method(moveTo, {
				Js_Parse_Args(Vec2, 0, "path.moveTo(to)");
				self->moveTo(arg0);
			});
			Js_Class_Method(lineTo, {
				Js_Parse_Args(Vec2, 0, "path.lineTo(to)");
				self->lineTo(arg0);
			});
			Js_Class_Method(quadTo, {
				Js_Parse_Args(Vec2, 0, "path.quadTo(control,to)");
				Js_Parse_Args(Vec2, 1, "path.quadTo(control,to)");
				self->quadTo(arg0, arg1);
			});
			Js_Class_Method(cubicTo, {
				Js_Parse_Args(Vec2, 0, "path.cubicTo(control1,control2,to)");
				Js_Parse_Args(Vec2, 1, "path.cubicTo(control1,control2,to)");
				Js_Parse_Args(Vec2, 2, "path.cubicTo(control1,control2,to)");
				self->cubicTo(arg0, arg1, arg2);
			});
			Js_Class_Method(ovalTo, {
				Js_Parse_Args(Rect, 0, "path.ovalTo(rect,ccw?)");
				Js_Parse_Args(bool, 1, "path.ovalTo(rect,ccw?)", (false));
				self->ovalTo(arg0, arg1);
			});
			Js_Class_Method(rectTo, {
				Js_Parse_Args(Rect, 0, "path.rectTo(rect,ccw?)");
				Js_Parse_Args(bool, 1, "path.rectTo(rect,ccw?)",(false));
				self->rectTo(arg0, arg1);
			});
			Js_Class_Method(arcTo, {
				Js_Parse_Args(Rect, 0, "path.arcTo(rect,startAngle,sweepAngle,useCenter?)");
				Js_Parse_Args(float, 1, "path.arcTo(rect,startAngle,sweepAngle,useCenter?)");
				Js_Parse_Args(float, 2, "path.arcTo(rect,startAngle,sweepAngle,useCenter?)");
				Js_Parse_Args(bool, 3, "path.arcTo(rect,startAngle,sweepAngle,useCenter?)",(false));
				self->arcTo(arg0, arg1, arg2, arg3);
			});
			Js_Class_Method(arc, {
				Js_Parse_Args(Vec2, 0, "path.arc(center,radius,startAngle,sweepAngle,useCenter?)");
				Js_Parse_Args(Vec2, 1, "path.arc(center,radius,startAngle,sweepAngle,useCenter?)");
				Js_Parse_Args(float, 2, "path.arc(center,radius,startAngle,sweepAngle,useCenter?)");
				Js_Parse_Args(float, 3, "path.arc(center,radius,startAngle,sweepAngle,useCenter?)");
				Js_Parse_Args(bool, 4, "path.arc(center,radius,startAngle,sweepAngle,useCenter?)",(false));
				self->arc(arg0, arg1, arg2, arg3, arg4);
			});
			Js_Class_Method(close, {
				self->close();
			});
			Js_Class_Method(concat, {
				Js_Parse_Args(PathPtr, 0, "path.concat(path)");
				if (arg0)
					self->concat(*arg0);
			});
			Js_Class_Method(ptsAt, {
				Js_Parse_Args(uint32_t, 0, "path.ptsAt(index)");
				Js_Check(arg0 < self->ptsLen(), "path.ptsAt(index) index out of range");
				Js_Return( worker->types()->jsvalue(self->pts()[arg0]) );
			});
			Js_Class_Method(verbsAt, {
				Js_Parse_Args(uint32_t, 0, "path.verbsAt(index)");
				Js_Check(arg0 < self->verbsLen(), "path.verbsAt(index) index out of range");
				Js_Return( worker->types()->jsvalue(self->verbs()[arg0]) );
			});
			Js_Class_Method(getEdgeLines, {
				Js_Parse_Args(float, 0, "path.getEdgeLines(epsilon?)", (1.0f));
				auto pts = self->getEdgeLines(arg0);
				Js_Return( worker->types()->jsvalue(pts) );
			});
			Js_Class_Method(getTriangles, {
				Js_Parse_Args(float, 0, "path.getTriangles(epsilon?)", (1.0f));
				auto vertex = self->getTriangles(arg0).vertex;
				Js_Return( worker->types()->jsvalue(vertex) );
			});
			Js_Class_Method(getAAFuzzStrokeTriangle, {
				Js_Parse_Args(float, 0, "path.getAAFuzzStrokeTriangle(width,epsilon?)");
				Js_Parse_Args(float, 1, "path.getAAFuzzStrokeTriangle(width,epsilon?)", (1.0f));
				auto vertex = self->getAAFuzzStrokeTriangle(arg0, arg1).vertex;
				Js_Return( worker->types()->jsvalue(vertex) );
			});
			Js_Class_Method(dashPath, {
				Js_Parse_Args(ArrayFloat, 0, "path.dashPath(stage,offset?)");
				Js_Parse_Args(float, 1, "path.dashPath(stage,offset?)", (0));
				auto path = self->dashPath(arg0.val(), arg0.length(), arg1);
				Js_Return( worker->types()->jsvalue(new Path(std::move(path))) );
			});
			Js_Class_Method(strokePath, {
				Js_Parse_Args(float, 0, "path.strokePath(width,cap?,join?,miterLimit?)");
				Js_Parse_Args(int32_t, 1, "path.strokePath(width,cap?,join?,miterLimit?)", (Paint::kButt_Cap));
				Js_Parse_Args(int32_t, 2, "path.strokePath(width,cap?,join?,miterLimit?)", (Paint::kMiter_Join));
				Js_Parse_Args(float, 3, "path.strokePath(width,cap?,join?,miterLimit?)", (0.0f));
				auto path = self->strokePath(arg0, Path::Cap(arg1), Path::Join(arg2), arg3);
				Js_Return( worker->types()->jsvalue(new Path(std::move(path))) );
			});
			Js_Class_Method(normalizedPath, {
				Js_Parse_Args(float, 0, "epsilon = %s", (1.0f));
				self->normalizedPath(arg0);
				Js_Return( self );
			});
			Js_Class_Method(transform, {
				Js_Parse_Args(Mat, 0, "path.transform(matrix)");
				self->transform(arg0);
			});
			Js_Class_Method(scale, {
				Js_Parse_Args(Vec2, 0, "path.scale(scale)");
				self->scale(arg0);
			});
			Js_Class_Method(seal, {
				self->seal();
			});
			Js_Class_Method(getBounds, {
				if (args.length() == 0) {
					Js_Return( worker->types()->jsvalue(self->getBounds()) );
				} else {
					Js_Parse_Args(Mat, 0, "path.getBounds(matrix?)");
					Js_Return( worker->types()->jsvalue(self->getBounds(&arg0)) );
				}
			});
			Js_Class_Method(copy, {
				auto path = new Path(*self);
				Js_Return( worker->types()->jsvalue(path) );
			});
			Js_Class_Method(hashCode, {
				Js_Return( self->hashCode() & 0xffffffff );
			});
			cls->exports("Path", exports);
			// ------------------------------------------------------------------------------------
			// static method
			Js_Method(oval, {
				Js_Parse_Args(Rect, 0, "Path.oval(rect,ccw?)");
				Js_Parse_Args(bool, 1, "Path.oval(rect,ccw?)", (false));
				auto path = new Path();
				path->ovalTo(arg0, arg1);
				Js_Return( worker->types()->jsvalue(path) );
			});
			Js_Method(arc, {
				Js_Parse_Args(Rect, 0, "Path.arc(rect,startAngle,sweepAngle,useCenter?,close?)");
				Js_Parse_Args(float, 1, "Path.arc(rect,startAngle,sweepAngle,useCenter?,close?)");
				Js_Parse_Args(float, 2, "Path.arc(rect,startAngle,sweepAngle,useCenter?,close?)");
				Js_Parse_Args(bool, 3, "Path.arc(rect,startAngle,sweepAngle,useCenter?,close?)", (false));
				Js_Parse_Args(bool, 4, "Path.arc(rect,startAngle,sweepAngle,useCenter?,close?)", (true));
				auto path = new Path(Path::MakeArc(arg0, arg1, arg2, arg3, arg4));
				Js_Return( worker->types()->jsvalue(path) );
			});
			Js_Method(rect, {
				Js_Parse_Args(Rect, 0, "Path.rect(rect,ccw?)");
				Js_Parse_Args(bool, 1, "Path.rect(rect,ccw?)", (false));
				auto path = new Path();
				path->rectTo(arg0, arg1);
				Js_Return( worker->types()->jsvalue(path) );
			});
			Js_Method(circle, {
				Js_Parse_Args(Vec2, 0, "Path.circle(center,radius,ccw?)");
				Js_Parse_Args(float, 1, "Path.circle(center,radius,ccw?)");
				Js_Parse_Args(bool, 2, "Path.circle(center,radius,ccw?)", (false));
				auto path = new Path(Path::MakeCircle(arg0, arg1, arg2));
				Js_Return( worker->types()->jsvalue(path) );
			});
			Js_Method(rrect, {
				Js_Parse_Args(Rect, 0, "Path.rrect(rect,radius)");
				Js_Parse_Args(BorderRadius, 1, "Path.rrect(rect,radius)", ({}));
				auto path = new Path(Path::MakeRRect(arg0, arg1));
				Js_Return( worker->types()->jsvalue(path) );
			});
			Js_Method(rrectOutline, {
				Js_Parse_Args(Rect, 0, "Path.rrectOutline(outside,inside,radius)");
				Js_Parse_Args(Rect, 1, "Path.rrectOutline(outside,inside,radius)");
				Js_Parse_Args(BorderRadius, 2, "Path.rrectOutline(outside,inside,radius)", ({}));
				auto path = new Path(Path::MakeRRectOutline(arg0, arg1, arg2));
				Js_Return( worker->types()->jsvalue(path) );
			});
			Js_Method(getBoundsFromPoints, {
				Js_Parse_Args(ArrayVec2, 0, "Path.getBoundsFromPoints(pts,matrix?)");
				if (args.length() > 1) {
					Js_Parse_Args(Mat, 1, "Path.getBoundsFromPoints(pts,matrix?)");
					auto r = Path::getBoundsFromPoints(arg0.val(), arg0.length(), &arg1);
					Js_Return( worker->types()->jsvalue(r) );
				} else {
					auto r = Path::getBoundsFromPoints(arg0.val(), arg0.length());
					Js_Return( worker->types()->jsvalue(r) );
				}
			});
		}
	};

	Js_Module(_path, MixPath);
} }
