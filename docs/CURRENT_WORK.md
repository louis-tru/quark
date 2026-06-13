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

The prototype uses local tile edge lists plus a per-tile **backdrop**: the
winding already accumulated at the tile's left boundary. `build_tile_edges()`
walks each edge across X tile columns using one `dy/dx` slope. A step does
nothing unless its discrete Y-sample-grid position changes. Changed sample
ranges add one original-edge reference with a local sample range to crossed
tiles and append one backdrop event. CPU construction never expands backdrop
winding across tiles to the right. During coverage, each tile threadgroup
cooperatively resolves its tile row's backdrop events into threadgroup memory.
This removes the previous edge-bounding-box fill, repeated crossing
calculations, per-tile backdrop storage/writes, backdrop delta buffer, and CPU X
prefix sum. The X-tile scan now advances directly in sample-grid Y coordinates:
fixed power-of-two sample/tile divisions use shifts, tile-boundary Y uses
incremental addition, and the inner X-tile loop contains no division or
per-iteration multiplication.

The Compute GRID AA prototype algorithm is now considered complete enough to
enter Quark's rendering architecture. It remains intentionally isolated, but
correctness, data contracts, tile classification, coverage generation,
batch-oriented CPU output, and representative performance have been validated.
The intended production model builds one batch of path data, lays all tiles in
that batch into one coverage atlas, and generates/encodes the CPU/GPU data
together. Production integration may require small adjustments to fit Quark's
existing render-command, resource-ownership, buffer/texture-pool, and lifetime
structures; it does not require revisiting the core CPU allocation strategy.
The current algorithm and data contracts are documented in Chinese in
`test/compute_aa/README.md`.

The CPU construction pass has now been tightened around a thread-local
`LinearAllocator`: temporary tile/backdrop buckets use arena-backed arrays,
ordinary Compute AA records skip unnecessary construction/destruction, path
translation has a multiply-free fast path, and edge/tile traversal avoids
several repeated calculations. The temporary-allocation issue is resolved, and
the final arrays are already continuous and batch-upload-friendly. Treat
CPU-side algorithm and allocation optimization as complete for now; formal Qk
integration should preserve this batch model while adapting the concrete
renderer data structures. The next profiling and optimization work should
focus on GPU coverage/composition and reducing the full-atlas round trip.

The Metal coverage kernel now uses one `16 * sampleGrid`-thread group per 16x16
tile. Each thread computes one complete Y-sample row, evaluating every relevant
edge only once and producing a 64-bit inside mask. After one threadgroup
barrier, the same threads cooperatively merge and write all tile pixels. This
removes per-pixel repeated edge traversal without requiring atomics or
subgroup-shuffle support, and the mapping supports sample grids 1, 2, and 4.

### Compute AA Experiment Branches

There are five saved experiment branches:

| Branch | Head | Purpose / structure | Measured total |
| --- | --- | --- | --- |
| `experiment/compute-aa-row-mask` | `9bf955c3e` | Current 64-thread GPU-backdrop baseline: one thread per Y sample, private `windingDelta[64]`, shared `insideMask[64]`, one barrier, render-pass composite. | `0.733-0.773ms` |
| `experiment/compute-aa-cpu-backdrop` | `8e8e2682a` | Fastest result so far: CPU builds complete per-tile/sample backdrop using 2D difference/prefix sums; GPU uses 16 threads per tile, paired as two tiles per SIMD32, one thread per pixel row, private delta, no shared mask/barrier. | about `0.60ms` |
| `experiment/compute-aa-gpu-backdrop-private-delta` | `db5b53d29` | Same 16-thread/two-tile row-coverage structure as the CPU-backdrop branch, but each Y sample scans compact GPU backdrop events instead of reading a CPU-built complete backdrop. | `0.730-0.772ms` |
| `experiment/compute-aa-boundary-tiles` | `25279aca6` | CPU classifies boundary/uniform tiles during the existing X-tile scan; a separate compute pass writes uniform 0/1 tiles, and the GRID kernel dispatches only boundary tiles. | stable `0.32-0.40ms`; transient lows `0.23-0.24ms` |
| `experiment/compute-aa-sorted-crossings` | current branch | Selected integration baseline: boundary/uniform split plus sparse X crossing buckets and interval-mask fill. | Boundary share about `9%`; Composite about `78%` |

