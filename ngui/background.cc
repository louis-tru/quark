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

XX_NS(ngui)

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
      m_next->set_allow_multi_holder(m_allow_multi_holder);
    }
    mark(View::M_BACKGROUND);
  }

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
  
  Texture* get_texture() {
    if (!m_texture) {
      if (!m_src.is_empty()) {
        reset_texture();
      }
    }
    return m_texture;
  }
  
  void set_source(cString& src, bool has_base64) {
    if (has_base64 || src != m_src) {
      if ( src.is_empty() ) {
        set_texture(nullptr);
      } else {
        reset_texture();
      }
      m_src = src;
      m_has_base64_src = has_base64;
    }
  }
  
};

Background::Background()
: m_next(nullptr)
, m_host(nullptr)
, m_allow_multi_holder(false)
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
  if (m_allow_multi_holder || ref_count() <= 0) {
    return Reference::retain();
  }
  return false;
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
 * @func set_allow_multi_holder(value)
 */
void Background::set_allow_multi_holder(bool value) {
  if (m_allow_multi_holder != value) {
    m_allow_multi_holder = value;
    if (m_next) {
      m_next->set_allow_multi_holder(value);
    }
  }
}

void Background::mark(uint mark_value) {
  if (m_host) {
    m_host->mark(mark_value);
  }
}

BackgroundImage::BackgroundImage()
: m_src()
, m_tex_level(Texture::LEVEL_0)
, m_texture(nullptr)
, m_repeat(Repeat::REPEAT)
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
  target->m_repeat = m_repeat;
  target->m_position_x = m_position_x;
  target->m_position_y = m_position_y;
  target->m_size_x = m_size_x;
  target->m_size_y = m_size_y;
  target->set_texture(m_texture);
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
  }
}

void BackgroundImage::set_repeat(Repeat value) {
  if (m_repeat != value) {
    m_repeat = value;
    mark(View::M_BACKGROUND);
  }
}

void BackgroundImage::set_position_x(BackgroundPosition value) {
  if (value != m_position_x) {
    m_position_x = value;
    mark(View::M_BACKGROUND);
  }
}

void BackgroundImage::set_position_y(BackgroundPosition value) {
  if (value != m_position_y) {
    m_position_y = value;
    mark(View::M_BACKGROUND);
  }
}

void BackgroundImage::set_size_x(BackgroundSize value) {
  if (value != m_size_x) {
    m_size_x = value;
    mark(View::M_BACKGROUND);
  }
}

void BackgroundImage::set_size_y(BackgroundSize value) {
  if (value != m_size_y) {
    m_size_y = value;
    mark(View::M_BACKGROUND);
  }
}

void BackgroundImage::draw(Draw* draw, Box* host) {
  // TODO ..
  // ..
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

void BackgroundGradient::draw(Draw* draw, Box* host) {
  // TODO ..
  // ..
}

XX_END
