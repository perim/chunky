# Repository Guidelines

## Project Structure & Module Organization
- Core library sources live at the repo root: `chunky.cpp`, `chunky.h`.
- `chunkgen.cpp` builds the CLI sample generator.
- Tests are in `tests/`, currently `tests/basic_test.cpp`.
- Vendored dependencies live in `external/` (e.g., `external/libdicey`).
- `build/` is a local CMake build directory and should not be committed.

## Build, Test, and Development Commands
- `cmake -S . -B build` configures the project with an out-of-tree build.
- `cmake --build build` builds the library and test/CLI executables.
- `ctest --test-dir build` runs the CTest targets (`basic_test`, `chunkgen_test1`).
- `./build/chunkgen` runs the CLI generator for a quick sanity check.

## Coding Style & Naming Conventions
- C++ with tabs for indentation; braces are typically on the next line.
- Types are lowercase (e.g., `chunk`, `room`, `chunkconfig`), functions use `snake_case`.
- Constants/macros use `ALL_CAPS` (e.g., `TILE_DOOR`, `CHUNK_ASSERT`).
- Keep helpers in `chunky.cpp` unless they are part of the public API.

## Testing Guidelines
- Tests are CTest-driven via the `tests/` executables.
- Name new tests with a `_test.cpp` suffix and register them in `CMakeLists.txt`.
- Prefer deterministic seeds for generation-related tests to avoid flaky output.
- Each class has a `self_test` method that can be invoked to verify its internal consistency.

## Commit & Pull Request Guidelines
- Commit messages are short, imperative, and scoped (e.g., "Fix crash in room split").
- PRs should include a concise description, the tests run, and any linked issues.
- For generation changes, include a small before/after snippet or output note.

## Configuration Tips
- If adding new options, update `chunkgen.cpp` and add tests for them.
- Extra functionality is usually added as standalone `filter` functions.
