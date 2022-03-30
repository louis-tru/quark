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

#ifndef __flare__effect__
#define __flare__effect__

#include "./value.h"
#include "./render/source.h"
#include "./util/handle.h"

F_NAMESPACE_START

/**
* @class Effect, Single linked list struct
*/
class F_EXPORT Effect: public Reference {
public:
	
	enum Type {
		M_INVALID,
		M_COLOR,
		M_IMAGE,
		M_GRADIENT,
		M_SHADOW,
		M_BLUR,
	};

	enum HolderMode {
		M_INDEPENDENT,
		M_SHARED,
		M_DISABLE,
	};

	/**
	 * @constructor
	 */
	Effect();
	
	/**
	* @destructor
	*/
	virtual ~Effect();

	/**
	* @func type()
	*/
	virtual Type type() const = 0;

	/**
	* @func hold(left, right)
	* @ret return left value
	*/
	static Effect* assign(Effect* left, Effect* right);

	/**
	* @func allow_multi_holder()
	*/
	inline HolderMode holder_mode() const { return _holder_mode; }

	/**
	* @func set_holder_mode(value)
	*/
	Effect* set_holder_mode(HolderMode mode);

	/**
	 * @override
	 */
	virtual bool retain() override;

	/**
	* @func copy(to)
	*/
	virtual Effect* copy(Effect* to) = 0;

	/**
	* @func set_next(value)
	*/
	Effect* set_next(Effect* value);

	/**
	* @func next()
	*/
	inline Effect* next() { return _next; }

protected:
	/**
	* @func mark()
	*/
	void mark();
	bool check_loop_reference(Effect* value);
	void _set_next(Effect* value);
	static Effect* _Assign(Effect* left, Effect* right);

	Effect*     _next;
	HolderMode  _holder_mode;
};

/**
 * @class BoxShadow
 */
class F_EXPORT BoxShadow: public Effect {
public:
	BoxShadow();
	BoxShadow(Shadow value);
	BoxShadow(float x, float y, float s, Color color);
	F_DEFINE_PROP(Shadow, value);
	virtual Type    type() const override;
	virtual Effect* copy(Effect* to) override;
};

/**
* @class Fill
*/
class F_EXPORT Fill: public Effect {
public:
	inline Fill* set_next(Fill* value) { Effect::set_next(value); return this; }
	inline Fill* next() { return static_cast<Fill*>(_next); }
};

/**
* @class FillColor
*/
class F_EXPORT FillColor: public Fill {
public:
	FillColor();
	FillColor(Color color);
	FillColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a);
	F_DEFINE_PROP(Color, value);
	virtual Type    type() const override;
	virtual Effect* copy(Effect* to) override;
};

/**
* @class FillImage
*/
class F_EXPORT FillImage: public Fill, public SourceHold {
public:
	FillImage();
	FillImage(cString& src);
	F_DEFINE_PROP(Repeat, repeat);
	F_DEFINE_PROP(FillPosition, position_x);
	F_DEFINE_PROP(FillPosition, position_y);
	F_DEFINE_PROP(FillSize, size_x);
	F_DEFINE_PROP(FillSize, size_y);
	virtual Type    type() const override;
	virtual Effect* copy(Effect* to) override;
	static bool  compute_size(FillSize size, float host, float& out);
	static float compute_position(FillPosition pos, float host, float size);
};

/**
* @class FillGradient
*/
class F_EXPORT FillGradient: public Fill {
public:
	FillGradient();
	virtual Type    type() const override;
	virtual Effect* copy(Effect* to) override;
};


F_NAMESPACE_END
#endif
