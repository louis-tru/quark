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

#include "./vk_canvas.h"
#include "./vk_render.h"
#include "../source.h"

#define Qk_useTexture0(paint, dstSlot, ...) bool isYuv = false; \
	auto set1 = useTexture0(allocateDescriptorSet(shader.sets(1)), paint, dstSlot, &isYuv); \
		if (!set1) return __VA_ARGS__

namespace qk {
	void setTex_SourceImage(ImageSource* s, cPixelInfo &i, cTexStat *tex);

	void VulkanCanvas::setSurface(const Mat4 &root, Vec2 surfaceSize, Vec2 scale) {
		Mat4 matrix = root;
		matrix.scale_y(-1);
		matrix.translate_y(-surfaceSize.y() / scale.y());
		matrix.translate_z(0.5f);
		matrix.scale_z(0.5f);
		GPUCanvas::setSurface(matrix, surfaceSize, scale);
	}

	void VulkanCanvas::setDefaultTarget(VkTexture *tex) {
		_outTex = tex;
		Qk_ASSERT(_outTex, "Failed to create Vulkan canvas output texture");
		clearFramebuffer();
		_framebuffer.framebuffer = vk_create_framebuffer(
			_device, getRenderPass(_outTex->format), _outTex->view, _outTex->extent);
		_framebuffer.view = _outTex->view;
		Qk_ASSERT(_framebuffer.framebuffer, "Failed to create Vulkan canvas framebuffer");
		_target = tex; // set current render target to default output texture
	}

	void VulkanCanvas::setSurfaceCmd(bool changeSize) {
		endPass(); // end old pass if exist

		_cmdPack->reset(this); // clear old command pack
		// clear buffer allocators for new frame
		_cmdPack->clearAllocator();
		_cmdPackFront->clearAllocator(true);

		// create new output texture if size changed
		if (changeSize) {
			setDefaultTarget(_resource->newTexture(_surfaceSize, _opts.colorType, 1, kNone_TextureFlags));
		}
	}

	void VulkanCanvas::setMatrixCmd() {
		if (_cmdPack->renderPass)
			setViewMatrixBuffer();
	}

	void VulkanCanvas::setBlendModeCmd() {
	}

	void VulkanCanvas::drawClipCmd(const VertexData &vertex, GC_State::Clip *last, GC_State::Clip *clip, ClipOp rawOp) {
		auto begin = clip->bounds.begin,
				 end = clip->bounds.end, size = end - begin;
		auto blend = _blendMode; // save current blend mode
		auto lastTarget = _target; // save current color texture
		// switch blend mode to src
		_blendMode = kSrc_BlendMode;
		// output to clip mask texture
		_target = vk_get_texture(clip->mask.get());

		endPass(); // end current pass

		auto drawClipMask = [&](bool black, bool clip) {
			auto scale = Vec2(1) / _surfaceScale;
			Vec4 surface = {-begin.x(), -begin.y(), scale.x(), scale.y()};
			// Difference clip cannot directly render solid black with AA,
			// otherwise edge blending becomes incorrect.
			// Instead, invert the AA coverage curve.
			// This produces a smooth subtractive mask edge.
			int flags = black ? Qk_FLAG_AASIDE_Inverted : 0; // Qk_FLAG_AASIDE_Inverted
			flags |= Qk_CLIP(clip); // set clip flag if have clip
			drawColor(vertex, Color4f(1,1,1,1), surface, flags);
		};
		if (rawOp == Canvas::kIntersect_ClipOp || !last) {
			// clear clipTex with black color
			clearColor({0,0,0,0}, nullptr);
			// draw clip shape to clipTex with white color
			drawClipMask(false, last);
		} else { // if (rawOp == Canvas::kDifference_ClipOp)
			beginPass(0, false); // begin a new pass with don't load color
			// copy last clip color to clipTex as the clear color
			copyImage(last->mask.get(), begin - last->bounds.begin, {0,size}, size);
			// draw clip shape to clipTex with white color if last op equal difference,
			// or black color if last op equal intersect
			auto black = last->op == Canvas::kIntersect_ClipOp;
			// draw clip shape to clipTex with color
			drawClipMask(black, true);
		}
		endPass();
		makeTextureMipReadable(_target); // make clip mask texture readable for shader
		// restore framebuffer and blend mode
		_blendMode = blend;
		_target = lastTarget;
	}

