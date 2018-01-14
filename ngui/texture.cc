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

#include "app.h"
#include "texture.h"
#include "draw.h"
#include "base/fs.h"
#include "base/buffer.h"
#include "display-port.h"

XX_NS(ngui)

typedef PixelData::Format PixelFormat;

static const Texture::Level levels_table[] {
  Texture::LEVEL_0, // 0
  Texture::LEVEL_0, // 1
  Texture::LEVEL_1, // 2
  Texture::LEVEL_1, // 3
  Texture::LEVEL_2, // 4
  Texture::LEVEL_2, // 5
  Texture::LEVEL_2, // 6
  Texture::LEVEL_2, // 7
  Texture::LEVEL_3, // 8
  Texture::LEVEL_3, // 9
  Texture::LEVEL_3, // 10
  Texture::LEVEL_3, // 11
  Texture::LEVEL_3, // 12
  Texture::LEVEL_3, // 13
  Texture::LEVEL_3, // 14
  Texture::LEVEL_3, // 15
  Texture::LEVEL_4, // 16
  Texture::LEVEL_4, // 17
  Texture::LEVEL_4, // 18
  Texture::LEVEL_4, // 19
  Texture::LEVEL_4, // 20
  Texture::LEVEL_4, // 21
  Texture::LEVEL_4, // 22
  Texture::LEVEL_4, // 23
  Texture::LEVEL_4, // 24
  Texture::LEVEL_4, // 25
  Texture::LEVEL_4, // 26
  Texture::LEVEL_4, // 27
  Texture::LEVEL_4, // 28
  Texture::LEVEL_4, // 29
  Texture::LEVEL_4, // 30
  Texture::LEVEL_4, // 31
  Texture::LEVEL_5, // 32
  Texture::LEVEL_5, // 33
  Texture::LEVEL_5, // 34
  Texture::LEVEL_5, // 35
  Texture::LEVEL_5, // 36
  Texture::LEVEL_5, // 37
  Texture::LEVEL_5, // 38
  Texture::LEVEL_5, // 39
  Texture::LEVEL_5, // 40
  Texture::LEVEL_5, // 41
  Texture::LEVEL_5, // 42
  Texture::LEVEL_5, // 43
  Texture::LEVEL_5, // 44
  Texture::LEVEL_5, // 45
  Texture::LEVEL_5, // 46
  Texture::LEVEL_5, // 47
  Texture::LEVEL_5, // 48
  Texture::LEVEL_5, // 49
  Texture::LEVEL_5, // 50
  Texture::LEVEL_5, // 51
  Texture::LEVEL_5, // 52
  Texture::LEVEL_5, // 53
  Texture::LEVEL_5, // 54
  Texture::LEVEL_5, // 55
  Texture::LEVEL_5, // 56
  Texture::LEVEL_5, // 57
  Texture::LEVEL_5, // 58
  Texture::LEVEL_5, // 59
  Texture::LEVEL_5, // 60
  Texture::LEVEL_5, // 61
  Texture::LEVEL_5, // 62
  Texture::LEVEL_5, // 63
  Texture::LEVEL_6, // 64
  Texture::LEVEL_6,
  Texture::LEVEL_6,
  Texture::LEVEL_6,
  Texture::LEVEL_6,
  Texture::LEVEL_6,
  Texture::LEVEL_6,
  Texture::LEVEL_6,
  Texture::LEVEL_6,
  Texture::LEVEL_6,
  Texture::LEVEL_6,
  Texture::LEVEL_6,
  Texture::LEVEL_6,
  Texture::LEVEL_6,
  Texture::LEVEL_6,
  Texture::LEVEL_6,
  Texture::LEVEL_6,
  Texture::LEVEL_6,
  Texture::LEVEL_6,
  Texture::LEVEL_6,
  Texture::LEVEL_6,
  Texture::LEVEL_6,
  Texture::LEVEL_6,
  Texture::LEVEL_6,
  Texture::LEVEL_6,
  Texture::LEVEL_6,
  Texture::LEVEL_6,
  Texture::LEVEL_6,
  Texture::LEVEL_6,
  Texture::LEVEL_6,
  Texture::LEVEL_6,
  Texture::LEVEL_6,
  Texture::LEVEL_6,
  Texture::LEVEL_6,
  Texture::LEVEL_6,
  Texture::LEVEL_6,
  Texture::LEVEL_6,
  Texture::LEVEL_6,
  Texture::LEVEL_6,
  Texture::LEVEL_6,
  Texture::LEVEL_6,
  Texture::LEVEL_6,
  Texture::LEVEL_6,
  Texture::LEVEL_6,
  Texture::LEVEL_6,
  Texture::LEVEL_6,
  Texture::LEVEL_6,
  Texture::LEVEL_6,
  Texture::LEVEL_6,
  Texture::LEVEL_6,
  Texture::LEVEL_6,
  Texture::LEVEL_6,
  Texture::LEVEL_6,
  Texture::LEVEL_6,
  Texture::LEVEL_6,
  Texture::LEVEL_6,
  Texture::LEVEL_6,
  Texture::LEVEL_6,
  Texture::LEVEL_6,
  Texture::LEVEL_6,
  Texture::LEVEL_6,
  Texture::LEVEL_6,
  Texture::LEVEL_6,
  Texture::LEVEL_6, // 127
  Texture::LEVEL_7, // 128
  Texture::LEVEL_7,
  Texture::LEVEL_7,
  Texture::LEVEL_7,
  Texture::LEVEL_7,
  Texture::LEVEL_7,
  Texture::LEVEL_7,
  Texture::LEVEL_7,
  Texture::LEVEL_7,
  Texture::LEVEL_7,
  Texture::LEVEL_7,
  Texture::LEVEL_7,
  Texture::LEVEL_7,
  Texture::LEVEL_7,
  Texture::LEVEL_7,
  Texture::LEVEL_7,
  Texture::LEVEL_7,
  Texture::LEVEL_7,
  Texture::LEVEL_7,
  Texture::LEVEL_7,
  Texture::LEVEL_7,
  Texture::LEVEL_7,
  Texture::LEVEL_7,
  Texture::LEVEL_7,
  Texture::LEVEL_7,
  Texture::LEVEL_7,
  Texture::LEVEL_7,
  Texture::LEVEL_7,
  Texture::LEVEL_7,
  Texture::LEVEL_7,
  Texture::LEVEL_7,
  Texture::LEVEL_7,
  Texture::LEVEL_7,
  Texture::LEVEL_7,
  Texture::LEVEL_7,
  Texture::LEVEL_7,
  Texture::LEVEL_7,
  Texture::LEVEL_7,
  Texture::LEVEL_7,
  Texture::LEVEL_7,
  Texture::LEVEL_7,
  Texture::LEVEL_7,
  Texture::LEVEL_7,
  Texture::LEVEL_7,
  Texture::LEVEL_7,
  Texture::LEVEL_7,
  Texture::LEVEL_7,
  Texture::LEVEL_7,
  Texture::LEVEL_7,
  Texture::LEVEL_7,
  Texture::LEVEL_7,
  Texture::LEVEL_7,
  Texture::LEVEL_7,
  Texture::LEVEL_7,
  Texture::LEVEL_7,
  Texture::LEVEL_7,
  Texture::LEVEL_7,
  Texture::LEVEL_7,
  Texture::LEVEL_7,
  Texture::LEVEL_7,
  Texture::LEVEL_7,
  Texture::LEVEL_7,
  Texture::LEVEL_7,
  Texture::LEVEL_7,
  Texture::LEVEL_7,
  Texture::LEVEL_7,
  Texture::LEVEL_7,
  Texture::LEVEL_7,
  Texture::LEVEL_7,
  Texture::LEVEL_7,
  Texture::LEVEL_7,
  Texture::LEVEL_7,
  Texture::LEVEL_7,
  Texture::LEVEL_7,
  Texture::LEVEL_7,
  Texture::LEVEL_7,
  Texture::LEVEL_7,
  Texture::LEVEL_7,
  Texture::LEVEL_7,
  Texture::LEVEL_7,
  Texture::LEVEL_7,
  Texture::LEVEL_7,
  Texture::LEVEL_7,
  Texture::LEVEL_7,
  Texture::LEVEL_7,
  Texture::LEVEL_7,
  Texture::LEVEL_7,
  Texture::LEVEL_7,
  Texture::LEVEL_7,
  Texture::LEVEL_7,
  Texture::LEVEL_7,
  Texture::LEVEL_7,
  Texture::LEVEL_7,
  Texture::LEVEL_7,
  Texture::LEVEL_7,
  Texture::LEVEL_7,
  Texture::LEVEL_7,
  Texture::LEVEL_7,
  Texture::LEVEL_7,
  Texture::LEVEL_7,
  Texture::LEVEL_7,
  Texture::LEVEL_7,
  Texture::LEVEL_7,
  Texture::LEVEL_7,
  Texture::LEVEL_7,
  Texture::LEVEL_7,
  Texture::LEVEL_7,
  Texture::LEVEL_7,
  Texture::LEVEL_7,
  Texture::LEVEL_7,
  Texture::LEVEL_7,
  Texture::LEVEL_7,
  Texture::LEVEL_7,
  Texture::LEVEL_7,
  Texture::LEVEL_7,
  Texture::LEVEL_7,
  Texture::LEVEL_7,
  Texture::LEVEL_7,
  Texture::LEVEL_7,
  Texture::LEVEL_7,
  Texture::LEVEL_7,
  Texture::LEVEL_7,
  Texture::LEVEL_7,
  Texture::LEVEL_7,
  Texture::LEVEL_7,
  Texture::LEVEL_7,
  Texture::LEVEL_7,
  Texture::LEVEL_7, // 255
};

