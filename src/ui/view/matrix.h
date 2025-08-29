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

#ifndef __quark__view__matrix__
#define __quark__view__matrix__

#include "./box.h"

namespace qk {

	class Qk_EXPORT MatrixView {
	public:
		Qk_DEFINE_VIEW_PROPERTY(Vec2,  translate, Const); // matrix displacement for the view
		Qk_DEFINE_VIEW_PROPERTY(Vec2,  scale, Const); // Matrix scaling
		Qk_DEFINE_VIEW_PROPERTY(Vec2,  skew, Const); // Matrix skew, (radian)
		Qk_DEFINE_VIEW_PROPERTY(float, rotate_z, Const); // z-axis rotation of the matrix
		// Start the matrix transform from this origin point start
		// with border as the starting point.
		// The origin_value is the final value by computing the origin.
		Qk_DEFINE_VIEW_PROP_GET(Vec2,  origin_value, Const);
		Qk_DEFINE_VIEW_PROPERTY(BoxOrigin, origin_x, Const); // x-axis transform box origin
		Qk_DEFINE_VIEW_PROPERTY(BoxOrigin, origin_y, Const); // y-axis transform box origin
		Qk_DEFINE_VIEW_ACCESSOR(ArrayOrigin, origin, Const); // x-axis and y-axis
		Qk_DEFINE_VIEW_ACCESSOR(float, x, Const); // x-axis matrix displacement for the view
		Qk_DEFINE_VIEW_ACCESSOR(float, y, Const); // y-axis matrix displacement for the view
		Qk_DEFINE_VIEW_ACCESSOR(float, scale_x, Const); // x-axis matrix scaling for the view
		Qk_DEFINE_VIEW_ACCESSOR(float, scale_y, Const); // y-axis matrix scaling for the view
		Qk_DEFINE_VIEW_ACCESSOR(float, skew_x, Const); // x-axis matrix skew for the view
		Qk_DEFINE_VIEW_ACCESSOR(float, skew_y, Const); // y-axis matrix skew for the view
		// The host view of the matrix
		Qk_DEFINE_VIEW_PROP_GET(View*, host);

		// Constructor
		MatrixView(View* host);

		/**
		 * Returns the final matrix of the view, parent transform * Mat
		 * @return {Mat}
		*/
		const Mat& matrix() const { return _matrix; }

	protected:
		void solve_origin_value(); // compute origin value

		Mat _matrix; // parent transform View * Mat(translate, scale, skew, rotate_z);
		// friend classes
		friend class Matrix;
		friend class SpriteView;
		friend class UIDraw;
	};

	/**
		* Box matrix transform
		*
	 * @class Matrix
		*/
	class Matrix: public Box, public MatrixView {
	public:
		Matrix();
		virtual ViewType viewType() const override;
		virtual MatrixView* asMatrixView() override;
		virtual void layout_reverse(uint32_t mark) override;
		virtual Vec2 layout_offset_inside() override;
		virtual void solve_marks(const Mat &mat, View *parent, uint32_t mark) override;
		virtual void solve_rect_vertex(const Mat &mat, Vec2 vertexOut[4]) override; // compute rect vertex
		virtual void draw(UIDraw *render) override;
	};
}
#endif
