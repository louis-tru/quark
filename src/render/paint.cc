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

#include "./paint.h"

namespace quark {

	void Paint::setImage(ImageSource* _image, const Rect& dest, const Rect& src) {
		type = kImage_Type;
		image = _image;
		Vec2 scale(dest.size.x() / src.size.x(), dest.size.y() / src.size.y());
		*reinterpret_cast<Rect*>(&color) = {
			Vec2(
				src.origin.x() * scale.x() - dest.origin.x(),
				src.origin.y() * scale.y() - dest.origin.y()
			),
			// shader scale
			Vec2(scale.x() / float(_image->width()), scale.y() / float(_image->height())),
		};
	}

	void Paint::setImage(ImageSource* image, const Rect& dest) {
		setImage(image, dest, { Vec2(0,0), Vec2(image->width(), image->height()) });
	}

	void Paint::setGradient(GradientPaint* gradient) {
		type = kGradient_Type;
		image = reinterpret_cast<ImageSource*>(gradient);
	}

}
