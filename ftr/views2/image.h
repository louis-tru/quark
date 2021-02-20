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

#ifndef __ftr__image__
#define __ftr__image__

#include "div.h"

/**
 * @ns ftr
 */

namespace ftr {

using value::Repeat;

/**
 * @class Image
 */
class FX_EXPORT Image: public Div {
	public:
	FX_DEFINE_GUI_VIEW(IMAGE, Image, image);
	
	Image();
	
	virtual ~Image();
	
	/**
	 * @fuc create
	 */
	static Image* create(cString& src);
	
	/**
	 * @func source_width
	 */
	uint32_t source_width() const;
	
	/**
	 * @func source_width
	 */
	uint32_t source_height() const;
	
	/**
	 * @func src 图像路径
	 */
	String src() const;
	
	/**
	 * @func set_src
	 */
	void set_src(cString& value);
	
	/**
	 * @func texture get 图像纹理数据
	 */
	inline Texture* texture() { return _texture; }
	
	/**
	 * @func set_texture
	 */
	virtual void set_texture(Texture* value);
	
	protected:
	/**
	 * @func source
	 */
	virtual String source() const;
	
	/**
	 * @func source
	 */
	virtual void set_source(cString& value);
	
	/**
	 * @overwrite
	 */
	virtual void draw(Draw* draw);
	virtual void set_layout_explicit_size();
	virtual void set_layout_content_offset();
	virtual void set_draw_visible();
	
	private:
	int       _tex_level;
	Texture*  _texture; // 图像纹理数据
	
	FX_DEFINE_INLINE_CLASS(Inl);
};

}
#endif
