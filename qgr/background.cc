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

#include "background.h"
#include "texture.h"
#include "display-port.h"

XX_NS(qgr)

XX_DEFINE_INLINE_MEMBERS(Background, Inl) {
#define _inl(self) static_cast<Background::Inl*>(static_cast<Background*>(self))
public:
	
	bool check_loop_reference(Background* value) {
		if (value) {
			auto bg = value;
			do {
				if (bg == this) {
					return true;
				}
				bg = bg->m_next;
			} while (bg);
		}
		return false;
	}
	
	static Background* assign(Background* left, Background* right) {
		if (right) {
			if (left == right) {
				return left;
			} else {
				if (right->retain()) {
					if (left) {
						left->release();
					}
					return right;
				} else { // copy
					auto new_left = right->copy(left);
					if (new_left != left) {
						if (left) {
							left->release();
						}
						bool ok = new_left->retain();
						XX_ASSERT(ok);
					}
					return new_left;
				}
			}
		} else {
			if (left) {
				left->release();
			}
			return nullptr;
		}
	}
	
	void set_next(Background* value) {
		m_next = assign(m_next, value);
		if (m_next) {
			m_next->set_host(m_host);
			m_next->set_holder_mode(m_holder_mode);
		}
		mark(View::M_BACKGROUND);
	}

};

Background::Background()
: m_next(nullptr)
, m_host(nullptr)
, m_holder_mode(M_INDEPENDENT)
{
}

Background::~Background() {
	if (m_next) {
		m_next->release();
		m_next = nullptr;
	}
}

void Background::set_next(Background* value) {
	if (value != m_next) {
		if (_inl(this)->check_loop_reference(value)) {
			XX_ERR("Box background loop reference error");
		} else {
			_inl(this)->set_next(value);
		}
	} else {
		mark(View::M_BACKGROUND);
	}
}

Background* Background::assign(Background* left, Background* right) {
	if (left == right) {
		return left;
	} else {
		if (left && right && _inl(left)->check_loop_reference(right->m_next)) {
			XX_ERR("Box background loop reference error");
			return left;
		} else {
			return Inl::assign(left, right);
		}
	}
}

bool Background::retain() {
	if (m_holder_mode == M_DISABLE) {
		return false;
	} else if (m_holder_mode == M_INDEPENDENT) {
		if (ref_count() > 0) {
			return false;
		}
	}
	return Reference::retain();
}

void Background::release() {
	set_host(nullptr);
	Reference::release();
}

void Background::set_host(Box* host) {
	if (m_host != host) {
		m_host = host;
		if (m_next) {
			m_next->set_host(host);
		}
	}
}

/**
 * @func set_holder_mode(mode)
 */
void Background::set_holder_mode(HolderMode mode) {
	if (m_holder_mode != mode) {
		m_holder_mode = mode;
		if (m_next) {
			m_next->set_holder_mode(mode);
		}
	}
}

void Background::mark(uint mark_value) {
	if (m_host) {
		m_host->mark(mark_value);
	}
}

enum {
	BI_flag_src = (1 << 0),
	BI_flag_texture = (1 << 1),
	BI_flag_repeat = (1 << 2),
	BI_flag_position_x = (1 << 3),
	BI_flag_position_y = (1 << 4),
	BI_flag_size_x = (1 << 5),
	BI_flag_size_y = (1 << 6),
};

XX_DEFINE_INLINE_MEMBERS(BackgroundImage, Inl) {
#define _inl2(self) static_cast<BackgroundImage::Inl*>(self)
public:
	void texture_change_handle(Event<int, Texture>& evt) { // 收到图像变化通知
		GUILock lock;
		int status = *evt.data();
		if (status & TEXTURE_CHANGE_OK) {
			mark(View::M_BACKGROUND);
		}
	}
	
	void reset_texture() {
		auto pool = tex_pool();
		if (pool) {
			if (m_has_base64_src) {
				XX_UNIMPLEMENTED(); // TODO ...
			} else {
				set_texture(pool->get_texture(m_src));
			}
		}
	}
	
//  Texture* get_texture() {
//    if (!m_texture) {
//      if (!m_src.is_empty()) {
//        reset_texture();
//      }
//    }
//    return m_texture;
//  }
	
	void set_source(cString& src, bool has_base64) {
		if (has_base64 || src != m_src) {
			m_src = src;
			m_has_base64_src = has_base64;
			if ( src.is_empty() ) {
				set_texture(nullptr);
			} else {
				reset_texture();
			}
			m_attributes_flags |= BI_flag_src;
		}
	}
	
};

