# AI Context

This file is long-lived project memory for AI coding assistants. Read it before
making broad assumptions about Quark. Short-lived implementation status belongs
in `docs/CURRENT_WORK.md`, not here.

Also read `docs/ENGINEERING_RULES.md`. It contains hard, long-term decision
rules that take priority over generic or standards-only advice.

## What Quark Is

Quark is a cross-platform GUI framework for Android, iOS, macOS, and Linux. It is implemented primarily in C++ and exposes a JavaScript / JSX runtime for application logic and UI declaration.

Quark is not a browser runtime. It has its own view tree, layout engine, style system, event model, rendering pipeline, and platform integration. Do not assume DOM, browser CSS layout, or web rendering semantics.

## Runtime Shape

The high-level flow is:

```txt
JS / JSX application code
  -> Quark view tree and style/layout system
  -> Painter / Canvas APIs
  -> GPUCanvas shared rendering logic
  -> backend-specific command implementation
  -> platform surface
```

Important source areas:

- `src/ui/`: UI/view integration and painter usage.
- `src/render/`: render abstractions, canvas, paint, path, image, font, and GPU backends.
- `src/render/gl/`: OpenGL backend and current behavior reference for many render features.
- `src/render/metal/`: Metal backend and direct Metal command encoding.
- `src/render/shader/`: source GLSL shader files and generated backend shader
  outputs. Check the generator/source relationship before editing generated
  files.
- `src/platforms/`: platform window/app integration. `src/platforms/linux/` may be relevant only for Linux platform work.
- `tools/` and `libs/qkmake/`: build tooling.
- `test/compute_aa/`: isolated Metal/WebGPU Compute AA research prototype. It is
  not part of the production renderer.

Avoid reading dependency or generated-output directories unless the user explicitly asks. They are usually not useful for project-level context and consume a lot of tokens:

- `deps/`
- any `node_modules/`, including `tools/node_modules/`, `libs/qkmake/node_modules/`, and `libs/qktool/node_modules/`
- `tools/ndk/`
- `tools/pkgs/`
- `tools/linux/`
- `out/`

## Build And Tooling

The README describes the public workflow:

- Install qkmake: `sudo npm install -g qkmake`
- Sync dependencies: `make sync`
- Build app resources: `qkmake build`
- Export projects: `qkmake export ios` or `qkmake export android`
- Debug server: `qkmake watch`

The README notes that the toolchain is currently mainly for macOS and not Windows.

For focused code edits, do not run heavy builds by default. Prefer a light local check or targeted grep unless the user asks for a build/test pass.

## AI Working Rules

- Follow `docs/ENGINEERING_RULES.md`: ground technical claims in Quark's actual
  code, supported toolchains, generated code, measurements, and reproducible
  behavior. Do not mechanically constrain or rewrite the project based only on
  abstract standards advice.
- Keep edits small and tied to the current request.
- Do not format whole files or perform unrelated cleanup.
- The worktree often contains user changes. Never revert or overwrite unrelated changes.
- Prefer `rg` / `rg --files` for search.
- Do not browse or inspect dependency/output directories by default: `deps/`, `node_modules/`, `tools/ndk/`, `tools/pkgs/`, `tools/linux/`, `out/`.
- If touching render code, read the matching GL or Metal implementation before inventing behavior.
- If a file is generated or shader-derived, check the generator path before manually editing generated output.

## Current Render Architecture

The render code is organized around shared GPU canvas behavior:

- `src/render/gpu_canvas.*` contains common Canvas behavior: state stack,
  matrices, clipping orchestration, filters, path filling, text/image drawing,
  and read/output-image flow.
- Backend classes implement the `*Cmd` methods declared by `GPUCanvas`.
- GL uses command packing through `GLC_CmdPack`.
- Metal records directly into Metal command buffers and render/blit encoders
  through `MTL_CmdPack`.

This split is important. New Canvas-level behavior usually belongs in `GPUCanvas`; backend-specific resource handling and draw calls belong in GL/Metal `*Cmd` implementations.

The normal GL/Metal AASide path no longer uses render z depth or depth/stencil
ordering. Coverage and compositing must work through geometry, shaders,
clipping, and blending.

GL remains the behavior reference for many established features, but Metal now
implements the main GL-aligned drawing path, including clipping, blur,
readback, output images, and render-target switching. Match semantics without
copying GL's implicit state-machine habits into Metal.

## Anti-Aliasing Direction

- AASide is the current production fast AA baseline and the GL/GLES-compatible
  fallback. Its signed edge attribute is interpreted in shaders using
  derivatives.
- AASide is intentionally bounded; tiny shapes, narrow channels, overlapping
  coverage, and difficult transforms can exceed what one interpolated edge
  value can represent.
- The next high-precision direction is tile-based Compute AA for Metal/Vulkan
  class backends. The isolated prototype in `test/compute_aa/` validates local
  tile edge lists, tile-left backdrop winding, and 4x4 sample coverage.
- Detailed AA semantics and research notes live in
  `docs/GPU_2D_ANTIALIASING.md`.

## Persistent Concepts

- `ImageSource` is not just an image loader. It is the bridge between decoded pixels, GPU textures, mipmaps, load state, and shared view usage.
- `TexStat` is the backend texture handle container. GL stores numeric texture IDs; Metal stores retained Objective-C texture pointers.
- `RenderResource` owns backend-local upload/unload operations for textures and vertex data.
- `PaintImage::setCanvas()` consumes an already rendered canvas as a texture source.
- `Canvas::outputImage()` changes where subsequent drawing goes.
- `PathvCache` caches normalized paths, fill triangles, AASide geometry, and
  common rect/rrect/outline path data. It coordinates cached vertex resources
  with the active backend.
- `Canvas::swapBuffer()` returns `bool` because a backend may apply
  back-pressure instead of accepting a new frame immediately.
- `Canvas::drawTriangles(copyData=true)` explicitly requests ownership-safe
  copying when deferred command execution outlives transient input buffers.
- Metal render-target changes require ending the current render encoder, but
  they do not inherently require creating a new command buffer.

## Developer Support

- IDE configuration templates live in `.ide/`. `tools/configure.js` writes the
  ignored root `.clangd` and `.lldbinit-Xcode` files with local paths.
- Xcode LLDB formatters for Quark vectors, matrices, strings, containers, and
  iterators live in `.ide/qk_lldb.py`.
- Hard-to-diagnose failures and durable prevention rules belong in
  `docs/TROUBLESHOOTING.md`.

## Documentation Maintenance

When an AI assistant finishes meaningful work, update:

- `docs/CURRENT_WORK.md` if the active implementation state changed.
- `docs/RENDERING.md` if rendering architecture or backend rules changed.
- `docs/GPU_2D_ANTIALIASING.md` if AA semantics or Compute AA direction changed.
- `docs/TROUBLESHOOTING.md` for a diagnosed failure and its prevention rule.
- `docs/ENGINEERING_RULES.md` only for durable hard decision rules.
- `AGENTS.md` only if the entry instructions need to change.

Keep these files practical. They are memory for future work, not marketing documentation.
