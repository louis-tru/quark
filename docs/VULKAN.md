# Vulkan Backend

This document is the durable architecture and backlog for Quark's Vulkan
renderer. Short-lived priorities belong in `CURRENT_WORK.md`.

## Status

The implementation milestone is complete. The backend is now in runtime
validation and stabilization rather than initial feature bring-up:

- shared device/resource management is implemented;
- texture and vertex resources can be created, uploaded, used, and destroyed;
- generated SPIR-V reflection drives shader modules, layouts, descriptors, and
  pipelines;
- most ordinary `GPUCanvas` commands have Vulkan implementations;
- command submission and non-blocking completion tracking are connected;
- the shared Android/Linux surface, swapchain, direct Canvas rendering, submit,
  and present path is implemented and has been exercised on multiple Android
  devices;
- CAPA pass/resource/descriptor/dispatch encoding is implemented; device and
  driver runtime validation is still pending.

The readable Vulkan learning/smoke test remains `test/android/vk/`. It is a
NativeActivity test independent of the production backend and deliberately uses
simple synchronization.

## Source Map

- `vk_device.cc`: instance creation, physical-device scoring, queue-family
  selection, logical device, queue, and capability discovery.
- `vk_render.*`: shared `VulkanRenderResource`, shaders, pipeline layouts,
  pipelines, samplers, queue submission, and completion tracking.
- `vk_resource.cc`: textures, mipmaps, staging upload, vertex buffers, and
  resource destruction.
- `vk_shader.*`: generated SPIR-V reflection-facing shader wrappers.
- `vk_canvas.*`: command packs, descriptor pools, render passes, framebuffers,
  descriptors, pipelines, submission assembly, and Canvas state.
- `vk_canvas_cmd.cc`: backend implementations of the `GPUCanvas` command hooks.
- `vk_canvas_capa.cc`: Vulkan CAPA compute passes, storage buffers, runtime
  image/sampler descriptor arrays, barriers, and ordered composite dispatch.
- `vk_render_linux.cc`: shared Android/Linux platform surface, swapchain,
  acquire semaphores, swapchain-image targets, and present flow.
- `vk_util.*`: Vulkan format, memory, sampler, render-pass, framebuffer, image
  view, command-buffer, and buffer-allocation helpers.

Platform-specific surface/swapchain implementations should remain separate from
the platform-independent shared resource and Canvas layers.

## Android/Linux Presentation Model

The main Canvas renders directly into the acquired swapchain image when its
format is a valid CAPA storage target. If the platform surface exposes BGRA
instead of CAPA's required RGBA8 format, Canvas retains its RGBA8 default
output and the present path blits it into the acquired swapchain image. The
shared platform implementation:

1. creates an Android or Xlib `VkSurfaceKHR`;
2. creates a FIFO swapchain with direct-render usage for a compatible target,
   or transfer-destination usage for the RGBA-to-BGRA presentation path;
3. acquires one image with a per-frame acquire semaphore;
4. either wraps that image as the main Canvas target without taking ownership
   of its image/memory, or keeps the Canvas-owned RGBA8 output as the target;
5. records ordinary Canvas commands against the selected target;
6. appends a pre-recorded presentation command that either performs only the
   presentation-layout transition, or uses `vkCmdBlitImage` to convert the
   Canvas RGBA8 output into the BGRA swapchain image before that transition;
7. submits the Canvas command list through the shared queue and presents
   while holding the same queue-serialization contract used by other windows.

The platform alternates between two acquire semaphores. The Canvas current/front
command-pack model permits at most one submitted frame plus one acquired frame
waiting to be submitted, so the earlier acquire semaphore is available again
when the index wraps without adding a fence-status check to the display path.
Render-finished semaphores belong to swapchain images, so each is reused only
after that same image is acquired again. Swapchain recreation waits for the
shared queue and rebuilds surface-dependent state. The next acquired image
replaces the Canvas default target through the ordinary `setDefaultTarget()`
path only for direct presentation. Every image in one swapchain must select the
same direct-or-copy path; this is guarded by a debug assertion. The platform
renderer retains the Canvas default output used by all pre-recorded blit commands
until the queue is idle and those commands have been released.

