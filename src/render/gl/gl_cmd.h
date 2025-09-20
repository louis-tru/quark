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

#ifndef __quark_render_gl_cmd__
#define __quark_render_gl_cmd__

#define Qk_USE_GLC_CMD_QUEUE 1

#include "./glsl_shaders.h"
#include "./gl_canvas.h"

namespace qk {

	typedef Canvas::V3F_T2F_C4B_C4B V3F_T2F_C4B_C4B;
	typedef Canvas::Triangles Triangles;

	constexpr bool isAlignUp(uint32_t ptr) {
		constexpr auto alignment = alignof(void*);
		return ((ptr + (alignment - 1)) & ~(alignment - 1)) == ptr;
	}

	class GLC_CmdPack {
		Qk_DISABLE_COPY(GLC_CmdPack);
	public:
		enum CmdType { // gl canvas cmd type
			kEmpty_CmdType, // empty cmd
			kMatrix_CmdType,
			kBlend_CmdType,
			kSwitch_CmdType,
			kClear_CmdType,
			kClip_CmdType,
			kBlurFilterBegin_CmdType,
			kBlurFilterEnd_CmdType,
			kColor_CmdType, // draw cmd
			kRRectBlurColor_CmdType,
			kImage_CmdType,
			kImageMask_CmdType,
			kSDFImageMask_CmdType,
			kGradient_CmdType,
			kColors_CmdType,
			kTriangles_CmdType,
			kReadImage_CmdType,
			kOutputImageBegin_CmdType,
			kOutputImageEnd_CmdType,
			kFlushCanvas_CmdType,
			kSetBuffers_CmdType,
			kDrawBuffers_CmdType,
		};

		struct Cmd { // Cmd list
			CmdType        type;
			uint32_t       size; // cmd size
		};

		struct DrawCmd: Cmd {
			VertexData     vertex;
			float          depth;
			bool           aafuzz,aaclip;
		};

		struct alignas(void*) MatrixCmd: Cmd {
			Mat            matrix;
		};

		struct alignas(void*) BlendCmd: Cmd {
			BlendMode      mode;
		};

		struct alignas(void*) SwitchCmd: Cmd {
			GLenum         id;
			bool           isEnable;
		};

		struct alignas(void*) ClearCmd: Cmd {
			float          depth;
			Color4f        color;
			Region         region;
			bool           fullClear;
		};

		struct alignas(void*) ClipCmd: Cmd { //!
			GLC_State::Clip clip;
			float          depth;
			uint32_t       ref;
			Sp<ImageSource> recover;
			bool           revoke;
		};

		struct alignas(void*) BlurFilterBeginCmd: Cmd {
			float           depth;
			Region          bounds;
			float           size; // blur size
			bool            isClipState;
		};

		struct alignas(void*) BlurFilterEndCmd: Cmd {
			float           depth;
			Region          bounds;
			float           size; // blur size
			Sp<ImageSource> recover; // recover output dest
			int             n,lod; // sampling rate and image lod
			BlendMode       backMode;
			bool            isClipState;
		};

		struct alignas(void*) ColorCmd: DrawCmd { //!
			Color4f    color;
		};

		struct alignas(void*) ColorRRectBlurCmd: Cmd { //!
			float      depth;
			Rect       rect;
			float      radius[4];
			Color4f    color;
			float      blur;
			bool       aaclip;
		};

		struct alignas(void*) GradientCmd: DrawCmd { //!
			Color4f        color;
			PaintGradient  paint;
		};

		struct alignas(void*) ImageCmd: DrawCmd { //!
			float          allScale;
			Color4f        color;
			PaintImage     paint; // rgb or y, u of yuv420p or uv of yuv420sp, v of yuv420p
			~ImageCmd();
		};

		struct alignas(void*) ImageMaskCmd: DrawCmd { //!
			float          allScale;
			Color4f        color;
			PaintImage     paint;
			~ImageMaskCmd();
		};

		struct alignas(void*) SDFImageMaskCmd: ImageMaskCmd { //!
			Color4f        strokeColor;
			float          strokeWidth;
		};