	void VulkanCanvas::restoreClipCmd(GC_State::Clip* clip) {
		if (_cmdPack->renderPass == VK_NULL_HANDLE)
			return; // no render pass, no need to set clip
		if (clip) {
			MSLColor::ClipStatBlock clipStat = { *(Vec4*)clip->bounds.begin.val, clip->op };
			_cmdPack->buffers[2] = makeBufferInfoT(_cmdPack, &clipStat);
			_clipState = clip; // set current clip state
			updateCommonDescriptorSet(false);
		}
	}

	void VulkanCanvas::copyImage(ImageSource *src, Vec2 srcOffset, Range dst, Vec2 resolution) {
		float x1 = dst.begin.x(), y1 = dst.begin.y();
		float x2 = dst.end.x(), y2 = dst.end.y();
		float vertex[] = { x1,y1,0, x2,y1,0, x1,y2,0, x2,y2,0, };
		auto &cp = _shaders.cp;
		auto scale = resolution / src->size();
		auto offset = (srcOffset - dst.begin) / src->size();
		auto coord = Vec4(offset.x(), offset.y(), scale.x(), scale.y());
		auto cmd = usePipeline(cp, vertex, 12);
		MSLCp::PcArgs pc{ resolution, resolution, coord, 0, 0 };
		auto set = allocateDescriptorSet(cp.sets(1));
		setTextureParam(set, cp.image.binding, vk_get_texture(src));
		bindDescriptorSet(set, cp);
		vkCmdPushConstants(cmd, cp.layout(), cp.pc.stages, 0, sizeof(pc), &pc);
		vkCmdDraw(cmd, 4, 1, 0, 0);
	}

	void VulkanCanvas::drawColor(const VertexData &vertex, const Color4f &color, Vec4 offset, uint32_t flags) {
		auto &shader = _shaders.color;
		auto cmd = usePipeline(shader, vertex);
		SpvColor::PcArgs pc{color, offset, flags};
		vkCmdPushConstants(cmd, shader.layout(), shader.pc.stages, 0, sizeof(pc), &pc);
		vkCmdDraw(cmd, vertex.vCount, 1, 0, 0);
	}

	void VulkanCanvas::clearColor(const Color4f &color, const Range *surfaceRange) {
		if (!surfaceRange) {
			beginPass(0, false, &color);
		} else {
			// clear color by drawing a rect
			Range fullScreen{{0,0}, _surfaceSize};
			surfaceRange = surfaceRange ? surfaceRange : &fullScreen;
			auto begin = surfaceRange->begin.floor();
			auto end = surfaceRange->end.ceil();
			float x1 = begin.x(), y1 = begin.y(), x2 = end.x(), y2 = end.y();
			float vertex[] = { x1,y1,0, x2,y1,0, x1,y2,0, x2,y2,0 };
			auto &clear = _shaders.clear;
			auto cmd = usePipeline(clear); // use pipeline state for clear shader
			// set color and other args for shader push constants
			MSLClear::PcArgs pc{ color, 0 };
			vkCmdPushConstants(cmd, clear.layout(), clear.pc.stages, 0, sizeof(pc), &pc);
			vkCmdDraw(cmd, 4, 1, 0, 0);
		}
	}

	void VulkanCanvas::drawColorCmd(const VertexData &vertex, const Color4f &color) {
		drawColor(vertex, premul_alpha(color), Vec4(0), _flags);
	}

	void VulkanCanvas::clearColorCmd(const Color4f &color, GC_ClearFlags) {
		endPass(); // end current pass if exist
		beginPass(0, false, &color);
	}

	bool VulkanCanvas::drawCAPACmd(CAPADrawData& data) {
		auto edgeCount = data.edges.length();
		if (!edgeCount)
			return false;
		return false;
	}

