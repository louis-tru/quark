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

#include "./path.h"
#include "tesselator.h"
#include "../util/handle.h"

namespace flare {

	PathLine PathLine::Oval(Rect r, Vec2 offset) {
		//return PathLine(nullptr, 0, nullptr, 0);
	}

	PathLine PathLine::Rect(Rect r, Vec2 offset) {
		//return PathLine(nullptr, 0, nullptr, 0);
	}

	PathLine PathLine::Circle(float r, Vec2 offset) {
		//return PathLine(nullptr, 0, nullptr, 0);
	}

	PathLine::PathLine(const Vec2* pts, uint32_t len, const PathVerb *verbs, uint32_t verbsLen) {
		for (int i = 0; i < len; i++)
			push(pts[i]);
		for (int i = 0; i < verbsLen; i++)
			_verbs.push(verbs[i]);
	}

	Array<Vec2>  PathLine::to_edge_line() const {

	}

	Array<Vec2i> PathLine::to_edge_line_i(float scale) const {

	}

	Array<Vec2>  PathLine::to_polygon(int polySize) const {
		TESStesselator* tess = tessNewTess(nullptr);
		ClearScope clear([tess]() { tessDeleteTess(tess); });

		Array<Vec2> vertex, tmpV;
		int polyVertex = 0;

		for (auto verb: _verbs) {
			switch(verb) {
				case kVerb_Move:
					if (polyVertex) {
						tessAddContour(tess, 2, tmpV.val()[tmpV.length() - polyVertex].val, sizeof(Vec2), polyVertex);
						polyVertex = 0;
					}
					break;
				case kVerb_Line:
					// tmpV.push();
					break;
				case kVerb_Quad:
					break;
				case kVerb_Conic:
					break;
				case kVerb_Cubic:
					break;
				default: // close
					break;
			}
		}

		if (polyVertex) { // closure
			// tessAddContour(tess, 2, tmpV.val()[tmpV.length() - polyVertex].val, sizeof(Vec2), polyVertex);
			// polyVertex = 0;
		}

		return vertex;
	}

	Array<Vec2i> PathLine::to_polygon_i(int polySize, float scale) const {
		TESStesselator* tess = tessNewTess(nullptr);
		ClearScope clear([tess]() { tessDeleteTess(tess); });
	}

}