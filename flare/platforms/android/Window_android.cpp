/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "tools/sk_app/WindowContext.h"
#include "tools/sk_app/android/WindowContextFactory_android.h"
#include "tools/sk_app/android/Window_android.h"

namespace sk_app {

Window* Window::CreateNativeWindow(void* platformData) {
    Window_android* window = new Window_android();
    if (!window->init((SkiaAndroidApp*)platformData)) {
        delete window;
        return nullptr;
    }
    return window;
}

bool Window_API::init(SkiaAndroidApp* skiaAndroidApp) {
    SkASSERT(skiaAndroidApp);
    fSkiaAndroidApp = skiaAndroidApp;
    fSkiaAndroidApp->fWindow = this;
    return true;
}

void Window_API::setTitle(const char* title) {
    fSkiaAndroidApp->setTitle(title);
}

void Window_API::setUIState(const char* state) {
    fSkiaAndroidApp->setUIState(state);
}

bool Window_API::attach(BackendType attachType) {
    fBackendType = attachType;

    // We delay the creation of fWindowContext until Android informs us that
    // the native window is ready to use.
    // The creation will be done in initDisplay, which is initiated by kSurfaceCreated event.
    return true;
}

void Window_API::initDisplay(ANativeWindow* window) {
    SkASSERT(window);
    switch (fBackendType) {
#ifdef SK_GL
        case kNativeGL_BackendType:
        default:
            fWindowContext =
                    window_context_factory::MakeGLForAndroid(window, fRequestedOptions);
            break;
#else
        default:
#endif
        case kRaster_BackendType:
            fWindowContext =
                    window_context_factory::MakeRasterForAndroid(window, fRequestedOptions);
            break;
#ifdef SK_VULKAN
        case kVulkan_BackendType:
            fWindowContext =
                    window_context_factory::MakeVulkanForAndroid(window, fRequestedOptions);
            break;
#endif
    }
    this->onBackendCreated();
}

void Window_API::onDisplayDestroyed() {
    detach();
}

void Window_API::onInval() {
    fSkiaAndroidApp->postMessage(Message(kContentInvalidated));
}

void Window_API::paintIfNeeded() {
    if (fWindowContext) { // Check if initDisplay has already been called
        onPaint();
    } else {
        markInvalProcessed();
    }
}

}   // namespace sk_app
