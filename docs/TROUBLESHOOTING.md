# Troubleshooting

This file records problems that were hard to diagnose, along with the checks
that helped and the final fix. Keep entries practical: symptom,
misleading clues, root cause, and the code rule that prevents the issue from
coming back.

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
