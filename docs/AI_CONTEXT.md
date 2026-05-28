# AI Context

This file is long-lived project memory for AI coding assistants. Read it before making broad assumptions about Quark.

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
- `src/render/metal/`: Metal backend, currently being filled out and aligned with GL semantics.
- `src/render/shader/`: source GLSL shader files used to generate backend shader wrappers.
- `src/platforms/`: platform window/app integration. `src/platforms/linux/` may be relevant only for Linux platform work.
- `tools/` and `libs/qkmake/`: build tooling.

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

- Keep edits small and tied to the current request.
- Do not format whole files or perform unrelated cleanup.
- The worktree often contains user changes. Never revert or overwrite unrelated changes.
- Prefer `rg` / `rg --files` for search.
- Do not browse or inspect dependency/output directories by default: `deps/`, `node_modules/`, `tools/ndk/`, `tools/pkgs/`, `tools/linux/`, `out/`.
- If touching render code, read the matching GL or Metal implementation before inventing behavior.
- If a file is generated or shader-derived, check the generator path before manually editing generated output.

## Current Architectural Direction

The render code is being reorganized around shared GPU canvas behavior:

- `src/render/gpu_canvas.*` contains common Canvas behavior: state stack, matrix, clipping orchestration, z depth, filters, path filling, text image drawing, read/output image flow.
- Backend classes implement the `*Cmd` methods declared by `GPUCanvas`.
- GL uses command packing through `GLC_CmdPack`.
- Metal records directly into Metal command buffers and render encoders through `MTL_CmdPack`.

This split is important. New Canvas-level behavior usually belongs in `GPUCanvas`; backend-specific resource handling and draw calls belong in GL/Metal `*Cmd` implementations.

## Persistent Concepts

- `ImageSource` is not just an image loader. It is the bridge between decoded pixels, GPU textures, mipmaps, load state, and shared view usage.
- `TexStat` is the backend texture handle container. GL stores numeric texture IDs; Metal stores retained Objective-C texture pointers.
- `RenderResource` owns backend-local upload/unload operations for textures and vertex data.
- `PaintImage::setCanvas()` consumes an already rendered canvas as a texture source.
- `Canvas::outputImage()` changes where subsequent drawing goes.
- `PathvCache` caches generated path vertices and anti-alias distance geometry.
- `zDepthNextUnit` is used to order draw calls in GPU depth.

## Documentation Maintenance

When an AI assistant finishes meaningful work, update:

- `docs/CURRENT_WORK.md` if the active implementation state changed.
- `docs/RENDERING.md` if rendering architecture or backend rules changed.
- `AGENTS.md` only if the entry instructions need to change.

Keep these files practical. They are memory for future work, not marketing documentation.