	void VulkanCanvas::drawImageCmd(const VertexData &vertex, const GC_ImageDrawInfo &info) {
		auto &shader = _shaders.image;
		// set texture for slot 0 and return encoder, if texture not ready, return nil and skip draw call
		Qk_useTexture0(info.paint, shader.image.binding); // slot 0 default match dst slot to 1
		if (info.kind == kImage_DrawKind && isYuv) { // yuv420p or yuv420sp
			auto &yuv = _shaders.imageYuv;
			auto cmd = usePipeline(yuv, vertex);
			auto src = info.paint->image;
			Qk_ASSERT(shader.image.binding == yuv.image.binding, "image slot should match"); // y
			Qk_ASSERT_EQ(true, useTexture(set1, src, 1, yuv.image_uv.binding, info.paint)); // u or uv
			int format = 0; // default to yuv420sp
				if (src->pixel(1)->type() == kYUV420P_U_8_ColorType) {
					Qk_ASSERT_EQ(true, useTexture(set1, src, 2, yuv.image_v.binding, info.paint)); // v
					format = 1; // yuv420p
				} else {
					// image_v is still evaluated by mix(); bind a valid placeholder for YUV420SP.
					setTextureParam(set1, yuv.image_v.binding, vk_get_texture(src));
				}
			MSLImageYuv::PcArgs pc{
				.texCoords=*((Vec4*)info.paint->coord.begin.val),
				.color=premul_alpha(info.color),
				.format=format,
				.flags=_flags
			};
			vkCmdPushConstants(cmd, yuv.layout(), yuv.pc.stages, 0, sizeof(pc), &pc);
			bindDescriptorSet(set1, yuv);
		} else {
			auto &shader = _shaders.image;
			auto cmd = usePipeline(shader, vertex);
			auto type = info.paint->_isCanvas ? kRGBA_8888_ColorType: info.paint->image->type();
			// set color and other args for shader push constants
			MSLImage::PcArgs pc{
				.texCoords=*((Vec4*)info.paint->coord.begin.val),
				.color=premul_alpha(info.color),
				.strokeColor=premul_alpha(info.stroke <= 0 ? info.color: info.strokeColor),
				.strokeWidth=info.stroke,
				.alphaIndex=info.kind == kMask_DrawKind ?
					(type == kAlpha_8_ColorType ? 0 : type == kLuminance_Alpha_88_ColorType ? 1 : 3): 0,
				.flags = _flags |
					(info.kind == kMask_DrawKind ? Qk_FLAG_IMAGE_MASK: 0) |
					(info.kind == kSDFMask_DrawKind ? Qk_FLAG_IMAGE_SDF_MASK: 0)
			};
			vkCmdPushConstants(cmd, shader.layout(), shader.pc.stages, 0, sizeof(pc), &pc);
			bindDescriptorSet(set1, shader);
		}
		vkCmdDraw(_cmdPack->current, vertex.vCount, 1, 0, 0);
	}

	void VulkanCanvas::drawGradientCmd(const VertexData &vertex, const PaintGradient *paint, const Color4f &color) {
		int count = Qk_Min(64, paint->count);
		auto &shader = _shaders.colorGradient;
		auto set1 = allocateDescriptorSet(shader.sets(1));
		auto cmd = usePipeline(shader, vertex);
		MSLColorGradient::PcArgs pc{
			.range=*((Vec4*)paint->origin.val),
			.color=premul_alpha(color),
			.count=count,
			.flags=_flags |
				(count == 2 ? Qk_FLAG_GRADIENT_COUNT2: 0) |
				(paint->type == PaintGradient::kRadial_Type ? Qk_FLAG_RADIAL_GRADIENT: 0)
		};
		Array<Color4f> colors(count);
		for (int i = 0; i < count; i++) {
			colors[i] = premul_alpha(paint->colors[i]);
		}
		setBuffer(set1, shader.colors.binding, colors.val(), count * sizeof(Color4f));
		setBuffer(set1, shader.positions.binding, paint->positions, count * sizeof(float));
		vkCmdPushConstants(cmd, shader.layout(), shader.pc.stages, 0, sizeof(pc), &pc);
		bindDescriptorSet(set1, shader, 1, 2); // bind descriptor set to pipeline
		vkCmdDraw(cmd, vertex.vCount, 1, 0, 0);
	}