The old `experiment/compute-aa-row-mask` all-shared branch head was deliberately
replaced with the useful 64-thread/private-delta baseline. The rejected
all-shared comparison branch was deleted after measuring `1.194-1.274ms`;
its result remains documented below.

Selection status:

- `experiment/compute-aa-sorted-crossings` is the selected baseline for formal
  renderer integration.
- It combines compact GPU backdrop events, boundary/uniform tile separation,
  one thread per Y-sample row, sparse X crossing buckets, interval mask fill,
  shared inside masks, and one barrier.
- AASide remains the faster path for suitable simple geometry; Compute AA is
  intended as the more general and stable high-quality path.
- Further Boundary kernel micro-optimization is low priority because Composite
  now dominates the measured GPU cost.

Do not compare old total-frame numbers directly unless clear/composite use the
same render-pass structure. Xcode GPU Capture labels the two remaining stages
as `Compute AA Coverage` and `Compute AA Composite`.

Observed Compute AA performance direction:

- The original three-compute-pass version measured around `1.4-1.6ms`.
- CPU backdrop + private per-thread delta + one render clear/composite pass
  measured around `0.60ms`.
- The equivalent AASide scene measured around `0.20ms`.
- Removing edge-crossing work did not remove most Coverage cost; shared delta
  initialization, X prefix winding, inside-mask production/consumption, full
  atlas writes, and composition are all material.
- Repeatedly rescanning edges to avoid delta storage regressed badly. Do not
  revisit that structure without a new complexity argument.

The fair shared-row-mask measurement is now complete:

- Total GPU time: `1.194-1.274ms`.
- Coverage: `72.25%`, approximately `0.86-0.92ms`.
- Composite: `27.75%`, approximately `0.33-0.35ms`.
- Shared-row-mask Coverage alone is slower than the CPU-backdrop/private-delta
  branch's approximately `0.60ms` complete frame. This rejected the combined
  shared-row-mask structure, but did not yet identify which component caused
  the cost.

The GPU-backdrop/private-delta single-variable experiment measured:

- Total GPU time: `0.733-0.773ms`.
- Coverage: `65.61%`, approximately `0.48-0.51ms`.
- Composite: `34.39%`, approximately `0.25-0.27ms`.
- Replacing only shared delta reduced Coverage by approximately `44%-45%` and
  total frame time by approximately `39%`.

This establishes shared `windingDelta[64][64]` as the major old Coverage
bottleneck. GPU backdrop events are not free, but may be worth keeping to avoid
CPU backdrop construction/upload. Replacing shared `insideMask` and the barrier
with private masks exchanged through simdgroup shuffles regressed total GPU
time from `0.733-0.773ms` to `0.858-0.882ms`, despite retaining all 64 lanes in
the write stage. Keep the shared mask/barrier baseline. The larger architectural
priority remains an edge-tile-only Compute AA path that avoids full-atlas
coverage and composition.

The next quality-oriented Compute AA direction is analytic signed-area
coverage. It should replace fixed `N*N` inside samples with continuous
edge/cell area plus cover accumulation, while retaining flattened directed
edges, tile binning, scanline fill rules, and an incoming row-cover concept.
This is not SDF rendering. Keep the current GRID branches as comparison
baselines; avoid deep GRID micro-optimization before testing an analytic-area
reference. Detailed notes are in `docs/GPU_2D_ANTIALIASING.md`.

A major shared breakthrough for GRID and analytic-area coverage is explicit
boundary-tile classification. A tile with no effective boundary crossing is
topologically uniform: all outside or all inside. Keep one complete R8 coverage
texture and one regular Coverage compute pass; uniform tiles cheaply write all
0 or all 1, while only boundary tiles run expensive coverage. Do not leave
outside tiles uninitialized or leak sparse-valid-tile semantics into later
composite shaders. Detailed rationale is recorded in
`docs/GPU_2D_ANTIALIASING.md`.

The active `experiment/compute-aa-boundary-tiles` working branch validates this
design from the 64-thread baseline. Boundary marking is now folded into the
existing CPU X-tile scan instead of using a second geometry/DDA construction
pass. Horizontal edges use zero winding and participate only in boundary
marking. Uniform tiles resolve their 0/1 state from one Y-sample backdrop value.
Boundary tiles are emitted directly as a compact tile array; the prototype no
longer uploads a full tile array plus a second boundary index list.

