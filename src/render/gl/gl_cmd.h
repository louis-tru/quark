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

#include "../canvas.h"

#define Qk_GL_USE_CMD_PACK 1
#define Qk_MCCmd_Option_Capacity 1024
#define Qk_MCCmd_VertexBlock_Capacity 65535
#define Qk_MCCmd_OptBlock_Capacity 16384

namespace qk {
	class GLRender; // gl render backend
	class GLCanvas;

	struct GLC_State { // gl canvas state
		struct Clip { // gl canvas clip
			Pathv           path;
			Canvas::ClipOp  op;
			bool            aa; // anti alias
		};
		Mat         matrix;
		Array<Clip> clips;
	};

#if Qk_GL_USE_CMD_PACK
	// -------------------------------------------------------------

	enum GLC_CmdType { // gl canvas cmd type
		kEmpty_GLC_CmdType, // empty cmd
		kMatrix_GLC_CmdType,
		kBlend_GLC_CmdType,
		kClear_GLC_CmdType,
		kClip_GLC_CmdType,
		kColor_GLC_CmdType, // draw cmd
		kImage_GLC_CmdType,
		kImageMask_GLC_CmdType,
		kGradient_GLC_CmdType,
		kMultiColor_GLC_CmdType,
	};

	struct GLC_Cmd { // Cmd list
		GLC_CmdType type;
		uint32_t    size; // cmd size
	};
	struct GLC_DrawCmd: GLC_Cmd { Array<Vec3> vertex; float depth; };
	struct GLC_MatrixCmd: GLC_Cmd { Mat matrix; };
	struct GLC_BlendCmd: GLC_Cmd { BlendMode mode; };

	struct GLC_ClearCmd: GLC_Cmd {
		float          depth;
		Color4f        color;
		bool           isBlend;
	};

	struct GLC_ClipCmd: GLC_Cmd {
		float          depth;
		bool           revoke;
		GLC_State::Clip clip;
	};

	struct GLC_ColorCmd: GLC_DrawCmd {
		Color4f        color;
	};

	struct GLC_GradientCmd: GLC_DrawCmd {
		float          alpha;
		GradientPaint  paint;
	};

	struct GLC_ImageCmd: GLC_DrawCmd {
		float          alpha;
		ImagePaint     paint; // rgb or y, u of yuv420p or uv of yuv420sp, v of yuv420p
		~GLC_ImageCmd();
	};

	struct GLC_ImageMaskCmd: GLC_DrawCmd {
		Color4f        color;
		ImagePaint     paint;
		~GLC_ImageMaskCmd();
	};

	struct GLC_MultiColorCmd: GLC_Cmd {
		struct Option { // subcmd option
			int     flags; // reserve
			float   depth; // depth
			Mat     matrix; // 2d mat2x3
			Color4f color;  // color
		};
		Vec4           *vertex; // vertex + option index
		Option         *opts;  // subcmd option
		uint32_t       vCount; // vertex count
		int            subcmd; // subcmd count
	};

	struct GLC_CmdPack {
		typedef GLC_MultiColorCmd::Option MCOpt;
		template<class T>
		struct MemBlock {
			T       *val;
			uint32_t size,capacity;
		};
		template<class T>
		struct ArrayMemBlock {
			Array<MemBlock<T>> blocks;
			MemBlock<T>       *current;
			uint32_t           index;
		};
		ArrayMemBlock<Vec4>  vertexBlocks; // vertex storage
		ArrayMemBlock<MCOpt> optionBlocks; //
		MemBlock<GLC_Cmd> cmds;
		GLC_Cmd          *lastCmd;
						GLC_CmdPack();
						~GLC_CmdPack();
		GLC_Cmd* allocCmd(uint32_t size);
		GLC_MultiColorCmd* newMColorCmd();
		GLC_MultiColorCmd* getMColorCmd();
		void     clear();
	};

	// -------------------------------------------------------------
#endif

}

#endif
