# Quark Engineering Rules

This file records hard, long-term rules for engineering decisions in Quark.
These rules apply to humans and AI assistants. They take priority over generic
advice that is not grounded in the project's actual environment.

## 1. Reality Comes Before Abstract Rules

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

## 2. AI Assistants Do Not Compile By Default

AI assistants must not run C++ builds, full project builds, or other
time-consuming compilation commands unless the user explicitly requests them.

Prefer source inspection, targeted searches, syntax checks, diff checks, and
required code generators. The user will run compilation and provide any
resulting errors for follow-up fixes.

## 3. AI Assistants Do Not Edit Code Without Explicit Approval

When the user is asking for inspection, explanation, review, debugging help, or
whether code is correct, AI assistants must not modify source code or generated
code unless the user explicitly asks for a change.

If the assistant finds a concrete problem, it should first report the issue,
the relevant file/location, and the suggested fix. Wait for the user's explicit
approval before editing code. Documentation may be updated only when the user
explicitly asks for that documentation change.

## 4. Avoid Large Shader Struct Copies

In GPU shader code, do not copy large SSBO/storage-buffer structs into local
variables just for convenience. For structs such as `CAPABoundaryTile`, copying
the whole value can move many words per invocation and hide expensive generated
code. Read only the needed fields directly from the storage buffer, or cache
small scalar/vector fields individually when reuse is worthwhile.

## 5. DANGER: Do Not Use Custom Spin Locks In GPU Shaders

Do not implement shader-level mutexes/spin locks that wait for another GPU
invocation to release a value, even if the lock is built from atomics such as
`atomicCompSwap`.

GPU execution does not guarantee that the invocation which acquired a custom
lock will continue to its unlock path before other invocations that are spinning
on that lock. Within SIMD/SIMT execution, sibling lanes or work items can wait
in a way that prevents the releasing path from making forward progress. This
can deadlock or produce vendor/driver-dependent behavior.

Allowed shader synchronization must be limited to mechanisms with a real GPU
execution guarantee for the scope being used, such as workgroup barriers within
one workgroup and non-blocking atomic allocation/append patterns that never
wait for another invocation to make progress. For cross-workgroup coordination,
use separate dispatch passes, bounded non-blocking atomics, fixed-capacity
structures, or explicit overflow/debug paths instead of locks.

## 6. IMPORTANT: Avoid Same-Pass Shared Small-Chunk Append Protocols

GPU atomics are acceptable for simple, one-step coordination such as allocating
a unique index with `atomicAdd`, linking one fully written node with
`atomicExchange`, or computing coarse min/max bounds. Do not assume that a
larger lock-free container protocol is correct merely because each individual
operation is atomic.

As a hard design rule, do not build complex same-pass atomic write-then-read
dependency protocols. Within one dispatch/pass, avoid designs where one
invocation atomically writes a shared state variable and other invocations read
that newly written value in the same pass to drive more shared state changes,
overflow repair, ownership transfer, or linked-container mutation. Split those
state transitions into separate passes, or reduce the protocol to one-step
atomic allocation/linking where every invocation writes only storage it
uniquely owns.

CAPA exposed a new failure mode in `capa_bin.glsl`: multiple invocations shared
a small fixed-capacity short-edge chunk, used `atomicAdd(chunk.count)` to claim
slots, and created/linked overflow chunks when the old chunk was full. The
individual atomic operations appeared valid, and chunk chains were reachable,
but static input still lost edge slots and jittered. Replacing the protocol with
one short-edge node per emitted edge made the counts stable.

Treat same-pass "append into a shared small bucket, then dynamically repair
overflow" designs as high risk. Prefer structures where each invocation writes
only storage it uniquely owns, or split the work into count/prefix/fill passes.
If a linked list is needed, link fully written per-edge nodes rather than having
many invocations mutate the same chunk's slot cursor.

Before adding scans or dynamic per-tile allocators, look for a producer-side
hard bound that can define ownership. CAPA's short-edge binning became simpler
and faster by using the invariant that one short-edge task can touch at most
three tiles: each task owns three `CAPAShortEdge` node slots and links only the
slots it uses. This kind of ownership change is preferred over clever
synchronization.
