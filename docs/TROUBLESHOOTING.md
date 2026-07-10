# Troubleshooting

This file records problems that were hard to diagnose, along with the checks
that helped and the final fix. Keep entries practical: symptom,
misleading clues, root cause, and the code rule that prevents the issue from
coming back.

General decision rules, including the requirement to ground advice in Quark's
actual engineering environment, live in `docs/ENGINEERING_RULES.md`.

## GLES: Text/Image UV Breaks After Large Scroll

Symptom:

- On iOS GLES, text/image textures started sampling from visibly wrong
  positions after scrolling far down a large text scene.
- Vertex positions still looked correct, and the same scene did not show the
  issue on macOS.
- Repeating the texture made the problem easier to see: geometry stayed in the
  expected place while UVs drifted or snapped.

Misleading clues:

- It looked like `glUniform4fv()` or Metal `setVertexBytes()` might be
  truncating the uploaded values.
- The vertex shader also used large coordinates for position, so it was easy to
  assume UV math should have the same precision behavior.

Root cause:

- The generated ES300 shader had ordinary `pc_*` uniforms expanded from
  `PcArgs`, but float members were emitted as `mediump`.
- Image UVs use:

  ```glsl
  coords = (pc.texCoords.xy + vertexIn.xy) / pc.texCoords.zw;
  ```

- During large scrolls, `pc.texCoords.xy` and `vertexIn.xy` can be large values
  with opposite signs. The useful result is the small residual after
  cancellation. `mediump` can quantize the large offset enough that the residual
  becomes wrong, even when the final screen position still looks correct.

Final fix:

- Preserve precision on `PcArgs` members during ES300 expansion.
- Keep coordinate-sensitive values `highp`: vertex positions, matrices, surface
  offsets, and image `texCoords`.
- Keep color/coverage values `mediump` where appropriate.

Prevention rule:

- Do not rely on shader compiler default precision for generated ES uniforms or
  uniform blocks.
- Same-name uniforms/blocks shared across stages must have matching type and
  precision after generation, otherwise iOS GLES can fail link with precision
  mismatch errors.
- Any shader expression that subtracts/cancels large scroll-space coordinates
  must keep the whole coordinate path high precision.

## GL: Strange Gradient Or Black Output With Complete FBOs

Symptom:

- `test/test-layout.cc` rendered a diagonal gradient or black output after GL
  rendering refactors.
- The problem looked related to the final viewport copy path at first.
- Forcing `vportCp` to output a constant red color still did not produce red.
- FBO checks showed both the canvas FBO and default FBO were complete.

Misleading clues:

- The failure appeared near final present/copy, so FBO binding and texture
  sampling looked suspicious.
- Disabling clip logic did not fix it.
- The visible result was a gradient, not an obvious uninitialized uniform/block
  failure.

Root cause:

- `_uboClip` was bound to a uniform block binding point, but no buffer storage
  had been allocated until the first clip restore/update.
- Several shaders have an active `ClipStatBlock`. Even when runtime flags mean
  clip should not be used, the driver may still require the bound UBO to have
  valid storage.
- On Apple GL, especially with software vertex/fragment fallback, this undefined
  state can produce misleading pixels instead of a clear GL error.

Final fix:

- Allocate initial storage for persistent UBOs immediately after creation and
  binding, so no uniform block binding point references a storage-less buffer.

```cpp
glGenBuffers(6, &_uboRMat);
for (int i = 0; i < 5; i++) {
	glBindBuffer(GL_UNIFORM_BUFFER, (&_uboRMat)[i]);
	glBindBufferBase(GL_UNIFORM_BUFFER, i, (&_uboRMat)[i]);
	glBufferData(GL_UNIFORM_BUFFER, 128, nullptr, GL_DYNAMIC_DRAW);
}
```

Prevention rule:

- `glGenBuffers()` creates names only. It does not allocate storage.
- Persistent UBO/VBO objects should receive initial storage before any draw can
  use shaders that reference their binding points.
- Shared UBOs such as clip state should have a valid zero/default state before
  the first draw, even if the feature is logically disabled.

Future debugging hint:

When output is black, stale, or a strange gradient, and forcing the final copy
shader to output a constant color still does not work, do not stop at FBO,
texture binding, or final-copy shader checks. Also verify active uniform blocks
and buffer binding points have allocated storage and sane default contents.

## GL: readImage() Corrupts The Current FBO Output Texture

Symptom:

- `test/test-blur.cc` rendered normally until `Canvas::readImage()` was called
  on the current canvas/FBO contents.
- After the read, following frames drew in the wrong place, almost offscreen, or
  flashed black while dragging the window.
- The blur path itself looked correct when the readback/copy step was disabled.

Misleading clues:

- The failure appeared after blur, so temporary blur textures, blend mode, and
  FBO restore logic looked suspicious.
- The read path used a shader copy from source to destination, so the problem
  looked like a copy-coordinate or viewport issue.