`vkAcquireNextImageKHR()` currently uses a zero timeout. If acquisition fails or
the swapchain is out of date, the render tick is discarded. Swapchain reload is
driven by the external surface lifecycle rather than from `renderDisplay()`.

The current Android path requests `VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR` and
renders using the native window extent, keeping Qk's surface/matrix behavior
aligned with GL and Metal after orientation changes. Using
`capabilities.currentTransform` instead requires a complete pre-rotation model
(fixed logical extent plus matching render/input transforms); changing only the
swapchain flag produces incorrectly scaled or rotated output.

On Linux, swapchain and Vulkan surface destruction during window close is
queued onto the X11 main thread. The X11 message queue is FIFO, so this WSI
cleanup runs before the subsequently queued `WindowPlatform` destruction and
`XDestroyWindow()`. Keep this ordering: destroying the swapchain from the
application loop while the X11 thread waits in `XNextEvent()` can stall inside
the NVIDIA Xlib WSI implementation. Resize-time swapchain recreation already
runs directly on the X11 thread and does not use the deferred close path.

## Device And Queue Model

`VulkanRenderResource` owns the application-wide Vulkan objects:

- `VkInstance`
- selected `VkPhysicalDevice`
- one `VkDevice`
- the graphics queue family
- one shared graphics `VkQueue`

Device selection does not require a window surface. It prefers a performant
graphics-capable device and can prefer compute capability, while actual
presentation support remains a platform/surface concern.

On Android, the production backend currently requires Android 10 / API 29 and
Vulkan 1.1 or newer. Older systems or Vulkan implementations return no shared
Vulkan resource and fall through to GL. The `--gl` process argument forces GL
for the whole application so `Render::Make()` and shared image resources always
select the same backend. The `--aaside` argument keeps Vulkan selected but
disables CAPA capability enablement, forcing Canvas antialiasing through the
AASide path. This is useful for compatibility checks and Vulkan AASide/CAPA
comparisons.

All rendering and upload submissions use the shared queue. Vulkan requires
external synchronization around queue submit/present calls, so the resource's
commit mutex serializes those host API calls. Recording uses per-Canvas command
pools and can happen independently.

A shared queue gives a single GPU execution order. Resources uploaded earlier
in that queue can be consumed by later rendering without adding a second queue
or cross-queue ownership protocol.

## Resource Model

### Textures

`VkTexture` owns:

- `VkImage`
- the all-mip base `VkImageView`
- `VkDeviceMemory`
- extent, format, and usage
- `Array<VkTextureLevelInfo> levels`, one record per mip, containing its layout
  and lazily cached one-level image view/framebuffer

`transitionLayout()` reads the recorded old layout, skips ranges already in the
target layout, combines adjacent mips with the same old layout into one image
barrier, records the barrier, and updates the corresponding entries.

The all-mip base view is suitable only when the operation legitimately addresses
that full subresource range. A framebuffer attachment always needs a view whose
level count is exactly one. Sampling a mip whose layout differs from other mips
likewise uses the cached single-level view.

Texture creation is generic. `newTexture()` adds color-attachment usage only
when the format supports it; the caller selecting a texture as a render target
owns the capability requirement.

### Upload

Texture and cached vertex uploads use:

1. a host-visible staging buffer/memory allocation;
2. a one-time command buffer from the shared resource command pool;
3. transfer commands and required image layout transitions;
4. shared-queue submission;
5. non-blocking completion polling before staging and command resources are
   released.

The current path intentionally uses direct Vulkan allocations and coarse
resource locking. If profiling later proves allocation overhead material,
introduce memory-type-aware suballocation without creating a second upload
architecture.

### Destruction

Vulkan destroy/free calls are host operations, not queued GPU commands. Qk uses
submission completion for transient upload resources and its existing
frame-aware delayed tasks for longer-lived texture/vertex destruction.

