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

namespace quark {
	class SkiaCanvas;
}

#define AutoUpdateQRBounds AutoUpdateQRBounds; friend class quark::SkiaCanvas

#include "./skia_canvas.h"
#include "../../display.h"
#include "../../app.h"
#include "../render.h"
#include <math.h>

using namespace quark;

struct Layer;
struct BackImage;

class SkCanvas::MCRec {
public:
	std::unique_ptr<Layer> fLayer;
	SkBaseDevice* fDevice;
	std::unique_ptr<BackImage> fBackImage;
	SkM44 fMatrix;
	int fDeferredSaveCount;
};

class SkMatrixProvider {
protected:
	virtual ~SkMatrixProvider() = default;
	SkM44    fLocalToDevice;
	SkMatrix fLocalToDevice33;
};

class SkBaseDevice: public SkRefCnt, public SkMatrixProvider {
public:
	void setLocalToDevice(const SkM44& ctm) {
		fLocalToDevice = ctm;
		// fLocalToDevice.normalizePerspective();
		// Map from the global CTM state to this device's coordinate system.
		// fLocalToDevice.postConcat(fGlobalToDevice);
		fLocalToDevice33 = fLocalToDevice.asM33();
	}
private:
	SkMarkerStack* fMarkerStack = nullptr;
	const SkImageInfo    fInfo;
	const SkSurfaceProps fSurfaceProps;
	SkM44 fDeviceToGlobal;
	SkM44 fGlobalToDevice;
};

#include "./skia_canvas.h"

namespace quark {

	void SkiaCanvas::setMatrix(const Mat& mat) {
		SkM44 m4(mat[0],  mat[1], 0, mat[2],
							mat[3], mat[4], 0, mat[5],
							0,           0, 1, 0,
							0,           0, 0, display()->atom_pixel());
		//SkCanvas::setMatrix(m4);
		if (fMCRec->fDeferredSaveCount > 0) {
			doSave();
		}
		fMCRec->fMatrix = m4;
		fMCRec->fDevice->setLocalToDevice(m4);
		// didSetM44(m4); ignore
	}

}