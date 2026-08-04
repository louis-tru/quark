# Rendering Notes

This is the AI-facing map for Quark rendering. It focuses on code navigation and behavior-preserving edits.

## Main Layers

```txt
Canvas API
  -> GPUCanvas shared implementation
  -> backend `*Cmd` method
  -> GL command pack, Metal encoder, or Vulkan command buffer
  -> GPU surface / offscreen target
```

Key files:

- `src/render/canvas.h`: public Canvas API and low-level structs such as `Triangles`.
- `src/render/gpu_canvas.h`: `GPUCanvas` interface and required backend `*Cmd` methods.
- `src/render/gpu_canvas.cc`: shared implementation for draw paths, text, filters, read/output image, matrix/clip state.
- `src/render/gl/gl_canvas.*`: GLCanvas wrapper around `GLC_CmdPack`.
- `src/render/gl/gl_command.*`: GL command storage and actual GL draw calls.
- `src/render/metal/mtl_canvas.*`: MetalCanvas direct command encoding.
- `src/render/metal/mtl_render.*`: Metal resource upload, texture helpers, samplers, pipelines.
- `src/render/vulkan/vk_canvas.*`: Vulkan command packs, render passes,
  descriptors, framebuffers, and Canvas commands.
- `src/render/vulkan/vk_render.*`: shared Vulkan device resources, shaders,
  pipelines, queue submission, and completion state.
- `src/render/source.*`: `ImageSource` and `TexStat` texture lifecycle.
- `src/render/paint.*`: paint/image/gradient/filter state.
- `src/render/pathv_cache.*`: path triangulation and cached vertex data.

## Current Rendering Strategy

Quark now uses a hybrid GPU renderer rather than a single universal path.

- **AASide** remains the GL/GLES-compatible fast path and the preferred path
  for hairline strokes and text. Its distance/edge-band behavior often gives
  better perceptual quality on simple straight edges than strict area coverage.
- **CAPA** (Coverage Area Pipeline Anti-Aliasing) is the primary Metal-class
  compute path for most AA filled paths. It batches path commands, transforms
  edges, bins them into tiles, computes per-tile area coverage, builds ordered
  global-tile layer spans, and composites in order to avoid multi-primitive
  background leakage.
- **CGAA** is no longer the active direction. Keep it only as a historical
  reference/fallback if it remains in a branch; do not extend it unless a task
  explicitly asks for that.

CAPA is not intended to replace every simple-edge renderer. Its value is
ordered coverage and correct composition for complex GUI scenes. If a future
quality pass is needed, prefer renderer selection or an explicit optional mode
over changing CAPA's area coverage semantics.

The current practical default is:

- most AA path fills: CAPA where available;
- hairline strokes and text: AASide;
- GL/GLES fallback: AASide;
- expensive Canvas state changes such as readback, output-image transitions,
  blur/filter passes, or unsupported clip changes may flush the CAPA batch.

Recent iOS validation:

- GL/GLES, Metal, and CAPA have all rendered the current kace/text-heavy iOS
  test scenes correctly and smoothly in simple smoke tests.
- Simulator/window scaling can hide aliasing because the device framebuffer is
  downsampled again by the host display path. Use 1:1 framebuffer inspection
  when judging AA quality.
- Mobile CPU cost still matters. CAPA performs well when it records large
  batches, but frequent flushes can dominate CPU time.
- Android GL/GLES profiling shows that dynamic R8 text/image texture upload can
  dominate CPU time on midrange devices. Treat GL as the correctness/fallback
  path there; Vulkan is the next likely performance path if upload/driver cost
  remains the limiter.

The current executable CAPA pass plan is documented in
`docs/CAPA_PASS_PROCESS.md`; shader source lives in `src/render/shader/capa/`.

## Shared GPUCanvas Responsibilities

`GPUCanvas` owns behavior that should not be duplicated in each backend:

- state stack, save/restore, current matrix
- clip-mask stack orchestration and restore behavior
- surface size, root matrix, scale values
- path normalization, fill/stroke selection, and AASide geometry selection
- text image selection and SDF text path
- blur filter lifetime wrapper
- high-level `readImage()` and `outputImage()` flow

Backends implement only the operations declared as pure virtual `*Cmd` methods in `gpu_canvas.h`.

- `GPUCanvas` should own behavior that is common to GL, Metal, and Vulkan.
- Backend files should focus on resource upload, command storage/encoding, pipeline state, and draw/read/output implementations.
- Avoid moving shared behavior back into GL just because GL is currently the most complete backend.

## GL Backend Shape

`GLCanvas` usually does little work directly. It records commands into `GLC_CmdPack`.

`GLC_CmdPack` owns:

- command allocation and deferred execution
- copies of transient command data when needed
- GL pipeline state restoration
- FBO/texture binding
- actual draw calls

Important GL notes after the refactor:

- `gl_command.*` replaces the old `gl_cmd.*` files.
- Matrix and blend changes are recorded through backend command hooks from `GPUCanvas`.
- Ordinary view position should not require a canvas matrix update. The painter
  can express simple positions in draw coordinates and reserve matrix changes
  for transform boundaries such as scroll/morph.
- Deferred commands must respect data lifetime. `drawTriangles(copyData=true)` is the explicit signal to copy transient vertex/index buffers.
- See `docs/TROUBLESHOOTING.md` for rendering failure modes, especially the
  requirement that persistent UBO/VBO objects have storage allocated before any
  shader using their binding point draws.
- GL remains the main behavior reference for Metal, but not every GL state-machine operation should be mirrored literally.

Use GL as a behavior reference for Metal, especially for:

- clipping and clip-mask combination behavior
- blur filter behavior
- read/output image semantics
- drawTriangles data lifetime
- mipmap generation rules

GLES-specific differences:

- Do not rely on core `GL_CLAMP_TO_BORDER` on GLES. ES backends should emulate
  transparent border sampling for `PaintImage::kDecal_TileMode` in shader code
  when needed, otherwise `GL_CLAMP_TO_EDGE` will stretch the edge texel and can
  create visible strips.
- ES300 shader output expands the `PcArgs` push-constant-style uniform into
  separate ordinary uniforms such as `pc_flags` and `pc_texCoords`. This avoids
  iOS GLES linker failures caused by struct uniform type or precision mismatch
  across vertex/fragment stages.
- ES300 expansion must preserve per-member precision. Coordinates, matrices,
  surface offsets, and image `texCoords` should be `highp`; colors, coverage,
  alpha values, and most sampler math can be `mediump`.
- Fragment shader integer precision should be explicit `highp` for flags and
  bit masks.
- Do not rely on implicit precision defaults for uniforms or blocks shared
  across vertex and fragment stages. Same-name uniforms/blocks need matching
  type and precision after generation. Varying precision can differ by stage,
  but the effective precision is constrained by the lower-precision side, so UV
  or coordinate varyings should be high precision end to end when large scroll
  offsets are involved.

## Metal Backend Shape

`MetalCanvas` encodes more directly than GL:

- `MTL_CmdPack` stores command buffers, current command buffer, current pass, current encoder, current pipeline, and a `recorded` flag.
- `beginPass()` selects the current output texture from `_state->output` or `_outTex`.
- `getEncoder()` lazily creates the current render encoder and uploads root/view matrices plus aaclip texture binding.
- `endPass()` ends any active encoder/pass and clears encoder/pipeline/pass state.
- `swapBuffer()` ends the pass, swaps current/front command packs, and starts a new current command buffer.
- `flushBuffer()` returns front command buffers to be committed by the render backend.

Metal-specific rule of thumb:

- Switching render targets means ending the current render encoder/pass.
- It does not automatically require a new command buffer.
- Use a new command buffer only when ordering/ownership makes it necessary.
- Prefer generated shader indices (`shader.fragment.*`, `shader.bufferIndex`) over fixed slot numbers.
- Metal implements the main GL-aligned clip/blur/read/output paths, but visual
  validation is still useful for difficult combinations and edge cases.

## Vulkan Backend Shape

Vulkan follows the same `GPUCanvas` command semantics while making implicit GL
state explicit:

- one application-wide device/graphics queue is owned by
  `VulkanRenderResource`;
- every Canvas owns a command pool and current/front `VkCmdPack`;
- command packs own descriptor pools, transient allocators, resource
  references, completion callbacks, and command-buffer lists;
- generated SPIR-V reflection drives descriptor-set layouts, pipeline layouts,
  vertex attributes, push constants, and pipeline creation;
- textures track layout per mip and record explicit barriers between transfer,
  render-target, and sampled use;
- submit/present calls on the shared queue require external synchronization,
  while command recording can use independent Canvas pools.

Most ordinary non-CAPA drawing commands are implemented. Framebuffer mip views,
discarded-pack layout state, platform presentation, and CAPA remain active work.
See [`VULKAN.md`](VULKAN.md) for the authoritative architecture and backlog.

### Geometry And Memory Cache Policy

`TexturePool` and `PathvCache` are high-level semantic caches. Their normal
contents are expected to remain alive and be reused for a long time; they are
not equivalent to the low-level Vulkan memory-allocation cache.

The intended future policy for Vulkan path vertices is:

- ordinary `PathvCache` entries are long-lived by default, analogous to
  textures carrying `kLongLife_TextureFlags`;
- long-lived final vertex allocations should not also occupy the reusable
  `VkMemoryAllocator` pool, because `PathvCache` already owns their caching
  policy;
- transient staging memory should continue to use `VkMemoryAllocator`, where
  reuse avoids repeated `vkAllocateMemory` calls;
- if dynamic SVG/path animation later produces continuously changing geometry,
  add an explicit dynamic cache policy rather than treating every path as
  transient; dynamic entries may be cleaned preferentially by `PathvCache`,
  and their released Vulkan memory may use the low-level allocator pool;
