# Current Work

This file is short-term memory for the current development thread. Update it when the active focus changes.

## Active Theme

The current major thread is GPU rendering quality after the GL/Metal backend alignment:

- Common Canvas behavior has moved into `src/render/gpu_canvas.*`.
- GL backend behavior lives in `src/render/gl/gl_canvas.*` and `src/render/gl/gl_command.*`.
- Metal backend behavior lives in `src/render/metal/mtl_canvas.*` and `src/render/metal/mtl_render.*`.
- GL is usually the behavior reference; Metal should match semantics without copying GL state-machine habits blindly.
- The next exploratory theme is improving `AASide` into a GPU-only signed-side edge AA path. Detailed notes live in `docs/GPU_2D_ANTIALIASING.md`.

## Recent Local State Observed

Commit `04995bae5` (`refactor(render): split GPUCanvas core and align GL/Metal backends`) landed a large render refactor. The codebase is now organized around `GPUCanvas` plus backend command implementations. Treat this as the current baseline, not a temporary diff.

Notable render-side state:

- Old `src/render/gl/gl_cmd.*` is deleted.
- New `src/render/gl/gl_command.*` exists and owns GL command packing/calls.
- New `src/render/gpu_canvas.*` exists and owns shared Canvas logic.
- GL blur has been refactored from fixed full-surface temporary textures to pooled dynamic textures. The GL path now keeps temporary blur textures alive through `GC_BlurFilter`, offsets the root matrix for local rendering, downsamples/mip-copies into the pooled texture, and uses shader UV offsets for final writeback.
- Troubleshooting notes are recorded in `docs/TROUBLESHOOTING.md`, including
  the GL UBO storage issue where `_uboClip` was bound but had no data store
  until clipping first ran, causing misleading black/gradient output.
- `Canvas::swapBuffer()` now returns `bool` to signal backend back-pressure/busy state.
- `Canvas::drawTriangles()` accepts `copyData`; GL deferred commands must copy transient triangle data when requested.
- `RenderResource` now exposes upload/unload resource ownership APIs (`uploadTexture`, `unloadTexture`, `uploadVertexData`, `unloadVertexData`) plus `RenderResource::useVertexData()`.
- `ImageSource` / `TexStat` were reworked around caller-owned texture state and multiple texture slots; do not reintroduce backend-owned `TexStat` allocation lifetimes.
- Metal `MTL_CmdPack` contains a `recorded` flag and tracks command buffers, current command buffer, pass descriptor, encoder, and pipeline.
- Metal `MetalCanvas::endPass()` centralizes ending the current encoder/pass and marking the command pack recorded.
- Metal `readImageCmd()` currently has a blit fast path when source/destination size and pixel format match, otherwise it uses the parameterized `cp` shader path.
- Metal `outputImageBeginCmd()` and `outputImageEndCmd()` are implemented at least partially through `mtl_rebuild_texture()`, render-target switching, and mipmap generation.
- Metal `drawClipCmd()` now renders clip masks into pooled `ImageSource` textures, restores clip state through shader clip texture/stat bindings, and follows GL intersect/difference mask combination semantics.
- Metal blur begin/end now uses pooled `tmpA`/`tmpB` textures from `GPUCanvas`, restores the root matrix after temp rendering, and uses the `cp`/`blur` shader paths for mip downsample and ping-pong blur; it still needs visual validation.

Current Metal backend state:

- Metal canvas command implementations have been split into `src/render/metal/mtl_canvas_cmd.mm`.
- The Metal backend now covers the GL-aligned draw path, including color/image/YUV/mask/SDF/gradient/rrect blur/triangles, clip masks, blur filters, `readImage()`, `outputImage()`, and viewport present copy.
- Metal command encoding now tracks explicit render-pass/encoder lifetime, front/current command packs, transient buffer allocation, sampler caching, and pipelines keyed by Metal pixel format.
- Visual validation is still useful for anti-aliased difference clips, nested clip restore behavior, blur edge sampling, and output-image mipmap use.

macOS live-resize note:

- The window-resize shimmer/jitter was investigated across `CAMetalLayer`, `MTKView`, main-thread redraw, `CATransaction`/`presentsWithTransaction`, `drawRect`/`setNeedsDisplay`, and freeze-during-resize variants.
- None of those experiments clearly improved the resize experience; GL and Metal now appear roughly comparable. Treat this as a low-priority macOS live-resize polish issue, not a blocking Metal backend bug.
- `CAMetalDisplayLink` on macOS 14+ was also tried as a replacement for `CVDisplayLinkCreateWithActiveCGDisplays`, but live window resizing showed worse behavior, including black edges during drag. It is currently disabled; keep the `CVDisplayLink` path as the active path for now.
- Keep the current `src/render/metal/mtl_apple.mm` baseline simple unless a new resize strategy is tested in isolation.

AA direction discussed:

- Do not pursue software AA / CPU coverage rasterization as the primary route. Quark should keep AA GPU-oriented.
- The current AA system has been renamed from `AAFuzz/aafuzz` to `AASide/aaSide`.
- The current working conclusion is that the existing vertex triplet `{x, y, aaSide}` is still enough; do not add extra vertex elements unless a later prototype proves it is necessary.
- Planned `aaSide` semantics: `aaSide < 0` means inside, `aaSide = 0` means the true edge, and `aaSide > 0` means outside. Vertex data currently stores side values at the two expanded band edges, not physical distance.
- The shader should use derivatives such as `fwidth(aaSide)` to estimate the device-pixel transition width and convert the signed side coordinate into coverage.
- The promising direction is to improve `AASide` into a GPU-only signed-side edge AA system, not a CPU/software coverage mask.
- `Path::getAASideStrokeTriangle()` now treats AA subpaths as closed when possible and signs generated `aaSide` from contour winding plus nesting level: inner side negative, outer side positive. Fragment shaders still mostly consume `abs(aaSide)`, so shader-side signed coverage remains a follow-up.
- See `docs/GPU_2D_ANTIALIASING.md` for the full design notes, including coverage-mask limitations, shader-side `fwidth` coverage, combined body/edge geometry, and hard cases.

