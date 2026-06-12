# Current Work

This file is short-term memory for the current development thread. Update it when the active focus changes.

## Active Theme

The `aa-side-refactor` rendering-quality thread is now at a stage closeout:
AASide has replaced the old AAFuzz/UI-compensation approach for normal GUI
drawing, and the branch is intended to be merged back to `master`.

The next rendering-quality theme should be a higher-precision Compute AA path
for Metal/Vulkan-class backends, with AASide retained as the fast GL/GLES
fallback. Detailed AA notes live in `docs/GPU_2D_ANTIALIASING.md`.

The current rendering architecture remains:

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
- AASide is now considered good enough for the current GL/GLES-compatible path:
  it is much more geometrically accurate than the previous UI-level shrink/
  overlap compensation and has allowed that compensation code to be removed.
- The existing vertex triplet `{x, y, aaSide}` remains the AASide vertex format.
- Current `aaSide` semantics: `aaSide < 0` means inside the shape,
  `aaSide = 0` means the true edge, and `aaSide > 0` means outside the shape.
- Fragment shaders derive the transition ramp with `fwidth(aaSide)` through
  `aaSideCoverage()`. This works well for ordinary edges, but it cannot express
  exact coverage where multiple nearby edges affect the same pixel.
- `Path::getAASideTriangle()` resolves visible boundaries with `boundaryPath()`,
  calibrates the `normalSide` sign from the first closed contour only, then
  reuses that mapping for all later contours. Do not recompute containment depth
  per contour; holes already reverse traversal direction and would otherwise be
  compensated twice.
- See `docs/GPU_2D_ANTIALIASING.md` for the full design notes, including
  coverage limitations, shader-side `fwidth` coverage, combined body/edge
  geometry, hard cases, and the Compute AA plan.

Active AASide experiment branch:

- Branch `aa-side-refactor` was created from `master` and pushed to `origin/aa-side-refactor`.
- Baseline commit: `3f4400674` (`wip(render): experiment with signed aa side`).
- The branch is no longer just a throwaway experiment; the current intent is to
  merge it into `master` as the new default AASide baseline.
- `clip.glsl` was split out so clip mask drawing no longer overloads `color.glsl` with clip-only parameters such as `surfaceOffset`.
- `color.glsl` and other fragment shaders now call shared `aaSideCoverage()` from `_util.glsl`.
- `aa_side_weight` was removed from the experiment; the goal is to get coverage from signed `aaSide`, not from CPU-side alpha weighting.
- `Pathv` has been removed from the Canvas-facing drawing API on this branch. `RectPath` / `RectOutlinePath` now store paths only, while `PathvCache` separately caches normalized paths and generated `VertexData` for fill, AASide, and clip draws.
- `Hash5381` has been replaced by the new `Hash` API, including explicit float/vector update helpers and 32/64-bit mixed hash output. JS exports now expose `Hash`, and `test/util/test-hash.cc` is wired into the test target.
- Painter-level AA compensation has been removed from the active UI path. Box rects, borders, outlines, and sprite image rects now use their exact geometry instead of `AAShrink` / `originAA` offsets, leaving edge overlap control to renderer-side AASide parameters.
- Render z-depth and depth/stencil attachments/state were removed from the
  AASide path. AASide blending/compositing should now work without relying on a
  body-over-edge depth ordering trick.
- `GPUCanvas` uses different AA radii for ordinary paths and rect/rrect-heavy UI
  paths. Translation-only UI rects currently use a tighter 0.5 px radius, while
  general transformed paths keep a wider radius for small-angle line stability.

Current AASide findings:

