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

#include "./scaner.h"

namespace flare {

	XLineScaner::XLineScaner(const PathLine& path, Rect clip, float scale): _activeEdges(nullptr) {
		Array<Vec2> edges;
		scale *= 6.0;
		edges = PathLine(path).scale(Vec2(scale)).to_edge_line();

		// clip paths
		if (clip.size.x() > 0 && clip.size.y() > 0) {
			if (scale != 1.0) {
				clip.origin *= Vec2(scale);
				clip.size *= Vec2(scale);
			}
			Vec2i clipOrigin = Vec2i(clip.origin.x(), clip.origin.y());
			Vec2i clipEnd = clipOrigin + Vec2i(clip.size.x(), clip.size.y());
			// TODO ..
		}
		
		for (auto edge: edges) {
			_edges.push({
				0,0,0,0,nullptr
			});
		}
	}

	bool XLineScaner::scan(ScanLine* line) {
		// TODO ...
		return true;
	}

}