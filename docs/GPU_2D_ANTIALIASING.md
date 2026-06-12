# GPU 2D Antialiasing Research

This document tracks future work on high-quality GPU antialiasing for Quark's 2D renderer.

## Why This Matters

Quark is a GUI framework, not a 3D game renderer. The quality bar for 2D edges is different:

- Text, icons, borders, rounded rects, and vector shapes are viewed at rest for long periods.
- Users notice small edge instability, uneven alpha ramps, corner artifacts, and shimmer.
- GUI surfaces often contain large flat colors next to crisp curves, so imperfect coverage is very visible.
- High-quality visual immersion depends on calm, precise 2D edges as much as on performance.

After the Metal and Vulkan backends become stable, antialiasing quality should become a dedicated rendering track.

## Current Approach

The current GPU path uses an `aaSide` style method for many vector edges:

- Geometry is drawn normally for the solid interior.
- An extra soft band is generated around the path edge.
- The soft band behaves somewhat like a simple SDF-like expansion.
- Fragment alpha is reduced across this AA edge band to hide jagged edges.
- It does not precisely compute the true coverage/opacity of every affected pixel.
- The data is currently still a `Vec3` triplet: `{x, y, aaSide}`. The current
  research direction is to improve the meaning of `aaSide`, not to add more
  vertex attributes.

Relevant areas:

- `src/render/gpu_canvas.*`: shared path draw/fill/stroke behavior and `_phy2Pixel` scale handling.
- `src/render/pathv_cache.*`: cached path triangles and AA side stroke geometry.
- `src/render/shader/_util.glsl`: shared AA clip and fragment helpers.
- `src/render/shader/color*.glsl`, `image*.glsl`, `triangles.glsl`: fragment paths that consume AA-related fields.
- `src/render/gl/gl_command.*`: current GL behavior reference.
- `src/render/metal/mtl_canvas.*`: Metal path still being aligned.

## Current Problem

The current fuzzy band is useful but not exact enough:

- The true coverage/opacity of each affected pixel is not calculated carefully.
- Edge alpha can be too strong or too weak depending on transform scale.
- Curves and sharp corners can show uneven ramps.
- The method is heuristic, so matching GL / Metal / future Vulkan exactly is hard.
- Subpixel translation can cause visible variation because coverage is not derived from exact pixel overlap.

In short: the renderer draws a soft edge, but it does not yet compute high-fidelity pixel coverage.

## Current Direction

The next AA track should stay GPU-first. Do not make CPU coverage rasterization
or software AA the primary route; many renderers already solve quality that way,
but Quark's current direction is to keep path filling in the GPU pipeline.

Important constraints from the current renderer:

- Curves are flattened before they reach the GPU, so the AA problem can be
  treated primarily as directed straight-edge coverage.
- A coverage mask is only a storage or compositing mechanism. It still needs an
  accurate coverage calculation.
- If the final path body is drawn only with the original fill triangles, pixels
  whose centers lie just outside those triangles will not run the fragment
  shader. Any mask-based path renderer would need to draw conservative expanded
  bounds, not just reuse the body triangles.
- The old `aaSide` behavior is still too close to a heuristic alpha ramp. The
  next version should make `aaSide` a signed edge coordinate so the shader can
  distinguish inside, edge, and outside without adding new vertex fields.

This points toward a GPU signed-side edge-AA system rather than a software mask
rasterizer.

Current naming and data contract:

- `aaSide < 0`: inside the shape.
- `aaSide = 0`: the true geometric edge.
- `aaSide > 0`: outside the shape.
- The existing triplet `{x, y, aaSide}` should remain the primary vertex format
  while prototyping. Avoid expanding the vertex layout unless the simple model
  fails for a concrete case.
- Fragment shaders should derive the local 1-pixel transition width using
  `fwidth(aaSide)` and convert the signed side coordinate to coverage with a
  calibrated smooth/clamp function.

## Design Goals

Future antialiasing work should aim for:

- Stable subpixel coverage under translation, scaling, and rotation.
- Consistent results across GL, Metal, and Vulkan.
- High-quality curves, joins, caps, rounded rects, and clipping edges.
- Predictable premultiplied-alpha behavior.
- Good performance for common GUI scenes.
- A path that can coexist with text rendering, image masks, blur, and render-to-texture.

## Candidate Directions

These options should be studied before implementation.

### Analytic Coverage

Compute coverage in the shader from edge equations or signed distance data.

Potential benefits:

- More accurate per-pixel edge alpha.
- Better behavior under subpixel transforms.
- Can make AA width explicitly tied to device pixels.

Risks:

- Harder for complex paths, self-intersections, and clips.
- Requires careful handling of joins, curves, and fill rules.

### Improved Geometry AA

Keep the current extra-band approach but compute a better alpha ramp.

Possible improvements:

- Generate edge distance or normalized coverage parameters in vertices.
- Calibrate the ramp using `_allScaleAverage` / `_allScaleMin` / `_phy2Pixel`.
- Treat per-pixel coverage explicitly rather than assuming a generic soft band.
- Improve corner and join handling.

Benefits:

- Smaller change from current renderer.
- Easier to align with the existing path cache.

Risks:

- Still heuristic unless coverage is derived from geometry/pixel overlap.
- May struggle with extreme transforms or very small shapes.

### Signed AASide Edge AA

This is the most promising replacement for the current heuristic `aaSide` path.

The CPU would generate conservative edge coverage geometry, but not final
per-pixel coverage. The important point is that the existing interpolated
`aaSide` value should become a signed edge-side coordinate:

- The edge itself has `aaSide = 0`.
- Inner expanded vertices carry negative `aaSide`.
- Outer expanded vertices carry positive `aaSide`.
- For straight edges, interpolation of this value across the band is enough for
  the shader to locate the true edge at `aaSide = 0`.

The fragment shader then computes coverage from the directed edge:

- Use shader derivatives such as `fwidth(aaSide)`, `dfdx`, and `dfdy` to derive
  the current device-pixel AA width instead of relying only on CPU-computed
  edge-band width.
- Convert the signed side coordinate to alpha coverage with a calibrated ramp.
- Keep premultiplied-alpha behavior explicit and consistent across GL, Metal,
  and Vulkan.

The intended draw order becomes:

1. Draw the directional AA edge band first.
2. Draw the solid body afterward.
3. Use depth so the body covers the inner half of the AA band.

This is different from the old `aaSide` model. The sign of `aaSide` matters
because the edge pass needs to know which side is inside and which side is
outside, even though that sign can still be carried by the existing third vertex
component.

Benefits:

- Keeps the expensive per-pixel coverage decision on the GPU.
- Avoids CPU rasterization of every affected edge pixel.
- Allows AA width to respond naturally to transforms through shader
  derivatives.
- Reuses the existing `{x, y, aaSide}` vertex format while making the coverage
  calculation more geometric and deterministic.

Risks:

- Joins and corners need a precise overlap policy.
- Very small shapes may have little or no stable body area, so they need a
  special fallback or unified edge/body treatment.
- Fill-rule and winding behavior must stay consistent with the body triangles.
- Difference clips, inverted clips, and nested clips need explicit validation.
- The body depth-over-edge strategy must not break batching or render-target
  switching.

### MSAA Or Hybrid AA

Use MSAA where available, possibly combined with shader AA for specific primitives.

Benefits:

- Hardware-supported coverage for triangle edges.
- Can improve general path edges with less shader complexity.

Risks:

- Costly for offscreen surfaces and render-to-texture.
- Does not solve all shader mask/image/filter edges.
- Backend support and resolve behavior must be kept consistent.

This is not a primary direction for Quark right now. It can remain a fallback
or comparison point, but the AA quality track should not depend on MSAA.

### Distance Field / Coverage Masks

Rasterize vector coverage or signed distance into an intermediate mask, then composite.