- AASide joins below 90 degrees now replace the sharp source vertex with two points on its adjacent edges. Both new corners then run through the normal `miterNormal` / `a` / `b` generation path, so the inserted cut segment receives normal AASide geometry. The cut decision is purely local geometry and does not use the global `normalSide` coverage sign.
- Latest AASide winding conclusion: `Path::getAASideTriangle()` now uses `boundaryPath()` / `TESS_BOUNDARY_CONTOURS` as the visible boundary source, calibrates the `aaSide` sign once from the first closed contour, and then reuses that sign mapping for later contours. Do not recompute `normalSide` per contour in the default path: holes already traverse in the opposite direction, so their generated normals naturally flip.
- A lightweight standalone SVG debugger now lives at `tools/stroke_debugger.html`. It supports draggable/addable/deletable points, stroke width/miter/filter controls, native SVG stroke comparison, custom offset preview, and point/normal overlays for experimenting with AASide simplification rules before moving them into C++.
- Superseded AASide finding: an earlier approach inferred `aaSide` by sampling containment depth and flipping on odd nesting levels. That is no longer the default after the tess boundary/winding analysis above, but the optional depth pass is kept in code for diagnostics and possible raw-contour callers.
- Confirmed problem 2: drawing the AA band before the body can reserve pixels for AA geometry when a path has multiple subpaths. After switching AASide generation to merged boundary + combined inner body geometry, the AASide path no longer needs a separate ordering buffer. Keep this as the active direction, but validate with nested clips, overlapping paths, and render-to-texture.
- Current experiment for problem 2: `Path::getAASideStrokeTriangle()` now also collects the inner offset contour, tessellates it as body geometry with `aaSide = -1`, and appends those body triangles to the AA band. `GPUCanvas` AA fill paths now draw this combined geometry instead of drawing AA first and then the original body. This should remove the AA/body ordering dependency for normal path fills.
- Remaining risk for problem 2: inner offset body contours can collapse or self-intersect for very small shapes, sharp joins, or degenerate contours. Visual validation is still needed before treating the combined AASide/body path as stable.
- Current experiment for overlapping subpaths: `Path::getAASideStrokeTriangle()` now always uses the private `boundaryPath()` helper to ask libtess2 for `TESS_BOUNDARY_CONTOURS` with the same `TESS_WINDING_POSITIVE` rule as `Path::getTriangles()`, then generates the inner body + AA band from that merged normalized line-only boundary. This should remove internal overlap edges from AASide generation and align AA geometry with the filled body. Visual validation is still needed for overlapping positives, holes/nested contours, and self-intersecting inputs.
- Depth/stencil removal follow-up: tag `aa-side-depth-baseline` preserves the last depth-enabled test snapshot. Current work removes render z-depth ordering, depth/stencil attachments/state, shader `pc.depth`, and AASide depth increments from GL/Metal/shared Canvas. The active validation target is now fully depth/stencil-free AASide blending/compositing.
- Degenerate/limit path cases remain for later cleanup. One known case is a full-circle arc with `useCenter = true`: center-to-boundary segments can become overlapping/parallel internal edges, and `getAASideStrokeTriangle()` / `strokePath()` do not currently have special handling for those seams. Extremely small contours, tiny angles, and repeated/near-repeated edges should be addressed together after the main AASide model is stable.

Stage closeout notes:

- Normal GUI rendering now looks stable enough to stop prioritizing AASide
  tuning. Remaining artifacts are known model limitations rather than obvious
  parameter bugs.
- Subpixel and 1 px UI borders improved after moving responsibility out of the
  UI painter and into renderer-side AASide. The old UI shrink/offset workaround
  is intentionally gone.
- SDF text mask rendering now uses `fwidth(dist)` in
  `image_sdf_mask.glsl`, which gives better scale-dependent edge softness than
  the old fixed transition width. Large outlined SDF text can still look blunt
  at corners because the SDF itself rounds expanded corners.
- CoreText image-mask text on Apple still needs more investigation. The current
  finding is that Apple paths are RGBA-oriented, not a reliable A8 texture
  generation path, and large generated glyph masks can still show jagged edges.
- JPEG decode now uses libjpeg-turbo extended RGBA/RGBX-style output for better
  GPU upload compatibility, because Metal has no native 24-bit RGB888 texture
  format.
- `PixelInfo::block_info()` now describes block-compressed formats so row bytes
  and upload sizes can be calculated for ETC/BC/PVRTC-style textures.
- GL `readImage()` had a state-machine bug where `glTexImage2D()` could
  reallocate the currently bound source texture instead of the destination
  texture. The destination texture must be bound before allocating storage, then
  the source texture rebound for sampling.
- Window resize jitter on macOS was traced to Root layout mark propagation, not
  GL/Metal presentation. After `layout_lock_width/height` updates the layout
  marks, the temporary local `mark` must be refreshed from `_mark_value` or
  child layout can lag by one frame.

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
- Keep AASide as the GL/GLES-compatible fast path and avoid spending more high
  priority time on its known extreme cases.
- Prototype Metal Compute AA with CPU-transformed flattened edges, CPU tile
  binning, tile backdrop winding, and per-pixel coverage written into a
  temporary coverage atlas.
- Add specialized analytic rect/rrect/border renderers for common UI primitives
  before attempting a universal complex-path renderer.
- Add or refresh small render regression demos for 1 px lines, rounded rects,
  readback, output-image, blur, and resize behavior.

## Verification Preference

For small render backend changes:

- Use targeted `rg` and local source inspection first.
- Do not run broad builds unless requested.
- If compiling is needed, ask or use the project’s known lightweight command if available.

## Compute AA Prototype State

An isolated Metal Compute AA prototype now lives in `test/compute_aa/` and is
wired into the macOS test target. It demonstrates CPU-flattened path edges,
16x16 tile binning, fixed-grid subpixel winding coverage, a coverage texture, and solid
color composition. A matching WebGPU/CPU comparison page also exists in the
same directory.

The active branch is `experiment/compute-aa-cpu-backdrop`. It is based on
commit `706ec42bc` on the pushed `experiment/compute-aa-row-mask` branch, but
its current CPU-backdrop/private-delta work is intentionally uncommitted for
continued measurement.

