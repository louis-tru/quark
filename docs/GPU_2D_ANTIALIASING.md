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

The current GPU path uses an `aafuzz` style method for many vector edges:

- Geometry is drawn normally for the solid interior.
- An extra soft band is generated around the path edge.
- The soft band behaves somewhat like a simple SDF-like expansion.
- Fragment alpha is reduced across this fuzzy outer band to hide jagged edges.
- It does not precisely compute the true coverage/opacity of every affected pixel.

Relevant areas:

- `src/render/gpu_canvas.*`: shared path draw/fill/stroke behavior and `_phy2Pixel` scale handling.
- `src/render/pathv_cache.*`: cached path triangles and AA fuzz stroke geometry.
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

### MSAA Or Hybrid AA

Use MSAA where available, possibly combined with shader AA for specific primitives.

Benefits:

- Hardware-supported coverage for triangle edges.
- Can improve general path edges with less shader complexity.

Risks:

- Costly for offscreen surfaces and render-to-texture.
- Does not solve all shader mask/image/filter edges.
- Backend support and resolve behavior must be kept consistent.

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

This is intentionally parked until the Metal and Vulkan backends are stable enough to compare behavior. The next serious AA pass should be treated as a major quality project, not a small tweak to `aafuzz`.
