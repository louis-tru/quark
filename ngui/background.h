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

#ifndef __ngui__background__
#define __ngui__background__

#include "box.h"

XX_NS(ngui)

/**
 * @class Background
 */
class Background: public Reference {
 public:
  
  enum Type {
    M_INVALID,
    M_CLOLR,
    M_IMAGE,
    M_GRADIENT,
  };
  
  Background();
  
  /**
   * @destructor
   */
  virtual ~Background();
  
  /**
   * @func next()
   */
  inline Background* next() { return m_next; }
  
  /**
   * @func set_next(value)
   */
  void set_next(Background* value) throw(Error);
  
  /**
   * @func type()
   */
  virtual Type type() const { return M_INVALID; }
  
 protected:
  
  /**
   * @func mark_value()
   */
  void mark(uint mark_value);
  
  /**
   * @func set_host(host)
   */
  void set_host(Box* host);
  
  /**
   * @func draw(draw)
   */
  virtual void draw(Draw* draw, Box* host) = 0;
  
  Background* m_next;
  Box*        m_host;
  
  friend class Box;
  friend class GLDraw;
};

/**
 * @class BackgroundColor
 */
class BackgroundColor: public Background {
 public:
  BackgroundColor();
  virtual Type type() const { return M_CLOLR; }
  inline Color color() const { return m_color; }
  void set_color(Color value);
 protected:
  virtual void draw(Draw* draw, Box* host);
 private:
  Color m_color;
  friend class GLDraw;
};

/**
 * @class BackgroundImage
 */
class BackgroundImage: public Background {
 public:
  BackgroundImage();
  virtual ~BackgroundImage();
  virtual Type type() const { return M_IMAGE; }
  inline Texture* texture() { return m_texture; }
  void set_texture(Texture* value);
 protected:
  virtual void draw(Draw* draw, Box* host);
 private:
  int       m_tex_level;
  Texture*  m_texture;
  XX_DEFINE_INLINE_CLASS(Inl);
  friend class GLDraw;
};

/**
 * @class BackgroundGradient
 */
class BackgroundGradient: public Background {
 public:
  BackgroundGradient();
  virtual Type type() const { return M_GRADIENT; }
 protected:
  virtual void draw(Draw* draw, Box* host);
  friend class GLDraw;
};

XX_END

#endif
