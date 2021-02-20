/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright © 2015-2016, xuewen.chu
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

#include "./draw.h"
#include "./font/pool.h"
#include "./texture.h"
#include "./display-port.h"

namespace ftr {

static Char empty_[4] = { 0, 0, 0, 0 };
static cPixelData empty_pixel_data(WeakBuffer(empty_, 4), 1, 1, PixelData::RGBA8888);

/**
 * @class TextureEmpty
 */
class TextureEmpty: public Texture {
 public:
	virtual void load() {
		if (_status == TEXTURE_NO_LOADED) {
			ASSERT(load_data(empty_pixel_data), "Load temp texture error");
		}
	}
};

Draw* Draw::_draw_ctx = nullptr; // 当前GL上下文

/**
 * @constructor
 */
Draw::Draw(GUIApplication* host, cJSON& options)
: FX_Init_Event(surface_size_change_r)
, _host(host)
, _multisample(0)
, _empty_texture( NewRetain<TextureEmpty>() )
, _font_pool(nullptr)
, _tex_pool(nullptr)
, _max_texture_memory_limit(512 * 1024 * 1024) // init 512MB
, _best_display_scale(1)
, _library(DRAW_LIBRARY_INVALID)
{
	ASSERT(!_draw_ctx, "At the same time can only run a GLDraw entity");
	_draw_ctx = this;
	
	cJSON& msample = options["multisample"];
	if (msample.is_uint32())
		_multisample = FX_MAX(msample.to_uint32(), 0);
	
	_font_pool = new FontPool(this); // 初始字体池
	_tex_pool = new TexturePool(this); // 初始文件纹理池
}

Draw::~Draw() {
	Release(_empty_texture); _empty_texture = nullptr;
	Release(_font_pool); _font_pool = nullptr;
	Release(_tex_pool); _tex_pool = nullptr;
	_draw_ctx = nullptr;
}

bool Draw::set_surface_size(Vec2 surface_size, CGRect* select_region) {
	CGRect region = select_region ? 
		*select_region : CGRect({ Vec2(), surface_size });
		
	if (_surface_size != surface_size ||
			_selected_region.origin != region.origin ||
			_selected_region.size != region.size
	) {
		_surface_size = surface_size;
		_selected_region = region;
		refresh_buffer();
		FX_Trigger(surface_size_change_r);
		return true;
	}
	return false;
}

/**
 * @func clear
 */
void Draw::clear(bool full) {
	_tex_pool->clear(full);
	_font_pool->clear(full);
}

void Draw::set_max_texture_memory_limit(uint64_t limit) {
	_max_texture_memory_limit = FX_MAX(limit, 64 * 1024 * 1024);
}

uint64_t Draw::used_texture_memory() const {
	return _tex_pool->_total_data_size + _font_pool->_total_data_size;
}

/**
 * @func adjust_texture_memory()
 */
bool Draw::adjust_texture_memory(uint64_t will_alloc_size) {
	
	int i = 0;
	do {
		if (will_alloc_size + used_texture_memory() <= _max_texture_memory_limit) {
			return true;
		}
		clear();
		i++;
	} while(i < 3);
	
	FX_WARN("Adjust texture memory fail");
	
	return false;
}

}
