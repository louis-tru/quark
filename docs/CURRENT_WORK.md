# Current Work

This file is short-term memory for the current development thread. Update it when the active focus changes.

## Active Theme

Active work has moved from the earlier AASide/CGAA closeout into **CAPA
(Coverage Area Pipeline Anti-Aliasing)** research on branch `experiment/capa`.

Current checkpoint:

- Commit `7564f7a92` (`Restore CAPA prototype snapshot`) was pushed to
  `origin/experiment/capa`.
- This is the version the user wants to keep as the recoverable CAPA baseline.
- Do not switch the current experiment back to CGAA unless explicitly asked.
  CGAA should remain present as a reference/fallback path, but the active
  research target is CAPA.
- The vNext implementation now uses the 12 shader pass file set in
  `src/render/shader/capa/`: `capa_prepare.glsl`, `capa_prepare1.glsl`,
  `capa_prepare2.glsl`, `capa_tile.glsl`, `capa_bin.glsl`,
  `capa_bin1.glsl`, `capa_backdrop.glsl`, `capa_prefix.glsl`,
  `capa_prefix1.glsl`, `capa_coverage.glsl`, `capa_order.glsl`, and
  `capa_composite.glsl`.

CAPA current state:

- CAPA and CGAA are intended to coexist in the tree.
- `GPUCanvas::drawPathColor()` currently tries CAPA first for AA color path
  fills and falls back to the older path if CAPA cannot build.
- The old prototype `capa_resolve.glsl` has been removed from the vNext path.
  `capa_tile.glsl` is now a new vNext pass dedicated to `CAPAPathTile`
  initialization.
- CAPA's active research goal is now narrowed to solving multi-primitive
  leakage / background seams through ordered tile resolve. If CAPA does not
  solve leakage, AASide and CGAA already cover the other quality/performance
  needs better.
- AASide, CGAA, and CAPA should coexist behind compile/runtime switches:
  AASide remains the fast default/stroke path, CGAA can remain the fast
  coverage/area path, and CAPA is the high-complexity no-leak compositor
  research path.
- Backend target split is intentional: GL/GLES is only expected to guarantee
  the AASide path. Do not spend effort making GL support CGAA or CAPA unless
  explicitly requested. CGAA/CAPA are Metal/Vulkan-class compute paths.

Current CAPA caution:

- Do not make small opportunistic CPU-side additions just to shave tiny GPU
  time. The user explicitly wants CPU work to trend toward uploading raw path
  data and doing the rest on GPU.
- Do not touch `buildColor` or unrelated CPU-side path preparation unless the
  next task specifically asks for it.
- The failed/abandoned direction was trying to morph the old CGAA shader into
  CAPA area coverage. It produced image/backdrop issues and did not preserve
  the intended behavior cleanly. Treat the restored CAPA prototype as the
  active base again.
- If the image regresses into horizontal strips or missing bands, inspect
  tile/bin counts, backdrop/tile fallback, and the `rawCount > 0u` path before
  changing higher-level dispatch.
- CAPA may flush/end at operations that are not practical to embed in the CAPA
  compositor, such as blur/filter/output-image/readback and possibly explicit
  clip changes. CAPA does not need to implement clip generation internally in
  the first version; it can consume the current clip state, and a new clip can
  end the current CAPA pass. This can leave rare post-flush interleaving leak
  cases, but current Qk GUI usage is expected to avoid most of them.
- Ordered CAPA image sampling should use a per-pass texture table. Vulkan can
  query usable sampled-image limits at runtime from `VkPhysicalDeviceLimits`
  and non-uniform descriptor indexing support from descriptor-indexing feature
  structs; if a device only supports 16 sampled images for the shader stage,
  cap CAPA passes at 16 textures. Metal should use argument-buffer indexed
  textures where available. A practical default target is 32 textures per CAPA
  pass, with runtime downgrade and pass flush when the table fills.
- Ordered CAPA color blending should not allocate per-path-tile RGBA storage.
  Only AA/edge path-tiles need cached coverage, preferably one `16x16` R8 page
  per non-uniform coverage tile. Empty tiles and full/uniform tiles carry flags
  or constant coverage and allocate no coverage/backdrop page. This assumption
  is foundational: solid interior tiles and empty tiles must be representable
  without boundary coverage or row-backdrop storage.
- CAPA coverage storage should use a fixed-budget GPU page pool plus an atomic
  allocator. CPU does not need to prove exact path-tile count from conservative
  bounds. `boundaryTileIndex` uses reserved values: `0` = empty/outside,
  `1` = solid/full, `2` = allocation failure/debug coverage, and `>=3` = real
  boundary tile. Pass6 processes real boundary tiles starting at index `3`;
  pass7 can still treat `2` as a valid debug coverage tile and blend it normally.
  Overflow is a budget/debug failure, not a normal shader correctness fallback.