inline static void set_texture_total_data_size(TexturePool* pool, int size);

inline static bool is_valid_texture(uint handle) {
  return handle && handle < Uint::max;
}

XX_DEFINE_INLINE_MEMBERS(Texture, Inl) {
public:
  /**
   * @func load_mipmap_data 通过像素数据载入mipmap纹理到GPU,如果成功返回true
   */
  bool load_mipmap_data(const Array<PixelData>& mipmap_data) {
    auto ctx = draw_ctx();
    uint size = 0;
    uint size_pixel = PixelData::get_pixel_data_size(mipmap_data[0].format());
    
    for (int i = 0; i < mipmap_data.length(); i++) {
      auto data = mipmap_data[i];
      size += data.width() * data.height() * size_pixel;
    }
    
    if (ctx->adjust_texture_memory(size)) {
      
      uint handle = ctx->load_texture(mipmap_data);
      if (handle) {
        if (m_handle[0]) {
          ctx->delete_texture(m_handle[0]);
        }
        cPixelData& pixel = mipmap_data[0];
        m_width  = pixel.width();
        m_height = pixel.height();
        m_format = pixel.format();
        m_handle[0] = handle;
        m_data_size[0] = size;
        m_use_count[0] = 1;
        m_repeat[0] = Repeat::NONE;
        set_texture_total_data_size(tex_pool(), size);
        
        if (mipmap_data.length() > 1) {
          m_status |= TEXTURE_HARDWARE_MIPMAP;
        } else {
          m_status &= ~TEXTURE_HARDWARE_MIPMAP;
        }
        return true;
      }
    }
    return false;
  }
  
  void generate_texture() {
    int status = TEXTURE_NO_LOADED;
    
    if (m_status & TEXTURE_HARDWARE_MIPMAP) {
      status |= (m_status & TEXTURE_CHANGE_LEVEL_MASK);
    } else {
      if (m_status & (TEXTURE_CHANGE_LEVEL_MASK & ~TEXTURE_CHANGE_LEVEL_0)) {
        auto ctx = draw_ctx();
        int mark = TEXTURE_CHANGE_LEVEL_1;
        uint prev_level_handle = m_handle[0];
        uint width = m_width / 2;
        uint height = m_height / 2;
        
        for (int i = LEVEL_1; i < LEVEL_NONE; i++) {
          uint handle = m_handle[i];
          if (width >= 16 && height >= 16) { // 不生成16像素以下的纹理
            if (is_valid_texture(handle)) {
              prev_level_handle = handle;
            } else {
              uint size = width * height * 4;
              if ((m_status & mark) && prev_level_handle &&
                  ctx->adjust_texture_memory(size)) { // gen target level
                handle = ctx->gen_texture(prev_level_handle, width, height);
                if (handle) {
                  m_handle[i] = handle;
                  m_data_size[i] = size;
                  m_repeat[i] = Repeat::NONE;
                  m_use_count[i] = 1;
                  set_texture_total_data_size(ctx->tex_pool(), size);
                  prev_level_handle = handle;
                  status |= mark;
                }
              }
            }
          } else {
            if (!handle)
              m_handle[i] = Uint::max;
          }
          width /= 2; height /= 2;
          mark <<= 1;
        }
        // for end
      }
      status |= (m_status & TEXTURE_CHANGE_LEVEL_0);
    }
    if (m_status & TEXTURE_COMPLETE) { // reload
      status |= TEXTURE_CHANGE_RELOADED;
    } else { // COMPLETE
      status |= TEXTURE_COMPLETE;
      m_status |= TEXTURE_COMPLETE;
    }
    m_status &= ~(TEXTURE_CHANGE_LEVEL_MASK | TEXTURE_LOADING); // delete mark
    main_loop()->post(Cb([this, status](Se& e) {
      XX_TRIGGER(change, status);
    }, this));
  }
  
  void clear() {
    auto ctx = draw_ctx();
    for (int i = 0; i < 8; i++) {
      if (is_valid_texture(m_handle[i])) {
        ctx->delete_texture(m_handle[i]); // 从GPU中删除纹理数据
        set_texture_total_data_size(tex_pool(), -m_data_size[i]);
        m_handle[i] = 0;
        m_data_size[i] = 0;
        m_use_count[i] = 0;
        m_repeat[i] = Repeat::NONE;
      }
    }
    m_status &= ~TEXTURE_HARDWARE_MIPMAP;
  }
};

