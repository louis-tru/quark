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