- A useful CPU-side coverage-page estimate is transformed total edge length
  divided by `kCAPATileSize` (`16`). Because multiple edges often share one
  tile, this should overestimate AA/edge tile count while staying far tighter
  than bounds-area allocation, especially for hollow/stroked paths. Use this
  estimate to size the R8 coverage page pool; if the estimate exceeds the
  allowed budget, split/flush the CAPA pass or fall back to AASide before
  launching CAPA.
- For the first production-oriented CAPA compositor, shader overflow can be
  treated as a tuning/debug failure instead of a correctness fallback: drop or
  visibly mark overflowed edge path-tiles in debug builds, then adjust the CPU
  edge-length budget multiplier. The target is that overflow is effectively
  absent in normal scenes after conservative length-based sizing.
- The current executable CAPA pass plan is documented in
  `docs/CAPA_PASS_PROCESS.md`. Treat `src/render/shader/capa/` as the source of
  truth: CPU pass0 plus 12 shader passes cover edge prepare, path tile
  allocation, indirect dispatch arg generation, path tile initialization,
  short-edge binning, boundary dispatch arg generation, backdrop, boundary-row
  chain building, boundary-only prefix/classification, boundary coverage,
  ordered global tile chains, and final ordered color compositing.
- Latest handoff note: preserve the current CAPA vNext semantics when coding:
  `capa_tile.glsl` is a 32-wide linear clear over `CAPAPathTile` records before
  binning, with one thread clearing one tile, not a path/global-tile traversal;
  `capa_order.glsl` now runs after coverage and builds the global tile pathTile chain while excluding
  `boundaryTileIndex == 0` empty tiles. `boundaryTileIndex` values are `0`
  empty, `1` solid/full, `2`
  allocation failure/debug coverage, and `>=3` real boundary tile;
  continuous SrcOver full tiles may be folded into one retained full
  `CAPAPathTile` by storing packed PMA RGBA8 in `CAPAPathTile.color`, without
  changing the boundary index semantics;
  `atomicAdd(boundaryTiles.count, 1u)` starts from count `3`; tileX0
  stores 16 per-row local backdrop floats by borrowing the same
  `boundaryTile.coverage` buffer words until pass6 overwrites them with packed
  R8 coverage.
- 2026-06-22 implementation checkpoint: CAPA color paths now batch in
  `CAPABuilder` and flush at
  the existing AA/draw/read/output/swap boundaries, so consecutive color paths
  can reach one ordered compositor pass. `CAPABuilder::endBuild()` now owns the
  common vNext budget (`CAPABudget`): surface tile span, path-tile count,
  short-edge task budget, max tile refs, matrix-scaled edge-length estimate, and
  boundary coverage-page estimate. Metal consumes this budget instead of
  re-deriving it. CAPA shader common types/helpers live in `_capa.glsl`.
  `drawCAPACmd()` must submit the current 12 shader pass set in order:
  prepare, prepare1, prepare2, tile, bin, bin1, backdrop, prefix, prefix1,
  coverage, order, and composite. Coverage storage is now the
  `CAPABoundaryTile` buffer, not a texture: pass3 allocates real boundary tiles
  from the fixed pool, pass4 writes local row backdrop into
  `boundaryTile.coverage`, pass5 builds per-row boundary chains, pass5.1
  rewrites those words as tile-left prefix backdrop and marks empty/full
  path-tiles, pass6 overwrites real boundary tiles with packed R8 coverage, and
  pass7 reads that buffer while handling
  `0` empty, `1` full, and `2` debug/failure coverage as constants. Blend mode
  is stored per `CAPAPath`; changing blend mode does not flush the CAPA batch,
  and `capa_composite.glsl` handles the implemented Porter-Duff modes
  internally before the batch is written back with SrcOver.

CAPA GPU data-structure direction:

- Xcode GPU capture is now usable by setting frame capture `Count` to 2. The
  old prototype profile showed `CAPA tile coverage` dominating GPU time
  (~93.8%), while `CAPA prepare` (~0.05%) and `CAPA short-edge bin`
  (~0.18%) are tiny. Treat tile coverage/full-path fallback as the main
  bottleneck; atomic binning is not currently the hot path.
- Avoid introducing prefix-scan/scan-based allocation into CAPA unless there is
  no simpler option. Scan gives elegant compact arrays, but for this renderer it
  adds multi-pass block scans, threadgroup barriers, recursive/block prefix
  handling, and substantial debugging complexity. It is not the preferred
  solution for CAPA tile short-edge storage.
