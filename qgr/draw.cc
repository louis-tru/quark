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

#include "draw.h"
#include "font.h"
#include "texture.h"
#include "display-port.h"

XX_NS(qgr)

static char empty_[4] = { 0, 0, 0, 0 };
static cPixelData empty_pixel_data(WeakBuffer(empty_, 4), 1, 1, PixelData::RGBA8888);

/**
 * @class TextureEmpty
 */
class TextureEmpty: public Texture {
 public:
	virtual void load() {
		if (m_status == TEXTURE_NO_LOADED) {
			XX_CHECK(load_data(empty_pixel_data), "Load temp texture error");
		}
	}
};

Draw* Draw::m_draw_ctx = nullptr; // 当前GL上下文

/**
 * @constructor
 */
Draw::Draw(GUIApplication* host, cJSON& options)
: XX_INIT_EVENT(surface_size_change_r)
, m_host(host)
, m_multisample(0)
, m_empty_texture( NewRetain<TextureEmpty>() )
, m_font_pool(nullptr)
, m_tex_pool(nullptr)
, m_max_texture_memory_limit(512 * 1024 * 1024) // init 512MB
, m_best_display_scale(1)
, m_library(DRAW_LIBRARY_INVALID)
{
	XX_CHECK(!m_draw_ctx, "At the same time can only run a GLDraw entity");
	m_draw_ctx = this;
	
	cJSON& msample = options["multisample"];
	if (msample.is_uint()) 
		m_multisample = XX_MAX(msample.to_uint(), 0);
	
	m_font_pool = new FontPool(this); // 初始字体池
	m_tex_pool = new TexturePool(this); // 初始文件纹理池
}

Draw::~Draw() {
	Release(m_empty_texture); m_empty_texture = nullptr;
	Release(m_font_pool); m_font_pool = nullptr;
	Release(m_tex_pool); m_tex_pool = nullptr;
	m_draw_ctx = nullptr;
}

bool Draw::set_surface_size(Vec2 surface_size, CGRect* select_region) {
	CGRect region = select_region ? 
		*select_region : CGRect({ Vec2(), surface_size });
		
	if (m_surface_size != surface_size ||
			m_selected_region.origin != region.origin ||
			m_selected_region.size != region.size
	) {
		m_surface_size = surface_size;
		m_selected_region = region;
		refresh_buffer();
		XX_TRIGGER(surface_size_change_r);
		return true;
	}
	return false;
}

/**
 * @func clear
 */
void Draw::clear(bool full) {
	m_tex_pool->clear(full);
	m_font_pool->clear(full);
}

void Draw::set_max_texture_memory_limit(uint64 limit) {
	m_max_texture_memory_limit = XX_MAX(limit, 64 * 1024 * 1024);
}

uint64 Draw::used_texture_memory() const {
	return m_tex_pool->m_total_data_size + m_font_pool->m_total_data_size;
}

/**
 * @func adjust_texture_memory()
 */
bool Draw::adjust_texture_memory(uint64 will_alloc_size) {
	
	int i = 0;
	do {
		if (will_alloc_size + used_texture_memory() <= m_max_texture_memory_limit) {
			return true;
		}
		clear();
		i++;
	} while(i < 3);
	
	XX_WARN("Adjust texture memory fail");
	
	return false;
}

XX_END
