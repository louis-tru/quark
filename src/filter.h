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
#include "./render/source.h"
#include "./util/handle.h"
#include "./util/array.h"

namespace qk {

	/**
	* @class Filter, Single linked list struct
	*/
	class Qk_EXPORT Filter: public Reference {
		Qk_HIDDEN_ALL_COPY(Filter);
	public:
		enum Type {
			M_INVALID,
			M_IMAGE,
			M_GRADIENT_Linear,
			M_GRADIENT_Radial,
			M_SHADOW,
			M_BLUR,
			M_BACKDROP_BLUR,
		};
		enum HolderMode {
			M_INDEPENDENT,
			M_SHARED,
			M_DISABLE,
		};
		Filter();
		virtual ~Filter();
		virtual Type type() const = 0;
		virtual Filter* copy(Filter* to) = 0;
		virtual bool retain() override;
		static Filter* assign(Filter* left, Filter* right);
		Qk_DEFINE_PROP(HolderMode, holder_mode); // holder mode
	protected:
		static Filter* assign_no_check(Filter* left, Filter* right);
		void onChange();
		bool check_loop_reference(Filter* value);
		void set_next_check(Filter* value);
		void set_next_no_check(Filter* value);
		Filter* _next;
	};

	class Qk_EXPORT Fill: public Filter {
	public:
		Qk_DEFINE_PROP_ACC(Fill*, next);
	};

	class Qk_EXPORT FillImage: public Fill, public ImageSourceHolder {
	public:
		struct Init {
			String src;
			FillSize size_x, size_y;
			FillPosition position_x, position_y;
			Repeat repeat;
		};
		FillImage();
		FillImage(cString& src, Init init = {});
		Qk_DEFINE_PROP(FillSize, size_x);
		Qk_DEFINE_PROP(FillSize, size_y);
		Qk_DEFINE_PROP(FillPosition, position_x);
		Qk_DEFINE_PROP(FillPosition, position_y);
		Qk_DEFINE_PROP(Repeat, repeat);
		virtual Type    type() const override;
		virtual Filter* copy(Filter* to) override;
		static bool  compute_size(FillSize size, float host, float& out);
		static float compute_position(FillPosition pos, float host, float size);
	};

	class Qk_EXPORT FillGradient: public Fill {
	public:
		FillGradient(const Array<float>& pos, const Array<Color>& colors);
		virtual ~FillGradient();
		inline const Array<float>& positions() const { return _pos; }
		inline const Array<Color>& colors() const { return *reinterpret_cast<const Array<Color>*>(&_colors); }
		inline const Array<uint32_t>& colors_argb_uint32_t() const { return _colors; }
		void set_positions(const Array<float>& pos);
		void set_colors(const Array<Color>& colors);
	private:
		Array<float>    _pos;
		Array<uint32_t> _colors;
	};

	class Qk_EXPORT FillGradientLinear: public FillGradient {
	public:
		FillGradientLinear(float angle, const Array<float>& pos, const Array<Color>& colors);
		Qk_DEFINE_PROP(float, angle);
		virtual Type    type() const override;
		virtual Filter* copy(Filter* to) override;
	private:
		void setRadian();
		float   _radian;
		uint8_t _quadrant;
	};

	class Qk_EXPORT FillGradientRadial: public FillGradient {
	public:
		FillGradientRadial(const Array<float>& pos, const Array<Color>& colors);
		virtual Type    type() const override;
		virtual Filter* copy(Filter* to) override;
	};

	class Qk_EXPORT BoxShadow: public Filter {
	public:
		BoxShadow();
		BoxShadow(Shadow value);
		BoxShadow(float x, float y, float s, Color color);
		Qk_DEFINE_PROP(Shadow, value);
		Qk_DEFINE_PROP_ACC(BoxShadow*, next);
		virtual Type    type() const override;
		virtual Filter* copy(Filter* to) override;
	};

}
#endif