- The shared small-chunk append direction is now considered unsafe for CAPA:
  do not have many invocations append into the same fixed-capacity short-edge
  chunk with `atomicAdd(count)` and then dynamically repair overflow in the
  same pass. Experiments on a static five-point star showed missing short-edge
  slots and jitter even though the chunk chains were reachable. A one-edge-per
  linked node experiment produced stable `1466/1466` short-edge counts.
- Prefer a simpler tile-owned linked list where each emitted `CAPAShortEdge`
  is itself the node (with a `next` field), or move to a multi-pass
  count/prefix/fill design. Atomic operations are still acceptable for unique
  node allocation and linking a fully written node, but not for complex
  same-pass small-bucket append/overflow state machines.
- Final pass3 storage insight from the painful iteration: stop asking "how many
  short edges can this tile receive?" and instead use the stronger task-side
  invariant, "one short-edge task can touch at most three tiles." Allocate
  `maxShortEdgeCount * 3` `CAPAShortEdge` nodes, let each task own
  `taskIndex * 3 + {0,1,2}`, and link each used node into the target
  `CAPAPathTile.shortEdgeHead`. This avoids scan/prefix allocation, avoids
  shared chunk slot atomics, avoids overflow repair, uses less space than
  4-slot chunks, and measured `CAPA short-edge bin` around `1.07%` in capture.
  The important design lesson: the beautiful data structure came from changing
  ownership to the dimension with a hard bound.
- HARD RULE for CAPA GPU data structures: do not create complex same-pass
  atomic write-then-read dependency protocols. If one invocation's atomic write
  must be read by another invocation to decide further shared mutations, move
  that decision to a later pass or redesign the data so each invocation owns the
  node/slot it writes.
- DANGER: do not use shader-level custom locks or spin waits for CAPA tile-list
  growth. GPU execution does not guarantee that an invocation holding a custom
  lock will run to the unlock path while other invocations are spinning. Tile
  list growth must use non-blocking atomic append/allocation, fixed-capacity
  inline storage with overflow marking, or multi-pass compaction/prefix
  structures that do not wait for another invocation's forward progress.
- Atomic operations are acceptable in CAPA when they are coarse and
  low-contention: per-edge `atomicAdd` for task allocation/completion,
  per-path/per-tile `atomicMin/Max` for conservative bounds, and per-tile
  append cursors. Avoid atomics in per-pixel/per-sample inner loops.
- `capa_prepare` is the right place to compute surface-space edge data and
  conservative path/screen bounds. Clip can shrink tile ranges, but X-left
  edges that contribute scanline backdrop/winding must not be discarded merely
  because they are left of the tile/clip; they must contribute to backdrop or
  prefix state.
- CAPA backdrop/prefix state must remain continuous signed area, not clamped
  alpha and not discrete GRID/parity state. Local tile edge contributions add
  to the row backdrop; they are not multiplied by it. Apply the fill rule only
  at final coverage conversion: NonZero/Positive/Negative use signed area
  directly, and EvenOdd folds `abs(area)` with a triangle wave (`0.5 -> 0.5`,
  `1.5 -> 0.5`, `2.0 -> 0.0`). Do not route EvenOdd back to GRID just because
  it is parity-like.
- Dynamic pass sizing can be GPU-driven by having prepare maintain counters
  and, if needed, a completion counter. The last completed prepare invocation
  can write indirect dispatch group counts for a later pass. This costs an
  extra coarse `atomicAdd`; measure it rather than assuming it is worse than an
  additional setup dispatch.
- When this structure is implemented, verify with Xcode capture that
  `CAPA prepare` and `CAPA short-edge bin` remain small, allocator overflow and
  tile append-failed flags stay zero, most tiles use only their initial chunk,
  and `CAPA tile coverage` drops after removing full-path fallback.
- 2026-06-27 pass6 shader checkpoint: `capa_coverage.glsl` is being shaped as a
  boundary-tile-only coverage pass. Its responsibility is limited to converting
  one real `CAPABoundaryTile`'s short-edge chunks plus row prefix/backdrop into
  that tile's own `16x16` packed R8 coverage page. It should not handle surface
  bounds or framebuffer/global visibility; earlier tile generation and final
  composite own those concerns. Current indexing uses `local_size_x=16,
  local_size_y=2`: one workgroup processes two boundary tiles, with 16 row
  threads per tile. `boundaryIndex = gl_WorkGroupID.x * gl_WorkGroupSize.y +
  gl_LocalInvocationID.y + 3u`, and `row = gl_LocalInvocationID.x`.
