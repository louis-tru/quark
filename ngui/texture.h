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

#ifndef __ngui__texture__
#define __ngui__texture__

#include "base/map.h"
#include "base/event.h"
#include "base/string.h"
#include "base/handle.h"
#include "base/buffer.h"
#include "image-codec.h"
#include "value.h"

/**
 * @ns ngui
 */

XX_NS(ngui)

class Draw;
class FileTexture;
class TexturePool;

/**
 * @enum TextureStatus 纹理的状态标识
 */
enum TextureStatus {
  TEXTURE_STATUS_NO_LOADED = 0,
  TEXTURE_STATUS_LOADING,
  TEXTURE_STATUS_ERROR,     // trigger change event
  TEXTURE_STATUS_COMPLETE,  // trigger change event
};

/**
 * @class Texture
 */
class XX_EXPORT Texture: public Reference {
  XX_HIDDEN_ALL_COPY(Texture);
 public:
  typedef PixelData::Format PixelFormat;
  
  /**
   * @event onchange 纹理变化事件,比如尺寸了生了变化
   */
  XX_EVENT(onchange, Event<float, Texture>);

  /**
   * @constructor
   */
  Texture(uint slot = 0);
  
  /**
   * @destructor
   */
  virtual ~Texture();
  
  /**
   * @func create 通过图像数据创建一个新的纹理对像,如果成功返回纹理对像
   */
  static Texture* create(cPixelData& data, uint slot = 0);
  
  /**
   * @func create 通过mipmap图像数据创建一个新的纹理对像,如果成功返回纹理对像
   */
  static Texture* create(const Array<PixelData>& data, uint slot = 0);
  
  /**
   * @func name 纹理的名称
   */
  virtual String name() const;
  
  /**
   * @func load_mipmap 通过数据载入mipmap纹理到GPU,如果成功返回true
   *                   如果GLDraw 没有开启mipmap不会载入mip纹理
   */
  bool load_mipmap(const Array<PixelData>& data);
  
  /**
   * @func load_data 通过数据载入纹理到GPU,如果成功返回true
   */
  bool load_data(cPixelData& data);
  
  /**
   * @func load 载入纹理数据到GUP
   */
  virtual void load() { /* noop */ }
  
  /**
   * @func unload 把GUP中的纹理释放掉,并不全删除纹理对像,会让纹理进入未准备状态
   *              只暂时释放内存资源,需要时可重新载入
   */
  virtual void unload();
    
  /**
   * @func use 加载并使用纹理返回true表示成功,如果当前没有ready会返回false
   *           但会调用load尝试加载纹理,载入成功后会触发change事件
   */
  virtual bool use(Repeat repeat = Repeat::NONE);
  
  /**
   * @func width
   */
  inline int width() const { return m_width; }
  
  /**
   * @func height
   */
  inline int height() const { return m_height; }
  
  /**
   * @func handle GPU中的纹理句柄
   */
  inline uint handle() const { return m_handle; }
  
  /**
   * @func is_ready 纹理数据是否已经准备好,
   *                如果准备好就表示GPU可以对它进行绘制,
   *                否则图形程序应该调用load函数,让其重新准备数据
   */
  inline bool is_ready() const { return m_status == TEXTURE_STATUS_COMPLETE; }

  /**
   * @func status 当前纹理的状态
   */
  inline TextureStatus status() const { return m_status; }
  
  /**
   * @func format 纹理的像素格式
   */
  inline PixelFormat format() const { return m_format; }
  
  /**
   * @func data_size 数据大小
   */
  inline uint data_size() const { return m_data_size; }
  
  /**
   * @func is_premultiplied_alpha 纹理是否对通道信息进行了预先处理
   *                              只有存在alpha通道的像素数据才有效.
   */
  inline bool is_premultiplied_alpha() const { return m_is_premultiplied_alpha; }
  
 protected:
  
  uint                m_handle;
  TextureStatus       m_status;
  uint                m_width;
  uint                m_height;
  uint                m_data_size;
  PixelFormat         m_format;
  bool                m_is_premultiplied_alpha;
  uint                m_slot;
  Repeat              m_repeat;
  
  friend class GLDraw;
  friend class GLES2Draw;
};

/**
 * @class TextureYUV
 */
class XX_EXPORT TextureYUV: public Texture {
 public:
  
  inline TextureYUV(): Texture(5) { }
  
  /**
   * @destructor
   */
  virtual ~TextureYUV();
  
  /**
   * @func load_yuv420
   */
  bool load_yuv(cPixelData& data);
  
  /**
   * @func uv_handle
   */
  inline uint uv_handle() const { return m_uv_handle; }
  
  /**
   * @overwrite
   */
  virtual bool use(Repeat repeat = Repeat::NONE);
  virtual void unload();
  
 private:
  
  uint m_uv_handle;
  
  friend class GLDraw;
  friend class GLES2Draw;
};

/**
 * @class FileTexture
 */
class XX_EXPORT FileTexture: public Texture {
 public:
  typedef ImageCodec::ImageFormat ImageFormat;
  
  /**
   * @destructor
   */
  virtual ~FileTexture();
  
  /**
   * @overwrite
   */
  virtual String name() const;
  virtual void load();
  virtual void unload();
  virtual bool use(Repeat repeat = Repeat::NONE);
  
  /**
   * @func image_format 返回纹理的原始路径中的格式
   */
  inline ImageFormat image_format() const { return m_image_format; }
  
 private:
  
  FileTexture(cString& path);
  
  String        m_path;
  uint          m_load_id;
  ImageFormat   m_image_format;
  TexturePool*  m_pool;
  uint64        m_use_count;
  
  friend class TexturePool;
};

/**
 * @struct TexturePoolEventData 纹理池事件数据
 */
struct TexturePoolEventData {
  float         progress;
  FileTexture*  texture;
};

typedef Event<TexturePoolEventData, TexturePool> TexturePoolEvent;

/**
 * @class TexturePool 统一管理纹理数据的池
 */
class XX_EXPORT TexturePool: public Object {
  XX_HIDDEN_ALL_COPY(TexturePool);
 public:
  
  /**
   * @event onchange 纹理载入变化事件
   */
  XX_EVENT(onchange, TexturePoolEvent);
  
  /**
   * @constructor
   */
  TexturePool(Draw* ctx);
  
  /**
   * @destructor
   */
  virtual ~TexturePool();
  
  /**
   * @func get_texture 通过本地或网络路径获取纹理对像
   */
  FileTexture* get_texture(cString& path);
  
  /**
   * @func load 通过路径列表批量加载纹理
   */
  void load(const Array<String>& paths);
  
  /**
   * @func load 载入当前池中所有未载入的纹理对像
   */
  void load_all();
  
  /**
   * @func unload 卸载池上所有纹理数据,并不删除纹理对像
   */
  void unload_all();
  
  /**
   * @func progress 当前载入完成的百分比
   */
  float progress() const;
  
  /**
   * @func clear
   */
  void clear(bool full = false);
  
 private:
  
  Draw* m_draw_ctx;
  Map<String, FileTexture*> m_textures;
  Map<PrtKey<Texture>, Texture*> m_completes;
  
  XX_DEFINE_INLINE_CLASS(Inl)
};

XX_END
#endif
