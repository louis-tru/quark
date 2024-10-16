
/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef WindowContextFactory_unix_DEFINED
#define WindowContextFactory_unix_DEFINED

// webgpu_cpp.h and X.h don't get along. Include this first, before X11 defines None, Success etc.
#ifdef SK_DAWN
#include "dawn/webgpu_cpp.h"
#endif
#include <X11/Xlib.h>
#include <GL/glx.h>

#include <memory>

typedef Window XWindow;

namespace sk_app {

class WindowContext;
struct Options;

namespace window_context_factory {

struct XlibWindowInfo {
    Display*     fDisplay;
    XWindow      fWindow;
    GLXFBConfig* fFBConfig;
    XVisualInfo* fVisualInfo;
    int          fWidth;
    int          fHeight;
};

std::unique_ptr<WindowContext> MakeVulkanForXlib(const XlibWindowInfo&, const Options&);

std::unique_ptr<WindowContext> MakeGLForXlib(const XlibWindowInfo&, const Options&);

#ifdef SK_DAWN
std::unique_ptr<WindowContext> MakeDawnVulkanForXlib(const XlibWindowInfo&, const Options&);
#endif

std::unique_ptr<WindowContext> MakeRasterForXlib(const XlibWindowInfo&, const Options&);

}  // namespace window_context_factory

}  // namespace sk_app

#endif