BackgroundImage::BackgroundImage()
: m_src()
, m_texture(nullptr)
, m_repeat(Repeat::REPEAT)
, m_attributes_flags(0)
{
}

BackgroundImage::~BackgroundImage() {
	if (m_texture) {
		m_texture->XX_OFF(change, &Inl::texture_change_handle, _inl2(this));
		m_texture->release(); // 释放纹理
	}
}

Background* BackgroundImage::copy(Background* to) {
	BackgroundImage* target = (to && to->type() == M_IMAGE) ?
			static_cast<BackgroundImage*>(to) : new BackgroundImage();
	target->m_attributes_flags |= m_attributes_flags;
	if (m_attributes_flags & BI_flag_src) {
		target->m_src = m_src;
		target->m_has_base64_src = m_has_base64_src;
	}
	if (m_attributes_flags & BI_flag_repeat) target->m_repeat = m_repeat;
	if (m_attributes_flags & BI_flag_position_x) target->m_position_x = m_position_x;
	if (m_attributes_flags & BI_flag_position_y) target->m_position_y = m_position_y;
	if (m_attributes_flags & BI_flag_size_x) target->m_size_x = m_size_x;
	if (m_attributes_flags & BI_flag_size_y) target->m_size_y = m_size_y;
	if (m_attributes_flags & BI_flag_texture) target->set_texture(m_texture);
	_inl(target)->set_next(m_next);
	return target;
}

String BackgroundImage::src() const {
	return m_src;
}

void BackgroundImage::set_src(cString& value) {
	_inl2(this)->set_source(value, false);
}

void BackgroundImage::set_src_base64(cString& value) {
	_inl2(this)->set_source(value, true);
}

void BackgroundImage::set_texture(Texture* value) {
	if (value != m_texture) {
		if (m_texture) {
			m_texture->XX_OFF(change, &Inl::texture_change_handle, _inl2(this));
			m_texture->release(); // 释放
		}
		m_texture = value;
		if (value) {
			m_texture->retain(); // 保持
			m_texture->XX_ON(change, &Inl::texture_change_handle, _inl2(this));
		}
		mark(View::M_BACKGROUND);
		m_attributes_flags |= BI_flag_texture;
	}
}

void BackgroundImage::set_repeat(Repeat value) {
	if (m_repeat != value) {
		m_repeat = value;
		mark(View::M_BACKGROUND);
		m_attributes_flags |= BI_flag_repeat;
	}
}

void BackgroundImage::set_position_x(BackgroundPosition value) {
	if (value != m_position_x) {
		m_position_x = value;
		mark(View::M_BACKGROUND);
		m_attributes_flags |= BI_flag_position_x;
	}
}

void BackgroundImage::set_position_y(BackgroundPosition value) {
	if (value != m_position_y) {
		m_position_y = value;
		mark(View::M_BACKGROUND);
		m_attributes_flags |= BI_flag_position_y;
	}
}

void BackgroundImage::set_size_x(BackgroundSize value) {
	if (value != m_size_x) {
		m_size_x = value;
		mark(View::M_BACKGROUND);
		m_attributes_flags |= BI_flag_size_x;
	}
}

void BackgroundImage::set_size_y(BackgroundSize value) {
	if (value != m_size_y) {
		m_size_y = value;
		mark(View::M_BACKGROUND);
		m_attributes_flags |= BI_flag_size_y;
	}
}

