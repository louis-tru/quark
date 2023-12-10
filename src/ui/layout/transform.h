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

#ifndef __quark__layout__transform__
#define __quark__layout__transform__

#include "./box.h"

namespace qk {

	/**
		* Box matrix transform
		*
	 * @class Transform
		*/
	class TransformLayout: public BoxLayout {
	public:
		TransformLayout(Window *win);
		// define props
		Qk_DEFINE_PROP(Vec2, translate); // matrix displacement for the view
		Qk_DEFINE_PROP(Vec2, scale); // Matrix scaling
		Qk_DEFINE_PROP(Vec2, skew); // Matrix skew, (radian)
		Qk_DEFINE_PROP(float, rotate); // z-axis rotation of the matrix
		Qk_DEFINE_PROP(BoxOrigin,  origin_x); //  x-axis transform origin
		Qk_DEFINE_PROP(BoxOrigin,  origin_y); //  y-axis transform origin
		// Start the matrix transform from this origin point start.
		// with border as the starting point.
		Qk_DEFINE_PROP_GET(Vec2, origin_value);
		Qk_DEFINE_PROP_ACC(float, x); // x-axis matrix displacement for the view
		Qk_DEFINE_PROP_ACC(float, y); // y-axis matrix displacement for the view
		Qk_DEFINE_PROP_ACC(float, scale_x); // x-axis matrix scaling for the view
		Qk_DEFINE_PROP_ACC(float, scale_y); // y-axis matrix scaling for the view
		Qk_DEFINE_PROP_ACC(float, skew_x); // x-axis matrix skew for the view
		Qk_DEFINE_PROP_ACC(float, skew_y); // y-axis matrix skew for the view
		// --------------- o v e r w r i t e ---------------
		virtual bool layout_forward(uint32_t mark) override;
		virtual bool layout_reverse(uint32_t mark) override;
		virtual Vec2 layout_offset_inside() override;
		virtual Mat  layout_matrix() override;
		virtual Vec2 position() override;
		virtual void solve_rect_vertex(Vec2 vertexOut[4]) override; // compute rect vertex
		virtual void draw(UIRender *render) override;
	private:
		void solve_origin_value(); // compute origint value
		friend class UIRender;
	};

	class Qk_EXPORT Transform: public Box {
	public:
		Qk_Define_View(Transform, Box);
		Qk_DEFINE_PROP_ACC(Vec2, translate); // matrix displacement for the view
		Qk_DEFINE_PROP_ACC(Vec2, scale); // Matrix scaling
		Qk_DEFINE_PROP_ACC(Vec2, skew); // Matrix skew, (radian)
		Qk_DEFINE_PROP_ACC(float, rotate); // z-axis rotation of the matrix
		Qk_DEFINE_PROP_ACC(BoxOrigin,  origin_x); //  x-axis transform origin
		Qk_DEFINE_PROP_ACC(BoxOrigin,  origin_y); //  y-axis transform origin
		Qk_DEFINE_PROP_ACC_GET(Vec2, origin_value);
		Qk_DEFINE_PROP_ACC(float, x); // x-axis matrix displacement for the view
		Qk_DEFINE_PROP_ACC(float, y); // y-axis matrix displacement for the view
		Qk_DEFINE_PROP_ACC(float, scale_x); // x-axis matrix scaling for the view
		Qk_DEFINE_PROP_ACC(float, scale_y); // y-axis matrix scaling for the view
		Qk_DEFINE_PROP_ACC(float, skew_x); // x-axis matrix skew for the view
		Qk_DEFINE_PROP_ACC(float, skew_y); // y-axis matrix skew for the view
	};

}
#endif
