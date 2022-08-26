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

#ifndef __noug__effect__
#define __noug__effect__

#include "./value.h"
#include "./render/source.h"
#include "./util/handle.h"
#include "./util/array.h"

namespace noug {

	class SkiaRender;

	/**
	* @class Copying, Single linked list struct
	*/
	class N_EXPORT Copying: public Reference {
		N_HIDDEN_ALL_COPY(Copying);
	public:
		enum Type {
			M_INVALID,
			M_IMAGE,
			M_GRADIENT_Linear,
			M_GRADIENT_Radial,
			M_SHADOW,
			M_BLUR,
		};
		enum HolderMode {
			M_INDEPENDENT,
			M_SHARED,
			M_DISABLE,
		};
		Copying();
		virtual ~Copying();
		virtual Type type() const = 0;
		virtual Copying* copy(Copying* to) = 0;
		virtual bool retain() override;
		static Copying* assign(Copying* left, Copying* right);
		N_Define_Prop(HolderMode, holder_mode); // holder mode
	protected:
		static Copying* assign2(Copying* left, Copying* right);
		void onChange();
		bool check_loop_reference(Copying* value);
		void set_next2(Copying* value);
		N_Define_Prop(Copying*, next);
	};

	class Effect: public Copying {
	public:
		inline Effect* next() { return static_cast<Effect*>(Copying::next()); }
		inline Effect* set_next(Effect* value) { Copying::set_next(value); return this; }
	};

	class Fill: public Copying {
	public:
		inline Fill* next() { return static_cast<Fill*>(Copying::next()); }
		inline Fill* set_next(Fill* value) { Copying::set_next(value); return this; }
	};

	class N_EXPORT FillImage: public Fill, public SourceHold {
	public:
		struct Init {
			String src;
			FillSize size_x, size_y;
			FillPosition position_x, position_y;
			Repeat repeat;
		};
		FillImage();
		FillImage(cString& src, Init init = {});
		N_Define_Prop(FillSize, size_x);
		N_Define_Prop(FillSize, size_y);
		N_Define_Prop(FillPosition, position_x);
		N_Define_Prop(FillPosition, position_y);
		N_Define_Prop(Repeat, repeat);
		virtual Type     type() const override;
		virtual Copying* copy(Copying* to) override;
		static bool  compute_size(FillSize size, float host, float& out);
		static float compute_position(FillPosition pos, float host, float size);
	};

	class FillGradient: public Fill {
	public:
		FillGradient(const Array<float>& pos, const Array<Color>& colors);
		virtual ~FillGradient();
		inline uint32_t count() const { return _count; }
		inline const Array<float>& positions() const { return _pos; }
		inline const Array<Color>& colors() const { return *reinterpret_cast<const Array<Color>*>(&_colors); }
		inline const Array<uint32_t>& colors_argb_uint32_t() const { return _colors; }
		void set_positions(const Array<float>& pos);
		void set_colors(const Array<Color>& colors);
	private:
		Array<float> _pos;
		Array<uint32_t> _colors;
		uint32_t _count;
	};

	class N_EXPORT FillGradientLinear: public FillGradient {
	public:
		FillGradientLinear(float angle, const Array<float>& pos, const Array<Color>& colors);
		N_Define_Prop(float, angle);
		virtual Type     type() const override;
		virtual Copying* copy(Copying* to) override;
	private:
		void setRadian();
		float _radian;
		uint8_t _quadrant;
		friend class SkiaRender;
	};

	class N_EXPORT FillGradientRadial: public FillGradient {
	public:
		FillGradientRadial(const Array<float>& pos, const Array<Color>& colors);
		virtual Type     type() const override;
		virtual Copying* copy(Copying* to) override;
	};

	class N_EXPORT BoxShadow: public Effect {
	public:
		BoxShadow();
		BoxShadow(Shadow value);
		BoxShadow(float x, float y, float s, Color color);
		N_Define_Prop(Shadow, value);
		virtual Type     type() const override;
		virtual Copying* copy(Copying* to) override;
	};

}
#endif