- Current pass6 algorithm notes: avoid copying full `CAPABoundaryTile` storage
  structs into local variables; read only scalar fields such as `pathTileIndex`,
  `pathIndex`, and `tileCoord`, and write coverage words directly. The shader
  keeps per-row `localArea[16]` for boundary-pixel exact area plus
  `crossingDelta[16]` for right-side prefix events, inspired by CGAA's
  crossing-mask logic but without a `deltaMask`. `backpack[row]` is used as the
  initial running prefix only at final coverage write time, not pre-added to
  every local pixel. `capa_pack4` now lives in `capa_coverage.glsl` and assumes
  inputs have already been clamped by `capa_area_to_coverage`.
- SIMT/load-balance note for pass6: although each row thread can skip edges
  whose `beginY/endY` do not hit that row, the 16 row lanes in a tile will still
  largely walk the same short-edge chunk list in lockstep; non-hit rows are
  mostly masked/idle for the heavier area math. This is an accepted first-pass
  tradeoff: imbalance is local to the two boundary tiles in one workgroup, large
  full/empty tiles skip pass6 entirely, and the cost is tied to boundary short
  edge count rather than surface area. A later optimization, only if capture
  proves pass6 hot, could bin short edges per row or create finer edge-row
  tasks, but that adds storage, atomics, and merge complexity.
- 2026-06-27 pass7 shader checkpoint: `capa_composite.glsl` has been rewritten
  as a compute shader for the first pure-color compositor landing. It uses
  `layout(local_size_x=16, local_size_y=16, local_size_z=1)`, one workgroup per
  global tile and one thread per pixel. Dispatch should be 2D over
  `globalTileSpan`; the shader derives `globalTileIndex` from
  `gl_WorkGroupID.xy` and `gl_NumWorkGroups.x`, so it no longer needs the CAPA
  environment buffer in this pass. Pixel coordinates are computed from
  `pc.surfaceOffset + tileCoord * CAPA_TILE_SIZE_U + localPixel`; pass7 does not
  do `surfaceSize` bounds checks because valid global tile bounds and the image
  write target own that responsibility.
- Current pass7 behavior: if `globalTiles[globalTileIndex].head == CAPA_NIL`,
  return immediately unless `CAPA_COMPOSITE_CLEAR_DST` is set, in which case
  write `pc.clearColor`. Otherwise initialize `dst` from `pc.clearColor` when
  clearing is requested or `imageLoad(dstImage, pixel)` when preserving the
  previous target. Traverse the ordered `CAPAGlobalTile` pathTile chain, resolve
  coverage (`0` empty, `1` full, `2` and `>=3` read packed R8 boundary coverage),
  premultiply solid `path.color` by alpha and coverage, and apply
  `path.blendMode` through `capa_blend`. This first landing intentionally does
  not handle image/gradient paint, clip sampling, inter-layer leakage fixes, or
  a fragment-shader A/B implementation.
- 2026-06-27 Metal wiring checkpoint: `MetalCanvas::drawCAPACmd()` must submit
  the current CAPA shader passes directly, including
  `capaComposite` writing `_outColorTex`. Startup parameters for `tile`, `order`,
  `bin`, `backdrop`, `prefix`, `prefix1`, `coverage`, and `composite` are generated in
  shader and stored in `CAPAEnvironment`; Metal must dispatch those passes with
  indirect threadgroup counts from the corresponding `uvec4` fields.
- Dispatch argument direction: CAPA pass group counts in `CAPAEnvironment`
  should be stored as 16-byte-aligned `uvec4` values so they can map cleanly to
  Metal indirect dispatch arguments. Current shader fields include
  `tilePassGroups_Size32`, `orderPassGroups_Size32`, `binPassGroups_Size64`,
  `backdropPassGroups_Size16_2`, `prefixPassGroups_Size32`,
  `prefix1PassGroups_Size16_2`, and `compositePassGroups_Size16_16`.
  `capa_prepare2.glsl` writes the composite
  args as `uvec4(globalTileSpan.x, globalTileSpan.y, 1, 0)`;
  `capa_bin1.glsl` writes `backdropPassGroups_Size16_2`, which is also used by
  the coverage pass because both process real boundary tiles.
- 2026-06-27 executable CAPA checkpoint: the first Metal vNext path is
  now producing an image and the measured GPU time has dropped to about
  `0.3ms` in the current test scene, a major improvement from the earlier
  ~`10ms` prototype path and below the observed CGAA cost. The image is still
  visually incorrect, with horizontal striping / missing coverage bands, so the
  next debugging focus should be correctness in backdrop, prefix, boundary
  coverage, and chunk-chain edge completeness rather than performance.