		struct alignas(void*) ColorsCmd: Cmd {
			struct Option { // subcmd option
				int          flags; // reserve
				float        depth; // depth
				Mat          matrix; // 2d mat2x3
				Color4f      color;  // color
			}; // 48b
			Vec4           *vertex; // vertex + option index
			Option         *opts;  // subcmd option
			uint32_t       vCount; // vertex count
			int            subcmd; // subcmd count
			bool           aaclip;
		};

		struct alignas(void*) TrianglesCmd: Cmd {
			Triangles      triangles;
			PaintImage     paint;
			float          depth;
			Color4f        color;
			bool           aaclip;
			~TrianglesCmd();
		};

		struct alignas(void*) ReadImageCmd: Cmd {
			Rect            src;
			Sp<ImageSource> img;
			Vec2            canvasSize;
			Vec2            surfaceSize;
			float           depth;
		};

		struct alignas(void*) OutputImageBeginCmd: Cmd {
			Sp<ImageSource> img;
		};

		struct alignas(void*) OutputImageEndCmd: Cmd {
			Sp<ImageSource> img;
		};

		struct alignas(void*) FlushCanvasCmd: Cmd {
			GLCanvas        *srcC;
			GLC_CmdPack     *srcCmd;
			Mat4            root;
			Mat             mat;
			BlendMode       mode;
			~FlushCanvasCmd();
		};

		struct alignas(void*) SetBuffersCmd: Cmd {
			Vec2 size;
			Sp<ImageSource> recover;
			bool chSize;
		};

		struct alignas(void*) DrawBuffersCmd: Cmd {
			GLsizei num;
			GLenum  buffers[2];
		};

		GLC_CmdPack(GLRender *render, GLCanvas *canvas);
		~GLC_CmdPack();
		bool isHaveCmds();
		void flush();
		void setMatrix();
		void setBlendMode(BlendMode mode);
		void switchState(GLenum id, bool isEnable); // call glEnable or glDisable
		void drawColor(const VertexData &vertex, const Color4f &color, bool aafuzz); // add cmd
		void drawRRectBlurColor(const Rect& rect, const float *radius, float blur, const Color4f &color);
		void drawImage(const VertexData &vertex, const PaintImage *paint, const Color4f &color, bool aafuzz);
		void drawImageMask(const VertexData &vertex, const PaintImage *paint, const Color4f &color, bool aafuzz);
		void drawSDFImageMask(const VertexData &vertex, const PaintImage *paint, const Color4f &color,
				const Color4f &strokeColor, float stroke, bool aafuzz);
		void drawTriangles(const Triangles& triangles, const PaintImage *paint, const Color4f &color);
		void drawGradient(const VertexData &vertex, const PaintGradient *paint, const Color4f &color, bool aafuzz);
		void drawClip(const GLC_State::Clip &clip, uint32_t ref, ImageSource *recover, bool revoke);
		void clearColor(const Color4f &color, const Region &region, bool fullClear);
		void blurFilterBegin(Region bounds, float size);
		int  blurFilterEnd(Region bounds, float size, ImageSource* recover);
		void readImage(const Rect &src, ImageSource* img);
		void outputImageBegin(ImageSource* img);
		void outputImageEnd(ImageSource* img);
		void setBuffers(Vec2 size, ImageSource *recover, bool chSize);
		void drawBuffers(GLsizei num, const GLenum buffers[2]);
	private:
		typedef ColorsCmd::Option CGOpt;
		template<class T> struct MemBlock {
			T *val; uint32_t size,capacity;
		};
		template<class T> struct MemBlockArray {
			Array<MemBlock<T>> blocks;
			MemBlock<T>        *current;
			uint32_t           index;
		};
		MemBlockArray<Cmd>   _cmds; // cmd queue
		MemBlockArray<Vec4>  _vertexBlocks; // vertex storage
		MemBlockArray<CGOpt> _optionBlocks; //
		Cmd                  *_lastCmd;
		GLRender             *_render;
		GLCanvas             *_canvas;
		PathvCache           *_cache;

		Qk_DEFINE_INLINE_CLASS(Inl);
	};

}

#endif