## Shader, Layout, And Pipeline Model

`gen_glsl_natives.js` emits SPIR-V code and reflection metadata. `VkShader`
provides:

- stage code and shader-module lookup;
- reflected descriptor bindings and vertex attributes;
- `VkPipelineLayoutData`, containing the pipeline layout and descriptor-set
  layouts;
- cached graphics/compute pipeline references.

Descriptor-set conventions shared with GLSL/Metal:

- ordinary graphics resources use set 0 for common clip/root/view state;
- shader-specific sampled images and buffers use their reflected sets/bindings;
- compute/CAPA layouts follow the generated shader declarations rather than
  hard-coded Vulkan-only numbering.

Set 0 currently contains:

- binding 0: clip image/sampler;
- binding 1: dynamic root-matrix uniform buffer;
- binding 2: dynamic view-matrix uniform buffer;
- binding 3: dynamic clip-state uniform buffer.

Uniform data is allocated linearly per command pack. Dynamic offsets select the
current records. A new descriptor set is required only when the underlying
buffer object changes, not when only an offset changes.

Pipeline-layout compatibility includes descriptor-set layouts and push-constant
ranges. Rebind set 0 with the active shader layout when the pipeline/layout
changes; do not assume one shader's pipeline layout remains compatible with all
others.

Graphics pipelines are cached by stable rendering state such as shader kind,
blend mode, and color format. The temporary compatible render pass used during
pipeline creation is destroyed after pipeline creation; the pipeline does not
own it.

## Canvas Command Model

Each `VulkanCanvas` owns one command pool and two `VkCmdPack` objects:

- `_cmdPack`: records the later/current frame;
- `_cmdPackFront`: holds the accepted frame waiting for or undergoing
  submission.

A command pack owns:

- command-buffer handles allocated by its Canvas pool;
- descriptor pools and descriptor sets;
- host/device buffer allocators;
- strong references and completion callbacks;
- subcanvas references;
- render-pass, pipeline, common-set, target, mip, and load/store state;
- a submission-completion pointer.

`beginPass()` only selects target/load/store/clear state. Actual
`vkCmdBeginRenderPass()` is deferred until a command needs the pass, matching
the Metal architecture where pass selection and encoder creation are separate.
`usePipeline()` intentionally requires a selected pass and must not silently
choose one.

Implemented non-CAPA command paths include:

- color and clear;
- image, YUV, mask, and SDF image;
- gradients;
- rounded-rect blur color;
- clip-mask creation and restoration;
- blur-filter mip/ping-pong passes;
- indexed triangles;
- shader image copy/read;
- output-image target switching and mipmap generation;
- subcanvas command integration.

## Render Passes, Framebuffers, And Layouts

Render-pass objects are cached from attachment format, load/store operations,
and initial/final layouts. They are lightweight immutable compatibility/state
objects; the clear color and framebuffer are supplied at begin time.

The active color attachment subpass uses
`VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL`, and the current final layout remains
color-attachment optimal. Sampling performs an explicit transition to
`VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL`.

For `LOAD`, the target mip must preserve its contents and be transitioned from
its recorded layout to color-attachment optimal before beginning the pass.
For `CLEAR` or `DONT_CARE`, the pass can use `UNDEFINED` as the initial layout
and discard previous contents.

Framebuffer attachments use the texture's base view only when it contains one
mip. Multi-mip textures lazily create and retain one-level views and compatible
framebuffers per mip. These objects follow the texture lifetime, avoiding the
former per-pass framebuffer/view creation cost in blur and mip rendering.

## Submission And Completion

The shared queue submission uses pooled `VkSubmitResult` records containing a
fence, reference count, and completion flag. `vkGetFenceStatus()` performs
non-blocking CPU observation. A submission result can be shared by the parent
and every participating subcanvas, so all canvases can independently observe
the same GPU completion without resetting the fence early.

Upload callbacks are polled on a throttled path and resolve only after the fence
signals. There is no Vulkan command-completion callback equivalent to Metal's
completed handler in the baseline API.

