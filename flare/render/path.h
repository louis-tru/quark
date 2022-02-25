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

#ifndef __flare__render__path__
#define __flare__render__path__

#include "../value.h"
#include "../util/array.h"

namespace flare {

	class F_EXPORT PathLine: public Array<Vec2> {
		public:
			enum PathVerb: uint8_t {
				kVerb_Move = 0, // start
				kVerb_Line,  // straight line
				kVerb_Quad,  // quadrilateral
				kVerb_Conic, // quadratic bezier
				kVerb_Cubic, // three times bezier
				kVerb_Close, // close
			};
			static PathLine Oval(Rect r, Vec2 offset);
			static PathLine Rect(Rect r, Vec2 offset);
			static PathLine Circle(float r, Vec2 offset);
			PathLine(const Vec2 pts[], uint32_t len, const PathVerb verbs[], uint32_t verbsLen);
			inline const Vec2* pts() const { return _val; }
			inline Array<PathVerb>& verbs_arr() { return _verbs; }
			inline const PathVerb* verbs() const { return *_verbs; }
			inline const uint32_t verbs_len() const { return _verbs.length(); }
			Array<Vec2>  to_edge_line() const;
			Array<Vec2i> to_edge_line_i(float scale) const;
			Array<Vec2>  to_polygon(int polySize) const;
			Array<Vec2i> to_polygon_i(int polySize, float scale) const;
		private:
			Array<PathVerb> _verbs;
	};
}

#endif
