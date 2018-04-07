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

XX_DEFINE_INLINE_MEMBERS(BackgroundImage, Inl) {
#define _inl2(self) static_cast<BackgroundImage::Inl*>(self)
public:
  /**
   * @func texture_change_handle()
   */
  void texture_change_handle(Event<int, Texture>& evt) { // 收到图像变化通知
//    GUILock lock;
    int status = *evt.data();
//
    if (status & TEXTURE_CHANGE_OK) {
//      mark_pre(M_LAYOUT | M_SIZE_HORIZONTAL | M_SIZE_VERTICAL | M_TEXTURE); // 标记
    }
    if (status & TEXTURE_CHANGE_ERROR) {
//      Error err(ERR_IMAGE_LOAD_ERROR, "Image load error, %s", *evt.sender()->id());
//      Handle<GUIEvent> evt = New<GUIEvent>(this, err);
//      trigger(GUI_EVENT_ERROR, **evt);
    } else if (status & TEXTURE_COMPLETE) {
//      display_port()->next_frame(Cb([this](Se& e) {
//        trigger(GUI_EVENT_LOAD);
//      }, this));
    }
  }
};

Background::Background()
: m_next(nullptr)
, m_host(nullptr)
{
}

Background::~Background() {
  Release(m_next);
}

void Background::set_next(Background* value) throw(Error) {
  if (value) {
    auto bg = value;
    do {
      if (bg == this) {
        XX_THROW(ERR_BACKGROUND_NEXT_LOOP_REF, "Box background loop reference error");
      }
      bg = bg->m_next;
    } while (bg);
  }
  
  if (m_next) {
    m_next->set_host(nullptr);
    m_next->release();
  }
  m_next = value;
  
  if (value) {
    value->retain();
    value->set_host(m_host);
  }
}

void Background::set_host(Box* host) {
  if (m_host != host) {
    m_host = host;
    if (m_next) {
      m_next->set_host(host);
    }
  }
}

void Background::mark(uint mark_value) {
  if (m_host) {
    m_host->mark(mark_value);
  }
}

BackgroundColor::BackgroundColor()
{
}

void BackgroundColor::set_color(Color value) {
  m_color = value;
  mark(View::M_BACKGROUND);
}

void BackgroundColor::draw(Draw* draw, Box* host) {
  // TODO ..
  // ..
}

BackgroundImage::BackgroundImage()
: m_tex_level(Texture::LEVEL_0)
, m_texture(draw_ctx()->empty_texture())
{
  m_texture->retain(); // 保持纹理
}

BackgroundImage::~BackgroundImage() {
  m_texture->XX_OFF(change, &Inl::texture_change_handle, _inl2(this));
  m_texture->release(); // 释放纹理
}

void BackgroundImage::set_texture(Texture* value) {
  XX_ASSERT(value);
  if (value == m_texture) return;
  m_texture->XX_OFF(change, &Inl::texture_change_handle, _inl2(this));
  m_texture->release(); // 释放
  m_texture = value;
  m_texture->retain(); // 保持
  m_texture->XX_ON(change, &Inl::texture_change_handle, _inl2(this));
  mark(View::M_BACKGROUND);
}

void BackgroundImage::draw(Draw* draw, Box* host) {
  // TODO ..
  // ..
}

BackgroundGradient::BackgroundGradient()
{
}

void BackgroundGradient::draw(Draw* draw, Box* host) {
  // TODO ..
  // ..
}

XX_END