Benefits:

- Can provide high-quality reusable masks for complex paths.
- May unify clipping and path AA.

Risks:

- Extra passes and memory.
- Needs careful caching/invalidation.
- Small features can be lost without high-resolution masks or exact coverage.

## Skia Source Study

The sibling `../skia` checkout has been used to study both the older Ganesh-era
CCPR implementation and the newer Graphite/Vello compute path renderer. These
are different generations of Skia path rendering and should not be mixed
together when evaluating an approach for Quark.

The main architectural lesson is that Skia does not use one universal AA
algorithm. `GrPathRendererChain` selects among specialized renderers based on
shape type, complexity, transform, GPU capabilities, and requested AA mode:

- Simple convex fills use `GrAAConvexPathRenderer`. Straight edges carry signed
  device-space distance, while quadratic curves use an implicit curve equation
  and shader derivatives to convert that equation into coverage.
- Hairlines use `GrAAHairLinePathRenderer`. Lines are expanded into conservative
  geometry with explicit per-vertex coverage. Segments shorter than one pixel
  reduce their inner coverage by segment length so they remain stable during
  subpixel translation.
- Small concave fills can use `GrTriangulatingPathRenderer`. It resolves the
  entire path topology first, extracts inner and outer boundaries, collapses
  overlapping AA regions, and emits a single triangulated mesh with a one-pixel
  coverage ramp.
- More complicated paths can use CCPR. CCPR decomposes paths into triangles and
  convex curve primitives, renders signed winding coverage into a floating-point
  atlas with additive blending, then samples the resolved coverage mask.
- Tessellation paths use hardware MSAA or mixed samples rather than analytic AA.
- Paths too complicated for the GPU renderers can fall back to a software
  rasterized A8 coverage mask.
- Rects, rounded rects, ovals, and similar common shapes have dedicated render
  operations instead of always going through the general path renderer.

Important implementation details:

- Skia's common coverage ramp is tied to device pixels. The convex tessellator
  uses a `0.5` device-pixel AA radius, and conservative coverage geometry usually
  spans about one device pixel.
- Skia does use signed distance and `dFdx` / `dFdy`, but only where the geometry
  guarantees make the interpolated or implicit value meaningful. It restricts
  the signed-distance convex renderer to simple convex paths with known winding.
- Complex-path AA solves fill topology before generating coverage. It does not
  independently blend an AA strip for every original contour edge.
- Corner treatment is explicit. Convex geometry emits corner wedges and the CCPR
  path has separate corner attenuation logic. A single averaged join normal is
  not treated as sufficient for every angle.
- Very small and degenerate geometry has explicit policy. Skia drops some nearly
  zero-area convex paths, converts degenerate curves to lines, scales coverage
  for subpixel hairlines, and falls back when a specialized algorithm is outside
  its safe domain.

Direct implications for Quark:

- Keep signed `aaSide` as a useful straight-edge coverage coordinate, but do not
  expect `fwidth(aaSide)` alone to solve corners, overlaps, self-intersections,
  or subpixel-thin geometry.
- The current use of `boundaryPath()` is directionally correct because it removes
  internal overlap edges before AA generation. The next robustness step should
  similarly make the final AA mesh topology-aware, rather than allowing
  independently generated edge strips to overlap and blend.
- Replace unbounded averaged-normal miter expansion with explicit join geometry
  and a miter-limit/bevel fallback. This matches Skia's treatment and directly
  addresses acute-angle spikes in `stroke.cc`.
- Add a dedicated subpixel hairline path. When the effective stroke width is
  below one device pixel, coverage should be scaled by width/length rather than
  relying on a normal fill AASide band.
- Add specialized rect and rounded-rect AA paths before investing in a universal
  complex-path solution. These cover a large portion of GUI rendering and allow
  more accurate, cheaper coverage.
- For arbitrary complex paths, the realistic high-quality choices are a
  topology-resolved coverage mesh or a coverage-count/mask atlas. Skia's source
  does not support the idea that independent edge bands alone are sufficient.

