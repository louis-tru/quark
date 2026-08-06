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
  direct Canvas rendering into acquired images, shared-queue
  submission/present, and externally driven surface reload;
- Android runtime bring-up on multiple devices;
- Android system clipboard plain-text read, write, presence, and clear support;
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

- Linux runtime validation of platform presentation;
- Vulkan CAPA runtime validation;
- broader runtime validation across Android and desktop Vulkan drivers;
- re-enable the currently disabled macOS GL `readPixels()` path only after its
  behavior is validated.

## Deferred Non-Vulkan Work

- A small intermittent direct-touch scrolling stutter remains on simple iOS
  Metal scenes. Heavy continuously updating scenes are smooth, suggesting an
  on-demand render-cadence issue rather than raw GPU cost. Revisit only after
  the Vulkan milestone.
- Android GLES can spend substantial CPU time uploading dynamic R8 text/image
  textures. Treat GL/GLES as the correctness fallback; optimize only with new
  profiling evidence.
- Linux clipboard has a build-wired placeholder. A real implementation should
  integrate with the active window-system backend; the current Xlib path needs
  X11 selection ownership and asynchronous request/notify handling.
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
