# Current Work

This file is the short handoff for active work only. Durable architecture belongs
in the backend/topic documents, and diagnosed failures belong in
`TROUBLESHOOTING.md`.

## Active Theme

The active renderer task is completing the Vulkan backend on `master`.

- Shared Vulkan device/resource management is implemented.
- Most non-CAPA `VulkanCanvas` commands now have Metal-aligned implementations.
- Framebuffer/mip-view and command-pack completion ownership have been handled.
- A shared Android/Linux surface/swapchain/present implementation now exists;
  Android build/runtime bring-up is the immediate next checkpoint, followed by
  Linux validation and CAPA.
- Detailed architecture, invariants, and the Vulkan backlog live in
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
  submission/present, and externally driven surface reload.

Immediate correctness work:

1. `readImageCmd()` must not request attachment `LOAD` for a newly allocated,
   undefined destination merely because the blend mode reads destination color.
2. Concurrent CPU transitions of the same texture mip are intentionally not
   synchronized yet. Normal Qk sharing is expected to sample an already
   shader-readable texture; add synchronization only when a real
   conflicting-writer path appears.

Still incomplete:

- Android and Linux runtime validation of platform presentation;
- Vulkan CAPA command encoding (`drawCAPACmd()` still returns `false`);
- runtime validation across Android and desktop Vulkan drivers.

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

## Verification Rules

- Do not run broad C++ builds unless the user asks.
- Prefer targeted source inspection and `git diff --check`.
- Do not run the shader generator automatically during renderer iteration.
- Preserve user changes and untracked Vulkan platform/draft files unless they
  are explicitly brought into scope.