Texture* Texture::create(cPixelData& data) {
  Texture* tex = new Texture();
  if ( Inl_Texture(tex)->load_mipmap_data({ data }) ) {
    return tex;
  }
  tex->release();
  return nullptr;
}

Texture* Texture::create(const Array<PixelData>& data) {
  Texture* tex = new Texture();
  if ( Inl_Texture(tex)->load_mipmap_data(data) ) {
    return tex;
  }
  tex->release();
  return nullptr;
}

/**
 * @func get_texture_level()
 */
Texture::Level Texture::get_texture_level(uint ratio) {
  // return LEVEL_0;
  if (ratio > 255) {
    return LEVEL_7;
  } else {
    return levels_table[ratio];
  }
}

/**
 * @func get_texture_level_from_convex_quadrilateral(quadrilateral_vertex)
 */
Texture::Level Texture::get_texture_level_from_convex_quadrilateral(Vec2 quadrilateral_vertex[4]) {
  if (m_width) {
    float scale = display_port()->scale();
    float width = sqrt(pow(quadrilateral_vertex[0][0] - quadrilateral_vertex[1][0], 2) +
                       pow(quadrilateral_vertex[0][1] - quadrilateral_vertex[1][1], 2)) * scale;
    float height = sqrt(pow(quadrilateral_vertex[0][0] - quadrilateral_vertex[3][0], 2) +
                        pow(quadrilateral_vertex[0][1] - quadrilateral_vertex[3][1], 2)) * scale;
    uint ratio_0 = floorf(m_width / XX_MAX(width, 16));
    uint ratio_1 = floorf(m_height / XX_MAX(height, 16));
    return get_texture_level(XX_MIN(ratio_0, ratio_1));
  } else {
    return Texture::LEVEL_0;
  }
}