The prototype uses local tile edge lists plus a per-tile **backdrop**: the
winding already accumulated at the tile's left boundary. `build_tile_edges()`
walks each edge across X tile columns using one `dy/dx` slope. A step does
nothing unless its discrete Y-sample-grid position changes. Changed sample
ranges add one original-edge reference with a local sample range. The active
branch records backdrop as per-row 2D deltas, then performs CPU Y/X prefix
passes to upload one final winding for every tile-local Y sample.

The current prototype remains intentionally CPU-heavy and rebuilds/uploads data
frequently. Important future work includes caching immutable path data, pooled
GPU buffers/textures, dirty-path rebuilds, batching multiple paths into an
atlas, and eventually moving more binning/backdrop work onto the GPU.
The current algorithm and data contracts are documented in Chinese in
`test/compute_aa/README.md`.

The CPU construction pass has now been tightened around a thread-local
`LinearAllocator`: temporary tile/backdrop buckets use arena-backed arrays,
ordinary Compute AA records skip unnecessary construction/destruction, path
translation has a multiply-free fast path, and edge/tile traversal avoids
several repeated calculations. Treat CPU-side prototype optimization as mostly
complete for now; the next profiling and optimization work should focus on GPU
coverage/composition, dispatch shape, buffer upload/reuse, and reducing
CPU-to-GPU synchronization.

The active Metal coverage kernel uses one SIMD32-sized threadgroup for two
horizontally adjacent 16x16 tiles. Each tile gets 16 threads, and each thread
computes one complete pixel row and directly accumulates its samples into 16
pixel coverage values. Each thread uses one private 64-entry winding delta for
the current Y sample row, so edges are traversed once per sample row without
using a large threadgroup `windingDelta`, `insideMask`, or a barrier. Measure
register/local-memory pressure before keeping this structure.

Clear and solid composite are no longer compute kernels. Composite is a normal
render pass that uses `MTLLoadActionClear`, draws one atlas-sized quad, samples
the R8 coverage texture, and uses fixed-function premultiplied-alpha blending.
This removed the standalone clear pass and explicit drawable read-modify-write.

### Latest Measurements

Measurements used the rotating/scaled comparison path, a roughly 1039x1085
atlas, 4x4 samples, and about 4420 tiles. Values vary by capture/run, but the
direction is consistent:

- Saved row-mask/GPU-event versions were around 1.4-1.6 ms total.
- CPU-precomputed backdrop reduced the total to about 1.11-1.30 ms, but raised
  CPU construction/upload cost.
- Deleting shared delta/mask data and repeatedly searching unsorted crossings
  regressed to roughly 2.0-2.4 ms; do not repeat this approach.
- A 16-thread single-tile row kernel left half of SIMD32 idle. Pairing two
  adjacent tiles into one 32-thread group brought that experiment near 1.2 ms.
- The active 32-thread/private-delta kernel plus render composite measured about
  0.88-0.91 ms before pass cleanup.
- Replacing compute clear/composite with one clear+composite render pass reduced
  total GPU time to about 0.60 ms. Xcode showed roughly 62% Coverage and 38%
  Composite.
- The equivalent AASide comparison remains around 0.20 ms, so full-atlas
  Compute AA is still about 3x slower even after the major structural wins.

### Findings And Next Direction

- Edge intersection math is not the dominant cost. Commenting it out left much
  of Coverage time intact.
- Dense shared `windingDelta[64][64]`, inside-mask construction, and the barrier
  had substantial fixed cost. The private per-thread delta structure is better.
- CPU-precomputed per-tile backdrop helps GPU time only modestly and trades away
  CPU time plus upload bandwidth.
- CPU sorting intersections per Y sample would approach CPU scanline
  rasterization and does not solve the full-atlas/intermediate-texture cost.
- Even with a free Coverage pass, sampling and compositing a full coverage atlas
  is already near the AASide total cost.
- The most promising next architectural experiment is to dispatch precise
  Compute AA only for boundary/edge tiles, skip exterior tiles, and fill solid
  interior regions through ordinary hardware rasterization. Full-atlas Compute
  AA is unlikely to match AASide for dynamic paths.

Metal shader compilation, metallib creation, ObjC++ syntax checks, and
`git diff --check` currently pass. Existing warnings are the unused
`QK_COMPUTE_AA_NON_ZERO` constant and the unrelated Clipboard visibility
warning.

## Debugger Helpers

Custom Xcode LLDB formatters live in `.lldb/qk_lldb.py`, with the committed
template `.lldb/lldbinit-Xcode`. `tools/configure.js` generates the root
`.lldbinit-Xcode` using the absolute local script path.

Current formatters cover common Quark vectors, matrices, ranges, strings,
arrays, lists, dictionaries, sets, and iterators. Container synthetic children
retain useful raw members and limit displayed data items to avoid expanding
very large containers. Xcode does not render newlines inside value summaries,
so `Mat` and `Mat4` summaries must remain compact single-line matrices.

Long-term guidance for Quark's union-based `bitwise_cast` is recorded in
`docs/TROUBLESHOOTING.md`. Do not mechanically replace it based only on abstract
standards advice; inspect actual types, toolchains, generated code, and the
replacement's misuse surface first.
