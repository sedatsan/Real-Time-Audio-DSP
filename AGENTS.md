# AGENTS.md

Drop-in operating instructions for coding agents. Read this file before every task.

**Working code only. Finish the job. Plausibility is not correctness.**

This file follows the [AGENTS.md](https://agents.md) open standard (Linux Foundation / Agentic AI Foundation). Claude Code, Codex, Cursor, Windsurf, Copilot, Aider, Devin, Amp read it natively. For tools that look elsewhere, symlink:

```bash
ln -s AGENTS.md CLAUDE.md
ln -s AGENTS.md GEMINI.md
```

---

## 0. Non-negotiables

These rules override everything else in this file when in conflict:

1. **No flattery, no filler.** Skip openers like "Great question", "You're absolutely right", "Excellent idea", "I'd be happy to". Start with the answer or the action.
2. **Disagree when you disagree.** If the user's premise is wrong, say so before doing the work. Agreeing with false premises to be polite is the single worst failure mode in coding agents.
3. **Never fabricate.** Not file paths, not commit hashes, not API names, not test results, not library functions. If you don't know, read the file, run the command, or say "I don't know, let me check."
4. **Stop when confused.** If the task has two plausible interpretations, ask. Do not pick silently and proceed.
5. **Touch only what you must.** Every changed line must trace directly to the user's request. No drive-by refactors, reformatting, or "while I was in there" cleanups.

---

## 1. Before writing code

- State your plan in one or two sentences before editing.
- Read the files you will touch. Match existing patterns in the codebase.
- Surface assumptions out loud.
- Present tradeoffs for non-trivial approaches.

---

## 2. Writing code: simplicity first

- No features beyond what was asked.
- No abstractions for single-use code.
- Bias toward deleting code over adding code.

---

## 3. Surgical changes

- Change only what the request requires.
- Match the project's existing style exactly.

---

## 4. Goal-driven execution

- Define success as something you can verify.
- Loop until verified.

---

## 5. Tool use and verification

- Prefer running the code to guessing.
- Never report "done" based on a plausible-looking diff alone.
- Address root causes, not symptoms.

---

## 6. Session hygiene

- Keep the context clean. Use subagents for exploration.
- Write descriptive commit messages.

---

## 7. Communication style

- Direct, concise, and prose-based.
- No padding or ceremonial closings.

---

## 8. When to ask, when to proceed

- Ask if choices materially affect output or touch load-bearing parts.
- Proceed for trivial, reversible tasks or when ambiguity is resolvable via code/commands.

---

## 9. Self-improvement loop

- Update section 11 with concrete rules after user corrections.

---

## 10. Project context

### Stack
- Language and version: C++23 (MSVC 19.44)
- Build System: CMake 3.15+ / MSBuild (Visual Studio 2022)
- Audio I/O: PortAudio
- Runtime / deployment target: Windows (x86_64)

### Commands
- Configure: `& "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe" -B build -DCMAKE_TOOLCHAIN_FILE="C:\Users\Sedat\vcpkg\scripts\buildsystems\vcpkg.cmake"`
- Build: `& "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe" --build build --config Release`
- Run locally: `.\build\Release\RealTimeAudioDSP.exe`

### Layout
- Source lives in: `src/`
- Headers live in: `include/`
- Build artifacts: `build/`

### Conventions specific to this repo
- Naming: CamelCase for classes, camelCase for variables/functions.
- Concurrency: `ThreadSafeQueue.h` using `std::mutex` and `std::condition_variable`.
- Real-time: Minimize locking and avoid allocation in audio callbacks.

### Forbidden
- No `new`/`malloc` in audio callbacks.
- No long-held locks in high-priority threads.

---

## 11. Project Learnings

- Always explicitly include `<algorithm>` when using `std::clamp` (MSVC does not transitively include it).
- Link to the `portaudio` CMake target (vcpkg exports `portaudio` on Windows and `portaudio` or `portaudio_static` on Linux).
- Precompute complex twiddle factors using `float` variables to prevent double-to-float narrowing warnings.
- Explicitly cast unused `[[nodiscard]]` return values (like `std::expected` from `stopStream()`) to `(void)` to prevent compiler warnings.

---

## 12. How this file was built

(Standard AGENTS.md boilerplate)