	void VulkanCanvas::drawRRectBlurColorCmd(const Rect& rect, const float *radius, float blur,
			const Color4f &color) {
		blur = Qk_Max(blur, 0.5);
		float s1 = blur * 1.15, s2 = blur * 2.0;
		float min_edge = Qk_Min(rect.size[0], rect.size[1]);
		float rmax = 0.5 * min_edge;
		Vec2 size = rect.size * 0.5;
		Vec2 c = rect.begin + size;
		float w = size[0] + blur, h = size[1] + blur;
		float x1 = c[0] - w, x2 = c[0] + w;
		float y1 = c[1] - h, y2 = c[1] + h;
		Vec2 horns[] = { {x1,y1}, {x2,y1}, {x2,y2}, {x1,y2} };
		auto& sh = _shaders.colorRrectBlur;
		auto cmd = usePipeline(sh);
		float s_inv = 1.0f / blur;

		for (int i = 0; i < 4; i++) {
			auto horn = horns[i];
			float v[] = { c[0],c[1],0, horn[0],c[1],0, horn[0],horn[1],0, c[0],horn[1],0 };
			float r0 = F32::min(Vec2(radius[i], s1).length(), rmax);
			float r1 = F32::min(Vec2(radius[i], s2).length(), rmax);
			float n = 2.0 * r1 / r0;
			MSLColorRrectBlur::PcArgs pc{
				horn,
				color,
				{ Vec3(r1, n, 1.0 / n), 0 },
				min_edge,
				s_inv,
				_flags,
			};
			auto buf = makeBufferInfoT(_cmdPack, v, 12);
			vkCmdBindVertexBuffers(cmd, 0, 1, &buf.buffer, &buf.offset);
			vkCmdPushConstants(cmd, sh.layout(), sh.pc.stages, 0, sizeof(pc), &pc);
			vkCmdDraw(cmd, 4, 1, 0, 0);
		}
	}

	void VulkanCanvas::drawTrianglesCmd(const Triangles& triangles, const PaintImage *paint,
			const Color4f &color, bool copyData) {
		if (!triangles.verts || !triangles.indices || !triangles.vertCount || !triangles.indexCount)
			return;
		Qk_ASSERT_EQ(triangles.indexCount % 3, 0, "Triangle index count should be a multiple of 3");
		auto &shader = _shaders.triangles;
		Qk_useTexture0(paint, shader.image.binding);
		auto cmd = usePipeline(shader);
		auto vbuf = makeBufferInfoT(_cmdPack, triangles.verts, triangles.vertCount);
		auto ibuf = makeBufferInfoT(_cmdPack, triangles.indices, triangles.indexCount);

		bindDescriptorSet(set1, shader); // bind texture descriptor set to pipeline

		MSLTriangles::PcArgs pc{
			color,
			_flags | (triangles.isDarkColor ? Qk_FLAGS_DARK_COLOR : 0)
		};
		vkCmdPushConstants(cmd, shader.layout(), shader.pc.stages, 0, sizeof(pc), &pc);
		vkCmdBindVertexBuffers(cmd, 0, 1, &vbuf.buffer, &vbuf.offset);
		vkCmdBindIndexBuffer(cmd, ibuf.buffer, ibuf.offset, VK_INDEX_TYPE_UINT16);
		vkCmdDrawIndexed(cmd, triangles.indexCount, 1, 0, 0, 0);
	}

	void VulkanCanvas::blurFilterBeginCmd(Range bounds, Mat4 &blurRootMat, ImageSource *tmpA) {
		auto outTexA = vk_get_texture(tmpA);
		Qk_ASSERT(outTexA, "blurFilterBeginCmd tmpA texture is null");
		_rootMatrix = blurRootMat;
		_target = outTexA; // output to texture A for blur filter then do post processing
		clearColor({0,0,0,0}, nullptr);
	}

