/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright (c) 2015, Louis.chu
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of Louis.chu nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL Louis.chu BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * ***** END LICENSE BLOCK ***** */

// @private head

#ifndef __quark_render_gl_command__
#define __quark_render_gl_command__

#include "./glsl_shaders.h"
#include "./gl_canvas.h"

namespace qk {

	typedef Canvas::V3F_T2F_C4B_C4B V3F_T2F_C4B_C4B;
	typedef Canvas::Triangles Triangles;

	class GLC_CmdPack {
		Qk_DISABLE_COPY(GLC_CmdPack);
	public:
		enum CmdType: uint8_t { // gl canvas cmd type
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
			kColorBatch_CmdType,
			kTriangles_CmdType,
			kReadImage_CmdType,
			kOutputImageBegin_CmdType,
			kOutputImageEnd_CmdType,
			kFlushCanvas_CmdType,
			kSetSurface_CmdType,
			kDrawBuffers_CmdType,
			kRestoreClip_CmdType,
			kCmdTypeCount,
		};

		struct PipelineState {
			Vec2 surfaceSize;
			Mat4 rootMatrix;
			Mat  matrix;
			BlendMode mode;
		};

		struct Cmd { // Cmd list
			uint32_t       size; // cmd size
			CmdType        type; // cmd type
			bool           isClip, isPMA; // is cmd with clip state, is premultiplied alpha
		};

		struct DrawCmd: Cmd { // draw base cmd
			VertexData     vertex;
			float          depth;
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
			GC_ClearFlags  flags; // 0-clear and blend, 1-clear color, 2-clear color/depth/stencil
		};

		struct alignas(void*) ClipCmd: DrawCmd { //!
			VertexData      aadist;
			Sp<GC_State::Clip> lastClip;
			Sp<GC_State::Clip> clip;
			Sp<ImageSource> recover;
			float           surfaceScale;
			Canvas::ClipOp  rawOp;
		};

		struct alignas(void*) BlurFilterBeginCmd: Cmd {
			float           depth;
			Range           bounds;
			Mat4            blurRootMatrix;
			Sp<ImageSource> tmpA; // temporary blur textures
		};

		struct alignas(void*) BlurFilterEndCmd: Cmd {
			float           depth;
			Range           bounds;
			float           radius, clearPad; // blur radius, clear padding for blur edge
			float           surfaceScale; // canvas surface scale
			Vec2            surfaceSize; // canvas surface size
			Mat4            rootMatrix; // recover root matrix
			int             sample,imageLod; // sample size and image lod
			BlendMode       recoverMode; // recover blend mode
			Sp<ImageSource> recover; // recover output dest
			Sp<ImageSource> tmpA, tmpB; // temporary blur textures
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

		struct alignas(void*) ColorBatchCmd: Cmd {
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
		};

		struct alignas(void*) TrianglesCmd: Cmd {
			Triangles      triangles;
			PaintImage     paint;
			float          depth;
			Color4f        color;
			bool           copyData;
			~TrianglesCmd();
		};

		struct alignas(void*) ReadImageCmd: Cmd {
			Rect            srcRect;
			Sp<ImageSource> src;
			Sp<ImageSource> dest;
			Vec2            canvasSize;
			Vec2            surfaceSize;
			float           depth;
		};

		struct alignas(void*) OutputImageBeginCmd: Cmd {
			Sp<ImageSource> dst;
		};

		struct alignas(void*) OutputImageEndCmd: Cmd {
			Sp<ImageSource> exit;
			Sp<ImageSource> next;
		};

		struct alignas(void*) FlushCanvasCmd: Cmd {
			GLCanvas        *srcC;
			GLC_CmdPack     *srcCmdPack;
			PipelineState   restoreState;
			~FlushCanvasCmd();
		};

		struct alignas(void*) SetSurfaceCmd: Cmd {
			Vec2            surfaceSize;
			Mat4            rootMatrix;
			bool            changeSize;
		};

		struct alignas(void*) DrawBuffersCmd: Cmd {
			GLsizei num;
			GLenum  buffers[2];
		};

		struct alignas(void*) RestoreClipCmd: Cmd {
			Sp<GC_State::Clip> clip;
		};

		GLC_CmdPack(GLRender *render, GLCanvas *canvas);
		~GLC_CmdPack();
		bool isEmpty();
		void flush();
		void setMatrix();
		void setBlendMode();
		void switchState(GLenum id, bool isEnable); // call glEnable or glDisable
		void drawColor(const VertexData &vertex, const Color4f &color); // add cmd
		void drawRRectBlurColor(const Rect& rect, const float *radius, float blur, const Color4f &color);
		void drawImage(const VertexData &vertex, const PaintImage *paint, const Color4f &color);
		void drawImageMask(const VertexData &vertex, const PaintImage *paint, const Color4f &color);
		void drawSDFImageMask(const VertexData &vertex, const PaintImage *paint, const Color4f &color,
				const Color4f &strokeColor, float stroke);
		void drawTriangles(const Triangles& triangles, const PaintImage *paint, const Color4f &color, bool copyData);
		void drawGradient(const VertexData &vertex, const PaintGradient *paint, const Color4f &color);
		void drawClip(const VertexData &vertex, const VertexData &aadist, GC_State::Clip *lastClip,
				GC_State::Clip *clip, Canvas::ClipOp rawOp);
		void clearColor(const Color4f &color, GC_ClearFlags flags);
		void blurFilterBegin(Range bounds, Mat4 &rootMat, ImageSource *tmpA);
		void blurFilterEnd(Range bounds, float radius, float clearPad, int sample, int imageLod,
				ImageSource *tmpA, ImageSource *tmpB);
		void readImage(const Rect &srcRect, ImageSource* src, ImageSource* dst);
		void outputImageBegin(ImageSource* dst);
		void outputImageEnd(ImageSource* exit, ImageSource* next);
		void setSurface(bool changeSize);
		void drawBuffers(GLsizei num, const GLenum buffers[2]);
		void savePipelineState();
		void restoreClip(GC_State::Clip *clip);
	private:
		inline Color4f premul_alpha(const Color4f &color) const;
		typedef ColorBatchCmd::Option CGOpt;
		template<class T> struct MemBlock {
			T *val; uint32_t size,capacity;
		};
		template<class T> struct MemBlockArray {
			Array<MemBlock<T>> blocks;
			MemBlock<T>        *currentBlock;
			uint32_t           index;
		};
		MemBlockArray<Cmd>   _cmds; // cmd queue
		MemBlockArray<Vec4>  _vertexBlocks; // vertex storage
		MemBlockArray<CGOpt> _optionBlocks; //
		Cmd                  *_lastCmd;
		GLRender             *_render;
		GLCanvas             *_canvas;
		PathvCache           *_cache;
		PipelineState        _pipelineState; // current state

		Qk_DEFINE_INLINE_CLASS(Inl);
	};

}

#endif