Texture::Texture()
: XX_INIT_EVENT(change)
, m_status(TEXTURE_NO_LOADED)
, m_width(0)
, m_height(0)
, m_format(PixelData::INVALID)
{
  memset(m_handle, 0, sizeof(uint) * 24 + sizeof(Repeat) * 8);
}

Texture::~Texture() {
  Inl_Texture(this)->clear();
}

String Texture::id() const {
  return String();
}

/**
 * @func load_data 通过数据载入纹理到GPU,如果成功返回true
 */
bool Texture::load_data(cPixelData& data) {
  return Inl_Texture(this)->load_mipmap_data({ data });
}

bool Texture::use(uint slot, Level level, Repeat repeat) {
  uint handle = m_handle[level];
  if (!is_valid_texture(handle)) {
    load(level);
    if (!is_valid_texture(handle = m_handle[level])) {
      for (int i = level - 1; i >= 0; i--) {
        if (is_valid_texture(handle = m_handle[i])) {
          level = (Level)i;
          goto use;
        }
      }
      for (int i = level + 1; i < LEVEL_NONE; i++) {
        if (is_valid_texture(handle = m_handle[i])) {
          level = (Level)i;
          goto use;
        }
      }
      return false;
    }
  }
 use:
  if ( repeat == m_repeat[level] ) {
    draw_ctx()->use_texture(handle, slot);
  } else {
    draw_ctx()->use_texture(handle, repeat, slot);
    m_repeat[level] = repeat;
  }
  m_use_count[level]++;
  return true;
}