bool BackgroundImage::get_background_image_data(Box* v,
																								Vec2& final_size,
																								Vec2& final_position, int& level) {
	bool ok = false;
	auto tex = m_texture;
	if (!tex) {
		return ok;
	}
	if (!tex->is_available()) {
		tex->load(); return ok;
	}
	
	float tex_width = tex->width();
	float tex_height = tex->height();
	auto sx = m_size_x, sy = m_size_y;
	auto px = m_position_x, py = m_position_y;
	
	float final_size_x, final_size_y, final_position_x, final_position_y;
	
	if (sx.type == BackgroundSizeType::AUTO) {
		switch (sy.type) {
			case BackgroundSizeType::AUTO:
				final_size_x = tex_width;
				final_size_y = tex_height;
				break;
			case BackgroundSizeType::PIXEL:
				final_size_y = sy.value;
				final_size_x = tex_width * final_size_y / tex_height;
				break;
			default: // case BackgroundSizeType::PERCENT:
				final_size_y = sy.value * v->m_final_height;
				final_size_x = tex_width * final_size_y / tex_height;
				break;
		}
	} else {
		if (sx.type == BackgroundSizeType::PIXEL) {
			final_size_x = sx.value;
		} else {
			final_size_x = sx.value * v->m_final_width;
		}
		switch (sy.type) {
			case BackgroundSizeType::AUTO:
				final_size_y = tex_height * final_size_x / tex_width;
				break;
			case BackgroundSizeType::PIXEL:
				final_size_y = sy.value;
				break;
			default:// case BackgroundSizeType::PERCENT:
				final_size_y = sy.value * v->m_final_height;
				break;
		}
	}
	
	switch (px.type) {
		case BackgroundPositionType::PIXEL:     /* 像素值  px */
			final_position_x = px.value;
			break;
		case BackgroundPositionType::PERCENT:   /* 百分比  % */
			final_position_x = px.value * v->m_final_width;
			break;
		case BackgroundPositionType::RIGHT:     /* 居右  % */
			final_position_x = v->m_final_width - final_size_x;
			break;
		case BackgroundPositionType::CENTER:    /* 居中 */
			final_position_x = (v->m_final_width - final_size_x) / 2;
			break;
		default:
			final_position_x = 0; break;
	}
	
	switch (py.type) {
		case BackgroundPositionType::PIXEL:     /* 像素值  px */
			final_position_y = py.value;
			break;
		case BackgroundPositionType::PERCENT:   /* 百分比  % */
			final_position_y = py.value * v->m_final_height;
			break;
		case BackgroundPositionType::BOTTOM:     /* 居下  % */
			final_position_y = v->m_final_height - final_size_y;
			break;
		case BackgroundPositionType::CENTER:    /* 居中 */
			final_position_y = (v->m_final_height - final_size_y) / 2;
			break;
		default:
			final_position_y = 0; break;
	}
	
	final_size = Vec2(final_size_x, final_size_y);
	final_position = Vec2(final_position_x, final_position_y);
	
	// Computing texture level
	float dpscale = display_port()->scale();
	// screen size
	auto vertex = v->m_final_vertex;
	float box_screen_scale_width = sqrt(pow(vertex[0][0] - vertex[1][0], 2) +
																			pow(vertex[0][1] - vertex[1][1], 2)) / v->m_final_width;
	float box_screen_scale_height = sqrt(pow(vertex[0][0] - vertex[3][0], 2) +
																			 pow(vertex[0][1] - vertex[3][1], 2)) / v->m_final_height;
	float tex_screen_width = final_size_x * box_screen_scale_width;
	float tex_screen_height = final_size_y * box_screen_scale_height;
	
	float ratio = (tex->width() / XX_MAX(tex_screen_width, 16) +
								 tex->height() / XX_MAX(tex_screen_height, 16)) / 2 / dpscale;
	
	level = tex->get_texture_level(floorf(ratio));
	
	return true;
}

BackgroundGradient::BackgroundGradient()
{
}

Background* BackgroundGradient::copy(Background* to) {
	BackgroundGradient* target = (to && to->type() == M_GRADIENT) ?
		static_cast<BackgroundGradient*>(to) : new BackgroundGradient();
	// TODO ..
	_inl(target)->set_next(m_next);
	return target;
}

XX_END