### CCPR Conclusion

CCPR renders signed winding coverage into a coverage-count atlas, then samples
the atlas during the final paint draw. Its triangle processor expands each
logical triangle into hull, edge-correction, and corner-correction geometry.
The correction geometry and additive winding resolve overlap and approximate
pixel coverage.

Important conclusions from studying CCPR:

- The atlas is not an already-AA image that receives another AA pass. AA is
  computed while generating the atlas coverage.
- Separate paths are allocated separate atlas regions. CCPR does not union every
  draw in a frame into one giant boolean operation.
- CCPR avoids CPU path boolean operations, but requires substantial custom GPU
  geometry generation, correction triangles, blending rules, atlas management,
  and final sampling integration.
- Corner correction is difficult to understand and maintain. It relies on
  carefully designed interpolated correction surfaces rather than a simple
  edge-normal expansion.
- CCPR was removed from newer Skia architecture. It is useful research material,
  but it is not a planned implementation direction for Quark.

Quark should not implement CCPR. Its complexity is too high relative to the
remaining limitations and newer compute-based alternatives.

### Graphite And Vello Compute AA

Newer Graphite can prefer a Vello-based Compute Path Atlas for suitable complex
paths, while retaining tessellation plus stencil/MSAA and CPU raster atlases as
fallbacks. Skia still selects specialized analytic renderers for simple shapes;
there is no single universal AA path.

The Vello-style compute pipeline does not generate final fill triangles. It
encodes path edges and computes a coverage mask through multiple GPU compute
passes:

```text
Path verbs / points / transforms
  -> path reduction and segmentation
  -> bin segments into tiles
  -> allocate per-tile segment lists
  -> coarse raster / backdrop winding
  -> fine raster per pixel
  -> write coverage atlas
  -> draw bounds quad and sample coverage
```

The fine raster stage calculates winding and AA together. Depending on strategy,
it can use analytic area coverage or software-style fixed samples inside the
compute shader. Compute `MSAA8/16` means evaluating several sample positions and
writing one final coverage value; it does not require an 8x/16x framebuffer.

Compute AA resolves fill topology per pixel rather than producing boolean-result
geometry:

```text
non-zero fill: winding != 0
even-odd fill: winding & 1
positive fill: winding > 0
```

This naturally handles tiny shapes, narrow channels, self-intersections, holes,
and transformed geometry without requiring a stable inset body.

### Compute Shader Model

A compute shader is not part of the vertex/triangle/fragment raster pipeline:

```text
graphics:
vertices -> vertex shader -> triangles -> rasterizer -> fragment shader

compute:
dispatch thread grid -> read buffers/textures -> arbitrary calculation
                     -> write buffers/textures
```

For a `1024x1024` target with `16x16` threads per group:

```text
groupCount = 64x64 groups
each group = 16x16 logical threads
usually one logical thread handles one output pixel
```

One workgroup can process one `16x16` tile. Threads in the group can share a
small fast threadgroup buffer containing the tile's relevant edges.

Each tile needs more than a list of edges that cross it. Fully covered interior
tiles may contain no crossing edge, so rasterization also needs a backdrop or
initial winding value.

### Quark Compute AA Feasibility

Quark's current GL path cannot directly run compute shaders:

- macOS GL uses OpenGL 3.2 / GLSL 330.
- iOS, Android, and Linux GL use GLES 3.0 / GLSL ES 300.
- Compute requires OpenGL 4.3 or GLES 3.1.
- Apple GL cannot be upgraded to a viable compute path.
- Metal supports compute, but Quark does not yet have compute pipeline,
  storage-buffer, dispatch, or compute-to-render synchronization abstractions.

The first practical Quark prototype should therefore target Metal, while GL
continues using AASide. Vulkan and GLES 3.1 can follow after the algorithm is
validated.

Quark already flattens curves and can continue generating stroke outlines on
the CPU. A first compute implementation does not need Vello's full curve/stroke
pipeline:

```text
CPU:
  normalized fill path / CPU-generated stroke path
  -> transform line edges into device/atlas space
  -> assign edges to 16x16 tiles
  -> calculate tile backdrop winding
  -> upload edge and tile buffers

GPU:
  one workgroup per tile
  -> calculate winding and 4x4 sampled coverage per pixel
  -> write R8/R16 coverage atlas

graphics:
  draw path bounds quad and sample coverage atlas
```

CPU binning is acceptable for the first prototype. It is approximately
proportional to edge count plus the number of tiles crossed by edges and avoids
the much larger engineering cost of GPU prefix scans and dynamic allocation.
GPU binning can be added only if profiling proves CPU binning is a bottleneck.

### Stencil And MSAA

Stencil/MSAA avoids CPU boolean-result geometry by allowing triangles to overlap
and accumulating winding in multisampled stencil. It is robust and mature, but
it is not free:

- sample coverage and stencil operations scale with sample count;
- multisample attachments increase tile/storage pressure;
- resolve adds work;
- low sample counts still provide discrete coverage levels.

Full-surface 8x MSAA is especially unattractive for high-resolution GUI
surfaces. An 8K RGBA8 8x color attachment alone is roughly 1 GiB before
depth/stencil, resolve targets, buffering, and temporary surfaces. Tile-based
GPUs and transient/memoryless attachments reduce external memory traffic, but
do not remove raster/sample cost.

Graphite treats stencil/MSAA as an important general fallback, not necessarily
the preferred path for every complex shape. Quark should not make full-surface
MSAA its primary AA strategy.

## libtess2 Cost In Current AASide

`libtess2` is not merely a triangle cutter. Every `tessTesselate()` call runs a
full planar-arrangement sweep:

```text
build half-edge mesh
  -> sweep edges and detect intersections
  -> split/splice topology
  -> accumulate winding and classify inside regions
  -> divide into monotone regions
  -> triangulate or output boundary contours
```

The monotone triangulation stage is relatively cheap. Intersection discovery,
topology repair, winding classification, and mesh allocation are the expensive
parts. Pathological intersection counts can be quadratic, and this libtess2
implementation uses a linked-list active-edge dictionary, which can also make
unfriendly inputs expensive.

Current AASide calls libtess2 twice:

1. `boundaryPath()` resolves the original path into final visible contours.
2. `body.getTriangles()` processes and triangulates the inset body.

The second call is often heavier than necessary for ordinary simple bodies, but
it also repairs inset contours that collapse or self-intersect. libtess2 has no
public option to skip its sweep/boolean stage and perform only triangulation.

Possible future optimization:

- cache `boundaryPath()` independently of AA width;
- triangle-fan convex bodies;
- use ear clipping/Earcut for verified simple inset bodies;
- retain libtess2 as fallback for holes, self-intersections, collapse, and
  uncertain topology.

## AASide Current Assessment

AASide is the current completed fast AA baseline. It remains useful as a fast
GLES 3.0-compatible path for normal-sized, mostly static geometry. It is not a
universal exact-AA solution.

Current strengths:

- works on all existing Quark GPU backends;
- uses the existing `{x, y, aaSide}` vertex format;
- cached geometry is cheap to draw;
- merged `boundaryPath()` removes internal overlap edges before AA generation;
- combined inset body and AA band avoid the previous body/edge draw-order issue;
- sharp corners below 90 degrees are now cut into two source points, and both
  new corners run through the normal miter/AASide generation path.
- allowed old UI-level painter compensation to be removed. UI rects, borders,
  outlines, sprites, and adjacent views can now draw at their real coordinates
  instead of shrinking or nudging geometry to hide overlap seams.
- no longer relies on render z-depth or stencil/depth attachments for normal
  AASide compositing.

Current fundamental limitations:

- **Tiny shapes and narrow channels:** opposing AA bands overlap and a single
  interpolated `aaSide` cannot represent the combined coverage from multiple
  nearby edges. The inset body can collapse or self-intersect.