Command buffers from multiple command pools can be submitted together. The
pool is not needed by `vkQueueSubmit()`, but it remains the allocator/owner.
`commands` keeps the flattened parent/child submission order, while
`ownCommands` contains only handles allocated from that Canvas command pool.
Each pack retains those handles and reuses them from `nextIdx` after completion
or discarded recording, avoiding per-frame free/allocate churn.

## Ownership Contracts

- `_outTex` is the Canvas-owned strong default-output texture.
- `_target` is a weak pointer to `_outTex` or to a texture whose lifetime is
  held by the current image/temp/clip state.
- `_cmdPackFront` is older than `_cmdPack`; deferred destruction recorded into
  a submitted current pack therefore occurs after earlier front work in the
  single shared queue.
- Per-level framebuffers and one-level views are owned and destroyed by their
  `VkTexture`. The texture's all-level base view remains a separate owned view.
- `VkShader` pipeline entries are borrowed references to resource-owned cached
  pipelines.

## Known Issues And Backlog

### Correctness

1. **Concurrent CPU layout transitions**

   Multiple CPU threads transitioning the same texture mip can race across the
   read/record/write sequence. The shared GPU queue serializes submitted work
   but does not make command recording an atomic transaction. This is deferred
   because current Qk sharing normally samples an already readable texture and
   does not concurrently render into it.

2. **Undefined read destination**

   `readImageCmd()` must initialize a newly allocated destination before using
   attachment `LOAD`; blend mode alone does not define destination contents.

3. **Compute-to-color attachment synchronization**

   If CAPA/compute has written a target and an ordinary graphics pass writes the
   same target next, beginning the color-attachment pass needs an explicit
   compute-to-color-attachment memory barrier. Selecting `CLEAR` or `DONT_CARE`
   and discarding the previous contents does not synchronize the two GPU writes.

4. **Mali-G51 driver compatibility candidate**

   One Honor LRA-AL00 / Kirin 710F device terminates the application from inside
   `vkCreateGraphicsPipelines()` instead of returning a `VkResult` while
   compiling Qk fragment pipelines that contain the common clip sampling path.
   Removing only the vector negation does not help; removing the complete clip
   branch lets the color pipeline pass, after which the image pipeline triggers
   the same process exit. The exact compiler pattern has not been isolated.

   Record the complete driver signature rather than treating every Mali-G51 as
   unsupported:

   - `vendorID`: `0x13b5`
   - `deviceID`: `0x70901010`
   - `apiVersion`: `VK_MAKE_VERSION(1, 0, 66)`
   - `driverVersion`: `0x03800000`

   Keep this as a compatibility candidate while testing other devices. If no
   practical shader workaround is found, reject this exact four-field signature
   before constructing `VulkanRenderResource` and fall back to GL. Do not wait
   for pipeline creation to fail because this driver exits the process without
   returning control to Qk.

### Missing Work

- Vulkan CAPA runtime validation and driver-specific corrections;
- driver/device capability validation and fallback behavior;
- broader Android device/driver profiling beyond the current test devices;
- broader Linux/Xlib driver testing and cross-architecture build validation.
  Ubuntu 20.04 x64 Release builds, links against the system Vulkan loader, and
  starts on a native X11 display. Forwarded X11 connections such as SSH/XQuartz
  are rejected before driver WSI queries and fall back to GL.

## Review Guardrails

- Do not report `clearFramebuffer()` as unsafe solely because the older front
  pack may reference the framebuffer; evaluate the actual single-queue order
  and deferred callback ownership.
- Do not make `usePipeline()` lazily begin a pass.
- Do not reject generic texture creation merely because a format is not a color
  attachment; validate render-target use at the caller.
- Do not add per-texture locks or atomic layout containers without an actual
  conflicting-writer path and measured need.
- Do not introduce a second queue/upload architecture before profiling the
  current shared path.
- Do not run broad builds automatically; use source inspection and let the user
  request compilation.
