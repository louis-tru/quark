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


#ifndef __ftr__render__raster__
#define __ftr__render__raster__

#include "quark/util/util.h"
#include "quark/ui/types.h"
#include "quark/render/path.h"

namespace qk {

	class Qk_EXPORT XLineScaner: public Object {
		Qk_DISABLE_COPY(XLineScaner);
	public:
		typedef void (*ScanCb)(int32_t left, int32_t right, int32_t y, void* ctx);
		XLineScaner(const Path& path, Rect clip, float scale = 1.0, bool is_convex_polygon = false);
		void scan(ScanCb cb, void* ctx);
	private:
		void scan_polygon(ScanCb cb, void* ctx);
		void scan_convex_polygon(ScanCb cb, void* ctx);
		void check_new_edges(int y);
		void clip(Array<iVec2>& edges, Rect clip);
		struct Edge {
			int32_t min_y, max_y;
			int32_t x, incr_x;
			Edge* next;
		};
		Array<Edge>  _edges;
		Array<Edge*> _newEdges;
		Edge* _firstLineEdges;
		Edge _activeEdges;
		int32_t _start_y, _end_y;
		bool _is_convex_polygon;
	};

}
#endif