- **Subpixel-thin borders/hairlines:** normal fill AASide often becomes too dark
  or unstable because independent edge coverage is blended instead of resolved
  as one shape.
- **General transforms:** local-space equal-width expansion does not remain
  equal-width after strong non-uniform scale, shear, or perspective.
  `fwidth(aaSide)` can improve the ramp but cannot repair incorrect expanded
  geometry or join positions.
- **CPU cost:** boundary resolution, inset generation, second tessellation, and
  expanded triangle upload are expensive for dynamic complex paths.

Do not attempt to solve tiny-shape safety by calculating an exact maximum inset
distance for every edge. Correctly detecting mid-edge proximity, offset
collisions, and topology changes approaches medial-axis/straight-skeleton
complexity, can become `O(n^2)`, and still does not solve overlapping coverage
composition.

AASide should be completed as a bounded fast path, then development should move
to Compute AA. As of the `aa-side-refactor` merge point, the bounded fast path
is considered complete enough for normal rendering; remaining issues should be
tracked as future renderer-selection or Compute AA work rather than more
parameter tuning.

## Recommended Quark Strategy

Use multiple renderers instead of forcing every primitive through one AA method:

```text
rect / rounded rect / common GUI border
  -> dedicated analytic shader, including subpixel-width borders

normal-sized general path on all backends
  -> AASide fast path

complex/tiny/transformed path on compute-capable backends
  -> Compute Coverage Atlas

legacy GL fallback
  -> AASide, libtess2, or limited CPU coverage fallback where correctness wins
```

Recommended implementation order:

1. Land the current AASide baseline and keep it as the GL/GLES fallback.
2. Add specialized analytic rect/rrect/border rendering to solve common GUI
   subpixel borders without waiting for general Compute AA.
3. Prototype Metal Compute AA with CPU-transformed line edges, CPU tile binning,
   backdrop winding, and 4x4 sample coverage written into a local mask.
4. Add a transient multi-path coverage atlas, buffer pools, batching, and
   compute-to-render synchronization.
5. Add automatic renderer selection and retain AASide fallback.
6. Port the proven compute path to Vulkan and optionally GLES 3.1.

CCPR is explicitly not planned.

## Evaluation Plan

Do not rely only on visual impressions. Build repeatable test scenes:

- 1 px and sub-1 px lines at many fractional positions.
- Rects and rounded rects at fractional coordinates.
- Circles and cubic curves at small, medium, and large sizes.
- Acute joins, bevel joins, round joins, and line caps.
- Nested clips and AA clip recovery.
- Render-to-texture, readback, and final-present paths.
- High-DPI and non-uniform scale cases.

Useful checks:

- Compare GL, Metal, and later Vulkan screenshots.
- Inspect alpha ramps numerically near representative edges.
- Check stability while translating by small fractions of a pixel.
- Test on light and dark backgrounds because PMA errors show differently.

## Implementation Notes

- Keep shared AA policy in `GPUCanvas` or path/cache code when possible.
- Backend shaders should receive explicit coverage/AA parameters rather than inferring magic constants.
- Avoid backend-specific fixes that make GL, Metal, and Vulkan diverge.
- Be careful with `_surfaceScale`, `_scale`, `_allScaleAverage`, `_allScaleMin`, and `_phy2Pixel`; these decide how logical units map to physical pixels.
- If changing fragment alpha, verify premultiplied-alpha blending and image/mask paths together.
- Clip AA and path AA should be considered together; mismatched edge behavior is very visible in GUI scenes.

## Open Questions

- What should be the exact definition of AA width in Quark coordinates?
- Should per-pixel edge alpha be analytic per primitive, or approximated by a calibrated ramp?
- Can `PathvCache` store enough edge metadata to improve coverage without rebuilding the pipeline?
- Should rounded rects and simple rects get specialized analytic shaders?
- How should AA interact with blur filters and render-to-texture mipmap generation?
- What is the minimum acceptable cross-backend screenshot tolerance?