Active AASide experiment branch:

- Branch `aa-side-refactor` was created from `master` and pushed to `origin/aa-side-refactor`.
- Baseline commit: `3f4400674` (`wip(render): experiment with signed aa side`).
- This branch is intentionally experimental; do not merge into `master` until the AA model is stable.
- `clip.glsl` was split out so clip mask drawing no longer overloads `color.glsl` with clip-only parameters such as `surfaceOffset`.
- `color.glsl` and other fragment shaders now call shared `aaSideCoverage()` from `_util.glsl`.
- `aa_side_weight` was removed from the experiment; the goal is to get coverage from signed `aaSide`, not from CPU-side alpha weighting.
- `Pathv` has been removed from the Canvas-facing drawing API on this branch. `RectPath` / `RectOutlinePath` now store paths only, while `PathvCache` separately caches normalized paths and generated `VertexData` for fill, AASide, and clip draws.
- `Hash5381` has been replaced by the new `Hash` API, including explicit float/vector update helpers and 32/64-bit mixed hash output. JS exports now expose `Hash`, and `test/util/test-hash.cc` is wired into the test target.

Current AASide findings:

- Latest AASide winding conclusion: `Path::getAASideTriangle()` now uses `boundaryPath()` / `TESS_BOUNDARY_CONTOURS` as the visible boundary source, calibrates the `aaSide` sign once from the first closed contour, and then reuses that sign mapping for later contours. Do not recompute `normalSide` per contour in the default path: holes already traverse in the opposite direction, so their generated normals naturally flip. The optional depth/containment pass in `collectContours(..., computeDepth)` is retained only as diagnostic/alternate context.
- A lightweight standalone SVG debugger now lives at `tools/stroke_debugger.html`. It supports draggable/addable/deletable points, stroke width/miter/filter controls, native SVG stroke comparison, custom offset preview, and point/normal overlays for experimenting with AASide simplification rules before moving them into C++.
- Superseded AASide finding: an earlier approach inferred `aaSide` by sampling containment depth and flipping on odd nesting levels. That is no longer the default after the tess boundary/winding analysis above, but the optional depth pass is kept in code for diagnostics and possible raw-contour callers.
- Confirmed problem 2: drawing the AA band before the body can reserve pixels for AA geometry when a path has multiple subpaths. After switching AASide generation to merged boundary + combined inner body geometry, the AASide path no longer needs a separate ordering buffer. Keep this as the active direction, but validate with nested clips, overlapping paths, and render-to-texture.
- Current experiment for problem 2: `Path::getAASideStrokeTriangle()` now also collects the inner offset contour, tessellates it as body geometry with `aaSide = -1`, and appends those body triangles to the AA band. `GPUCanvas` AA fill paths now draw this combined geometry instead of drawing AA first and then the original body. This should remove the AA/body ordering dependency for normal path fills.
- Remaining risk for problem 2: inner offset body contours can collapse or self-intersect for very small shapes, sharp joins, or degenerate contours. Visual validation is still needed before treating the combined AASide/body path as stable.
- Current experiment for overlapping subpaths: `Path::getAASideStrokeTriangle()` now always uses the private `boundaryPath()` helper to ask libtess2 for `TESS_BOUNDARY_CONTOURS` with the same `TESS_WINDING_POSITIVE` rule as `Path::getTriangles()`, then generates the inner body + AA band from that merged normalized line-only boundary. This should remove internal overlap edges from AASide generation and align AA geometry with the filled body. Visual validation is still needed for overlapping positives, holes/nested contours, and self-intersecting inputs.
- Depth/stencil removal follow-up: tag `aa-side-depth-baseline` preserves the last depth-enabled test snapshot. Current work removes render z-depth ordering, depth/stencil attachments/state, shader `pc.depth`, and AASide depth increments from GL/Metal/shared Canvas. The active validation target is now fully depth/stencil-free AASide blending/compositing.
- Degenerate/limit path cases remain for later cleanup. One known case is a full-circle arc with `useCenter = true`: center-to-boundary segments can become overlapping/parallel internal edges, and `getAASideStrokeTriangle()` / `strokePath()` do not currently have special handling for those seams. Extremely small contours, tiny angles, and repeated/near-repeated edges should be addressed together after the main AASide model is stable.

Metal upload performance note:

- In profiling, `MetalRenderResource::uploadTexture()` showed significant CPU in `newBufferWithBytes:length:options:` and `newTextureWithDescriptor:`.
- Text drawing in tests can amplify this by creating/uploading fresh text mask textures, but the backend issue is broader: upload staging buffers should be reused, and texture reuse should be considered when dimensions/format match.
- Do not work on this before the current AA thread unless explicitly requested; keep it as a later Metal performance task.

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

- Add visual regression coverage for Metal clipping, blur, readback, and output-image behavior against the GL reference output.
- Audit existing `AASide` generation and shader consumption in GL/Metal before changing behavior.
- Prototype GPU-only signed-side `AASide` coverage for simple straight-edge polygons, using shader-side `fwidth(aaSide)` coverage and combined body/edge geometry.
- Add or refresh small render regression demos if the project has an existing lightweight path for them.

## Verification Preference

For small render backend changes:

- Use targeted `rg` and local source inspection first.
- Do not run broad builds unless requested.
- If compiling is needed, ask or use the project’s known lightweight command if available.
