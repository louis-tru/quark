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

XX_NS(ngui)

typedef PixelData::Format PixelFormat;

Texture::Texture(uint slot)
: XX_INIT_EVENT(change)
, m_handle(0)
, m_status(TEXTURE_STATUS_NO_LOADED)
, m_width(0)
, m_height(0)
, m_data_size(0)
, m_format(PixelData::INVALID)
, m_is_premultiplied_alpha(false)
, m_slot(slot)
, m_repeat(Repeat::NONE)
{
  
}

/**
 * @func create # 通过图像数据创建一个新的纹理对像,如果成功返回纹理对像
 */
Texture* Texture::create(cPixelData& data, uint texture) {
  Texture* tex = new Texture(texture);
  if ( tex->load_data(data) ) {
    return tex;
  }
  Release(tex);
  return nullptr;
}

/**
 * @func create # 通过mipmap图像数据创建一个新的纹理对像,如果成功返回纹理对像
 */
Texture* Texture::create(const Array<PixelData>& data, uint texture) {
  Texture* tex = new Texture(texture);
  if ( tex->load_mipmap(data) ) {
    return tex;
  }
  Release(tex);
  return nullptr;
}

Texture::~Texture() {
  unload();
}

String Texture::name() const {
  return String();
}

/**
 * @func load_mipmap 通过数据载入mipmap纹理到GPU,如果成功返回true
 *                   如果GLDraw 没有开启mipmap不会载入mip纹理
 */
bool Texture::load_mipmap(const Array<PixelData>& data) {
  
  if ( draw_ctx()->load_texture(this, data) ) {
    cPixelData& pixel = data[0];
    
    m_width  = pixel.width();
    m_height = pixel.height();
    m_format = pixel.format();
    m_is_premultiplied_alpha = pixel.is_premultiplied_alpha();
    m_data_size = m_width * m_height * PixelData::get_pixel_data_size(m_format);
    
    if ( draw_ctx()->mipmap() || data.length() > 1 ) { // mipmap
      m_data_size += m_data_size / 3.6;
    }
    
    main_loop()->post(Cb([this](Se& e) {
      XX_TRIGGER(change);
    }, this));
    
    return true;
  }
  return false;
}

/**
 * @func load_data 通过数据载入纹理到GPU,如果成功返回true
 */
bool Texture::load_data(cPixelData& data) {
  Array<PixelData> ls; ls.push(data);
  return load_mipmap(ls);
}

/**
 * 释放纹理,并不全删除纹理对像
 * 只暂时释放内存资源,需要时可重新载入
 */
void Texture::unload() {
  if ( is_ready() ) {
    auto ctx = draw_ctx();
    if ( ctx ) {
      if ( ctx->m_cur_bind_textures[ m_slot ] == this ) {
        ctx->m_cur_bind_textures[ m_slot ] = nullptr;
      }
      ctx->delete_texture(m_handle); // 从GPU中删除纹理数据
      m_status = TEXTURE_STATUS_NO_LOADED;
      m_handle = 0;
    }
  }
}

bool Texture::use(Repeat repeat) {
  if ( is_ready() ) {
    Texture** textures = draw_ctx()->m_cur_bind_textures;
    if ( repeat == m_repeat ) {
      if ( textures[m_slot] != this ) {
        draw_ctx()->use_texture(m_handle, m_slot);
        textures[m_slot] = this;
      }
    } else {
      m_repeat = repeat;
      draw_ctx()->use_texture(m_handle, repeat, m_slot);
      textures[m_slot] = this;
    }
    return true;
  }
  load();
  
  return false;
}

bool TextureYUV::use(Repeat repeat) {
  if ( is_ready() ) {
    Texture** textures = draw_ctx()->m_cur_bind_textures;
    if (repeat == m_repeat) {
      if ( textures[m_slot] != this ) {
        draw_ctx()->use_texture(m_handle, m_slot);
        textures[m_slot] = this;
      }
      if ( textures[m_slot + 1] != this ) {
        draw_ctx()->use_texture(m_uv_handle, m_slot + 1);
        textures[m_slot + 1] = this;
      }
    } else {
      m_repeat = repeat;
      draw_ctx()->use_texture(m_handle, repeat, m_slot);
      textures[m_slot] = this;
      draw_ctx()->use_texture(m_uv_handle, repeat, m_slot + 1);
      textures[m_slot + 1] = this;
    }
    return true;
  }
  return false;
}

