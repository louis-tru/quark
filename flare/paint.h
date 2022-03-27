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

#ifndef __flare__paint__
#define __flare__paint__

#include "./value.h"
#include "./render/source.h"
#include "./util/handle.h"

F_NAMESPACE_START

class PaintBase;
class PaintColor;
class PaintImage;
class PaintGradient;
class PaintShadow;
class PaintBorder; // top,right,bottom,left

typedef PaintBase* Paint;

/**
* @class PaintBase, Single linked list struct
*/
class F_EXPORT PaintBase: public Reference {
public:
	
	enum Type {
		M_INVALID,
		M_IMAGE,
		M_GRADIENT,
		M_SHADOW,
	};

	enum HolderMode {
		M_INDEPENDENT,
		M_SHARED,
		M_DISABLE,
	};
	
	/**
	 * @constructor
	 */
	PaintBase();
	
	/**
	* @destructor
	*/
	virtual ~PaintBase();
	
	/**
	* @func next()
	*/
	inline Paint next() { return _next; }
	
	/**
	* @func set_next(value)
	*/
	Paint set_next(Paint value);
	
	/**
	* @func type()
	*/
	virtual Type type() const;
	
	/**
	* @func hold(left, right)
	* @ret return left value
	*/
	static Paint assign(Paint left, Paint right);
	
	/**
	* @func allow_multi_holder()
	*/
	inline HolderMode holder_mode() const { return _holder_mode; }
	
	/**
	* @func set_holder_mode(value)
	*/
	Paint set_holder_mode(HolderMode mode);
	
	/**
	 * @override
	 */
	virtual bool retain() override;

	/**
	* @func copy(to)
	*/
	virtual Paint copy(Paint to) = 0;

	/**
	 * 
	 * draw Paint background
	 *
	 * @func draw(host, canvas)
	 */
	// virtual void draw(Box* host, Canvas* canvas, uint8_t alpha, bool full) = 0;
	
protected:
	/**
	* @func mark()
	*/
	void mark();

	bool check_loop_reference(Paint value);
	void _Set_next(Paint value);
	static Paint _Assign(Paint left, Paint right);

	Paint        _next;
	HolderMode  _holder_mode;
};

/**
* @class PaintImage
*/
class F_EXPORT PaintImage: public PaintBase, public SourceHold {
public:
	PaintImage(cString& src = String());
	F_DEFINE_PROP(Repeat, repeat);
	F_DEFINE_PROP(FillPosition, position_x);
	F_DEFINE_PROP(FillPosition, position_y);
	F_DEFINE_PROP(FillSize, size_x);
	F_DEFINE_PROP(FillSize, size_y);
	virtual Type type() const override;
	virtual Paint copy(Paint to) override;
	// virtual void draw(Box* host, Canvas* canvas, uint8_t alpha, bool full) override;
private:
	bool  solve_size(FillSize size, float host, float& out);
	float solve_position(FillPosition pos, float host, float size);
};

/**
* @class PaintGradient
*/
class F_EXPORT PaintGradient: public PaintBase {
public:
	PaintGradient();
	virtual Type type() const override;
	virtual Paint copy(Paint to) override;
	// virtual void draw(Box* host, Canvas* canvas, uint8_t alpha, bool full) override;
private:
};

/**
 * @class PaintShadow
 */
class F_EXPORT PaintShadow: public PaintBase {
public:
	virtual Type type() const override;
	virtual Paint copy(Paint to) override;
	// virtual void draw(Box* host, Canvas* canvas, uint8_t alpha, bool full) override;
private:
};


F_NAMESPACE_END
#endif
