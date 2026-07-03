# Nebbie Editor

C++17 CMake project: a CLI world editor for the Nebbie Arcane MUD file formats
(`myst.zon`, `myst.wld`, `myst.mob`, `myst.obj`). See `README.md` for the full
command reference and `docs/ARCHITECTURE.md` for the module layout
(`nebbie-core` library + `nebbiedit` CLI + `tests`).

## Cursor Cloud specific instructions

- **Always build with `CXX=g++`.** The default system compiler is Clang, which
  fails at configure time with `cannot find -lstdc++`. All standard commands in
  `README.md` already prefix with `CXX=g++`; keep doing so:
  - Configure: `CXX=g++ cmake -S . -B build`
  - Build: `cmake --build build`
  - Test: `ctest --test-dir build --output-on-failure`
  If a `build/` tree was configured with Clang (e.g. by running plain `cmake`),
  delete it (`rm -rf build`) and re-configure with `CXX=g++`.
- **No separate lint step.** There is no clang-tidy/clang-format/pre-commit
  config; the compiler build (with warnings) is the effective lint, matching CI
  (`.github/workflows/ci.yml`).
- **The CLI is stateless per process.** `g_world` lives only for one invocation,
  so a `load` in one command does not persist to a later `zone list`/`room show`
  in a separate process. Use single-invocation commands that both load and act:
  `nebbiedit info <lib-dir>` or `nebbiedit convert lib roundtrip <lib-dir> <out>`.
  `tests/fixtures` is a ready-to-use sample lib directory.
