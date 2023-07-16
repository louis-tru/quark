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

#ifndef __quark_render_gl_glrender__
#define __quark_render_gl_glrender__

#include "../render.h"
#include "./gl_canvas.h"

namespace qk {

	class GLRender: public GLCanvas, public RenderBackend {
	public:
		virtual ~GLRender();
		virtual Object* asObject() override;
		virtual void reload() override;
		virtual uint32_t makeTexture(cPixel *src, uint32_t id) override;
		virtual void deleteTextures(const uint32_t *IDs, uint32_t count) override;
		inline  GLSLShader* shader(uint32_t i) { return _shaders[i]; }
		void setBlendMode(BlendMode blendMode);
	protected:
		GLRender(Options opts);
		virtual void setRenderBuffer(int width, int height);
		virtual void setMSAABuffer(int width, int height, int MSAASample);
		virtual void setStencilBuffer(int width, int height, int MSAASample);
		virtual void setDepthBuffer(int width, int height, int MSAASample);
		void setTexture(cPixel *pixel, int slot, const Paint &paint);
		// define props
		bool _Is_Support_Multisampled;
		bool _IsDeviceMsaa; // device anti alias, msaa
		BlendMode _blendMode;
		GLuint _frame_buffer,_msaa_frame_buffer;
		GLuint _render_buffer,_msaa_render_buffer, _stencil_buffer,_depth_buffer;
		// shader
		GLSLClear _clear;
		GLSLClip  _clip;
		GLSLColor _color;
		GLSLColorMask _colorMask;
		GLSLColorLinear _linear;
		GLSLColorRadial _radial;
		GLSLColorDotted _colorDotted;
		GLSLImage _image;
		GLSLImageYuv420p _yuv420p;
		GLSLImageYuv420sp _yuv420sp;
		GLSLColorSdf _colorSdf;
		GLSLColorMaskSdf _colorMaskSdf;
		GLSLColorLinearSdf _linearSdf;
		GLSLColorRadialSdf _radialSdf;
		GLSLImageSdf _imageSdf;
		GLSLShader  *_shaders[15];
		friend class GLCanvas;
	};
}
#endif