## Stage Closeout

The AASide branch is being closed as the current AA baseline. The important
stage outcomes are:

- AAFuzz naming and old painter-side shrink/overlap compensation are gone.
- AASide geometry is now based on visible `boundaryPath()` contours rather than
  raw overlapping source contours.
- The `normalSide` sign is calibrated once from the first closed contour and
  reused for later contours; do not recompute containment depth per contour.
- Fill AA uses combined inset-body plus edge-band geometry, so the path no
  longer depends on a separate depth body-over-edge pass.
- Depth/stencil render state was removed from the normal GL/Metal AASide path.
- `_util.glsl::aaSideCoverage()` uses shader derivatives to adapt the coverage
  ramp to device pixels.
- Ordinary GUI results are now good enough that further AASide tuning is low
  priority.

Known unresolved limits:

- Thin 1 px lines at small angles can prefer a wider AA radius, while tiny UI
  elements prefer a tighter one. This is a renderer-selection problem, not just
  a magic constant problem.
- Very small shapes, narrow channels, acute joins, and overlapping AA bands can
  exceed what a single interpolated `aaSide` can represent.
- Large SDF text strokes become blunt because the distance field expansion
  rounds corners.
- Apple/CoreText glyph-mask generation still needs investigation for large-text
  edge quality and RGBA/A8 upload behavior.

The next serious AA pass should be treated as a major Compute AA project, not a
small tweak to `aaSide`.

## Compute AA Prototype Follow-up

The Metal prototype in `test/compute_aa/` has validated the basic coverage
rasterization model:

```text
flattened edges
  -> local 16x16 tile edge lists
  -> tile-left backdrop winding
  -> 4x4 per-pixel sample evaluation
  -> coverage atlas
```

Backdrop is the winding already accumulated to the left of a tile. It allows a
tile shader to scan only locally intersecting edges without losing global fill
state. For multisample coverage, backdrop must match the Y sampling positions:
the current 4x4 prototype stores 16 pixel rows times 4 Y subsamples per tile.
A single winding per tile or one value per pixel-row is insufficient and can
produce horizontal artifacts where subsample windings differ.

CPU backdrop construction currently records each edge's contribution as a
delta at the first tile to its right, then performs an X prefix sum for each
tile-row/Y-subsample. This removes the original correctness-first behavior of
copying every edge into every tile to its right. The next optimization target
is reducing rebuild/allocation/upload cost rather than changing the core
winding algorithm.

## Analytic Area Compute Coverage Direction

The fixed-grid prototype is a useful baseline, but its quality and cost both
scale with the number of discrete samples:

```text
sampleGrid = N
per-pixel sample work ~= N * N
coverage levels = N * N + 1
```

A `4x4` grid produces only 17 coverage values. A `16x16` grid reaches full
8-bit coverage resolution, but requires 256 samples per pixel and is not a
practical general solution.

The next high-quality Compute AA direction should investigate **analytic signed
area coverage**. It is not SDF rendering:

- SDF searches for the nearest edge distance.
- Fixed-grid coverage tests many discrete points for inside/outside.
- Analytic area coverage clips each directed edge against crossed pixels/cells
  and accumulates continuous signed area plus cover deltas.

Conceptual flow:

```text
flattened directed edges
  -> bin edges into tiles / pixel rows
  -> clip an edge across crossed X cells
  -> accumulate per-cell signed area and cover delta
  -> scan X using incoming row cover/backdrop
  -> resolve fill rule and write continuous coverage
```

Potential advantages:

- One edge/cell area contribution can replace many discrete subpixel tests.
- Floating-point coverage naturally uses the full final `R8` range.
- Expensive work can be proportional to boundary crossings rather than full
  atlas pixels multiplied by `sampleGrid * sampleGrid`.
- Fully outside regions can be skipped and fully inside spans can be filled
  from scanline cover state.

