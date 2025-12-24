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

#include "./canvas.h"

namespace qk {

	void Canvas::clipRect(const Rect& rect, ClipOp op, bool antiAlias) {
		clipPath(Path::MakeRect(rect), op, antiAlias);
	}

	void Canvas::drawPathvColor(const Pathv& path, const Color4f &color, BlendMode mode, bool antiAlias) {
		Paint paint;
		paint.fill.color = color;
		paint.blendMode = mode;
		paint.antiAlias = antiAlias;
		drawPathv(path, paint);
	}

	void Canvas::drawRect(const Rect& rect, const Paint& paint) {
		drawPath(Path::MakeRect(rect), paint);
	}

	void Canvas::drawRRect(const Rect& rect, const Path::BorderRadius &radius, const Paint& paint) {
		drawPath(Path::MakeRRect(rect, radius), paint);
	}

	void Canvas::drawOval(const Rect& oval, const Paint& paint) {
		drawPath(Path::MakeOval(oval), paint);
	}

	void Canvas::drawCircle(Vec2 center, float radius, const Paint& paint) {
		drawPath(Path::MakeCircle(center, radius), paint);
	}

	bool Canvas::isGpu() {
		return false;
	}

	void Canvas::setSurface(Vec2 surfaceSize, float scale) {
		auto size = surfaceSize / scale;
		auto root = Mat4::ortho(0, size.x(), 0, size.y(), -1.0f, 1.0f);
		setSurface(root, surfaceSize, scale);
	}

	// ---------------------------------------------------------------------

	float get_level_font_size(float fontSize) {
		if (fontSize <= 0) {
			return 0;
		}
		if (fontSize <= 4) {
			return 4;
		}
		if (fontSize <= 6) {
			return 6;
		}
		if (fontSize <= 8) {
			return 6;
		}
		if (fontSize <= 10) {
			return 10;
		}
		if (fontSize <= 12) {
			return 12;
		}
		if (fontSize <= 14) {
			return 14;
		}
		if (fontSize <= 16) {
			return 16;
		}
		if (fontSize <= 18) {
			return 18;
		}
		if (fontSize <= 20) {
			return 20;
		}
		if (fontSize <= 26) {
			return 26;
		}
		if (fontSize <= 32) {
			return 32;
		}
		if (fontSize <= 46) {
			return 64;
		}
		if (fontSize <= 128) {
			return 128;
		}
		return 256;
	}

}