bool TextureYUV::load_yuv(cPixelData& data) {
  
  auto pool = tex_pool();
  int old_size = m_data_size[0] + m_data_size[1];
  set_texture_total_data_size(pool, -old_size);
  int size = data.width() * data.height();
  int new_size = size + size / 2;
  
  if (draw_ctx()->adjust_texture_memory(new_size)) {
    if ( draw_ctx()->load_yuv_texture(this, data) ) {
      set_texture_total_data_size(pool, new_size);
      
      if (m_width != data.width() ||
          m_height != data.height() || m_format != data.format()) {
        m_width = data.width();
        m_height = data.height();
        m_format = data.format();
        m_status = TEXTURE_COMPLETE;
        m_data_size[0] = size;
        m_data_size[1] = size / 2;
        main_loop()->post(Cb([this](Se& e) {
          XX_TRIGGER(change, TEXTURE_CHANGE_RELOADED | TEXTURE_CHANGE_LEVEL_MASK);
        }, this));
      }
      return true;
    }
  }
  set_texture_total_data_size(pool, old_size);
  
  return false;
}

bool TextureYUV::unload(Level level) {
  Inl_Texture(this)->clear();
  return true;
}

FileTexture::FileTexture(cString& path)
: m_path(f_reader()->format(path))
, m_load_id(0)
, m_image_format(ImageCodec::get_image_format(m_path))
, m_pool(nullptr) {
}

FileTexture::~FileTexture() {
  XX_ASSERT(m_pool == nullptr);
  XX_ASSERT(m_load_id == 0);
}

String FileTexture::id() const {
  return m_path;
}

void FileTexture::load(Level level) {
  
  if (level == LEVEL_NONE) {
    if (m_status & (TEXTURE_ERROR | TEXTURE_COMPLETE)) return;
  } else {
    if (m_handle[level]) return;
    if (m_status & (TEXTURE_ERROR | TEXTURE_HARDWARE_MIPMAP)) return;
    int mark = TEXTURE_CHANGE_LEVEL_0 << level;
    int i = level;
    m_status |= mark;
    while (i) {
      i--; mark >>= 1;
      if (is_valid_texture(m_handle[i]) || (m_status & mark)) break;
    }
    if (mark == TEXTURE_CHANGE_LEVEL_0 && !m_handle[0]) {
      m_status |= TEXTURE_CHANGE_LEVEL_0;
    }
  }
  
  if (m_status & TEXTURE_LOADING) return;
  
  #define LoaderTextureError(err) { \
    m_status = TEXTURE_ERROR;  \
    XX_ERR(err, *m_path); \
    main_loop()->post(Cb([this](Se& e) { \
      XX_TRIGGER(change, TEXTURE_CHANGE_ERROR); \
    }, this)); \
  }
  
  m_status |= TEXTURE_LOADING;
  
  if (!(m_status & TEXTURE_CHANGE_LEVEL_0) && (m_status & TEXTURE_COMPLETE)) {
    Inl_Texture(this)->generate_texture();
    return;
  }
  
  struct DecodeContext {
    typedef NonObjectTraits Traits;
    Buffer input;
    Array<PixelData> output;
  };
  
  // load level 0
  m_load_id = f_reader()->read_file(m_path, Cb([this](Se& d) {
    GUILock lock;
    m_load_id = 0;
    if (d.error) {
      LoaderTextureError("Error: Error reading the image file, %s");
    }
    else if (m_status & TEXTURE_LOADING) {
      // 如果不等于`TEXTURE_STATUS_LOADING`表示已经被取消所以无需载入了
      ImageCodec* parser = ImageCodec::shared(m_image_format);
      if (!parser) {
        LoaderTextureError("Error: File format is not supported, %s");
        return;
      }
      auto ctx = new DecodeContext();
      ctx->input = *static_cast<Buffer*>(d.data);
      app()->render_loop()->work(Cb([this, ctx, parser](Se& e) {
        // 解码需要时间,发送到工作线程执行解码操作
        if (m_status & TEXTURE_CHANGE_LEVEL_0) {
          ctx->output = parser->decode(ctx->input);
        } else {
          PixelData pd = parser->decode_header(ctx->input);
          if (pd.width()) {
            ctx->output = { pd };
          }
        }
      }, this), Cb([this, ctx](Se& e) { // 完成
        Handle<DecodeContext> handle(ctx);
        if (!ctx->output.length()) {
          LoaderTextureError("Error: Loader image file error, %s");
          return;
        }
        if (ctx->output[0].body_count() &&
            ctx->output[0].body().length()) {
          if (!Inl_Texture(this)->load_mipmap_data(ctx->output)) {
            LoaderTextureError("Error: Loader image file error, %s");
            return;
          }
        } else {
          m_width = ctx->output[0].width();
          m_height = ctx->output[0].height();
          m_format = ctx->output[0].format();
        }
        Inl_Texture(this)->generate_texture();
      }));
    }
  }, this));
  
  #undef LoaderTextureError
}

