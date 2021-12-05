
/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkSurface.h"
#include "include/core/SkTypes.h"
#include "tools/sk_app/RasterWindowContext.h"
#include "tools/sk_app/android/WindowContextFactory_android.h"

using sk_app::RasterWindowContext;
using sk_app::Options;

namespace {
class RasterWindowContext_android : public RasterWindowContext {
public:
    RasterWindowContext_android(ANativeWindow*, const Options& params);

    sk_sp<SkSurface> getBackbufferSurface() override;
    void swapBuffers() override;

    bool isValid() override { return SkToBool(fNativeWindow); }
    void resize(int w, int h) override;
    void setOptions(const Options& params) override;

private:
    void setBuffersGeometry();
    sk_sp<SkSurface> fBackbufferSurface = nullptr;
    ANativeWindow* fNativeWindow = nullptr;
    ANativeWindow_Buffer fBuffer;
    ARect fBounds;

    using INHERITED = RasterWindowContext;
};

RasterWindowContext_API::RasterWindowContext_android(ANativeWindow* window,
                                                         const Options& params)
    : INHERITED(params) {
    fNativeWindow = window;
    fWidth = ANativeWindow_getWidth(fNativeWindow);
    fHeight = ANativeWindow_getHeight(fNativeWindow);
    this->setBuffersGeometry();
}

void RasterWindowContext_API::setBuffersGeometry() {
    int32_t format = 0;
    switch(fOptions.fColorType) {
        case kRGBA_8888_SkColorType:
            format = WINDOW_FORMAT_RGBA_8888;
            break;
        case kRGB_565_SkColorType:
            format = WINDOW_FORMAT_RGB_565;
            break;
        default:
            SK_ABORT("Unsupported Android color type");
    }
    ANativeWindow_setBuffersGeometry(fNativeWindow, fWidth, fHeight, format);
}

void RasterWindowContext_API::setOptions(const Options& params) {
    fOptions = params;
    this->setBuffersGeometry();
}

void RasterWindowContext_API::resize(int w, int h) {
    fWidth = w;
    fHeight = h;
    this->setBuffersGeometry();
}

sk_sp<SkSurface> RasterWindowContext_API::getBackbufferSurface() {
    if (nullptr == fBackbufferSurface) {
        ANativeWindow_lock(fNativeWindow, &fBuffer, &fBounds);
        const int bytePerPixel = fBuffer.format == WINDOW_FORMAT_RGB_565 ? 2 : 4;
        SkImageInfo info = SkImageInfo::Make(fWidth, fHeight,
                                             fOptions.fColorType,
                                             kPremul_SkAlphaType,
                                             fOptions.fColorSpace);
        fBackbufferSurface = SkSurface::MakeRasterDirect(
                info, fBuffer.bits, fBuffer.stride * bytePerPixel, nullptr);
    }
    return fBackbufferSurface;
}


void RasterWindowContext_API::swapBuffers() {
    ANativeWindow_unlockAndPost(fNativeWindow);
    fBackbufferSurface.reset(nullptr);
}
}  // anonymous namespace

namespace sk_app {
namespace window_context_factory {

std::unique_ptr<WindowContext> MakeRasterForAndroid(ANativeWindow* window,
                                                    const Options& params) {
    std::unique_ptr<WindowContext> ctx(new RasterWindowContext_android(window, params));
    if (!ctx->isValid()) {
        return nullptr;
    }
    return ctx;
}

}
}   // namespace sk_app
