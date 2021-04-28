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

#ifndef __ftr__view__box__
#define __ftr__view__box__

#include "./view.h"
#include "../background.h"

namespace ftr {

	/**
	 * @class Box
	 */
	class FX_EXPORT Box: public View {
		FX_Define_View(Box);
		public:

		/**
		 * @func width()
		 */
		inline float width() const {
			return _size.width();
		}
		
		/**
		 * @func height()
		 */
		inline float height() const {
			return _size.height();
		}
		
		/**
		 * @func background_color()
		 */
		inline Color background_color() const {
			return _background_color;
		}

		/**
		 * @func background()
		 */
		inline Background* background() {
			return _background;
		}

		private:
		// box attrs
		Vec2 _size; // width,height
		Vec4 _margin, _padding; // top,right,bottom,left
		float _border[4];
		float _border_radius[4]; // left-top,right-top,right-bottom,left-bottom
		Color _background_color; // background color
		Background *_background; // background, box-shadow
		// int _features;
		// LayoutAlign _layout_align_x, _layout_align_y; // left|center|right,top|center|bottom
	};

}

#endif