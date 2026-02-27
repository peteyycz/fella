# Fella — Clay UI Calendar App

## Overview
Weekly calendar application written in C using the Clay UI framework with a Raylib graphics backend. Google Calendar-inspired design.

## Build
```sh
cmake -B build            # configure
cmake --build build       # build
./build/fella             # run
```
- CMake 3.20+, C99 (`-Wall -Wextra -O2`)
- `compile_commands.json` is exported and symlinked to project root for editor LSP support

## Dependencies
- **Clay** (`clay.h`) — single-header UI layout library (v0.14), bundled in-tree
- **Raylib** — graphics/window library, linked via pkg-config
- **Nix flake** provides the dev environment (gcc, cmake, raylib, gdb, valgrind, clang-tools)

## Project Structure
```
main.c                    # Entry point: Clay init, Raylib window, render loop (small file)
calendar.h                # Calendar UI component (all layout/interaction logic lives here)
clay.h                    # Clay framework (vendored, do not edit)
clay_renderer_raylib.c    # Raylib renderer for Clay (vendored, do not edit)
CMakeLists.txt            # Build config
flake.nix                 # Nix dev environment & package
resources/
  Roboto-Regular.ttf      # UI font
```

## Architecture
- `main.c` is ~65 lines: initializes Clay + Raylib, runs the render loop, calls `Calendar_Render()`.
- All calendar UI lives in `calendar.h` — a single `Calendar_Render(uint32_t fontId)` function using Clay macros (`CLAY`, `CLAY_ID`, `CLAY_TEXT`, etc.).
- Clay uses an immediate-mode layout model: each frame builds a tree of elements, Clay computes layout, then the Raylib renderer draws it.
- `calendar.h` must include `clay.h` and `raylib.h` to be self-contained for LSP analysis.

## Key Patterns
- Clay element IDs: `CLAY_ID("Name")` for unique elements, `CLAY_IDI("Name", index)` for loops, `CLAY_IDI_LOCAL` for nested loops.
- Floating elements (`CLAY_ATTACH_TO_ROOT` / `CLAY_ATTACH_TO_PARENT`) are used for overlays and the current-time indicator.
- Pointer interaction: `Clay_PointerOver()` checks previous frame's layout; combine with Raylib's `IsMouseButtonPressed(0)` for click detection.
- Static locals persist UI state across frames (e.g. `static bool menuOpen`).

## Color Palette
| Name          | Value              |
|---------------|--------------------|
| primaryText   | `{60, 64, 67}`    |
| secondaryText | `{112, 117, 122}` |
| todayBlue     | `{26, 115, 232}`  |
| todayTint     | `{232, 240, 254}` |
| redLine       | `{234, 67, 53}`   |
| borderGray    | `{218, 220, 224}` |
