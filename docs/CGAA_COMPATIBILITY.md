# CGAA Compatibility

This file records platform and GPU requirements that must be preserved when
CGAA enters Quark's production renderer.

## Metal Shading Language Requirement

The generated CGAA compute shader currently targets **MSL 2.3**:

```txt
spirv-cross --msl-version 23000
```

SPIRV-Cross encodes the version as `MMmmpp`, so `23000` means MSL 2.3.0.
MSL 2.3 is the minimum tested version that compiles the current CGAA shader.
MSL 2.2 fails because the generated shader uses 64-bit `ctz` and `popcount`
operations.

MSL 2.3 requires:

- macOS 11.0 or later;
- iOS, iPadOS, or tvOS 14.0 or later;
- a GPU/device family that supports the required 64-bit integer operations.

## Quark Fallback Rule

Quark currently supports macOS 10.15. Production CGAA integration must not make
MSL 2.3 an unconditional renderer requirement.

- Use CGAA only on macOS 11.0 or later when the active Metal device supports
  the required 64-bit integer operations.
- Fall back to AASide on macOS 10.15.
- Also fall back to AASide when runtime GPU capability checks fail.

Do not rely only on the OS version. Add an explicit runtime capability check
before selecting CGAA, because GPU capabilities and backend availability are
separate from deployment-target compatibility.

## Generator Policy

Keep CGAA at the lowest MSL version that supports its actual generated code.
Do not raise it to the newest MSL version merely because the local Xcode
toolchain supports that version; doing so unnecessarily reduces deployment and
hardware compatibility.
