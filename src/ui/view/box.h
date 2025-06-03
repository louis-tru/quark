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

#ifndef __quark__view__box__
#define __quark__view__box__

#include "./view.h"
#include "../filter.h"

namespace qk {

	/**
		* @class Box
		*/
	class Qk_EXPORT Box: public View {
	public:
		// define props
		Qk_DEFINE_VIEW_PROPERTY(bool,       clip, Const); //!< is clip box display range
		Qk_DEFINE_VIEW_PROPERTY(Align,      align, ProtectedConst); //!< view align
		Qk_DEFINE_VIEW_ACCESSOR(BoxSize,    width, Const); //!< min width alias, if max width equal none then only use min width and not use limit width
		Qk_DEFINE_VIEW_ACCESSOR(BoxSize,    height, Const); //!< min height alias
		Qk_DEFINE_VIEW_PROPERTY(BoxSize,    min_width, Const); //!< limit min width if max width not equal none then limit min width
		Qk_DEFINE_VIEW_PROPERTY(BoxSize,    min_height, Const); //!< limit min height
		Qk_DEFINE_VIEW_PROPERTY(BoxSize,    max_width, Const); //!< limit max width if min width not equal none then limit max width
		Qk_DEFINE_VIEW_PROPERTY(BoxSize,    max_height, Const); //!< limit max width
		Qk_DEFINE_VIEW_ACCESSOR(ArrayFloat, margin, Const); //!< margin
		Qk_DEFINE_VIEW_PROPERTY(float,      margin_top, Const);
		Qk_DEFINE_VIEW_PROPERTY(float,      margin_right, Const);
		Qk_DEFINE_VIEW_PROPERTY(float,      margin_bottom, Const);
		Qk_DEFINE_VIEW_PROPERTY(float,      margin_left, Const);
		Qk_DEFINE_VIEW_ACCESSOR(ArrayFloat, padding, Const); //!< padding
		Qk_DEFINE_VIEW_PROPERTY(float,      padding_top, Const);
		Qk_DEFINE_VIEW_PROPERTY(float,      padding_right, Const);
		Qk_DEFINE_VIEW_PROPERTY(float,      padding_bottom, Const);
		Qk_DEFINE_VIEW_PROPERTY(float,      padding_left, Const);
		Qk_DEFINE_VIEW_ACCESSOR(ArrayFloat, border_radius, Const); //!< border_radius
		Qk_DEFINE_VIEW_PROPERTY(float,      border_radius_left_top, Const);
		Qk_DEFINE_VIEW_PROPERTY(float,      border_radius_right_top, Const);
		Qk_DEFINE_VIEW_PROPERTY(float,      border_radius_right_bottom, Const);
		Qk_DEFINE_VIEW_PROPERTY(float,      border_radius_left_bottom, Const);
		Qk_DEFINE_VIEW_ACCESSOR(ArrayBorder,border, Const); // border
		Qk_DEFINE_VIEW_ACCESSOR(BoxBorder,  border_top, Const);
		Qk_DEFINE_VIEW_ACCESSOR(BoxBorder,  border_right, Const);
		Qk_DEFINE_VIEW_ACCESSOR(BoxBorder,  border_bottom, Const);
		Qk_DEFINE_VIEW_ACCESSOR(BoxBorder,  border_left, Const);
		Qk_DEFINE_VIEW_ACCESSOR(ArrayFloat, border_width, Const);
		Qk_DEFINE_VIEW_ACCESSOR(ArrayColor, border_color, Const);
		Qk_DEFINE_VIEW_ACCESSOR(float,      border_width_top, Const); // border_width
		Qk_DEFINE_VIEW_ACCESSOR(float,      border_width_right, Const);
		Qk_DEFINE_VIEW_ACCESSOR(float,      border_width_bottom, Const);
		Qk_DEFINE_VIEW_ACCESSOR(float,      border_width_left, Const);
		Qk_DEFINE_VIEW_ACCESSOR(Color,      border_color_top, Const); // border_color
		Qk_DEFINE_VIEW_ACCESSOR(Color,      border_color_right, Const);
		Qk_DEFINE_VIEW_ACCESSOR(Color,      border_color_bottom, Const);
		Qk_DEFINE_VIEW_ACCESSOR(Color,      border_color_left, Const);
		Qk_DEFINE_VIEW_PROPERTY(Color,      background_color, Const); // fill background color
		Qk_DEFINE_VIEW_ACCESSOR(BoxFilter*, background); // fill background, image|gradient, async set
		Qk_DEFINE_VIEW_ACCESSOR(BoxShadow*, box_shadow); // box shadow, shadow, async set method
		Qk_DEFINE_VIEW_PROPERTY(Vec2,       weight, Const); // view weight
		Qk_DEFINE_VIEW_ACCE_GET(Vec2,       content_size, Const);
		Qk_DEFINE_VIEW_PROP_GET(Vec2,       client_size, Const); // border + padding + content
		Qk_DEFINE_VIEW_PROPERTY(Container,  container, ProtectedConst); // view container range

