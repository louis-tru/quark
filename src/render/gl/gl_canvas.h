/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright (c) 2015, blue.chu
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of blue.chu nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL blue.chu BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * ***** END LICENSE BLOCK ***** */

// @private head

#ifndef __quark_render_gl_glcanvas__
#define __quark_render_gl_glcanvas__

#include "../canvas.h"
#include "./glsl_shaders.h"

namespace qk {

	struct GLC_State { // gl canvas state
		struct Clip { // gl canvas clip
			Pathv           path;
			Canvas::ClipOp  op;
			bool            aa; // anti alias
		};
		Mat         matrix;
		Array<Clip> clips;
	};

	enum GLC_CmdType { // gl canvas cmd type
		kMatrix_GLC_CmdType,
		kBlendMode_GLC_CmdType,
		kClip_GLC_CmdType,
		kRestore_GLC_CmdType,
		kImage_GLC_CmdType,
		kRadial_GLC_CmdType,
		kLinear_GLC_CmdType,
		kGenerice_GLC_CmdType,
	};

	struct GLC_Cmd: public Object {
		GLC_CmdType            type;
	};

	struct GLC_DrawCmd: GLC_Cmd {
		Array<Vec3>            vertex;
		float                  zDepth;
	};

	struct GLC_ImageCmd: GLC_DrawCmd {
		enum Format {
			kRGB_Format, // 1 texure
			kYUV420SP_Format, // 2 texure
			kYUV420P_Format, // 3 texure
		};
		Region          coord;/*offset,scale*/
		float           opacity;
		Format          format;
		Sp<ImageSource> image; // rgb or y, u of yuv420p or uv of yuv420sp, v of yuv420p
	};

	struct GLC_GradientCmd: GLC_DrawCmd {
		Vec2           range[2];
		int            count;
		float          opacity;
		Array<Color4f> colors;
		Array<float>   positions;
	};

	struct GLC_GenericeCmd: GLC_DrawCmd {
		struct Option {
			enum Type {
				kColor,kColorMask,kImage
			};
			int     flags; // type: flags & 3, image: flags >> 2 sampler2D index
			float   zDepth; // depth
			Mat     matrix; // 2d mat2x3
			Color4f color;  // color
			Region  coord;  // image coord, offset,scale
		};
		Array<int>             optidx; // vertex option index
		Array<Option>          options; // vertex options data
		Array<Sp<ImageSource>> images; // image source
	};

	class GLRender; // gl render backend
	class GLCanvas: public Canvas {
	public:
		GLCanvas(GLRender *render);
		virtual ~GLCanvas();
		virtual int  save() override;
		virtual void restore(uint32_t count) override;
		virtual int  getSaveCount() const override;
		virtual const Mat& getMatrix() const override;
		virtual void setMatrix(const Mat& mat) override;
		virtual void translate(float x, float y) override;
		virtual void scale(float x, float y) override;
		virtual void rotate(float z) override;
		virtual bool readPixels(Pixel* dst, uint32_t srcX, uint32_t srcY) override;
		virtual void clipPath(const Path& path, ClipOp op, bool antiAlias) override;
		virtual void clipPathv(const Pathv& path, ClipOp op, bool antiAlias) override;
		virtual void clipRect(const Rect& rect, ClipOp op, bool antiAlias) override;
		virtual void clearColor(const Color4f& color) override;
		virtual void drawColor(const Color4f& color, BlendMode mode) override;
		virtual void drawPath(const Path& path, const Paint& paint) override;
		virtual void drawPathv(const Pathv& path, const Paint& paint) override;
		virtual void drawPathvColor(const Pathv& path, const Color4f &color, BlendMode mode) override;
		virtual void drawRect(const Rect& rect, const Paint& paint) override;
		virtual void drawRRect(const Rect& rect,
			const Path::BorderRadius &radius, const Paint& paint) override;
		virtual float drawGlyphs(const FontGlyphs &glyphs,
			Vec2 origin, const Array<Vec2> *offset, const Paint &paint) override;
		virtual void drawTextBlob(TextBlob *blob, Vec2 origin, float fontSize, const Paint &paint) override;
		void setRootMatrix(const Mat4& root, Vec2 surfaceScale); // set root matrix
	private:
		// define props
		Array<GLC_State> _stateStack;
		GLRender     *_render;
		GLC_State    *_state;
		Array<Sp<GLC_Cmd>> _drawCmds;
		GLuint _stencilRef, _stencilRefDecr;
		float  _surfaceScale, _transfromScale;
		float  _scale, _unitPixel; // surface scale * transfrom scale, _unitPixel = 2 / _scale
		Qk_DEFINE_INLINE_CLASS(Inl);
	};

}
#endif
