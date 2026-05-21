# Current Work

This file is short-term memory for the current development thread. Update it when the active focus changes.

## Active Theme

The current major thread is the GPU rendering refactor and backend alignment:

- Common Canvas behavior has moved into `src/render/gpu_canvas.*`.
- GL backend behavior lives in `src/render/gl/gl_canvas.*` and `src/render/gl/gl_command.*`.
- Metal backend behavior lives in `src/render/metal/mtl_canvas.*` and `src/render/metal/mtl_render.*`.
- GL is usually the behavior reference; Metal should match semantics without copying GL state-machine habits blindly.

## Recent Local State Observed

Commit `04995bae5` (`refactor(render): split GPUCanvas core and align GL/Metal backends`) landed a large render refactor. The codebase is now organized around `GPUCanvas` plus backend command implementations. Treat this as the current baseline, not a temporary diff.

Notable render-side state:

- Old `src/render/gl/gl_cmd.*` is deleted.
- New `src/render/gl/gl_command.*` exists and owns GL command packing/calls.
- New `src/render/gpu_canvas.*` exists and owns shared Canvas logic.
- Troubleshooting notes are recorded in `docs/TROUBLESHOOTING.md`, including
  the GL UBO storage issue where `_uboClip` was bound but had no data store
  until clipping first ran, causing misleading black/gradient output.
- `Canvas::swapBuffer()` now returns `bool` to signal backend back-pressure/busy state.
- `Canvas::drawTriangles()` accepts `copyData`; GL deferred commands must copy transient triangle data when requested.
- `RenderResource` now exposes upload/unload resource ownership APIs (`uploadTexture`, `unloadTexture`, `uploadVertexData`, `unloadVertexData`) plus `RenderResource::useVertexData()`.
- `ImageSource` / `TexStat` were reworked around caller-owned texture state and multiple texture slots; do not reintroduce backend-owned `TexStat` allocation lifetimes.
- Metal `MTL_CmdPack` contains a `recorded` flag and tracks command buffers, current command buffer, pass descriptor, encoder, and pipeline.
- Metal `MetalCanvas::endPass()` centralizes ending the current encoder/pass and marking the command pack recorded.
- Metal `readImageCmd()` currently has a blit fast path when source/destination size and pixel format match, otherwise it uses `vportCp`.
- Metal `outputImageBeginCmd()` and `outputImageEndCmd()` are implemented at least partially through `mtl_rebuild_texture()`, render-target switching, and mipmap generation.
- Metal blur begin/end has an active ping-pong/mipmap implementation and uses `clearPad` from `GPUCanvas` for scaled/mipmapped edge sampling; it is still part of the migration and needs careful validation.

Remaining Metal TODO / high-risk areas observed in `src/render/metal/mtl_canvas.mm`:

- `drawClipCmd`
- blur filter end path still contains GL-style calls/comments and should be ported/validated as Metal commands before being trusted.
- `drawTrianglesCmd()` currently allocates Metal buffers directly from provided data; revisit `copyData` and lifetime behavior after the command model settles.

## Important Cautions

- Do not assume earlier conversation state is still true; inspect the current file before patching. The user may have edited between turns.
- Avoid pushing current Metal command buffers into `_cmdPack.cmds` unless command ordering truly requires it.
- In Metal, switching render targets requires ending the current `MTLRenderCommandEncoder`, but not necessarily creating a new `MTLCommandBuffer`.
- Prefer shader slot indices from shader structs, such as `shader.fragment.image`, over hard-coded texture slots.
- `copyData` matters for deferred GL command data lifetime. Metal direct encoding may have different lifetime requirements.
- The render coordinate convention moved toward screen-space y-down with final-present Y flip in shaders; do not add ad hoc Y flips without checking `math.h`, `vport_cp.glsl`, and `vport_full_cp.glsl`.
- `PaintImage` bitfields and mipmap modes changed; check `paint.h` before assuming old filter/mipmap encodings.
- `Pixel::body()` was renamed to `Pixel::buffer()`.

## Suggested Next Render Tasks

- Finish Metal clipping support by matching GL stencil/aaclip semantics.
- Finish/validate Metal blur filter begin/end behavior and remove remaining GL-style calls from the Metal path.
- Audit Metal `readImageCmd()` coordinate math against GL `readImageCall()`.
- Confirm `outputImage()` restore behavior with nested `save()` / `restore()`.
- Add a small render regression/demo if the project has an existing lightweight path for it.

## Verification Preference

For small render backend changes:

- Use targeted `rg` and local source inspection first.
- Do not run broad builds unless requested.
- If compiling is needed, ask or use the project’s known lightweight command if available.