- do not reuse the texture-specific `kLongLife_TextureFlags` directly for
  vertices. Introduce a generic cache policy or a vertex-specific flag when
  this distinction is implemented.

`PathvCache` currently defaults to 1/256 of system memory (2GB -> 8MB,
4GB -> 16MB, 8GB -> 32MB), clamped to 8MB-128MB. This deliberately keeps the
high-level per-Canvas cache substantially smaller than the application-wide
Vulkan allocator. The first priority remains a high `PathvCache` semantic hit
rate; the Vulkan allocator is only a lower-level allocation-cost optimization.

This long-lived/dynamic distinction is a design note only. No dynamic-path
cache flag is implemented yet.

## Texture And Image Lifecycle

`ImageSource` manages image state and texture handles. It can represent decoded CPU pixels or a backend GPU texture.

`TexStat` is backend-neutral:

- GL uses `TexStat::id()`.
- Metal uses `TexStat::ptr()` with retained Objective-C texture pointers.
- Vulkan uses `TexStat::ptr()` with `VkTexture` reference-counted wrappers.

Important helpers:

- `ImageSource::markAsTexture(RenderResource*)`: ensure a CPU image is uploaded to a backend texture when explicit upload is needed.
- `setTex_SourceImage(...)`: internal helper that stores a new texture in an `ImageSource` and marks it loaded.
- `mtl_get_texture(...)`: Metal helper for reading a Metal texture from `TexStat`.
- `mtl_rebuild_texture(...)`: Metal helper for creating/reusing renderable shader-readable textures.
- `RenderResource::useVertexData(...)`: lazy helper for backend-local vertex upload.

When replacing a texture, preserve ownership semantics. Metal texture pointers stored in `TexStat` must be retained, and old resources should be released through the resource path.

Resource lifecycle rules:

- `RenderResource` backends upload/unload resources into caller-owned containers.
- Do not allocate `TexStat` in a backend and transfer ownership upward.
- `ImageSource` can hold multiple texture slots; check current `source.h` before assuming only slot 0 exists.
- Mipmap state is carried by `ImageSource` and `PaintImage`; avoid implicit upload paths in painter code unless explicitly intended.

## `readImage()` And `outputImage()`

`GPUCanvas::readImage()`:

- clamps source size to canvas size
- creates an `ImageSource` with requested destination size/type
- sets mipmap preference
- calls backend `readImageCmd(srcRect, currentOutput, dest)`

`GPUCanvas::outputImage()`:

- creates or accepts an output `ImageSource`
- stores it in current canvas state
- calls backend `outputImageBeginCmd()`
- drawing after this point targets the image until restore pops back to the previous output

`GPUCanvas::restore()` calls `outputImageEndCmd(exit)` when popping an output image state.

Do not confuse:

- `Canvas::outputImage()` produces a render target.
- `PaintImage::setCanvas()` samples an already rendered canvas as input.

Blur filter notes:

- `GPUCanvas` computes blur sampling, image LOD, and `clearPad`.
- `clearPad` guards blur sampling near temporary texture edges; it accounts for scaled/mipmapped sampling.
- `bounds` passed to blur backend methods already includes the blur radius.
- Metal blur uses pooled temporary `ImageSource` textures for ping-pong
  rendering and restores the previous output/root matrix afterward.

## Shader Slots

Do not hard-code texture and buffer slots when a shader wrapper exposes indices. Prefer:

- `shader.bufferIndex`
- `shader.fragment.image`
- `shader.fragment.aaclip`
- other generated `shader.vertex.*` / `shader.fragment.*` fields

This matters because generated GL/Metal/Vulkan shader wrappers may move slots.

## Current Validation Risks

The primary Metal drawing paths are implemented. Remaining cross-backend work
is mainly validation and Vulkan completion:

- anti-aliased difference clips and nested clip restore behavior
- blur edge sampling and temporary texture reuse
- output-image mipmap use and render-target transitions
- `drawTrianglesCmd()` transient buffer lifetime and buffer reuse
- upload staging-buffer and compatible texture reuse
- Vulkan framebuffer mip-view ownership and cache identity
- Vulkan provisional layout state when a command pack is discarded
- Vulkan surface/swapchain/present integration and CAPA

When changing these paths, read the GL equivalents for behavior and the current
Metal implementation for explicit encoder/resource lifetime.

## Common Mistakes

- Treating Quark as a web runtime.
- Putting shared Canvas behavior into one backend only.
- Hard-coding shader resource slots.
- Forgetting that GL commands are deferred and may need copied data.
- Creating extra Metal command buffers just to switch render targets.
- Reintroducing old depth/stencil or z-order assumptions into the AASide path.
- Losing mipmap generation when render output is used as a texture.
- Replacing `ImageSource` texture pointers without respecting `TexStat` ownership.
- Assuming files in `deps/`, `node_modules/`, `tools/ndk/`, `tools/pkgs/`, `tools/linux/`, or `out/` are useful for Quark architecture context.