This is a Compute AA milestone:

- The representative large test contains about `501-506` boundary tiles and
  `3983-3987` uniform tiles, so only about `11%` of tiles require GRID coverage.
- Stable total GPU time is approximately `0.32-0.40ms`, down from the
  `0.733-0.773ms` all-tile 64-thread baseline. Startup/transient measurements
  reached `0.23-0.24ms`, but are not treated as stable performance.
- GRID `1x1` and `4x4` total times are close because GPU Capture attributes
  roughly `70%-77%` of the frame to the final Composite. At `4x4`, representative
  shares were Uniform `9.79%`, Boundary Coverage `19.41%`, Composite `70.80%`.
- Disabling the uniform pass visualizes a continuous tile-width boundary band,
  confirming that expensive work now follows the path outline rather than the
  full atlas area.
- The path is now viable for Quark as a high-quality GPU AA option, but still
  costs more CPU and GPU time than AASide. The next major performance experiment
  is to avoid the complete R8 write/read round trip by applying coverage during
  final drawing.

The `experiment/compute-aa-sorted-crossings` branch is a focused Boundary
Coverage comparison built from the boundary-tile branch. Its current sparse
bucket variant directly accumulates winding into a private
`short crossingDelta[64]` indexed by discrete X sample. A 64-bit crossing mask
tracks initialized buckets; repeatedly extracting its lowest set bit visits
only real crossing positions in ascending X order and fills the intervals
between them. It requires no full-table clear, fixed 64-position prefix scan,
sorting, crossing-count limit, or fallback. CPU data, dispatch shape, Uniform
Tiles, and Composite remain unchanged, so Boundary Coverage time is the primary
comparison.

The crossing-interval experiments reduced Boundary Coverage time by
approximately half. The insertion-sorted event-list version measured `9.44%`
Boundary Coverage; the safer sparse-bucket version measured about `9.19%`.
Both are effectively in the same range, compared with the previous
representative Boundary Coverage share of `19.41%`. This confirms that typical
boundary Y-sample rows contain few effective crossing positions: processing
actual crossing positions and filling intervals is cheaper than clearing and
prefix-scanning all 64 X-sample buckets. The sparse-bucket version is selected
because it removes sorting and crossing-list overflow without losing measured
performance. Composite is now even more clearly the dominant remaining GPU
cost.

### Resolved CPU Profiling Trap: Per-Frame Window Titles

The apparent large CPU regression from writing `tile.boundary = true` was not
caused by the boundary-marker store. The Compute AA test updated the native
window title every frame with live edge/boundary/uniform counts:

```objc
self.window.title = [NSString stringWithFormat:...];
```

The compared boundary algorithms produced count strings with different
stability. The sample-derived version changed counts frequently while rotating,
causing AppKit to repeatedly update/layout/draw the title bar and communicate
with the Window Server. The geometry-derived `mark_boundary_range` version
usually produced more stable counts, so repeated equal titles were cheaper.
Instruments attributed this accumulated descendant work to
`-[NSWindow _dosetTitle:andDefeatWrap:]`, creating a misleading apparent
relationship between the boundary write and CPU usage.

For Compute AA CPU comparisons:

- Disable native window-title updates completely during profiling.
- Store counters in memory and print them once after the measurement.
- Use the same fixed or deterministic path motion, Release configuration, and
  measurement duration.
- Compare `buildDrawData()` directly; do not use whole-process percentages as
  the primary result.

The two boundary algorithms intentionally produce different counts:

- The sample-derived version marks a tile only when an edge affects the current
  discrete GRID samples. Shallow edges and threshold crossings can make its
  boundary count change frequently.
- `mark_boundary_range` marks every tile crossed by the continuous geometric
  boundary, including horizontal edges and edges that do not change a discrete
  sample row. It is more conservative and its counts are generally more stable.

Earlier prototype revisions passed Metal shader/metallib compilation and
ObjC++ syntax checks. The final sparse-crossing documentation/cleanup pass was
intentionally not compiled per the current testing workflow; only lightweight
source review and `git diff --check` were performed. The user has manually
reviewed the current code and will report any integration/compiler issue.

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
