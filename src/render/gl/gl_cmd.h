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

#include "./glsl_shaders.h"
#include "./gl_canvas.h"

#define Qk_USE_GLC_CMD_QUEUE 0

namespace qk {

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
			kColor_CmdType, // draw cmd
			kImage_CmdType,
			kImageMask_CmdType,
			kGradient_CmdType,
			kMultiColor_CmdType,
		};

		struct Cmd { // Cmd list
			CmdType        type;
			uint32_t       size; // cmd size
		};
		struct DrawCmd: Cmd {
			VertexData     vertex;
			float          depth;
		};
		struct MatrixCmd: Cmd {
			Mat            matrix;
		};
		struct BlendCmd: Cmd {
			BlendMode      mode;
		};

		struct ClearCmd: Cmd {
			float          depth;
			Color4f        color;
			bool           isBlend;
		};

		struct SwitchCmd: Cmd {
			GLenum         id;
			bool           value;
		};

		struct ClipCmd: Cmd { //!
			GLC_State::Clip clip;
			float          depth;
			uint32_t       ref;
			bool           revoke;
		};

		struct ColorCmd: DrawCmd { //!
			Color4f        color;
		};

		struct GradientCmd: DrawCmd { //!
			float          alpha;
			GradientPaint  paint;
		};

		struct ImageCmd: DrawCmd { //!
			float          alpha;
			ImagePaint     paint; // rgb or y, u of yuv420p or uv of yuv420sp, v of yuv420p
			~ImageCmd();
		};

		struct ImageMaskCmd: DrawCmd { //!
			Color4f        color;
			ImagePaint     paint;
			~ImageMaskCmd();
		};

		struct MultiColorCmd: Cmd {
			struct Option { // subcmd option
				int          flags; // reserve
				float        depth; // depth
				Mat          matrix; // 2d mat2x3
				Color4f      color;  // color
			};
			Vec4           *vertex; // vertex + option index
			Option         *opts;  // subcmd option
			uint32_t       vCount; // vertex count
			int            subcmd; // subcmd count
		};
		GLC_CmdPack(GLRender *render, GLCanvas *canvas);
		~GLC_CmdPack();
		Cmd*           allocCmd(uint32_t size);
		MultiColorCmd* newMColorCmd();
		MultiColorCmd* getMColorCmd();
		void flush();
		void setMetrixUnifromBuffer(const Mat &mat);
		void setBlendMode(BlendMode mode);
		void switchState(GLenum id, bool value);
		void drawColor4f(const VertexData &vertex, const Color4f &color); // add cmd
		void drawImage(const VertexData &vertex, const ImagePaint *paint, float alpha);
		void drawImageMask(const VertexData &vertex, const ImagePaint *paint, const Color4f &color);
		void drawGradient(const VertexData &vertex, const GradientPaint *paint, float alpha);
		void drawClip(const GLC_State::Clip &clip, uint32_t ref, bool revoke);
		void clearColor4f(const Color4f &color, bool isBlend);
	private:
		void switchStateCall(GLenum id, bool value);
		void drawColor4fCall(float depth, const VertexData &vertex, const Color4f &color); // call gl api
		void drawImageCall(float depth, const VertexData &vertex, const ImagePaint *paint, float alpha);
		void drawImageMaskCall(float depth, const VertexData &vertex, const ImagePaint *paint, const Color4f &color);
		void drawGradientCall(float depth, const VertexData &vertex, const GradientPaint *paint, float alpha);
		void drawClipCall(float depth, const GLC_State::Clip &clip, uint32_t ref, bool revoke);
		void clearColor4fCall(float depth, const Color4f &color, bool isBlend);
		void clear(); //
		void checkMetrix();
		void useShader(GLSLShader *shader, const VertexData &vertex);

		typedef MultiColorCmd::Option MCOpt;
		template<class T>
		struct MemBlock {
			T *val; uint32_t size,capacity;
		};
		template<class T>
		struct ArrayMemBlock {
			Array<MemBlock<T>> blocks;
			MemBlock<T>       *current;
			uint32_t           index;
		};
		ArrayMemBlock<Vec4>  vertexBlocks; // vertex storage
		ArrayMemBlock<MCOpt> optionBlocks; //
		MemBlock<Cmd>        cmds; // cmd queue
		Cmd                  *lastCmd;
		GLRender             *_render;
		GLCanvas             *_canvas;
		PathvCache           *_cache;
	};

}

#endif