- CAPA pass3 DANGER note: do not use shader-level spin locks or any loop that
  waits for another invocation to release a custom lock. A fixed-initial-chunk
  experiment using a custom `atomicCompSwap` lock was reverted; static five-star
  input still jittered, and GPU APIs do not guarantee that the lock holder will
  execute its release path before waiting invocations stall progress. Important:
  horizontal edges must still be allowed to claim boundary tiles even though
  they are not stored in short-edge chunks. They do not contribute local area,
  but the claimed tile may still need prefix-filled coverage/classification. Do
  not "optimize" by returning before boundary-tile allocation for
  `edge.winding == 0`; that drops tiles that should be filled by prefix state
  and creates unrelated errors.
- CAPA pass3 IMPORTANT result: the dynamic shared short-edge chunk protocol was
  isolated as a correctness risk. With expected short-edge refs around `1466`,
  chunk-list traversal saw only about `1457-1458` in failing versions. A debug
  pass showed no orphan chunks, invalid links, or cycles, and a one-edge-per
  node linked-list experiment restored stable `1466/1466` counts. Avoid
  same-pass shared `chunk.count` slot append plus overflow repair; use
  per-edge nodes or count/prefix/fill instead.
- CAPA pass3 final storage direction: `CAPAShortEdge` should be the linked-list
  node itself (`p0/p1/dxdy/winding/next`), not wrapped by an external chunk.
  Allocate three nodes per short-edge task because pass1 keeps a short segment
  short enough to touch at most three tiles. In `capa_bin.glsl`, each task writes
  only its own node slots and uses `atomicExchange(pathTile.shortEdgeHead,
  nodeIndex)` to link fully written nodes. Static test logs stabilized at
  `taskCount=944`, `pathTileCount=4556`, `boundaryTileCount=503`, and the bin
  pass became cheaper than the chunk version.
- CAPA debug marker note: Metal initializes reserved boundary tile slot `2`
  with a centered low-intensity `F` coverage page (`F = 32`) so budget/failure
  tiles are visible without overwhelming the output.
- 2026-06-28 CAPA compositor performance note: testing with 100 five-point
  stars showed AASide around `3ms`, CGAA around `4.5ms`, and current CAPA
  around `8ms`. Xcode capture showed the composite pass dominating because the
  current pixel-pull compositor makes every pixel in a global tile traverse the
  pathTile chain. Empty bbox tiles must be removed from the final chain, and
  the early order pass can be split conceptually into pathTile initialization
  first and final non-empty ordering after prefix/classification. Changing the
  chain from linked nodes to a compact contiguous layer list can improve memory
  behavior, but does not by itself change the `pixels * activeLayers` cost.
  A stronger future direction is late-order precomposition of tile-uniform
  solid full tiles: consecutive full SrcOver solid-color layers can be folded
  into a synthetic uniform layer until a boundary or unsupported layer forces a
  flush.
- 2026-06-29 CAPA performance checkpoint after empty-tile removal and SrcOver
  full-tile preblend: the 100 five-point-star test is now around `4.2ms`, with
  CPU time reported lower than AASide in the current local test. Xcode GPU
  capture pass share in one representative run: render encoder `3.42%`,
  prepare `0.76%`, prepare path tiles `0.13%`, prepare dispatch args
  `~0.01%`, path tile init `1.46%`, short-edge bin `6.19%`, bin dispatch args
  `~0.01%`, backdrop `9.05%`, prefix `13.98%`, coverage `22.08%`, ordered tile
  chain `5.95%`, composite `30.58%`. Composite, coverage, and prefix remain the
  largest GPU targets. A 4-pixels-per-thread composite experiment showed only a
  small improvement and made the shader much more complex, so it was reverted;
  revisit composite with a cleaner structural change later.
- 2026-06-29 CAPA prefix split experiment: `capa_prefix.glsl` is now a cheap
  32-wide row-chain pass that scans each path tile row once, writes
  `CAPAPathTileRow.boundaryTileIndex`, and links boundary tiles through
  `CAPABoundaryTile.nextBoundaryTileX`. The new `capa_prefix1.glsl` performs
  the real per-row prefix only along those boundary chains and marks full
  edge-free spans on row0. This is intended to remove the old 16-row-lane pass
  repeatedly walking every path tile in large rows.
- 2026-06-30 CAPA memory-access checkpoint: an inline short-edge slot
  experiment inside `CAPAPathTile` was reverted, but the capture was useful.
  Making `CAPAPathTile` wider immediately made the lightweight `prefix_pre`
  row scan, path-tile init, and ordered tile-chain passes much slower, while
  backdrop/coverage improved only moderately from more contiguous edge access.
  Treat this as evidence that the row scan over all path tiles is dominated by
  memory stride/cache behavior. Future edge-local storage should stay outside
  the main `CAPAPathTile` array, and the next structure direction is a small
  per-path tile buffer used for bin/prefix staging so threads avoid scattered
  reads from large path-tile records.
