# IDE Configuration

This directory contains portable IDE configuration templates and debugger
helpers.

Running `tools/configure.js` generates these ignored files in the repository
root, where clangd and Xcode expect to find them:

- `.clangd` from `.ide/clangd`
- `.lldbinit-Xcode` from `.ide/lldbinit-Xcode`
- `.ide/compile_commands.json` from `.ide/compile_flags.txt` and the current
  project source tree

The generated `.clangd` points clangd at `.ide/compile_flags.txt` and uses the
locally installed `rayanhtt.metal-lsp` extension paths when available. Set
`VSCODE_EXTENSIONS` when extensions are installed in a nonstandard directory.
The generated compilation database gives clangd a complete source-file list so
its background index can cache definitions before files are opened.
