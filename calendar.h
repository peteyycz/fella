#ifndef CALENDAR_H
#define CALENDAR_H

#include "clay.h"
#include "raylib.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

static const char *CALENDAR_DAY_NAMES[] = {"Mon", "Tue", "Wed", "Thu",
                                           "Fri", "Sat", "Sun"};

static const char *HOUR_LABELS[] = {
    "12 AM", "1 AM", "2 AM",  "3 AM",  "4 AM",  "5 AM", "6 AM",  "7 AM",
    "8 AM",  "9 AM", "10 AM", "11 AM", "12 PM", "1 PM", "2 PM",  "3 PM",
    "4 PM",  "5 PM", "6 PM",  "7 PM",  "8 PM",  "9 PM", "10 PM", "11 PM",
};

#define CAL_HOUR_HEIGHT 48.0f
#define CAL_GUTTER_WIDTH 60.0f
#define CAL_HEADER_HEIGHT 72.0f
#define CAL_GRID_TOTAL_HEIGHT (24.0f * CAL_HOUR_HEIGHT)

static void Calendar_Render(uint32_t fontId) {
  static bool menuOpen = false;

  // Toggle menu on hamburger button click
  if (Clay_PointerOver(Clay_GetElementId(CLAY_STRING("HamburgerBtn"))) &&
      IsMouseButtonPressed(0)) {
    menuOpen = !menuOpen;
  }
  // Close menu on scrim click
  if (menuOpen &&
      Clay_PointerOver(Clay_GetElementId(CLAY_STRING("MenuScrim"))) &&
      IsMouseButtonPressed(0)) {
    menuOpen = false;
  }

  time_t now = time(NULL);
  struct tm today_tm = *localtime(&now);
  int today_mday = today_tm.tm_mday;
  int today_mon = today_tm.tm_mon;
  int today_year = today_tm.tm_year;
  int today_hour = today_tm.tm_hour;
  int today_min = today_tm.tm_min;

  // Rewind to Monday of this week
  struct tm monday_tm = today_tm;
  int days_since_monday = (today_tm.tm_wday + 6) % 7;
  monday_tm.tm_mday -= days_since_monday;
  mktime(&monday_tm);

  // Pre-compute day info
  static char dayNumBufs[7][4];
  struct tm days[7];
  bool isToday[7];
  for (int i = 0; i < 7; i++) {
    days[i] = monday_tm;
    days[i].tm_mday = monday_tm.tm_mday + i;
    mktime(&days[i]);
    snprintf(dayNumBufs[i], sizeof(dayNumBufs[i]), "%d", days[i].tm_mday);
    isToday[i] = (days[i].tm_mday == today_mday &&
                  days[i].tm_mon == today_mon && days[i].tm_year == today_year);
  }

  // Colors
  Clay_Color borderGray = {218, 220, 224, 255};
  Clay_Color primaryText = {60, 64, 67, 255};
  Clay_Color secondaryText = {112, 117, 122, 255};
  Clay_Color todayBlue = {26, 115, 232, 255};
  Clay_Color todayTint = {232, 240, 254, 255};
  Clay_Color redLine = {234, 67, 53, 255};
  Clay_Color white = {255, 255, 255, 255};

  // Current time Y offset for the red line
  float timeLineY =
      ((float)today_hour + (float)today_min / 60.0f) * CAL_HOUR_HEIGHT;

  // Find which column is today (for the red line)
  int todayCol = -1;
  for (int i = 0; i < 7; i++) {
    if (isToday[i]) {
      todayCol = i;
      break;
    }
  }

  CLAY(CLAY_ID("CalendarContainer"),
       {
           .layout =
               {
                   .sizing = {.width = CLAY_SIZING_GROW(0),
                              .height = CLAY_SIZING_GROW(0)},
                   .layoutDirection = CLAY_TOP_TO_BOTTOM,
               },
           .backgroundColor = white,
       }) {

    // ── Day Header Row ──
    CLAY(CLAY_ID("DayHeaderRow"),
         {
             .layout =
                 {
                     .sizing = {.width = CLAY_SIZING_GROW(0),
                                .height = CLAY_SIZING_FIXED(CAL_HEADER_HEIGHT)},
                 },
             .border = {.color = borderGray, .width = {.bottom = 1}},
         }) {

      // Hamburger menu button (aligned with time labels gutter)
      CLAY(CLAY_ID("HamburgerBtn"),
           {
               .layout =
                   {
                       .sizing = {.width = CLAY_SIZING_FIXED(CAL_GUTTER_WIDTH),
                                  .height = CLAY_SIZING_GROW(0)},
                       .childAlignment = {.x = CLAY_ALIGN_X_CENTER,
                                          .y = CLAY_ALIGN_Y_CENTER},
                       .layoutDirection = CLAY_TOP_TO_BOTTOM,
                       .childGap = 4,
                   },
               .backgroundColor = Clay_Hovered()
                                      ? (Clay_Color){232, 232, 232, 255}
                                      : (Clay_Color){0, 0, 0, 0},
               .cornerRadius = CLAY_CORNER_RADIUS(4),
           }) {
        for (int bar = 0; bar < 3; bar++) {
          CLAY(CLAY_IDI("HamburgerBar", bar),
               {
                   .layout =
                       {
                           .sizing = {.width = CLAY_SIZING_FIXED(22),
                                      .height = CLAY_SIZING_FIXED(3)},
                       },
                   .backgroundColor = {60, 64, 67, 255},
                   .cornerRadius = CLAY_CORNER_RADIUS(1),
               }) {}
        }
      }

      // 7 day header cells
      for (int i = 0; i < 7; i++) {
        Clay_String dayName = {.length = 3, .chars = CALENDAR_DAY_NAMES[i]};
        Clay_String dayNum = {.length = (int32_t)strlen(dayNumBufs[i]),
                              .chars = dayNumBufs[i]};

        CLAY(CLAY_IDI("DayHeader", i),
             {
                 .layout =
                     {
                         .sizing = {.width = CLAY_SIZING_GROW(0),
                                    .height = CLAY_SIZING_GROW(0)},
                         .layoutDirection = CLAY_TOP_TO_BOTTOM,
                         .childAlignment = {.x = CLAY_ALIGN_X_CENTER,
                                            .y = CLAY_ALIGN_Y_CENTER},
                         .childGap = 2,
                     },
                 .border = {.color = borderGray, .width = {.left = 1}},
             }) {

          // Day name (e.g. "Mon")
          CLAY_TEXT(dayName,
                    CLAY_TEXT_CONFIG({
                        .fontId = fontId,
                        .fontSize = 12,
                        .textColor = isToday[i] ? todayBlue : secondaryText,
                    }));

          // Day number — blue circle if today
          if (isToday[i]) {
            CLAY(CLAY_IDI("TodayCircle", i),
                 {
                     .layout =
                         {
                             .sizing = {.width = CLAY_SIZING_FIXED(36),
                                        .height = CLAY_SIZING_FIXED(36)},
                             .childAlignment = {.x = CLAY_ALIGN_X_CENTER,
                                                .y = CLAY_ALIGN_Y_CENTER},
                         },
                     .backgroundColor = todayBlue,
                     .cornerRadius = CLAY_CORNER_RADIUS(18),
                 }) {
              CLAY_TEXT(dayNum, CLAY_TEXT_CONFIG({
                                    .fontId = fontId,
                                    .fontSize = 20,
                                    .textColor = white,
                                }));
            }
          } else {
            CLAY_TEXT(dayNum, CLAY_TEXT_CONFIG({
                                  .fontId = fontId,
                                  .fontSize = 20,
                                  .textColor = primaryText,
                              }));
          }
        }
      }
    }

    // ── Scrollable Area ──
    CLAY(CLAY_ID("ScrollArea"),
         {
             .layout =
                 {
                     .sizing = {.width = CLAY_SIZING_GROW(0),
                                .height = CLAY_SIZING_GROW(0)},
                 },
             .clip = {.vertical = true, .childOffset = Clay_GetScrollOffset()},
         }) {

      // TimeGrid: LEFT_TO_RIGHT, fixed height
      CLAY(CLAY_ID("TimeGrid"),
           {
               .layout =
                   {
                       .sizing = {.width = CLAY_SIZING_GROW(0),
                                  .height =
                                      CLAY_SIZING_FIXED(CAL_GRID_TOTAL_HEIGHT)},
                   },
           }) {

        // ── Time Labels Column ──
        CLAY(
            CLAY_ID("TimeLabels"),
            {
                .layout =
                    {
                        .sizing = {.width = CLAY_SIZING_FIXED(CAL_GUTTER_WIDTH),
                                   .height = CLAY_SIZING_GROW(0)},
                        .layoutDirection = CLAY_TOP_TO_BOTTOM,
                    },
            }) {
          for (int h = 0; h < 24; h++) {
            CLAY(CLAY_IDI("HourLabel", h),
                 {
                     .layout =
                         {
                             .sizing = {.width = CLAY_SIZING_GROW(0),
                                        .height =
                                            CLAY_SIZING_FIXED(CAL_HOUR_HEIGHT)},
                             .childAlignment = {.x = CLAY_ALIGN_X_RIGHT},
                             .padding = {0, 8, 0, 0},
                         },
                 }) {
              Clay_String label = {.length = (int32_t)strlen(HOUR_LABELS[h]),
                                   .chars = HOUR_LABELS[h]};
              CLAY_TEXT(label, CLAY_TEXT_CONFIG({
                                   .fontId = fontId,
                                   .fontSize = 11,
                                   .textColor = secondaryText,
                               }));
            }
          }
        }

        // ── Day Columns Area ──
        CLAY(CLAY_ID("DayColumnsArea"),
             {
                 .layout =
                     {
                         .sizing = {.width = CLAY_SIZING_GROW(0),
                                    .height = CLAY_SIZING_GROW(0)},
                     },
             }) {

          // 7 day columns
          for (int i = 0; i < 7; i++) {
            Clay_Color colBg = isToday[i] ? todayTint : white;

            CLAY(CLAY_IDI("DayColumn", i),
                 {
                     .layout =
                         {
                             .sizing = {.width = CLAY_SIZING_GROW(0),
                                        .height = CLAY_SIZING_GROW(0)},
                             .layoutDirection = CLAY_TOP_TO_BOTTOM,
                         },
                     .backgroundColor = colBg,
                     .border = {.color = borderGray, .width = {.left = 1}},
                 }) {
              // 24 hour slots
              for (int h = 0; h < 24; h++) {
                CLAY(CLAY_IDI_LOCAL("HourSlot", h),
                     {
                         .layout =
                             {
                                 .sizing = {.width = CLAY_SIZING_GROW(0),
                                            .height = CLAY_SIZING_FIXED(
                                                CAL_HOUR_HEIGHT)},
                             },
                         .border = {.color = borderGray, .width = {.top = 1}},
                     }) {}
              }
            }
          }

          // ── Current Time Line (floating) ──
          if (todayCol >= 0) {
            CLAY(CLAY_ID("CurrentTimeLine"),
                 {
                     .layout =
                         {
                             .sizing = {.width = CLAY_SIZING_GROW(0),
                                        .height = CLAY_SIZING_FIXED(2)},
                         },
                     .backgroundColor = redLine,
                     .floating =
                         {
                             .attachTo = CLAY_ATTACH_TO_PARENT,
                             .offset = {0, timeLineY},
                             .pointerCaptureMode =
                                 CLAY_POINTER_CAPTURE_MODE_PASSTHROUGH,
                         },
                 }) {
              // Red dot on left edge
              CLAY(CLAY_ID("CurrentTimeDot"),
                   {
                       .layout =
                           {
                               .sizing = {.width = CLAY_SIZING_FIXED(12),
                                          .height = CLAY_SIZING_FIXED(12)},
                           },
                       .backgroundColor = redLine,
                       .cornerRadius = CLAY_CORNER_RADIUS(6),
                       .floating =
                           {
                               .attachTo = CLAY_ATTACH_TO_PARENT,
                               .offset = {-6, -5},
                               .pointerCaptureMode =
                                   CLAY_POINTER_CAPTURE_MODE_PASSTHROUGH,
                           },
                   }) {}
            }
          }
        }
      }
    }
  }

  // ── Sidebar Menu Overlay ──
  if (menuOpen) {
    // Scrim / backdrop
    CLAY(CLAY_ID("MenuScrim"),
         {
             .layout =
                 {
                     .sizing = {.width = CLAY_SIZING_GROW(0),
                                .height = CLAY_SIZING_GROW(0)},
                 },
             .backgroundColor = {0, 0, 0, 120},
             .floating =
                 {
                     .attachTo = CLAY_ATTACH_TO_ROOT,
                     .zIndex = 100,
                     .pointerCaptureMode = CLAY_POINTER_CAPTURE_MODE_CAPTURE,
                 },
         }) {}

    // Sidebar panel
    CLAY(CLAY_ID("SidebarPanel"),
         {
             .layout =
                 {
                     .sizing = {.width = CLAY_SIZING_FIXED(280),
                                .height = CLAY_SIZING_GROW(0)},
                     .layoutDirection = CLAY_TOP_TO_BOTTOM,
                     .padding = {0, 0, 8, 8},
                 },
             .backgroundColor = white,
             .border = {.color = borderGray, .width = {.right = 1}},
             .floating =
                 {
                     .attachTo = CLAY_ATTACH_TO_ROOT,
                     .attachPoints = {.element = CLAY_ATTACH_POINT_LEFT_TOP,
                                      .parent = CLAY_ATTACH_POINT_LEFT_TOP},
                     .zIndex = 101,
                     .pointerCaptureMode = CLAY_POINTER_CAPTURE_MODE_CAPTURE,
                 },
         }) {

      // Menu items
      const char *menuItems[] = {"Settings", "About"};
      for (int mi = 0; mi < 2; mi++) {
        CLAY(CLAY_IDI("MenuItem", mi),
             {
                 .layout =
                     {
                         .sizing = {.width = CLAY_SIZING_GROW(0),
                                    .height = CLAY_SIZING_FIXED(44)},
                         .childAlignment = {.y = CLAY_ALIGN_Y_CENTER},
                         .padding = {16, 16, 0, 0},
                     },
                 .backgroundColor = Clay_Hovered()
                                        ? (Clay_Color){232, 240, 254, 255}
                                        : (Clay_Color){0, 0, 0, 0},
                 .cornerRadius = CLAY_CORNER_RADIUS(4),
             }) {
          Clay_String itemText = {
              .length = (int32_t)strlen(menuItems[mi]),
              .chars = menuItems[mi],
          };
          CLAY_TEXT(itemText, CLAY_TEXT_CONFIG({
                                  .fontId = fontId,
                                  .fontSize = 14,
                                  .textColor = primaryText,
                              }));
        }
      }
    }
  }
}

#endif