- 2026-06-28 CAPA correctness note: do not add a per-row full mask for an
  edge-free pathTile. If no path boundary crosses a connected tile, the tile's
  winding/inside state must be constant over the whole tile. The horizontal
  bands therefore point elsewhere: inspect whether boundary tiles are missing
  edges, whether prefix/backdrop row state is wrong, or whether full/empty
  classification is being applied before all relevant boundary information is
  present.
- 2026-06-28 CAPA left-clamp fix: when `capa_bin.glsl` clamps a short edge
  tile coordinate left of `path.tileRect` into the first local pathTile column,
  it must also clamp the stored `tileCoord.x` to `tileRect.x`. Otherwise the
  pathTile index and `CAPABoundaryTile.tileCoord` disagree, and backdrop /
  coverage use the wrong tile origin.
- 2026-06-28 CAPA backdrop fix: `capa_backdrop.glsl` must compute row prefix
  contribution as the vertical measure of an edge crossing that lies left of
  the tile boundary, not simply add the whole row `dy` whenever an edge touches
  a tile. For a slanted short edge crossing a tile's right boundary, only part
  of the row interval contributes to that tile's outgoing prefix. The current
  shader uses `capa_left_dy(..., tileLeft/tileRight)` and stores
  `rightDy - leftDy` as the tile-local prefix delta.
- 2026-06-28 CAPA coverage-prefix fix: `capa_coverage.glsl` starts each row
  from the tile-left prefix written by pass5. When an edge segment already has
  some vertical measure left of the tile's left boundary, pass6 must not add
  that same measure again through `crossingDelta[localEndX]`. The current
  shader clips each edge segment to the right side of the tile-left boundary
  before computing local pixel area and in-tile crossing deltas.
- 2026-06-28 CAPA binning experiment result: temporarily changing
  `capa_bin.glsl` from the exact three-tile short-edge traversal to conservative
  half-open 2D tile-bbox emission did not visibly change the boundary-only
  debug image. The experiment was reverted; current code keeps the original
  traversal plus the left-clamp `tileCoord.x = tileRect.x` fix.
- 2026-06-28 CAPA coverage formula experiment result: temporarily switching
  `capa_coverage.glsl` from the compact analytic integral to the older
  `capa_test.glsl`-style segmented right-area formula produced a very similar
  boundary-only debug image. The experiment was reverted; the compact formula
  is probably not the source of the current blocky/striped boundary artifact.
  Continue investigating prefix/backdrop row state and event timing.
- 2026-06-28 CAPA horizontal-edge boundary rule: do not skip boundary-tile
  allocation for horizontal edges. Horizontal edges do not need short-edge
  storage, but their tiles may still need prefix-filled coverage; skipping
  allocation is not related to the current artifact and can remove coverage that
  should be present.
- 2026-06-28 workflow rule for this CAPA debugging thread: do not proactively
  run the shader native generator (`tools/gen_glsl_natives.js`) after GLSL
  edits. It is too slow for this iteration loop. Edit source files only and let
  the user run generation/build; respond to any resulting errors when provided.
- 2026-06-28 CAPA visual clue: in the dynamic boundary-only image, a bad row
  often starts from the left and continues all the way to the right. Treat this
  as a row-prefix/backdrop propagation problem, not a local per-pixel area
  integral problem. Focus on consistency between `capa_backdrop.glsl` row
  deltas, pass5 prefix propagation, and `capa_coverage.glsl` crossing events.
- 2026-06-28 CAPA current test-scene constraint: the active debug scene is the
  unclipped five-point star. Do not chase clipping or `tileCoord.x < tileRect.x`
  / left-clamp duplicate-edge theories for this scene; the user has confirmed
  there should be no edges left of `tileX0` in the relevant failure. Focus on
  `capa_backdrop.glsl` and `capa_coverage.glsl` row-prefix semantics for the
  actual star input.
- 2026-06-28 CAPA debugging guardrail: stop making speculative changes to
  `capa_bin.glsl` for the current five-point-star artifact unless the user
  explicitly asks for a binning change or provides bin-specific evidence. Recent
  repeated binning guesses were unrelated to the active failure and wasted
  iteration time. Keep focus on `capa_backdrop.glsl` and
  `capa_coverage.glsl`.

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
- `clip.glsl` was later merged back into `color.glsl` after direct-target CGAA
  planning showed that solid-color and clip-mask output should share one paint
  path. `color.glsl` now accepts `surfaceOffset` and inverted AASide coverage;
  ordinary color draws pass a zero offset, while clip draws pass the clip-target
  offset. GL and Metal both use the single generated Color pipeline.
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
  coverage page atlas.
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