/**
 * @func load_yuv
 */
bool TextureYUV::load_yuv(cPixelData& data) {
  if ( draw_ctx()->load_yuv_texture(this, data) ) {
    if ( m_width != data.width() ||
        m_height != data.height() || m_format != data.format() ) {
      m_width = data.width();
      m_height = data.height();
      m_format = data.format();
      m_status = TEXTURE_STATUS_COMPLETE;
      m_data_size = m_width * m_height * 3;
      main_loop()->post(Cb([this](Se& e) {
        XX_TRIGGER(change);
      }, this));
    }
    return true;
  }
  return false;
}

TextureYUV::~TextureYUV() {
  unload();
}

void TextureYUV::unload() {
  if ( is_ready() ) {
    auto ctx = draw_ctx();
    if ( ctx ) {
      if ( ctx->m_cur_bind_textures[ m_slot ] == this ) {
        ctx->m_cur_bind_textures[ m_slot ] = nullptr;
      }
      if ( ctx->m_cur_bind_textures[ m_slot + 1 ] == this ) {
        ctx->m_cur_bind_textures[ m_slot + 1 ] = nullptr;
      }
      ctx->delete_texture(m_handle);     // 从GPU中删除纹理数据
      ctx->delete_texture(m_uv_handle);  // 从GPU中删除纹理数据
      m_status = TEXTURE_STATUS_NO_LOADED;
      m_handle = 0;
      m_uv_handle = 0;
    }
  }
}

FileTexture::FileTexture(cString& path)
: m_path(f_reader()->format(path))
, m_load_id(0)
, m_image_format(ImageCodec::get_image_format(m_path))
, m_pool(nullptr), m_use_count(0) {
  
}

String FileTexture::name() const {
  return m_path;
}

void FileTexture::load() {
  if (m_status == TEXTURE_STATUS_LOADING ||
      m_status == TEXTURE_STATUS_ERROR ||
      m_status == TEXTURE_STATUS_COMPLETE) {
    return;
  }
  
  #define LoaderTextureError(err) { \
    m_status = TEXTURE_STATUS_ERROR;  \
    XX_ERR(err, *m_path); \
    main_loop()->post(Cb([this](Se& e) { XX_TRIGGER(change); }, this)); \
  }
  
  struct ParserContext {
    typedef NonObjectTraits Traits;
    ~ParserContext() { Release(parser); }
    Buffer input;
    Array<PixelData> output;
    ImageCodec* parser;
  };
  
  m_status = TEXTURE_STATUS_LOADING; // trigger event
  
  m_load_id = f_reader()->read_file(m_path, Cb([this](Se& d) { m_load_id = 0;
    GUILock lock;
    if ( d.error ) {
      LoaderTextureError("Error: Error reading the image file, %s");
    }
    else if ( m_status == TEXTURE_STATUS_LOADING ) {
      // 如果不等于`TEXTURE_STATUS_LOADING`表示已经被取消所以无需载入了
      ImageCodec* parser = ImageCodec::create(m_image_format);
      if (!parser) {
        LoaderTextureError("Error: File format is not supported, %s");
      } else {
        auto ctx = new ParserContext;
        ctx->input = *static_cast<Buffer*>(d.data);
        ctx->parser = parser;
        // 解码需要时间,发送到工作线程执行解码操作
        app()->render_loop()->work(Cb([this, ctx](Se& e) {
          ctx->output = ctx->parser->decode(ctx->input);
        }, this), Cb([this, ctx](Se& e) { // 完成
          Handle<ParserContext> handle(ctx);
          m_status = TEXTURE_STATUS_COMPLETE;
          if (!load_mipmap(ctx->output)) {
            LoaderTextureError("Error: Loader image file error, %s");
          }
        }));
      }
      // end if ( parser.is_null() ) {
    }
  }, this));
  
  #undef LoaderTextureError
}

