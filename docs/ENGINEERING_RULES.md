# Quark Engineering Rules

This file defines hard, long-lived project rules for both Quark developers and
AI assistants. It is an internal engineering document, not public API
documentation or a user guide.

The rules here protect decisions that affect multiple modules, platforms, or
future maintenance sessions. Detailed architecture belongs in the matching
topic document, and the full history of a diagnosed failure belongs in
`TROUBLESHOOTING.md`.

When rules appear to compete, use this priority:

1. Preserve demonstrated correctness and supported-platform behavior.
2. Preserve explicit resource-lifetime, threading, and rendering invariants.
3. Prefer the smallest change justified by source inspection, measurements, or
   a reproducible failure.
4. Keep the reason discoverable in code comments and the appropriate project
   document.

## Decision Making

### Reality Comes Before Abstract Rules

All technical decisions must be evaluated against Quark's actual engineering
conditions:

- the concrete types and code being used;
- the supported compilers, platforms, CPUs, GPUs, and drivers;
- generated optimized machine code and real performance;
- observable runtime behavior, tests, diagnostics, and reproducible failures;
- compatibility requirements and the cost of changing existing behavior.

Language standards, specifications, static analyzers, sanitizers, portability
guidance, and common best practices are useful evidence. They are not automatic
vetoes and must not be applied mechanically or treated as substitutes for
understanding the real code.

Do not restrict, rewrite, or complicate Quark code solely because an abstract
rule says a pattern is discouraged or unspecified. First identify the actual
risk within Quark's supported environment and demonstrate how the proposed
change improves correctness, performance, maintainability, portability, or
debuggability without introducing a worse practical failure mode.

When theory and observed engineering reality appear to conflict:

1. Inspect the actual implementation and every relevant instantiated type.
2. Confirm the compiler/platform contract Quark intends to support.
3. Inspect optimized output or construct a focused reproducer when necessary.
4. Measure or reproduce the claimed problem.
5. Make the smallest change justified by that evidence.

Never use standards-only reasoning to limit the project's engineering options
without explaining the concrete consequence it prevents.

## Change And Verification Discipline

All contributors must inspect the relevant implementation and preserve
unrelated worktree changes. Avoid broad refactors, whole-file formatting, or
cross-module cleanup when a focused fix is sufficient. Verification should be
proportional to the actual risk of the change.

The following two permission rules are specific to AI collaboration. They do
not prevent developers from compiling or editing their own project; they keep
automated work within the scope explicitly granted by the developer.

### AI Assistants Do Not Compile By Default

AI assistants must not run C++ builds, full project builds, or other
time-consuming compilation commands unless the user explicitly requests them.

Prefer source inspection, targeted searches, syntax checks, diff checks, and
required code generators. The user will run compilation and provide any
resulting errors for follow-up fixes.

### AI Assistants Do Not Edit Code Without Explicit Approval

When the user is asking for inspection, explanation, review, debugging help, or
whether code is correct, AI assistants must not modify source code or generated
code unless the user explicitly asks for a change.

If the assistant finds a concrete problem, it should first report the issue,
the relevant file/location, and the suggested fix. Wait for the user's explicit
approval before editing code. Documentation may be updated only when the user
explicitly asks for that documentation change.

## Performance-Sensitive Runtime Code

Do not accept a source-level optimization assumption without checking the
optimized output on the platforms that matter. Preserve measured instruction
counts and the tested source variants when they justify a runtime rule.

### Do Not Dynamically Initialize Function-Local TLS In Hot Accessors

Do not put a `thread_local` variable with a run-time initializer inside a hot
accessor, for example:

```cpp
static thread_local Allocator* current = shared();
```

On macOS arm64, Apple Clang emits one TLV for the initialization guard and a
second TLV for the value. Qk's measured `Allocator::current()` stable path then
required 20 visible instructions and two TLV thunk calls; the first-use path
required about 28 visible instructions, three TLV thunk calls, and `shared()`.

For pointer or scalar thread state, prefer namespace-scope constant
initialization and explicit lazy binding:

```cpp
static thread_local Allocator* current = nullptr;
```

The measured macOS stable path for this form required 13 visible instructions
and one TLV thunk, with no compiler-generated TLS initialization guard. A
platform-specific TLS model may further optimize the address lookup where the
loader contract permits it.

This rule prohibits function-local TLS with dynamic initialization in hot
accessors. It does not claim that every function-local, constant-initialized TLS
variable necessarily has a guard; inspect final optimized output before relying
on such an exception. The measurements and assembly are recorded in
`THREAD_LOCAL_ASSEMBLY-cn.md`.

## Runtime Lifecycle And Process Exit

Thread, run-loop, platform, renderer, and process-static resource lifetimes
must be coordinated explicitly. A request to stop is not the same as completed
shutdown, and libc/static teardown must not overlap Qk-managed thread cleanup.

### All Active Qk Process Exits Must Use abort_exit()

`qk::abort_exit(exit_rc)` is the only supported active process-exit path for a
running Qk application. Do not call `::exit()`/`std::exit()` directly, and do
not let a platform system `main()` return while another thread is performing Qk
shutdown.

Qk must stop its managed worker/render threads before libc begins executing
`atexit()` callbacks and destroying static objects. Starting libc teardown too
early can destroy shared EGL, X11, renderer, run-loop, allocator, or other
process-static state while a Qk thread still uses it. The resulting failure may
appear later as a simple EGL call failure, `free(): corrupted unsorted chunks`,
or an unrelated segmentation fault.

The first `abort_exit()` caller owns shutdown. Repeated calls must not repeat
the Qk cleanup or call libc `exit()` again. `is_exit()` means shutdown has
started; it is not a completion barrier and does not authorize the platform
`main()` to return. Platform entry points must remain alive until the shutdown
owner terminates the process.

An `atexit()` callback is diagnostic/fallback protection only. Because atexit
callbacks and static destructors have registration-dependent ordering, it
cannot make arbitrary direct calls to `exit()` safe. The diagnosed Linux
failure sequence and runtime checks are recorded in `TROUBLESHOOTING.md` under
"Process Exit: libc Teardown Races Qk Thread Shutdown".

## Maintaining This Document

Keep only project-wide, durable rules here:

- Put current implementation status and temporary risks in `CURRENT_WORK.md`.
- Put complete failure symptoms, misleading clues, root causes, and diagnostic
  commands in `TROUBLESHOOTING.md`.
- Put backend and subsystem architecture in `RENDERING.md`, `VULKAN.md`,
  `GPU_2D_ANTIALIASING.md`, or the matching topic document.
- Put AI entry instructions and workspace procedure in `AGENTS.md`; repeat an
  AI rule here only when it represents a durable project collaboration policy.
- Do not delete the reasoning behind a rule when reorganizing documentation.
  Move the detail to the correct topic document and leave a concise invariant
  and link here.