The selected algorithm is now named **CGAA (Compute Grid Anti-Aliasing)**.
Historical references to Compute AA or Compute GRID AA in this document refer
to the CGAA research path.

### CGAA Current Findings

- CGAA is now visually passing the current canvas stress tests at a surface level:
  color fills, image fills, gradients, rotated/scaled paths, masks, and mixed
  overlap scenes appear broadly correct in the active test-canvas coverage.
- The remaining UI border seam problem is important and should be tracked as a
  separate primitive/compositing issue, not as a generic CGAA coverage failure.
  Rect/rrect borders can be made of multiple logical contours or four side paths;
  drawing those as independent AA paths with ordinary SrcOver can expose the
  framebuffer background at joins, especially on high-contrast backgrounds.
- The intended long-term fix is renderer-side primitive handling: treat border
  coverage as one logical outer-minus-inner shape, then decide side/corner color
  inside that covered region. Do not rely on upper UI compensation or blind
  framebuffer coverage addition; different-color borders need explicit color
  ownership at joins.
- Current CGAA CPU cost is high because edge bucketing into tiles is CPU-side.
  This should not be considered the final production shape. A follow-up track
  should investigate moving edge-to-tile binning/backdrop preparation to GPU
  compute so CPU work is closer to path flattening and upload only.
- The current 4x4 subpixel grid is also visibly jagged in some scenes. Treat it
  as a correctness/prototyping setting, not final quality. A production CGAA
  direction must solve both quality and cost: either a better coverage
  estimator, a higher/effective sample rate without exploding atlas cost, or a
  different GPU path-rendering approach.
- Near-term research priority after this context switch: study higher-quality
  GPU path renderers and GPU-side binning strategies before spending more time
  on the UI border logical-primitive coverage issue. The border seam remains
  important, but it should come after deciding whether the CGAA core can meet
  quality and CPU-cost requirements.

### CGAA Production Integration Baseline

Commit `d794e56ac` records the pre-integration baseline that starts moving CGAA
from `test/compute_aa/` into the production renderer:

- `src/render/cgaa.*` defines the production CPU/GPU data contract and
  `buildCGAADrawData()`, including flattened edges, compact boundary tiles,
  uniform tiles, tile-edge slices, and backdrop rows/events.
- `src/render/shader/cgaa.glsl` is currently compute-only and writes coverage
  into an R8 atlas region. Metal shader generation already retains its compute
  source and generated binding indices, but MetalCanvas does not yet encode or
  dispatch the CGAA compute command.
- That baseline removes the old device-MSAA path in preparation for
  CGAA experiments.
- `GPUCanvas::Inl::fillPathColor()` is temporarily disabled, so color path
  fills render nothing until the first Metal CGAA color command is connected.

The next integration step is deliberately narrow and test-oriented: add a
`GPUCanvas` backend command such as `drawCGAAColorCmd`, implement it first for
Metal, invoke it from `fillPathColor()`, and have the CGAA compute shader apply
solid premultiplied color directly to the current target. This is not yet the
final paint/blend/clip architecture. `buildCGAADrawData()` must also stop
emitting outside uniform tiles; production direct-target drawing only needs
boundary tiles and filled inside tiles.

That first test-oriented integration is now connected:

- `GPUCanvas::Inl::fillPathColor()` transforms paths into target-pixel space,
  builds CGAA data, and calls the new backend `drawCGAAColorCmd`.
- `src/render/cgaa.*` is now included in the core source target.
- Metal creates/caches a compute pipeline from the generated CGAA compute
  source, uploads the compact draw buffers, ends any active render pass, and
  dispatches boundary plus filled-uniform tiles directly into the current
  color target.
- `cgaa.glsl` applies solid premultiplied color with manual `SrcOver` instead
  of writing an intermediate R8 coverage atlas.
- `buildCGAADrawData()` no longer emits outside uniform tiles. Its optional
  clip assertions and empty clipped-range handling were also fixed for the
  production call site.

This remains a deliberate test path. Metal CGAA solid color currently ignores
the renderer's clip mask and always applies `SrcOver` regardless of the selected
blend mode. GL's `drawCGAAColorCmd` is an explicit no-op, and root-matrix display
offsets beyond the normal target-pixel mapping have not yet been integrated.

Follow-up integration added Metal `drawCGAAGradientCmd()` and
`drawCGAAImageCmd()` for path fills. Gradient/image CGAA draws build their atlas
immediately instead of joining the solid-color batch, reconstruct canvas-space
paint coordinates from target-pixel CGAA quads with a pixel-to-canvas matrix,
and reuse the existing gradient stops / image mask semantics. YUV images still
fall back to the non-CGAA image path.

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

