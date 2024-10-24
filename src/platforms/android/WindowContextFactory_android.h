
/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef WindowContextFactory_android_DEFINED
#define WindowContextFactory_android_DEFINED

#include <android/native_window_jni.h>

#include <memory>

namespace sk_app {

class WindowContext;
struct Options;

namespace window_context_factory {

std::unique_ptr<WindowContext> MakeVulkanForAndroid(ANativeWindow*, const Options&);

std::unique_ptr<WindowContext> MakeGLForAndroid(ANativeWindow*, const Options&);

std::unique_ptr<WindowContext> MakeRasterForAndroid(ANativeWindow*, const Options&);

}  // namespace window_context_factory

}  // namespace sk_app

#endif
