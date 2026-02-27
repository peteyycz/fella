# Fella

A weekly calendar application built in C with a neobrutalist UI design. Displays events from JSON files in a Google Calendar-inspired weekly grid view.

![C](https://img.shields.io/badge/C-00599C?style=flat&logo=c&logoColor=white)

## Features

- Weekly grid view (Monday-Sunday) with 24-hour time slots
- Current time indicator (red line)
- Multiple calendar support with per-calendar color coding and visibility toggles
- Timed and all-day events
- Clickable event detail popups with title, time, location, and description
- Sidebar menu with calendar list, settings, and about pages
- Auto-scrolls to current time on launch
- Resizable window

## Tech Stack

- **Clay** (v0.14) - immediate-mode UI layout library
- **Raylib** - graphics and windowing
- **cJSON** - JSON parsing
- **CMake** - build system

## Building

### Prerequisites

- CMake 3.20+
- C99-compatible compiler (gcc or clang)
- pkg-config
- Raylib

If using Nix, a development flake is provided:

```sh
nix develop
```

### Build & Run

```sh
cmake -B build
cmake --build build
./build/fella
```

## License

MIT
