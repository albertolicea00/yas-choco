# CLAUDE.md — YAS Choco

## What
Native GUI wrapper for **Chocolatey** (`choco`). Part of YAS suite.
Status: **scaffolded & unit-tested** — vendored core + adapter + QML shell compile, 3/3 QtTest suites pass (verified cross-compiling on macOS). Pending: build + QA on the real target platform.

## Stack
- C++20 + Qt 6.7+ (Qt Quick / QML), CMake ≥ 3.24, MSVC toolchain
- Native windowing via Qt QPA plugin: **windows** (Win32). WinRT only for optional extras.
- CLI execution: `QProcess` wrapping `choco`. Never bundle it.
- Architecture: **vendored core copy** (identical across suite, NO shared library by design) + `choco` adapter. Master template: `../yas-core/` local folder (not published). Core fixes must be replicated across repos.

## Target platform
Windows 10/11. x64.

## choco specifics
- **Install/uninstall/upgrade require Administrator.** Strategy: launch elevated helper process (`ShellExecuteEx` + `runas` verb → UAC) for mutating commands; keep GUI unelevated for search/list. This is the hardest platform problem in this app — design it first.
- Key commands: `choco search`, `choco info`, `choco list` (local), `choco install/uninstall/upgrade -y`, `choco pin add/remove`, `choco outdated`.
- Use `--limit-output` (`-r`) for machine-readable pipe-delimited output — always prefer it over default text.
- Community repo rate-limits unauthenticated API queries — cache aggressively.
- Licensed vs FOSS choco editions differ in features; target FOSS.

## Design (see DESIGN.md)
- UI shell: **Teams-style** — icon rail | list panel | detail pane (no in-app title header). Light/dark mode persisted via `YasManager` context property (`src/core/thememanager.*`, QSettings), toggle at rail bottom. Both palettes live in `qml/core/Theme.qml` (`Theme.dark`).
- Dark theme. Base `#222629`, accent **Chocolate `#7B3F00`**, highlight `#7B3F001A`, text `#F8F8F2` / `#ACADAD`.
- App tag: **CHOC**. Fonts: Outfit/Inter (UI), Fira Code or JetBrains Mono (CLI output).

## Conventions
- Conventional Commits (no co-author attribution), feature branches, PRs per CONTRIBUTING.md. Never push to origin without explicit ask.
- Planned layout (mirrors yas-brew, the reference scaffold): `src/core/` (vendored), `src/chocoadapter.*`, `src/main.cpp`, `qml/core/` (vendored) + `qml/Main.qml`, `tests/`, `assets/fonts/`, `icons/` (exists), CMakeLists.txt + CMakePresets.json.
- Packaging: installer + windeployqt; distributable as chocolatey package (dogfooding).

## Key files
README.md · DESIGN.md · CONTRIBUTING.md · EULA.md · SECURITY.md · icons/
