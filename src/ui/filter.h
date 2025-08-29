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

#include "./view_prop.h"
#include "../render/source.h"
#include "../util/handle.h"
#include "../util/array.h"

namespace qk {
	class View;
	class Window;

	/**
	* @class Box Filter, Single linked list struct
	*/
	class Qk_EXPORT BoxFilter: public Reference {
		Qk_HIDDEN_ALL_COPY(BoxFilter);
	public:
		enum Type {
			kImage,
			kGradientLinear, kGradientRadial,
			kShadow, kBlur, kBackdropBlur,
		};
		Qk_DEFINE_PROP_GET(Window*, window);
		Qk_DEFINE_PROP_GET_Atomic(View*, view);
		Qk_DEFINE_VIEW_PROPERTY_Atomic(BoxFilter*, next); // async set next

		BoxFilter();
		// virtual void release() override;
		virtual void destroy() override;
		virtual Type type() const = 0;
		virtual BoxFilter* copy(BoxFilter *dest, bool isRt) = 0; // @thread Rt
		virtual BoxFilter* transition(BoxFilter *dest, BoxFilter *to, float t, bool isRt) = 0; // @thread Rt
		static  BoxFilter* assign(BoxFilter *left, BoxFilter *right, View *view, bool isRt);

		template<class T>
		inline static T* Link(const std::initializer_list<T*>& list) {
			static_cast<T*>(link(*reinterpret_cast<std::initializer_list<BoxFilter*>*>(&list)));
		}
	protected:
		void set_next_no_check(BoxFilter *next, bool isRt);
	private:
		void set_view(View* value);
		static BoxFilter* link(const std::initializer_list<BoxFilter*>& list);
	};

	class Qk_EXPORT FillImage: public BoxFilter, public ImageSourceHold {
	public:
		struct Init {
			FillSize width, height;
			FillPosition x, y;
			Repeat repeat=Repeat::Repeat;
		};
		Qk_DEFINE_VIEW_ACCESSOR(String, src, Const);
		Qk_DEFINE_VIEW_PROPERTY(FillSize, width, Const);
		Qk_DEFINE_VIEW_PROPERTY(FillSize, height, Const);
		Qk_DEFINE_VIEW_PROPERTY(FillPosition, x, Const);
		Qk_DEFINE_VIEW_PROPERTY(FillPosition, y, Const);
		Qk_DEFINE_VIEW_PROPERTY(Repeat, repeat, Const);

		FillImage(cString& src, Init init = {.repeat=Repeat::Repeat});
		virtual Type type() const override;
		virtual BoxFilter* copy(BoxFilter* dest, bool isRt) override;
		virtual BoxFilter* transition(BoxFilter *to, BoxFilter* dest, float t, bool isRt) override;
		static bool compute_size(FillSize size, float host, float& out);
		static float compute_position(FillPosition pos, float host, float size);
	private:
		ImagePool* imgPool() override;
		void onSourceState(Event<ImageSource, ImageSource::State>& evt) override;
	};

	class Qk_EXPORT FillGradientRadial: public BoxFilter {
	public:
		FillGradientRadial(cArray<float>& pos, cArray<Color4f>& colors);
		inline cArray<float>& positions() const { return _pos; }
		inline cArray<Color4f>& colors() const { return _colors; }
		virtual Type type() const override;
		virtual BoxFilter* copy(BoxFilter* dest, bool isRt) override;
		virtual BoxFilter* transition(BoxFilter *to, BoxFilter* dest, float t, bool isRt) override;
	protected:
		void transition_g(BoxFilter* dest, BoxFilter *to, float t, bool isRt);
		Array<float> _pos;
		Array<Color4f> _colors;
	};

	class Qk_EXPORT FillGradientLinear: public FillGradientRadial {
	public:
		Qk_DEFINE_VIEW_PROPERTY(float, angle, Const);
		Qk_DEFINE_VIEW_PROP_GET(float, radian, Const);
		Qk_DEFINE_VIEW_PROP_GET(uint8_t, quadrant, Const);

		FillGradientLinear(cArray<float>& pos, cArray<Color4f>& colors, float angle/*0-360*/);
		virtual Type type() const override;
		virtual BoxFilter* copy(BoxFilter* dest, bool isRt) override;
		virtual BoxFilter* transition(BoxFilter *to, BoxFilter* dest, float t, bool isRt) override;
	private:
		void setRadian();
	};

	class Qk_EXPORT BoxShadow: public BoxFilter {
	public:
		Qk_DEFINE_VIEW_PROPERTY(Shadow, value, Const);
		BoxShadow(Shadow value);
		BoxShadow(float x, float y, float s, Color color);
		virtual Type type() const override;
		virtual BoxFilter* copy(BoxFilter* dest, bool isRt) override;
		virtual BoxFilter* transition(BoxFilter *to, BoxFilter* dest, float t, bool isRt) override;
	};
}

#endif