	void VulkanCanvas::blurFilterEndCmd(Range bounds, Mat4 &recoverRootMat, float radius, float clearPad,
			int sample, int imageLod, ImageSource *tmpA, ImageSource *tmpB) {
		auto texA = vk_get_texture(tmpA);
		auto texB = vk_get_texture(tmpB);
		Qk_ASSERT(texA && texB, "blurFilterEndCmd temp texture is null");
		auto offset = bounds.begin.max(0);
		auto begin = bounds.begin, end = bounds.end;
		float x1 = begin.x() - offset.x(), y1 = begin.y() - offset.y(),
					x2 = end.x() - offset.x(), y2 = end.y() - offset.y();
		float radius2 = radius * _surfaceScaleAverage; // radius in pixel unit
		Vec2 iR = tmpA->size(); // input resolution
		int oRw = iR.x(), oRh = iR.y();

		auto blend = _blendMode; // save current blend mode
		_blendMode = kSrc_BlendMode;
		_rootMatrix = recoverRootMat; // recover root matrix
		// get sampler state for paint image
		auto sampler = _resource->linearSampler();
		auto &cp = _shaders.cp;
		// Choosing the right blur shader
		auto blur = &_shaders.blur;

		if (imageLod) {
			if (oRw >> imageLod == 0 || oRh >> imageLod == 0) {
				_target = vk_get_texture_from(_state->output.get(), _outTex.get());
				_blendMode = blend;
				return endPass(); // end pass
			}
			// |r|r|rrrrrr|r|r|
			// |r|r|rrrrrr|r|r|
			// |r|r| body |r|r|
			// |r|r|rrrrrr|r|r|
			// |r|r|rrrrrr|r|r|
			int level = 0;
			float vertex[] = { x1,y1,0, x2,y1,0, x1,y2,0, x2,y2,0 };
			do { // Copy the image to smaller texture for next level
				oRw >>= 1; oRh >>= 1;
				MSLCp::PcArgs pc{ iR, Vec2(oRw, oRh), { 0, 0, 1, 1 }, 0, 0};
				beginPass(level+1, false); // begin new pass for next level
				makeTextureMipReadable(texA, level); // make texture level readable for shader
				auto cmd = usePipeline(cp, vertex, 12);
				vkCmdPushConstants(cmd, cp.layout(), cp.pc.stages, 0, sizeof(pc), &pc);
				auto set = allocateDescriptorSet(cp.sets(1));
				setTextureLevelParam(set, cp.image.binding, texA, level++, sampler);
				bindDescriptorSet(set, cp);
				vkCmdDraw(cmd, 4, 1, 0, 0);
			} while (level < imageLod);
		}
		{
			// The blur regions for x-axis
			// |r|rrrrrr|r|
			// |r|rrrrrr|r|
			// |r| body |r|
			// |r|rrrrrr|r|
			// |r|rrrrrr|r|
			_target = texB; // output to texture B
			beginPass(imageLod, false); // begin new pass for blur x-axis
			makeTextureMipReadable(texA, imageLod);
			// Making blur of the x-axis direction
			float vertex[] = { x1+radius,y1,0, x2-radius,y1,0, x1+radius,y2,0, x2-radius,y2,0 };
			MSLBlur::PcArgs pc{ iR, Vec2(oRw, oRh), Vec2(radius2 / iR.x(), 0), {0,0},
				1.0f/(sample-1), 0, 0 };
			auto cmd = usePipeline(*blur, vertex, 12);
			vkCmdPushConstants(cmd, blur->layout(), blur->pc.stages, 0, sizeof(pc), &pc);
			auto set = allocateDescriptorSet(blur->sets(1));
			setTextureLevelParam(set, blur->image.binding, texA, imageLod, sampler);
			bindDescriptorSet(set, *blur);
			// draw blur to texture B
			vkCmdDraw(cmd, 4, 1, 0, 0);
		}
		{
			// The blur regions for y-axis
			// |r|rrrrrr|r|
			// |r| body |r|
			// |r|rrrrrr|r|
			float padding = radius + clearPad;
			x1 = begin.x() + padding, y1 = begin.y() + padding;
			x2 = end.x() - padding, y2 = end.y() - padding;
			float vertex[] = { x1,y1,0, x2,y1,0, x1,y2,0, x2,y2,0 };
			auto uv_offset = -offset * _surfaceScale / iR;
			_target = vk_get_texture_from(_state->output.get(), _outTex.get());
			_blendMode = blend;
			beginPass(); // begin new pass for main render target
			makeTextureMipReadable(texB, imageLod);
			MSLBlur::PcArgs pc = { iR, iR, Vec2(0, radius2 / iR.y()), uv_offset,
				1.0f/(sample-1), 0, 0 };
			auto cmd = usePipeline(*blur, vertex, 12);
			vkCmdPushConstants(cmd, blur->layout(), blur->pc.stages, 0, sizeof(pc), &pc);
			auto set = allocateDescriptorSet(blur->sets(1));
			setTextureLevelParam(set, blur->image.binding, texB, imageLod, sampler);
			bindDescriptorSet(set, *blur);
			vkCmdDraw(cmd, 4, 1, 0, 0);
		}
	}

