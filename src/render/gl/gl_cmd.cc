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


#include "./gl_cmd.h"

namespace qk {

#if Qk_GL_USE_CMD_PACK

	GLC_ImageCmd::~GLC_ImageCmd() {
		paint.source->release();
	}

	GLC_ImageMaskCmd::~GLC_ImageMaskCmd() {
		paint.source->release();
	}

	GLC_CmdPack::GLC_CmdPack()
	{
		cmds.size = sizeof(GLC_Cmd);
		cmds.capacity = 65536;
		cmds.val = (GLC_Cmd*)malloc(65536);
		cmds.val->size = cmds.size;
		cmds.val->type = kEmpty_GLC_CmdType;
		lastCmd = cmds.val;
		vertexBlocks.blocks.push({
			(Vec4*)malloc(Qk_MCCmd_VertexBlock_Capacity * sizeof(Vec4)),0,Qk_MCCmd_VertexBlock_Capacity
		});
		optionBlocks.blocks.push({
			(MCOpt*)malloc(Qk_MCCmd_OptBlock_Capacity * sizeof(MCOpt)),0,Qk_MCCmd_OptBlock_Capacity
		});
		vertexBlocks.current = vertexBlocks.blocks.val();
		vertexBlocks.index = 0;
		optionBlocks.current = optionBlocks.blocks.val();
		optionBlocks.index = 0;
	}

	GLC_CmdPack::~GLC_CmdPack() {
		for (auto &i: vertexBlocks.blocks)
			free(i.val);
		for (auto &i: optionBlocks.blocks)
			free(i.val);
		clear();
		free(cmds.val);
	}

	void GLC_CmdPack::clear() {
		auto cmd = cmds.val;

		while (cmd <= lastCmd) {
			switch (cmd->type) {
				case kClear_GLC_CmdType:
					((GLC_ClearCmd*)cmd)->~GLC_ClearCmd();
					break;
				case kClip_GLC_CmdType:
					((GLC_ClipCmd*)cmd)->~GLC_ClipCmd();
					break;
				case kColor_GLC_CmdType:
					((GLC_ColorCmd*)cmd)->~GLC_ColorCmd();
					break;
				case kImage_GLC_CmdType:
					((GLC_ImageCmd*)cmd)->~GLC_ImageCmd();
					break;
				case kImageMask_GLC_CmdType:
					((GLC_ImageMaskCmd*)cmd)->~GLC_ImageMaskCmd();
					break;
				case kGradient_GLC_CmdType:
					((GLC_GradientCmd*)cmd)->~GLC_GradientCmd();
					break;
				default: break;
			}
			cmd = (GLC_Cmd*)(((char*)cmd) + cmd->size);
		}

		lastCmd = cmds.val;
		cmds.size = lastCmd->size;
	}

	GLC_Cmd* GLC_CmdPack::allocCmd(uint32_t size) {
		auto newSize = cmds.size + size;
		if (newSize > cmds.capacity) {
			cmds.capacity <<= 1;
			cmds.val = (GLC_Cmd*)realloc(cmds.val, cmds.capacity);
		}
		lastCmd = (GLC_Cmd*)(((char*)cmds.val) + cmds.size);
		lastCmd->size = size;
		cmds.size = newSize;
		return lastCmd;
	}

	GLC_MultiColorCmd* GLC_CmdPack::newMColorCmd() {
		auto cmd = (GLC_MultiColorCmd*)allocCmd(sizeof(GLC_MultiColorCmd));
		cmd->type = kMultiColor_GLC_CmdType;

		auto vertexs = vertexBlocks.current;
		auto opts    = optionBlocks.current;

		if (vertexs->size == Qk_MCCmd_VertexBlock_Capacity) {
			if (++vertexBlocks.index == vertexBlocks.blocks.length()) {
				vertexBlocks.blocks.push({
					(Vec4*)malloc(Qk_MCCmd_VertexBlock_Capacity * sizeof(Vec4)),0,Qk_MCCmd_VertexBlock_Capacity
				});
			}
			vertexBlocks.current = vertexs = vertexBlocks.blocks.val() + vertexBlocks.index;
		}

		if (opts->size == Qk_MCCmd_OptBlock_Capacity) {
			if (++optionBlocks.index == optionBlocks.blocks.length()) {
				optionBlocks.blocks.push({
					(MCOpt*)malloc(Qk_MCCmd_OptBlock_Capacity * sizeof(MCOpt)),0,Qk_MCCmd_OptBlock_Capacity
				});
			}
			optionBlocks.current = opts = optionBlocks.blocks.val() + optionBlocks.index;
		}

		cmd->vertex = vertexs->val + vertexs->size;
		cmd->opts    = opts->val    + opts->size;
		cmd->subcmd = 0;
		cmd->vCount = 0;

		return cmd;
	}

	GLC_MultiColorCmd* GLC_CmdPack::getMColorCmd() {
		auto cmd = (GLC_MultiColorCmd*)lastCmd;
		if (cmd->type == kMultiColor_GLC_CmdType) {
			if (vertexBlocks.current->size != Qk_MCCmd_VertexBlock_Capacity &&
					optionBlocks.current->size != Qk_MCCmd_OptBlock_Capacity &&
					cmd->subcmd != Qk_MCCmd_Option_Capacity
			) {
				return cmd;
			}
		}
		return newMColorCmd();
	}

#endif


}