void FileTexture::unload() {
  if ( m_status == TEXTURE_STATUS_LOADING ) {
    XX_ASSERT(m_load_id);
    // 取消正在载入的纹理,比如正在从http服务器下载纹理数据...
    f_reader()->abort(m_load_id);
    m_status = TEXTURE_STATUS_NO_LOADED;
  } else {
    Texture::unload();
  }
}

bool FileTexture::use(Repeat repeat) {
  m_use_count++;
  return Texture::use(repeat);
}

XX_DEFINE_INLINE_MEMBERS(TexturePool, Inl) {
public:
#define _inl_pool(self) static_cast<TexturePool::Inl*>(self)
  
  // 纹理事件处理器
  void texture_change_handle(Event<float, Texture>& evt) {
    GUILock lock;
    String name = evt.sender()->name();
    
    switch (evt.sender()->status()) {
      case TEXTURE_STATUS_ERROR:
      case TEXTURE_STATUS_NO_LOADED:
         m_completes.del(evt.sender()); // 从完成列表中移除
        break;
      case TEXTURE_STATUS_COMPLETE:
        m_completes.set(evt.sender(), evt.sender()); // 完成后加入完成列表
        break;
      default: // Loading
        return;
    }
    auto sender = static_cast<FileTexture*>(evt.sender());
    TexturePoolEventData data = { progress(), sender };
    XX_TRIGGER(change, data);
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
  
  void clear(bool full) { // render loop run
    bool del_mark = false;
    
    if (full) {
      for ( auto& i : m_textures ) {
        FileTexture* texture = i.value();
        texture->m_use_count = 0;
        texture->unload();
        XX_ASSERT( texture->ref_count() > 0 );
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
      
      uint64 total_data_size = 0;
      List<FileTexture*> sort_textures;
      
      // 先按使用使用次数排序纹理对像
      
      for ( auto& i : m_textures ) {
        FileTexture* texture = i.value();
        XX_ASSERT( texture->ref_count() > 0 );
        
        if ( texture->ref_count() == 1 ) { // 不需要使用的纹理可以删除
          texture->m_use_count = 0;
          texture->unload();
          _inl_pool(this)->del_texture_for_pool(texture);
          i.value() = nullptr;
          del_mark = true;
          m_textures.mark(i);
        } else if ( texture->is_ready() ) {
          
          auto it = sort_textures.end();
          
          for ( auto& j : sort_textures ) {
            if ( texture->m_use_count <= j.value()->m_use_count ) {
              it = j; break;
            }
          }
          if ( it.is_null() ) {
            sort_textures.push(texture);
          } else {
            sort_textures.before(it, texture);
          }
          total_data_size += texture->data_size();
        }
      }
      
      if ( del_mark ) {
        m_textures.del_mark();
        _inl_pool(this)->trigger_change();
      }
      
      if ( sort_textures.length() ) {
        uint64 total_data_size_1_3 = total_data_size / 3;
        uint64 del_data_size = 0;
        
        // 从排序列表顶部开始卸载总容量的1/3,并置零纹理使用次数
        for ( auto& i : sort_textures ) {
          FileTexture* texture = i.value();
          if ( del_data_size < total_data_size_1_3 ) {
            texture->unload();
            del_data_size += texture->data_size();
          }
          texture->m_use_count = 0;
        }
        
        XX_DEBUG("Texture memory clear, %ld", del_data_size);
      }
    }
    // clear()
  }
  
};

FileTexture::~FileTexture() {
  XX_ASSERT(m_pool == nullptr);
  unload();
}

TexturePool::TexturePool(Draw* ctx)
: XX_INIT_EVENT(change)
, m_draw_ctx(ctx)
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

void TexturePool::unload_all() {
  for ( auto& i : m_textures ) {
    i.value()->unload();
    i.value()->m_use_count = 0;
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
  _inl_pool(this)->clear(full);
}

XX_END