Root cause:

- GL texture storage calls operate on the texture currently bound to
  `GL_TEXTURE_2D`.
- In `readImageCall()`, the source texture was bound before allocating storage
  for the read destination.
- When reading from the current canvas, `srcTex` is the canvas output texture.
  Calling `glTexImage2D()` while `srcTex` is bound reallocates the canvas output
  texture itself, so the next frame renders into a damaged or wrongly sized
  render target.

Final fix:

- Bind the destination texture before `glTexImage2D()`.
- Attach that destination texture to the FBO.
- Rebind the source texture only for sampling during the copy draw.
- Restore the FBO attachment after the copy.

```cpp
glBindTexture(GL_TEXTURE_2D, tex); // destination
glTexImage2D(GL_TEXTURE_2D, 0, iformat, w, h, 0, format, type, nullptr);
glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);

glBindTexture(GL_TEXTURE_2D, srcTex); // source
glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, srcTex, 0);
```

Prevention rule:

- Before any `glTexImage2D()`, `glTexSubImage2D()`, or `glTexParameteri()` call,
  explicitly verify which texture is currently bound.
- In copy/readback paths, separate the destination-allocation phase from the
  source-sampling phase. Do not rely on comments or variable names; GL only sees
  the current binding.

## UI: Window Resize Jitter From Stale Root Layout Mark

Symptom:

- Resizing a macOS window produced visible shimmer/jitter or a one-frame layout
  delay.
- The issue looked like a GL/Metal presentation or live-resize rendering
  problem.
- Experiments with display links, transaction timing, view redraw paths, and
  backend present behavior did not clearly fix it.

Misleading clues:

- The artifact happened while dragging the native window, so render timing and
  surface resize looked like the obvious suspects.
- GL and Metal showed similar behavior, which made it easy to blame platform
  presentation rather than layout invalidation.

Root cause:

- `Root::layout_forward(uint32_t mark)` receives a temporary local `mark` value.
- During resize, `layout_lock_width()` / `layout_lock_height()` can add new
  layout marks to the root.
- The later layout code still used the old temporary `mark`, so child layout
  invalidation could be missed until the next frame.

Final fix:

- After locking the new root size, refresh the local temporary mark from
  `_mark_value` before continuing layout propagation.

```cpp
layout_lock_width(window()->size()[0]);
layout_lock_height(window()->size()[1]);
mark = _mark_value; // update mark value after layout lock
```

Prevention rule:

- If a layout function mutates `_mark_value` while also using a local snapshot
  of marks, refresh the local snapshot before making downstream decisions.
- Resize bugs that look like rendering jitter can still be layout invalidation
  bugs. Check mark propagation before spending more time on present timing.

## C++: Do Not Mechanically Replace Quark's Union bitwise_cast

Context:

- `qk::bitwise_cast<TO>(FROM)` currently uses an equal-sized union, writes the
  `FROM` member, then reads the `TO` member.
- It is used for deliberate low-level bit reinterpretation such as
  `float -> uint32_t`, `double -> uint64_t`, and JavaScriptCore encoded values.
- Quark targets known Clang/GCC-style toolchains where this common union
  type-punning pattern works as intended for the project's scalar/POD uses.

Misleading advice:

- A standards-only review may label every read of the other union member as
  invalid and recommend mechanically replacing the implementation with
  `memcpy`.
- That advice ignores the actual types, supported compilers, generated machine
  code, and the useful compile-time behavior of the current union.

Practical finding:

- For equal-sized scalar/POD types such as `float <-> uint32_t` and
  `double <-> uint64_t`, the current union implementation performs the desired
  bit reinterpretation on Quark's supported platforms.
- A union containing many non-trivial types cannot be default-constructed or
  destroyed, so incorrect `bitwise_cast` instantiations often fail to compile
  naturally.
- A naive `TO out; memcpy(&out, &in, sizeof(out));` replacement can allow some
  non-trivial `TO` types to compile and then overwrite live object state,
  creating a more concrete ownership/destruction problem.
- `reinterpret_cast<T&>` through unrelated references is a separate pattern;
  do not treat it as equivalent to the contained union implementation without
  checking its aliasing and lifetime behavior.

Prevention rule:

- Do not change low-level cast code solely because an abstract language rule
  dislikes the pattern.
- First inspect every instantiated type, the project's compiler/platform
  contract, the generated optimized code, and whether the proposed replacement
  accepts unsafe types that the old implementation rejected.
- Prefer changes that solve an observed bug, portability requirement, sanitizer
  failure, or unsupported compiler target. Avoid standards-compliance rewrites
  that do not improve Quark's actual behavior.
- Keep `bitwise_cast` limited to deliberate equal-sized bit reinterpretation;
  it is not a general object conversion or copying API.

## macOS Profiling: Per-Frame Window Titles Create False CPU Regressions

Symptom:

- Two Compute AA data-building algorithms appeared to have a large CPU
  difference even though their direct construction work was similar.
- Instruments showed substantial accumulated weight under
  `-[NSWindow _dosetTitle:andDefeatWrap:]`.
- Small changes to boundary marking appeared to cause unexpectedly large
  whole-process CPU changes.

Root cause:

- The test assigned a formatted statistics string to `NSWindow.title` every
  frame.
- One algorithm produced rapidly changing boundary/uniform counts, repeatedly
  triggering AppKit title-bar layout, drawing, notifications, and Window Server
  work. Another algorithm produced more stable counts and often reassigned an
  unchanged title.
- Instruments' `Weight` is accumulated inclusive sampled time. A large weight
  with a small `Self Weight` does not mean the title setter itself blocked for
  that duration; most time belongs to descendant AppKit work.

Prevention rule:

- Never update native window titles, labels, logs, or other diagnostic UI every
  frame during renderer CPU profiling.
- Collect counters in memory and emit them once after measurement.
- Compare identical deterministic workloads for equal durations and inspect the
  target function directly rather than relying on whole-process percentages.

## GPU Shaders: Custom Spin Locks Can Deadlock Or Jitter

Symptom:

- CAPA rendered a completely static five-point star with severe visual jitter.
- CPU budgets and stable GPU counters such as `taskCount`, `pathTileCount`,
  `pathTileRowCount`, and `boundaryTileCount` matched across frames.
- `shortEdgeChunkCount` still varied between otherwise identical frames.

Misleading clues:

- A custom shader lock built from `atomicCompSwap` can appear to work in a small
  capture or on one device.
- The code can look logically correct if read like CPU threading: one invocation
  takes the lock, initializes data, releases the lock, and other invocations
  wait.

Root cause / risk:

- GPU execution does not guarantee CPU-style forward progress between arbitrary
  invocations. An invocation that acquired a custom lock is not guaranteed to
  run to its unlock path before sibling lanes/work items spin on the lock.
- Such locks can deadlock or produce scheduling-dependent behavior. This is
  especially dangerous across workgroups, where workgroup execution order is
  not a synchronization primitive.

Prevention rule:

- DANGER: never use shader-level custom mutexes/spin locks that wait for another
  invocation to release a value.
- Use only synchronization with a real GPU guarantee for the current scope, such
  as workgroup barriers inside one workgroup.
- For cross-workgroup data structures, prefer non-blocking atomic append,
  fixed-capacity per-tile storage with explicit overflow/debug markers, or
  separate dispatch passes that publish data before later passes consume it.

## CAPA: Shared Short-Edge Chunks Dropped Edges Despite Valid Atomics

Symptom:

- A static CAPA five-point-star test jittered even after background and area
  math were ruled out.
- Per-pathTile fixed short-edge slots produced stable output, but the dynamic
  tile-local short-edge chunk list lost several edge slots.
- Debug counters showed the expected short-edge references were `1466`, while
  the chunk-list traversal saw only about `1457-1458` in failing versions.
- A chain-debug pass found no orphan chunks, invalid links, or cycles; making
  each emitted short edge its own linked node produced stable counts
  (`1466/1466`).

Misleading clues:

- The chunk list used `atomicAdd` and `atomicExchange`, so it looked like a
  standard GPU linked-list append structure.
- `atomicExchange` itself was not proven faulty. When each edge owned a node,
  the linked list behaved correctly.

Root cause / risk:

- The failing protocol let many invocations append into the same small
  fixed-capacity chunk via `atomicAdd(chunk.count)`. Threads that found the
  chunk full then allocated and linked overflow chunks in the same pass.
- This multi-step "claim slot, detect overflow, allocate repair chunk, publish
  new head" protocol was too complex to reason about reliably on the GPU.
  Individual atomics were not enough to guarantee that every emitted edge
  reached one final readable slot.
- The eventual stable structure changed data ownership: instead of sizing
  storage by "how many edges can a tile receive?", it used the hard invariant
  that one short-edge task can touch at most three tiles. Each task owns three
  `CAPAShortEdge` node slots and links the used nodes to tile chains.

Prevention rule:

- IMPORTANT: avoid same-pass shared small-chunk append plus dynamic overflow
  repair for GPU data structures.
- Do not build complex atomic write-then-read dependency protocols inside one
  pass. If later work needs to read atomic-written state to decide additional
  shared mutations, split it into another pass or redesign ownership so each
  invocation writes only its own node/slot.
- Prefer per-edge linked nodes, fixed owned slots, or a multi-pass
  count/prefix/fill design.
- If using atomics, keep each atomic protocol one-step and auditable: unique
  index allocation, one fully written node linked once, or simple counters.
- When a GPU binning problem looks like it needs dynamic per-tile capacity,
  first look for a stronger bound on the producer side. Moving ownership from
  the unbounded consumer tile to the bounded producer task can remove the need
  for scan and complex atomic synchronization entirely.
