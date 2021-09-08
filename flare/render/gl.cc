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

#include "include/core/SkCanvas.h"
#include "include/core/SkSurface.h"
#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/GrDirectContext.h"
#include "src/core/SkMathPriv.h"
#include "src/gpu/GrCaps.h"
#include "src/gpu/GrDirectContextPriv.h"
#include "src/gpu/gl/GrGLDefines.h"
#include "src/gpu/gl/GrGLUtil.h"
#include "src/image/SkImage_Base.h"
#include "./gl.h"

namespace flare {

	GLRender::GLRender(Application* host, const DisplayParams& params)
			: Render(host, params)
			, fBackendContext(nullptr)
			, fSurface(nullptr) {
		// fDisplayParams.fMSAASampleCount = GrNextPow2(fDisplayParams.fMSAASampleCount);
	}

	void GLRender::initializeContext() {
		SkASSERT(!fContext);

		fBackendContext = this->onInitializeContext();

		fContext = GrDirectContext::MakeGL(fBackendContext, fDisplayParams.fGrContextOptions);
		if (!fContext && fDisplayParams.fMSAASampleCount > 1) {
			fDisplayParams.fMSAASampleCount /= 2;
			this->initializeContext();
			return;
		}
	}

	void GLRender::destroyContext() {
		fSurface.reset(nullptr);

		if (fContext) {
			// in case we have outstanding refs to this (lua?)
			fContext->abandonContext();
			fContext.reset();
		}

		fBackendContext.reset(nullptr);

		this->onDestroyContext();
	}

	sk_sp<SkSurface> GLRender::getBackbufferSurface() {
		if (nullptr == fSurface) {
			if (fContext) {
				GrGLint buffer;
				GR_GL_CALL(fBackendContext.get(), GetIntegerv(GR_GL_FRAMEBUFFER_BINDING, &buffer));

				GrGLFramebufferInfo fbInfo;
				fbInfo.fFBOID = buffer;
				fbInfo.fFormat = GR_GL_RGBA8;

				GrBackendRenderTarget backendRT(fWidth,
												fHeight,
												fSampleCount,
												fStencilBits,
												fbInfo);

				fSurface = SkSurface::MakeFromBackendRenderTarget(fContext.get(), backendRT,
																kBottomLeft_GrSurfaceOrigin,
																kRGBA_8888_SkColorType,
																fDisplayParams.fColorSpace,
																&fDisplayParams.fSurfaceProps);
			}
		}

		return fSurface;
	}

	void GLRender::swapBuffers() {
		this->onSwapBuffers();
	}

	void GLRender::resize(Vec2 size, Rect surface_region) {
		this->destroyContext();
		this->initializeContext();
	}

	void GLRender::setDisplayParams(const DisplayParams& params) {
		fDisplayParams = params;
		this->destroyContext();
		this->initializeContext();
	}

	bool GLRender::isValid () {
		return return SkToBool(fBackendContext.get());
	}

}   // namespace flare