bool FileTexture::unload(Level level) {
  if (level == LEVEL_NONE) { // unload all
    if (m_status & TEXTURE_LOADING) {
      m_status &= ~TEXTURE_LOADING;
      if (m_load_id) { // 取消正在载入的纹理,比如正在从http服务器下载纹理数据...
        f_reader()->abort(m_load_id);
        m_load_id = 0;
      }
    }
    auto ctx = draw_ctx();
    for (int i = 0; i < LEVEL_NONE; i++) {
      if (is_valid_texture(m_handle[i])) {
        ctx->delete_texture(m_handle[i]); // 从GPU中删除纹理数据
        set_texture_total_data_size(tex_pool(), -m_data_size[i]);
      }
      m_handle[i] = 0;
      m_repeat[i] = Repeat::NONE;
      m_data_size[i] = 0;
      m_use_count[i] = 0;
    }
    m_status &= ~(TEXTURE_CHANGE_LEVEL_MASK | TEXTURE_HARDWARE_MIPMAP); // delete mark
  } else { // unload level
    if (is_valid_texture(m_handle[level])) {
      draw_ctx()->delete_texture(m_handle[level]);
      set_texture_total_data_size(tex_pool(), -m_data_size[level]);
      if (level == LEVEL_0)
        m_status &= ~TEXTURE_HARDWARE_MIPMAP;
    }
    m_handle[level] = 0;
    m_repeat[level] = Repeat::NONE;
    m_data_size[level] = 0;
    m_use_count[level] = 0;
    m_status &= ~(TEXTURE_CHANGE_LEVEL_0 << level); // delete mark
  }
  return true;
}

XX_DEFINE_INLINE_MEMBERS(TexturePool, Inl) {
public:
#define _inl_pool(self) static_cast<TexturePool::Inl*>(self)
  
  void texture_change_handle(Event<int, Texture>& evt) {
    int status = *evt.data();
    if (status & TEXTURE_CHANGE_COMPLETE) {
      GUILock lock;
      m_completes.set(evt.sender(), evt.sender()); // 完成后加入完成列表
      auto sender = static_cast<FileTexture*>(evt.sender());
      TexturePoolEventData data = { progress(), sender };
      XX_TRIGGER(change, data);
    }
  }
  
  void add_texture_for_pool(FileTexture* tex, cString& name) {
    XX_ASSERT(!tex->m_pool);
    XX_ASSERT(!m_textures[name]);
    m_textures[name] = tex;
    tex->retain();
    tex->m_pool = this;
  }
  
  void del_texture_for_pool(FileTexture* tex) {
    XX_ASSERT(tex->m_pool == this);
    XX_ASSERT(tex->ref_count() > 0);
    tex->m_pool = nullptr;
    m_completes.del(tex);
    tex->release();
  }
  
  void trigger_change() {
    main_loop()->post(Cb([this](Se& e) {
      TexturePoolEventData data = { progress(), nullptr };
      XX_TRIGGER(change, data);
    }));
  }
  
  void set_texture_total_data_size(int size) {
    m_total_data_size += size;
    XX_ASSERT(m_total_data_size >= 0);
  }
};

static void set_texture_total_data_size(TexturePool* pool, int size) {
  _inl_pool(pool)->set_texture_total_data_size(size);
}

