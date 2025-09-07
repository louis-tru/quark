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

	class GLC_CmdPack {
		Qk_HIDDEN_ALL_COPY(GLC_CmdPack);
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

		struct MatrixCmd: Cmd {
			Mat            matrix;
		};

		struct BlendCmd: Cmd {
			BlendMode      mode;
		};

		struct SwitchCmd: Cmd {
			GLenum         id;
			bool           isEnable;
		};

		struct ClearCmd: Cmd {
			float          depth;
			Color4f        color;
			Region         region;
			bool           fullClear;
		};

		struct ClipCmd: Cmd { //!
			GLC_State::Clip clip;
			float          depth;
			uint32_t       ref;
			Sp<ImageSource> recover;
			bool           revoke;
		};

		struct BlurFilterBeginCmd: Cmd {
			float           depth;
			Region          bounds;
			float           size; // blur size
			bool            isClipState;
		};

		struct BlurFilterEndCmd: Cmd {
			float           depth;
			Region          bounds;
			float           size; // blur size
			Sp<ImageSource> recover; // recover output dest
			int             n,lod; // sampling rate and image lod
			BlendMode       backMode;
			bool            isClipState;
		};

		struct ColorCmd: DrawCmd { //!
			Color4f    color;
		};

		struct ColorRRectBlurCmd: Cmd { //!
			float      depth;
			Rect       rect;
			float      radius[4];
			Color4f    color;
			float      blur;
			bool       aaclip;
		};

		struct GradientCmd: DrawCmd { //!
			float          alpha;
			GradientPaint  paint;
		};

		struct ImageCmd: DrawCmd { //!
			float          allScale;
			float          alpha;
			ImagePaint     paint; // rgb or y, u of yuv420p or uv of yuv420sp, v of yuv420p
			~ImageCmd();
		};

		struct ImageMaskCmd: DrawCmd { //!
			float          allScale;
			Color4f        color;
			ImagePaint     paint;
			~ImageMaskCmd();
		};

		struct ColorsCmd: Cmd {
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

		struct TrianglesCmd: Cmd {
			Triangles      triangles;
			ImagePaint     paint;
			float          depth;
			bool           aaclip;
			~TrianglesCmd();
		};

		struct ReadImageCmd: Cmd {
			Rect            src;
			Sp<ImageSource> img;
			Vec2            canvasSize;
			Vec2            surfaceSize;
			float           depth;
		};

		struct OutputImageBeginCmd: Cmd {
			Sp<ImageSource> img;
		};

		struct OutputImageEndCmd: Cmd {
			Sp<ImageSource> img;
		};

		struct FlushCanvasCmd: Cmd {
			GLCanvas        *srcC;
			GLC_CmdPack     *srcCmd;
			Mat4            root;
			Mat             mat;
			BlendMode       mode;
			~FlushCanvasCmd();
		};

		struct SetBuffersCmd: Cmd {
			Vec2 size;
			Sp<ImageSource> recover;
			bool chSize;
		};

		struct DrawBuffersCmd: Cmd {
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
		void drawImage(const VertexData &vertex, const ImagePaint *paint, float alpha, bool aafuzz);
		void drawImageMask(const VertexData &vertex, const ImagePaint *paint, const Color4f &color, bool aafuzz);
		void drawTriangles(const Triangles& triangles, const ImagePaint *paint);
		void drawGradient(const VertexData &vertex, const GradientPaint *paint, float alpha, bool aafuzz);
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