The preferred production CGAA architecture is now to apply coverage and paint
directly into the current color target instead of always writing a complete R8
coverage atlas and sampling it in a separate Composite pass. This directly
targets the measured approximately `78%` Composite share and enables a sparser
tile model. Outside uniform tiles no longer need to exist merely to write zero
coverage into a complete atlas; production batches should emit only boundary
tiles and filled inside tiles. Target-space tiles also remove dynamic atlas
region allocation, tile-position packing, and atlas-to-target coordinate
mapping. This is especially important for lines and paths with large bounds but
sparse covered area. Direct target writes must preserve Quark draw ordering and
ensure one writer per pixel within a dispatch. Because compute pipelines do not
receive fixed-function render blending, non-`Src` modes require destination
reads plus shared/manual blend logic. Refactor paint, clip, and blend shader
helpers so AASide fragment paths and CGAA compute paths share output semantics
while supplying coverage through different algorithms. Unlike atlas generation,
direct target output may cause different paths to compete for the same target
pixel; preserve draw order with ordered dispatches, non-overlap batching, or
ordered per-target-tile draw lists. Start with opaque `Src` and common
`SrcOver` direct-output fast paths. Keep the intermediate coverage atlas only as
a fallback for paint/blend paths not yet supported by direct CGAA output.

The first formal shader-tooling step is complete. `src/render/shader/cgaa.glsl`
contains `#vert`, `#frag`, and a final `#comp` stage. The GLSL native generator
skips compute for GL, while Metal native generation emits the compute MSL
source, std430-facing data structures, and compute resource binding slots.
`MSLShaderSource` can now retain a compute source function. Actual Metal
compute-pipeline creation and renderer command integration remain pending.
CGAA platform requirements and the required macOS 10.15 AASide fallback are
recorded in `docs/CGAA_COMPATIBILITY.md`.

Shader consolidation for direct-target CGAA has started and is validated on
the existing AASide path:

- `color_linear.glsl` and `color_radial.glsl` were merged into
  `color_gradient.glsl`. A draw-uniform flag selects radial behavior. Linear
  weight remains vertex-computed; radial distance remains fragment-computed.
- `clip.glsl` was removed and its target-offset/inverted-coverage behavior was
  merged into `color.glsl`; clip masks and ordinary solid color now share one
  pipeline.
- The native shader generator now recognizes pure `#comp` files. Such files do
  not generate vertex/fragment ASTs or GL wrappers; Metal records null
  vertex/fragment sources and only the compute source/bindings.
- Generated desktop reflection output was renamed from the misleading `es450`
  name to `gl450`, and generated Metal intermediate files now use the `.metal`
  suffix.

Ordinary image, alpha mask, and SDF mask now share `image.glsl` and one backend
draw command selected by uniform flags. YUV remains separate because it has
different texture inputs and conversion behavior. The next consolidation step
is to extract reusable color/gradient/image paint helpers for direct-target
CGAA.

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
| `experiment/compute-aa-cpu-backdrop` | `8e8e2682a` | Historical CPU-backdrop comparison: CPU builds complete per-tile/sample backdrop using 2D difference/prefix sums; GPU uses 16 threads per tile, paired as two tiles per SIMD32, one thread per pixel row, private delta, no shared mask/barrier. | about `0.60ms` |
| `experiment/compute-aa-gpu-backdrop-private-delta` | `db5b53d29` | Same 16-thread/two-tile row-coverage structure as the CPU-backdrop branch, but each Y sample scans compact GPU backdrop events instead of reading a CPU-built complete backdrop. | `0.730-0.772ms` |
| `experiment/compute-aa-boundary-tiles` | `25279aca6` | CPU classifies boundary/uniform tiles during the existing X-tile scan; a separate compute pass writes uniform 0/1 tiles, and the GRID kernel dispatches only boundary tiles. | stable `0.32-0.40ms`; transient lows `0.23-0.24ms` |
| `experiment/compute-aa-sorted-crossings` | current branch | Fastest measured and selected integration baseline: boundary/uniform split plus sparse X crossing buckets and interval-mask fill. | total about `0.32-0.40ms`; Boundary about `9%` |

The old `experiment/compute-aa-row-mask` all-shared branch head was deliberately
replaced with the useful 64-thread/private-delta baseline. The rejected
all-shared comparison branch was deleted after measuring `1.194-1.274ms`;
its result remains documented below.

Selection status:

- `experiment/compute-aa-sorted-crossings` is the selected baseline for formal
  renderer integration and the fastest measured Compute AA branch.
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
