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

#ifndef __quark__filter__
#define __quark__filter__

#include "./types.h"
#include "../render/source.h"
#include "../util/handle.h"
#include "../util/array.h"

namespace qk {
	class Layout;

	/**
	* @class Box Filter, Single linked list struct
	*/
	class Qk_EXPORT BoxFilter: public Object {
		Qk_HIDDEN_ALL_COPY(BoxFilter);
	public:
		enum Type {
			kImage,
			kGradientLinear,
			kGradientRadial,
			kShadow,
			kBlur,
			kBackdropBlur,
		};
		Qk_DEFINE_PROP(BoxFilter*, next, Protected);
		Qk_DEFINE_PROP(Layout*, host);
		BoxFilter();
		~BoxFilter();
		virtual Type type() const = 0;
		virtual BoxFilter* copy(BoxFilter* dest) = 0;
		virtual BoxFilter* transition(BoxFilter *to, float t, BoxFilter* dest) = 0;
		static  BoxFilter* assign(BoxFilter* left, BoxFilter* right, Layout *host);
	protected:
		void onChange();
		bool check_loop_reference(BoxFilter* value);
		void set_next_no_check(BoxFilter* value);
	};

	class Qk_EXPORT FillImage: public BoxFilter, public ImageSourceHolder {
	public:
		struct Init {
			FillSize size_x, size_y;
			FillPosition position_x, position_y;
			Repeat repeat;
		};
		Qk_DEFINE_PROP(FillSize, size_x, Const);
		Qk_DEFINE_PROP(FillSize, size_y, Const);
		Qk_DEFINE_PROP(FillPosition, position_x, Const);
		Qk_DEFINE_PROP(FillPosition, position_y, Const);
		Qk_DEFINE_PROP(Repeat, repeat, Const);
		FillImage();
		FillImage(cString& src, Init init = {});
		virtual Type type() const override;
		virtual BoxFilter* copy(BoxFilter* dest) override;
		virtual BoxFilter* transition(BoxFilter *to, float t, BoxFilter* dest) override;
		static bool compute_size(FillSize size, float host, float& out);
		static float compute_position(FillPosition pos, float host, float size);
	};

	class Qk_EXPORT FillGradient: public BoxFilter {
	public:
		FillGradient(cArray<float>& pos, cArray<Color4f>& colors);
		inline cArray<float>& positions() const { return _pos; }
		inline cArray<Color4f>& colors() const { return _colors; }
		void set_positions(cArray<float>& pos);
		void set_colors(cArray<Color4f>& colors);
		static Array<Color4f> to_color4fs(cArray<Color>& colors);
	protected:
		Array<float> _pos;
		Array<Color4f> _colors;
		bool transition_gradient(BoxFilter *to, float t, BoxFilter* dest);
	};

	class Qk_EXPORT FillGradientLinear: public FillGradient {
	public:
		FillGradientLinear(float angle, cArray<float>& pos, cArray<Color4f>& colors);
		Qk_DEFINE_PROP(float, angle, Const);
		Qk_DEFINE_PROP_GET(float, radian, Const);
		Qk_DEFINE_PROP_GET(uint8_t, quadrant, Const);
		virtual Type type() const override;
		virtual BoxFilter* copy(BoxFilter* dest) override;
		virtual BoxFilter* transition(BoxFilter *to, float t, BoxFilter* dest) override;
	private:
		void setRadian();
	};

	class Qk_EXPORT FillGradientRadial: public FillGradient {
	public:
		FillGradientRadial(cArray<float>& pos, cArray<Color4f>& colors);
		virtual Type type() const override;
		virtual BoxFilter* copy(BoxFilter* dest) override;
		virtual BoxFilter* transition(BoxFilter *to, float t, BoxFilter* dest) override;
	};

	// box shadow
	class Qk_EXPORT BoxShadow: public BoxFilter {
	public:
		Qk_DEFINE_PROP(Shadow, value, Const);
		BoxShadow();
		BoxShadow(Shadow value);
		BoxShadow(float x, float y, float s, Color color);
		virtual Type type() const override;
		virtual BoxFilter* copy(BoxFilter* dest) override;
		virtual BoxFilter* transition(BoxFilter *to, float t, BoxFilter* dest) override;
	};
}

#endif
