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

The current GPU path uses an `aadist` style method for many vector edges:

- Geometry is drawn normally for the solid interior.
- An extra soft band is generated around the path edge.
- The soft band behaves somewhat like a simple SDF-like expansion.
- Fragment alpha is reduced across this AA distance band to hide jagged edges.
- It does not precisely compute the true coverage/opacity of every affected pixel.

Relevant areas:

- `src/render/gpu_canvas.*`: shared path draw/fill/stroke behavior and `_phy2Pixel` scale handling.
- `src/render/pathv_cache.*`: cached path triangles and AA dist stroke geometry.
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
- The existing directionless `aadist` geometry cannot be drawn before the body
  safely because it does not know which side of the edge is inside or outside.

This points toward a directional GPU edge-AA system rather than a software mask
rasterizer.

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
- Calibrate the ramp using `_allScale` / `_phy2Pixel`.
- Treat per-pixel coverage explicitly rather than assuming a generic soft band.
- Improve corner and join handling.

Benefits:

- Smaller change from current renderer.
- Easier to align with the existing path cache.

Risks:

- Still heuristic unless coverage is derived from geometry/pixel overlap.
- May struggle with extreme transforms or very small shapes.

### Directional Edge AA

This is the most promising replacement for the current directionless `aadist`
path.

The CPU would generate conservative edge coverage geometry, but not final
per-pixel coverage. Each edge primitive should carry enough edge-local data for
the shader to know the true directed edge, for example:

- Edge start/end or equivalent line equation.
- Inside/outside orientation, usually via edge normal or signed winding side.
- A conservative maximum AA expansion width.
- Optional edge/join metadata if the join policy cannot be inferred locally.

The fragment shader then computes coverage from the directed edge:

- Compute signed distance from the pixel to the true edge in screen/device
  space.
- Use shader derivatives such as `fwidth`, `dfdx`, and `dfdy` to derive the
  current device-pixel AA width instead of relying on CPU-computed aadist width.
- Convert distance to alpha coverage with a calibrated ramp.
- Keep premultiplied-alpha behavior explicit and consistent across GL, Metal,
  and Vulkan.

The intended draw order becomes:

1. Draw the directional AA edge band first.
2. Draw the solid body afterward.
3. Use depth so the body covers the inner half of the AA band.

This is different from the current `aadist` model. Direction matters because the
edge pass needs to know which side should be preserved for the outside coverage
ramp and which side will be covered by the body.

Benefits:

- Keeps the expensive per-pixel coverage decision on the GPU.
- Avoids CPU rasterization of every affected edge pixel.
- Allows AA width to respond naturally to transforms through shader
  derivatives.
- Can reuse the existing idea of edge expansion while making the coverage
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
- Be careful with `_surfaceScale`, `_scale`, `_allScale`, and `_phy2Pixel`; these decide how logical units map to physical pixels.
- If changing fragment alpha, verify premultiplied-alpha blending and image/mask paths together.
- Clip AA and path AA should be considered together; mismatched edge behavior is very visible in GUI scenes.

## Open Questions

- What should be the exact definition of AA width in Quark coordinates?
- Should per-pixel edge alpha be analytic per primitive, or approximated by a calibrated ramp?
- Can `PathvCache` store enough edge metadata to improve coverage without rebuilding the pipeline?
- Should rounded rects and simple rects get specialized analytic shaders?
- How should AA interact with blur filters and render-to-texture mipmap generation?
- What is the minimum acceptable cross-backend screenshot tolerance?

## Current Priority

This is intentionally parked until the Metal and Vulkan backends are stable enough to compare behavior. The next serious AA pass should be treated as a major quality project, not a small tweak to `aadist`.