Useful parts of the current prototype remain: flattened directed line edges,
tile/tile-row binning, scanline fill-rule semantics, and an incoming left-side
cover/backdrop concept. The current integer winding per discrete Y sample,
`insideMask`, fixed sample grid, and sample delta representation must change.
Analytic rows need continuous incoming cover state, and a sloped edge can
contribute signed trapezoid area to several X cells in one pixel row.

The main implementation risks are efficient accumulation when multiple edges
touch the same cell, avoiding atomics/threadgroup memory as a new bottleneck,
and preserving fill-rule/self-intersection/shared-vertex semantics across tile
boundaries.

Recommended first prototype:

1. Build a CPU reference for directed line segments crossing one pixel row.
2. Compare continuous cell coverage numerically against high-resolution
   supersampling for simple polygons, holes, self-intersections, and fill rules.
3. Move only tile-local analytic cell accumulation to Metal.
4. Compare quality and GPU cost against the current `4x4` GRID branches.
5. Investigate edge-only tile dispatch and interior-span filling after the
   analytic data contract is proven.

Treat analytic area Compute AA as the primary future quality breakthrough
candidate. Keep the GRID implementation as its correctness/performance
comparison baseline rather than continuing deep GRID micro-optimization.

## Boundary-Tile Coverage Breakthrough

The major shared optimization opportunity for both fixed-GRID and future
analytic-area coverage is to distinguish **boundary tiles** from **uniform
tiles** before expensive coverage evaluation.

Topological invariant:

```text
if no effective path boundary crosses a connected tile:
    the whole tile is outside, or
    the whole tile is inside
```

A tile with no boundary crossing cannot contain both inside and outside
regions. Any transition between them would itself require a boundary crossing.
Therefore there is no third "partially filled but boundary-free" tile class.

For the current GRID rasterizer, CPU construction already has the information
needed to classify whether a tile requires local edge/sample correction and,
with backdrop/fill-rule state, whether a uniform tile is all outside or all
inside. The exact classification contract must match the active coverage
algorithm: an edge that has no effect on any GRID sample does not require GRID
coverage work, while an analytic-area implementation must classify actual
continuous boundary crossings.

The coverage texture contract is equally important:

```text
outside uniform tile -> every R8 texel must be 0
inside uniform tile  -> every R8 texel must be 1
boundary tile        -> every R8 texel receives calculated coverage
```

The R8 coverage texture must remain complete and independently sampleable by
all later composition shaders. Do not leave skipped tiles uninitialized, and
do not require downstream shaders to know a separate valid-tile map. That
would complicate every future paint/composite path and weaken the meaning of
the coverage texture.

Recommended first execution model:

```text
one regular Coverage compute pass over all tiles

uniform tile:
    run a very cheap fast path that writes 0 or 1 to all tile pixels

boundary tile:
    run expensive fixed-GRID or analytic-area coverage
```

This preserves:

- one complete R8 texture;
- one simple downstream composition contract;
- regular compute dispatch organization;
- no extra coverage clear pass;
- no requirement to merge interior/exterior tile spans;
- no dependency on ordinary render shaders for correctness.

It also changes the dominant work from "expensive coverage for every tile in
the atlas bounds" to "expensive coverage only for tiles touched by the
effective boundary". Uniform tiles still write their texels, but skip edge
iteration, intersection work, sample-grid scans, or analytic cell-area work.

This is potentially the largest remaining performance improvement for the
current GRID route because the test path contains many uniform tiles. It is
also foundational for analytic-area coverage: the continuous-area algorithm
should only run on boundary tiles, while the same uniform-tile fast path writes
complete 0/1 coverage elsewhere.

Do not conflate this optimization with:

- geometric boolean reconstruction of a body mesh;
- skipping outside tile writes and relying on uninitialized R8 data;
- composing only a sparse subset of tiles;
- merging uniform tiles into spans inside the compute pass.

Those are separate architectural choices and are not required for the first
boundary-tile experiment.