		Box();
		~Box();
		// --------------- o v e r w r i t e ---------------
		virtual void layout_forward(uint32_t mark) override;
		virtual void layout_reverse(uint32_t mark) override;
		virtual void layout_text(TextLines *lines, TextConfig *cfg) override;
		virtual Vec2 layout_offset() override;
		virtual Vec2 layout_size() override; // context + padding + border + margin
		virtual const Container& layout_container() override;
		/**
		 * @prop layout_weight
		* The scaling ratio of the project is defined here, which defaults to 0,
		* meaning that if there is remaining space, neither enlarge nor narrow will occur
		* in flex：size = size_raw + overflow * weight / weight_total * min(weight_total, 1)
		*/
		virtual Vec2 layout_weight() override;
		virtual Align layout_align() override;
		virtual bool is_clip() override;
		virtual ViewType viewType() const override;
		virtual Vec2 layout_offset_inside() override;
		virtual Vec2 layout_lock(Vec2 layout_size) override;
		virtual void set_layout_offset(Vec2 val) override;
		virtual void set_layout_offset_free(Vec2 size) override;
		virtual void solve_marks(const Mat &mat, uint32_t mark) override;
		virtual void solve_visible_region(const Mat &mat) override; // compute visible region
		virtual bool overlap_test(Vec2 point) override;
		virtual Vec2 center() override;
		virtual void draw(UIDraw *render) override;

		/**
			* client rect = border + padding + content
			* @method solve_rect_vertex(mat, vertex)
			* @safe Rt
			* @note Can only be used in rendering threads
			*/
		virtual void solve_rect_vertex(const Mat &mat, Vec2 vertexOut[4]); // compute rect vertex

	protected:
		/**
			* @method set_content_size(content_size)
			* @safe Rt
			* @note Can only be used in rendering threads
			*/
		void set_content_size(Vec2 content_size);

		/**
		 * @method solve_layout_content_width_pre()
		 * @Returns range
		 * @safe Rt
		 * @note Can only be used in rendering threads
		 */
		Container::Pre solve_layout_content_width_pre(const Container &pContainer);

		/**
		 * @method solve_layout_content_height_pre()
		 * @Returns range
		 * @safe Rt
		 * @note Can only be used in rendering threads
		 */
		Container::Pre solve_layout_content_height_pre(const Container &pContainer);

		/**
		 * @method solve_layout_content_size_pre()
		 * @safe Rt
		 * @note Can only be used in rendering threads
		 */
		virtual uint32_t solve_layout_content_size_pre(uint32_t &mark, View *parent);

		/**
		 * @method layout_typesetting_float
		 * @safe Rt
		*/
		Vec2 layout_typesetting_float();

		// ----------------------- define private props -----------------------
	private:
		std::atomic<BoxFilter*> _background;
		std::atomic<BoxShadow*> _boxShadow;
	protected:
		struct BoxBorderInl { // box border value
			float width[4];
			Color color[4]; // top/right/bottom/left
		};
		std::atomic<BoxBorderInl*> _border; // BoxBorder, top/right/bottom/left
		// box view attrs
		Vec2  _layout_offset; // The starting offset relative to the parent view（include margin）
		Vec2  _layout_size; // Size occupied by the layout（margin+border+padding+content）
		Vec2  _vertex[4]; // box vertex

		friend class UIDraw;
	};

	/**
	* @method overlap_test_from_convex_quadrilateral
	*/
	Qk_EXPORT bool overlap_test_from_convex_quadrilateral(Vec2 quadrilateral_vertex[4], Vec2 point);
	
	/**
	 * @method screen_region_from_convex_quadrilateral
	*/
	Qk_EXPORT Region screen_region_from_convex_quadrilateral(Vec2 quadrilateral_vertex[4]);

}
#endif