TexturePool::TexturePool(Draw* ctx)
: XX_INIT_EVENT(change)
, m_draw_ctx(ctx)
, m_total_data_size(0)
{
  XX_ASSERT(m_draw_ctx); // "Did not find GLDraw"
}

TexturePool::~TexturePool() {
  for ( auto& i : m_textures ) {
    auto tex = i.value();
    tex->m_pool = nullptr;
    XX_ASSERT( tex->ref_count() > 0 );
    if ( tex->ref_count() == 1 ) {
      Release(tex);
    }
  }
}

FileTexture* TexturePool::get_texture(cString& path) {
  String pathname = f_reader()->format(path);
  
  // 通过路径查找纹理对像
  auto it = m_textures.find(pathname);
  if ( !it.is_null() ) {
    return it.value();
  }
  
  // 在当前池中创建一个纹理
  FileTexture* texture = new FileTexture(pathname);
  _inl_pool(this)->add_texture_for_pool(texture, pathname);
  
  texture->XX_ON(change, &Inl::texture_change_handle, _inl_pool(this));
  
  return texture;
}

void TexturePool::load(const Array<String>& paths) {
  for ( auto& i : paths ) {
    get_texture(i.value())->load();
  }
}

void TexturePool::load_all() {
  for (auto& i : m_textures) {
    i.value()->load();
  }
}

float TexturePool::progress() const {
  if ( m_textures.length() > 0 ) {
    return m_completes.length() / float(m_textures.length());
  } else {
    return 1.0f;
  }
}

void TexturePool::clear(bool full) {
  bool del_mark = false;
  
  if (full) {
    for ( auto& i : m_textures ) {
      FileTexture* texture = i.value();
      texture->unload();
      XX_ASSERT(texture->ref_count() > 0);
      if ( texture->ref_count() == 1 ) { // 不需要使用的纹理可以删除
        _inl_pool(this)->del_texture_for_pool(texture);
        i.value() = nullptr;
        del_mark = true;
        m_textures.mark(i);
      }
    }
    if (del_mark) {
      m_textures.del_mark();
      _inl_pool(this)->trigger_change();
    }
  } else {
    struct TexWrap {
      FileTexture* tex;
      Texture::Level level;
      uint use_count;
    };
    List<TexWrap> texture_sort;
    uint64 total_data_size = 0;
    
    // 先按使用使用次数排序纹理对像
    for ( auto& i : m_textures ) {
      FileTexture* tex = i.value();
      XX_ASSERT( tex->ref_count() > 0 );
      
      if ( tex->ref_count() == 1 ) { // 不需要使用的纹理可以删除
        tex->unload();
        _inl_pool(this)->del_texture_for_pool(tex);
        i.value() = nullptr;
        del_mark = true;
        memset(tex->m_use_count, 0, sizeof(uint) * 8); // 置零纹理使用次数
      } else {
        for (int k = 0; k < Texture::LEVEL_NONE; k++) {
          auto level = Texture::Level(k);
          if (is_valid_texture(tex->m_handle[k])) {
            auto it = texture_sort.end();
            uint use_count = tex->m_use_count[k];
            for ( auto& j : texture_sort ) {
              if ( use_count <= j.value().use_count ) {
                it = j;
                break;
              }
            }
            if ( it.is_null() ) {
              texture_sort.push({ tex, level, use_count });
            } else {
              texture_sort.before(it, { tex, level, use_count });
            }
            total_data_size += tex->m_data_size[k];
            tex->m_use_count[k] /= 2;
          }
        }
      }
      m_textures.mark(i);
    }
    
    if ( del_mark ) {
      m_textures.del_mark();
      _inl_pool(this)->trigger_change();
    }
    
    if ( texture_sort.length() ) {
      uint64 total_data_size_1_3 = total_data_size / 3;
      uint64 del_data_size = 0;
      // 从排序列表顶部开始卸载总容量的1/3
      for ( auto& i : texture_sort ) {
        auto tex = i.value();
        if (del_data_size < total_data_size_1_3) {
          del_data_size += tex.tex->m_data_size[tex.level];
          tex.tex->unload(tex.level);
        } else {
          break;
        }
      }
      XX_DEBUG("Texture memory clear, %ld", del_data_size);
    }
  }
}

XX_END