	void VulkanCanvas::readImageCmd(const Rect &srcRect, ImageSource* src, ImageSource* dst) {
		auto dstSize = Vec2(dst->width(), dst->height());
		auto srcTex = vk_get_texture_from(src, _outTex.get());
		Qk_ASSERT(srcTex, "readImageCmd source texture is null");

		TexStat storeStat;
		uint8_t flags = dst->mipmap() ? kMipmap_TextureFlags: 0;
		auto texStat = vk_rebuild_texture(dstSize, dst->type(), dst->texture(0), storeStat, flags);
		if (!texStat)
			return;
		auto tex = vk_cast_texture(texStat);
		endPass(); // end current pass

		makeTextureMipReadable(srcTex); // make source texture readable for shader
		auto sampler = srcRect.size == dstSize ? _resource->nearestSampler() : _resource->linearSampler();
		auto target = _target;
		_target = tex;
		// load raw color if need to blend with existing color
		beginPass(0, _blendMode > kSrc_BlendMode ? true: false);
		auto &cp = _shaders.cp;
		float x2 = _size[0], y2 = _size[1]; // canvas size
		float vertex[] = { 0,0,0, x2,0,0, 0,y2,0, x2,y2,0 };
		auto cmd = usePipeline(cp, vertex, 12);
		auto begin = srcRect.begin / _surfaceSize;
		auto scale = srcRect.size / _surfaceSize;
		auto coord = Vec4(begin.x(), begin.y(), scale.x(), scale.y());
		MSLCp::PcArgs pc{ _surfaceSize, dstSize, coord, 0, 0 };
		vkCmdPushConstants(cmd, cp.layout(), cp.pc.stages, 0, sizeof(pc), &pc);
		auto set = allocateDescriptorSet(cp.sets(1));
		setTextureParam(set, cp.image.binding, srcTex, sampler);
		bindDescriptorSet(set, cp);
		vkCmdDraw(cmd, 4, 1, 0, 0);
		_cmdPack->ref(tex);
		_target = target; // restore output color texture
		endPass();

		// transition srcTex to color attachment layout for next pass
		srcTex->transitionLayout(_cmdPack->current,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

		if (dst->mipmap()) {
			tex->generateMipmaps(_cmdPack->current, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
		} else {
			makeTextureMipReadable(tex); // make texture readable for shader
		}

		setTex_SourceImage(dst, dst->info(), texStat);
	}

	void VulkanCanvas::outputImageBeginCmd(ImageSource* dst) {
		endPass(); // end pass, change outTex for next pass
		auto s = _surfaceSize; // surface size
		TexStat storeStat;
		uint8_t flags = dst->mipmap() ? kMipmap_TextureFlags: 0;
		auto tex = vk_rebuild_texture(s, _opts.colorType, dst->texture(0), storeStat, flags);
		if (!tex) {
			// texture rebuild failed after current pass was ended.
			// upper layer currently does not track this failure state,
			// so subsequent rendering commands may fail due to missing
			// output render target texture.
			return;
		}
		_target = vk_cast_texture(tex);
		setTex_SourceImage(dst, {int(s[0]),int(s[1]),_opts.colorType,dst->info().alphaType()}, tex);
	}

	void VulkanCanvas::outputImageEndCmd(ImageSource* exit) {
		endPass(); // end current pass, change outTex back to canvas's own texture for next pass
		// restore output color texture for next pass
		_target = vk_get_texture_from(_state->output.get(), _outTex.get());
		auto tex = vk_get_texture(exit);
		Qk_ASSERT(tex, "outputImageEndCmd exit texture is null");
		if (exit->mipmap()) {
			tex->generateMipmaps(_cmdPack->current, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
			_cmdPack->recorded = true; // mark cmd pack as recorded after encoding commands
		} else {
			makeTextureMipReadable(tex);
		}
	}

} // namespace qk
