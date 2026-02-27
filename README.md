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

## Project Structure

```
src/
  main.c                 # Entry point, render loop, Clay/Raylib init
  calendar.h             # Main calendar layout and rendering
  cal_common.h           # Color palette, shared constants, utilities
  events.h               # Event and calendar data structures
  events.c               # JSON event loading and datetime parsing
  components/
    calendar_checkbox.h   # Calendar visibility checkbox
    calendar_row.h        # Sidebar calendar row
    event_detail.h        # Event detail popup
    menu_item.h           # Sidebar menu item
    settings_page.h       # Settings page
    about_page.h          # About page
vendor/
  clay.h                  # Clay UI framework
  clay_renderer_raylib.c  # Raylib renderer for Clay
  cJSON.h / cJSON.c       # JSON parser
resources/
  SpaceMono-Bold.ttf      # UI font
  private-entries.json    # Private calendar events
  work-entries.json       # Work calendar events
```

## Calendar Data

Events are loaded from JSON files in `resources/` using the Google Calendar export format:

```json
{
  "items": [
    {
      "summary": "Meeting",
      "description": "Weekly sync",
      "location": "Room 4",
      "colorId": "7",
      "start": {
        "dateTime": "2026-02-27T09:00:00+01:00"
      },
      "end": {
        "dateTime": "2026-02-27T10:00:00+01:00"
      }
    },
    {
      "summary": "Holiday",
      "start": { "date": "2026-02-28" },
      "end": { "date": "2026-03-01" }
    }
  ]
}
```

Calendars are defined in `src/events.c` with a name, JSON file path, and display color. Events support an optional `colorId` (1-11) to override the calendar color.

## License

MIT
