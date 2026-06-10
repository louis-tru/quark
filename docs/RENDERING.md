# Rendering Notes

This is the AI-facing map for Quark rendering. It focuses on code navigation and behavior-preserving edits.

## Main Layers

```txt
Canvas API
  -> GPUCanvas shared implementation
  -> backend `*Cmd` method
  -> GL command pack or Metal encoder
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
- `src/render/source.*`: `ImageSource` and `TexStat` texture lifecycle.
- `src/render/paint.*`: paint/image/gradient/filter state.
- `src/render/pathv_cache.*`: path triangulation and cached vertex data.

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

Current baseline:

- Commit `04995bae5` introduced this split as the working architecture.
- `GPUCanvas` should own behavior that is common to GL and Metal.
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

## Texture And Image Lifecycle

`ImageSource` manages image state and texture handles. It can represent decoded CPU pixels or a backend GPU texture.

`TexStat` is backend-neutral:

- GL uses `TexStat::id()`.
- Metal uses `TexStat::ptr()` with retained Objective-C texture pointers.

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

This matters because generated GL/Metal shader wrappers may move slots.

## Current Validation Risks

The primary Metal drawing paths are implemented. Remaining work is mainly
validation and performance hardening:

- anti-aliased difference clips and nested clip restore behavior
- blur edge sampling and temporary texture reuse
- output-image mipmap use and render-target transitions
- `drawTrianglesCmd()` transient buffer lifetime and buffer reuse
- upload staging-buffer and compatible texture reuse

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
