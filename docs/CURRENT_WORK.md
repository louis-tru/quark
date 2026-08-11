# Current Work

This file is the short handoff for active work only. Durable architecture belongs
in the backend/topic documents, and diagnosed failures belong in
`TROUBLESHOOTING.md`.

## Active Theme

The Vulkan implementation milestone on `master` is complete. The backend now
has shared device/resource management, ordinary Canvas commands, Android/Linux
presentation, and CAPA command encoding. Further work is stabilization and
runtime validation rather than filling out the original backend skeleton.

Detailed architecture, invariants, and the remaining Vulkan backlog live in
[`VULKAN.md`](VULKAN.md).

## Vulkan Checkpoint

Implemented:

- physical/logical device selection without requiring a platform surface at
  shared-resource construction time;
- one application-wide graphics queue with serialized submit/present access;
- texture and vertex upload through staging buffers and non-blocking completion
  polling;
- SPIR-V reflection wrappers, pipeline layouts, graphics/compute pipeline caches,
  samplers, descriptor pools, and dynamic uniform-buffer binding;
- current/front command packs with per-pack allocators and completion state;
- ordinary color, image/YUV, gradient, rounded-rect blur, clip, blur, indexed
  triangle, image-copy/read, and output-image command encoding;
- per-mip `VkTexture::layouts` state and redundant-transition elimination;
- Android/Xlib surface creation, FIFO swapchain management, nonblocking acquire,
  direct Canvas rendering into compatible acquired images, RGBA-to-BGRA
  presentation blit when CAPA cannot target the swapchain format directly,
  shared-queue submission/present, and externally driven surface reload;
- Ubuntu 20.04 x64 Vulkan Release configuration, compilation, loader linkage,
  and native X11 startup; forwarded X11 displays are rejected before Vulkan WSI
  queries so the renderer can fall back to GL;
- Linux/Xlib shutdown queues `VkSwapchainKHR` and `VkSurfaceKHR` destruction on
  the X11 main thread before the native window close task. This avoids an
  NVIDIA Xlib WSI stall when the event thread is blocked in `XNextEvent()`;
- Android runtime bring-up on multiple devices;
- Android system clipboard plain-text read, write, presence, and clear support;
- Linux/X11 `CLIPBOARD` ownership, UTF-8 text read/write, target negotiation,
  selection request handling, presence checks, and clear support;
- Vulkan CAPA resource, descriptor, pass, barrier, dispatch, and ordered
  composite encoding;
- shader-reflection type cleanup, compatible render-pass creation, descriptor
  rebinding fixes, and clip-mask layout/readability fixes;
- rounded-rectangle shadow blur with shader-side difference clipping, aligned
  across GL, Metal, and Vulkan.

Immediate correctness work:

1. `readImageCmd()` must not request attachment `LOAD` for a newly allocated,
   undefined destination merely because the blend mode reads destination color.
2. Concurrent CPU transitions of the same texture mip are intentionally not
   synchronized yet. Normal Qk sharing is expected to sample an already
   shader-readable texture; add synchronization only when a real
   conflicting-writer path appears.

Remaining validation/stabilization:

- broader Linux runtime validation of platform presentation and additional drivers;
- Linux rendering performance and visual-quality work, with ordinary-DPI text
  rasterization/filtering quality as the main visible issue;
- Vulkan CAPA runtime validation;
- broader runtime validation across Android and desktop Vulkan drivers;
- re-enable the currently disabled macOS GL `readPixels()` path only after its
  behavior is validated.

## Linux Platform Notes

- Linux process shutdown must go through `qk::abort_exit()`. `is_exit()` is set
  before managed render/worker threads finish, so the X11 system `main()` stays
  parked after its event loop returns instead of starting libc/static teardown
  concurrently. `X11Application` is intentionally process-lifetime. Direct
  `::exit()`/`std::exit()` calls are unsupported; see the diagnosed failure in
  [`TROUBLESHOOTING.md`](TROUBLESHOOTING.md#process-exit-libc-teardown-races-qk-thread-shutdown).
- Linux clipboard uses the X11 `CLIPBOARD` selection through the application's
  hidden service window. It currently supports ordinary `UTF8_STRING` transfers.
  The normal X11 loop receives `SelectionNotify` and wakes the requesting
  thread; no synchronous task is posted to the main loop. A successful external
  read is cached by taking selection ownership, so a following
  `hasText()`/`getText()` call does not request the same text again. ICCCM
  `INCR`, legacy encodings, and clipboard-manager persistence after process exit
  are not implemented yet.
- Ordinary-DPI Linux text currently renders with visibly poor small-glyph
  quality. The first confirmed cause was a constructor-argument semantic error
  left by the Skia FreeType port: `Typeface_fontconfig` passed the Fontconfig
  fixed-pitch boolean as the `QkTypeface_FreeType` flags field. Fontconfig faces
  therefore normally selected `FT_LOAD_NO_HINTING`. FreeType construction now
  explicitly selects normal hinting on Linux, Android, and custom/stream font
  paths. Linux Fontconfig options retain the complete system hinting,
  embedded-bitmap, synthetic-bold, and auto-hinter mapping for future use, but
  the current Linux constructor adds Qk's normal-hinting bit so the effective
  policy is always normal or full. Runtime comparison is still required.
- Keep the remaining text-quality investigations separate: grayscale-only
  antialiasing, fractional glyph placement, glyph-atlas filtering, and Canvas
  font-size/coordinate quantization have not been changed yet.

## Deferred Non-Vulkan Work

- A small intermittent direct-touch scrolling stutter remains on simple iOS
  Metal scenes. Heavy continuously updating scenes are smooth, suggesting an
  on-demand render-cadence issue rather than raw GPU cost. Revisit only after
  the Vulkan milestone.
- Android GLES can spend substantial CPU time uploading dynamic R8 text/image
  textures. Treat GL/GLES as the correctness fallback; optimize only with new
  profiling evidence.
- CAPA is functionally closed enough for stabilization. Keep AASide for
  hairlines/text and use CAPA for complex ordered fills where batching makes
  sense. Algorithm and pass details live in
  [`GPU_2D_ANTIALIASING.md`](GPU_2D_ANTIALIASING.md) and
  [`CAPA_PASS_PROCESS.md`](CAPA_PASS_PROCESS.md).

## TLS Assembly Experiments

The active `thread_local` assembly investigation, including tested source
variants, macOS arm64 Release disassembly, and instruction counts, is recorded
in [`THREAD_LOCAL_ASSEMBLY-cn.md`](THREAD_LOCAL_ASSEMBLY-cn.md).

## Verification Rules

- Do not run broad C++ builds unless the user asks.
- Prefer targeted source inspection and `git diff --check`.
- Do not run the shader generator automatically during renderer iteration.
- Preserve user changes and untracked Vulkan platform/draft files unless they
  are explicitly brought into scope.
