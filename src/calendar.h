#ifndef CALENDAR_H
#define CALENDAR_H

#include "cal_common.h"
#include "raylib.h"

#include <stdio.h>
#include <time.h>

static const char *CALENDAR_DAY_NAMES[] = {"Mon", "Tue", "Wed", "Thu",
                                           "Fri", "Sat", "Sun"};

static const char *HOUR_LABELS[] = {
    "12 AM", "1 AM", "2 AM",  "3 AM",  "4 AM",  "5 AM", "6 AM",  "7 AM",
    "8 AM",  "9 AM", "10 AM", "11 AM", "12 PM", "1 PM", "2 PM",  "3 PM",
    "4 PM",  "5 PM", "6 PM",  "7 PM",  "8 PM",  "9 PM", "10 PM", "11 PM",
};

#define CAL_HOUR_HEIGHT 96.0f
#define CAL_GUTTER_WIDTH 80.0f
#define CAL_HEADER_HEIGHT 120.0f
#define CAL_ALLDAY_HEIGHT 48.0f
#define CAL_GRID_TOTAL_HEIGHT (24.0f * CAL_HOUR_HEIGHT)

// ── Event colors (neobrutalist palette) ──────────────────────────────────────
// colorId -> background + text color pairs
typedef struct {
  Clay_Color bg;
  Clay_Color text;
} EventColors;

static EventColors Calendar_EventColorForId(int colorId) {
  Clay_Color bk = {20, 20, 20, 255};
  Clay_Color wt = {255, 250, 240, 255};
  switch (colorId) {
  case 1:  return (EventColors){{160, 120, 255, 255}, bk}; // lavender
  case 2:  return (EventColors){{  0, 220,  80, 255}, bk}; // sage
  case 3:  return (EventColors){{180,  30, 120, 255}, wt}; // grape
  case 4:  return (EventColors){{255, 100,  70, 255}, bk}; // flamingo
  case 5:  return (EventColors){{255, 220,   0, 255}, bk}; // banana
  case 6:  return (EventColors){{ 50, 200, 160, 255}, bk}; // sage (alias)
  case 7:  return (EventColors){{  0, 150, 255, 255}, bk}; // peacock
  case 8:  return (EventColors){{ 70,  50, 220, 255}, wt}; // blueberry
  case 9:  return (EventColors){{  0, 160,  30, 255}, wt}; // basil
  case 10: return (EventColors){{240,  30,  30, 255}, wt}; // tomato
  case 11: return (EventColors){{255, 160,   0, 255}, bk}; // tangerine
  default: return (EventColors){{  0, 100, 255, 255}, bk}; // default blue
  }
}

// Resolve event color: use colorId override if set, otherwise calendar color
static EventColors Calendar_ResolveEventColor(const CalEvent *ev) {
  if (ev->colorId > 0) {
    return Calendar_EventColorForId(ev->colorId);
  }
  Clay_Color calColor = Calendar_GetCalendarColor(ev->calendarIndex);
  // Pick white or dark text based on perceived brightness
  float lum = 0.299f * calColor.r + 0.587f * calColor.g + 0.114f * calColor.b;
  Clay_Color textColor = (lum > 150.0f)
      ? (Clay_Color){20, 20, 20, 255}
      : (Clay_Color){255, 250, 240, 255};
  return (EventColors){calColor, textColor};
}

// Returns whether an all-day event spans the given date (1-indexed month)
static bool allday_covers(const CalEvent *ev, int year, int mon, int mday) {
  // Compare date triples: start <= date < end
  int d  = year * 10000 + mon * 100 + mday;
  int ds = ev->startYear * 10000 + ev->startMon * 100 + ev->startMday;
  int de = ev->endYear   * 10000 + ev->endMon   * 100 + ev->endMday;
  return d >= ds && d < de;
}

// Returns which weekday column (0=Mon..6=Sun) a UTC time_t falls in,
// using local time. Returns -1 if outside the displayed week.
static int timed_event_col(time_t t, struct tm *week_days_tm) {
  struct tm lt = *localtime(&t);
  for (int i = 0; i < 7; i++) {
    if (lt.tm_mday == week_days_tm[i].tm_mday &&
        lt.tm_mon  == week_days_tm[i].tm_mon  &&
        lt.tm_year == week_days_tm[i].tm_year) {
      return i;
    }
  }
  return -1;
}

// Return local fractional hour for a UTC time_t
static float timed_event_hour(time_t t) {
  struct tm lt = *localtime(&t);
  return (float)lt.tm_hour + (float)lt.tm_min / 60.0f + (float)lt.tm_sec / 3600.0f;
}

// ── Component functions ──────────────────────────────────────────────────────
#include "components/menu_item.h"
#include "components/calendar_checkbox.h"
#include "components/calendar_row.h"
#include "components/event_detail.h"
#include "components/settings_page.h"
#include "components/about_page.h"

static void Calendar_Render(uint32_t fontId) {
  // Load events once
  Calendar_LoadEvents();

  // Reset title buffer index each frame
  g_evtTitleBufIdx = 0;

  static bool menuOpen = false;
  static int  selectedEvent = -1; // index into g_events, -1 = none
  static uint32_t selectedEventElId = 0; // Clay element ID of clicked event
  static bool selectedEventOnLeft = true; // true = event is left of midline, popup goes right

  // Event bucketing arrays — static so previous frame's data is available for click detection
  static int colEvents[7][CAL_MAX_EVENTS];
  static int colEventCount[7];
  static int alldayEvents[7][CAL_MAX_EVENTS];
  static int alldayEventCount[7];

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

  // Navigate to Settings/About on menu item click
  if (menuOpen && IsMouseButtonPressed(0)) {
    if (Clay_PointerOver(Clay_GetElementIdWithIndex(CLAY_STRING("MenuItem"), 0))) {
      g_currentPage = PAGE_SETTINGS;
      menuOpen = false;
    }
    if (Clay_PointerOver(Clay_GetElementIdWithIndex(CLAY_STRING("MenuItem"), 1))) {
      g_currentPage = PAGE_ABOUT;
      menuOpen = false;
    }
  }

  // Back buttons on Settings/About pages
  if (IsMouseButtonPressed(0)) {
    if (g_currentPage == PAGE_SETTINGS &&
        Clay_PointerOver(Clay_GetElementId(CLAY_STRING("SettingsBackBtn")))) {
      g_currentPage = PAGE_CALENDAR;
    }
    if (g_currentPage == PAGE_ABOUT &&
        Clay_PointerOver(Clay_GetElementId(CLAY_STRING("AboutBackBtn")))) {
      g_currentPage = PAGE_CALENDAR;
    }
  }

  // Toggle calendar visibility on sidebar row click
  if (menuOpen && IsMouseButtonPressed(0)) {
    for (int ci = 0; ci < g_calendarCount; ci++) {
      if (Clay_PointerOver(Clay_GetElementIdWithIndex(CLAY_STRING("CalRow"), (uint32_t)ci))) {
        g_calendars[ci].visible = !g_calendars[ci].visible;
        break;
      }
    }
  }

  // Close event detail popup on click outside the card
  if (selectedEvent >= 0 && IsMouseButtonPressed(0) &&
      !Clay_PointerOver(Clay_GetElementId(CLAY_STRING("EvtDetailCard")))) {
    selectedEvent = -1;
    selectedEventElId = 0;
  }

  // Detect clicks on timed event blocks
  if (!menuOpen && selectedEvent < 0 && IsMouseButtonPressed(0)) {
    float midX = (float)GetScreenWidth() / 2.0f;
    for (int i = 0; i < 7; i++) {
      for (int ei = 0; ei < colEventCount[i]; ei++) {
        int evtId = i * CAL_MAX_EVENTS + ei;
        Clay_ElementId eid = Clay_GetElementIdWithIndex(CLAY_STRING("TimedEvt"), (uint32_t)evtId);
        if (Clay_PointerOver(eid)) {
          selectedEvent = colEvents[i][ei];
          selectedEventElId = eid.id;
          selectedEventOnLeft = (GetMouseX() < (int)midX);
          goto evt_click_done;
        }
      }
    }
    // Detect clicks on all-day event blocks
    for (int i = 0; i < 7; i++) {
      for (int ae = 0; ae < alldayEventCount[i]; ae++) {
        int adId = i * CAL_MAX_EVENTS + ae;
        Clay_ElementId eid = Clay_GetElementIdWithIndex(CLAY_STRING("AllDayEvtClick"), (uint32_t)adId);
        if (Clay_PointerOver(eid)) {
          selectedEvent = alldayEvents[i][ae];
          selectedEventElId = eid.id;
          selectedEventOnLeft = (GetMouseX() < (int)midX);
          goto evt_click_done;
        }
      }
    }
    evt_click_done:;
  }

  time_t now = time(NULL);
  struct tm today_tm = *localtime(&now);
  int today_mday = today_tm.tm_mday;
  int today_mon  = today_tm.tm_mon;
  int today_year = today_tm.tm_year;
  int today_hour = today_tm.tm_hour;
  int today_min  = today_tm.tm_min;

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
                  days[i].tm_mon  == today_mon  &&
                  days[i].tm_year == today_year);
  }

  // Current time Y offset for the red line
  float timeLineY =
      ((float)today_hour + (float)today_min / 60.0f) * CAL_HOUR_HEIGHT;

  // Find which column is today (for the red line)
  int todayCol = -1;
  for (int i = 0; i < 7; i++) {
    if (isToday[i]) { todayCol = i; break; }
  }

  // ── Bucket timed events per column ─────────────────────────────────────────
  memset(colEventCount, 0, sizeof(colEventCount));
  memset(alldayEventCount, 0, sizeof(alldayEventCount));

  for (int ei = 0; ei < g_eventCount; ei++) {
    const CalEvent *ev = &g_events[ei];
    if (!g_calendars[ev->calendarIndex].visible) continue;
    if (ev->allDay) {
      for (int i = 0; i < 7; i++) {
        // days[i].tm_mon is 0-based; ev->startMon is 1-based
        if (allday_covers(ev,
              days[i].tm_year + 1900,
              days[i].tm_mon  + 1,
              days[i].tm_mday)) {
          alldayEvents[i][alldayEventCount[i]++] = ei;
        }
      }
    } else {
      int col = timed_event_col(ev->startTime, days);
      if (col >= 0 && colEventCount[col] < CAL_MAX_EVENTS) {
        colEvents[col][colEventCount[col]++] = ei;
      }
    }
  }

  // Determine if any column has all-day events (to show the all-day row)
  bool hasAnyAllday = false;
  for (int i = 0; i < 7; i++) {
    if (alldayEventCount[i] > 0) { hasAnyAllday = true; break; }
  }
  (void)hasAnyAllday; // always render the all-day row for consistency

  if (g_currentPage == PAGE_CALENDAR) {

  CLAY(CLAY_ID("CalendarContainer"),
       {
           .layout =
               {
                   .sizing = {.width = CLAY_SIZING_GROW(0),
                              .height = CLAY_SIZING_GROW(0)},
                   .layoutDirection = CLAY_TOP_TO_BOTTOM,
               },
           .backgroundColor = cal_cream,
       }) {

    // ── Day Header Row ──
    CLAY(CLAY_ID("DayHeaderRow"),
         {
             .layout =
                 {
                     .sizing = {.width = CLAY_SIZING_GROW(0),
                                .height = CLAY_SIZING_FIXED(CAL_HEADER_HEIGHT)},
                 },
             .backgroundColor = cal_cream,
             .border = {.color = cal_borderColor, .width = {.bottom = 3}},
             .floating =
                 {
                     .attachTo = CLAY_ATTACH_TO_PARENT,
                     .zIndex   = 50,
                     .pointerCaptureMode = CLAY_POINTER_CAPTURE_MODE_PASSTHROUGH,
                 },
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
                                      ? cal_hoverYellow
                                      : (Clay_Color){0, 0, 0, 0},

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

               }) {}
        }
      }

      // 7 day header cells
      for (int i = 0; i < 7; i++) {
        Clay_String dayName = {.length = 3, .chars = CALENDAR_DAY_NAMES[i]};
        Clay_String dayNum  = {.length = (int32_t)strlen(dayNumBufs[i]),
                               .chars  = dayNumBufs[i]};

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
                 .border = {.color = cal_borderColor, .width = {.left = 2}},
             }) {

          // Day name (e.g. "Mon")
          CLAY_TEXT(dayName,
                    CLAY_TEXT_CONFIG({
                        .fontId    = fontId,
                        .fontSize  = 20,
                        .textColor = isToday[i] ? cal_accentBlue : cal_secondaryText,
                    }));

          // Day number — blue circle if today
          if (isToday[i]) {
            // Today square shadow
            CLAY(CLAY_IDI("TodayShadow", i),
                 {
                     .layout =
                         {
                             .sizing = {.width  = CLAY_SIZING_FIXED(48),
                                        .height = CLAY_SIZING_FIXED(48)},
                         },
                     .backgroundColor = cal_shadowColor,
                     .floating =
                         {
                             .attachTo      = CLAY_ATTACH_TO_ELEMENT_WITH_ID,
                             .parentId      = Clay_GetElementIdWithIndex(CLAY_STRING("TodayCircle"), (uint32_t)i).id,
                             .attachPoints  = {.element = CLAY_ATTACH_POINT_LEFT_TOP,
                                               .parent  = CLAY_ATTACH_POINT_LEFT_TOP},
                             .offset        = {2, 2},
                             .zIndex        = 0,
                             .pointerCaptureMode = CLAY_POINTER_CAPTURE_MODE_PASSTHROUGH,
                         },
                 }) {}
            CLAY(CLAY_IDI("TodayCircle", i),
                 {
                     .layout =
                         {
                             .sizing = {.width = CLAY_SIZING_FIXED(48),
                                        .height = CLAY_SIZING_FIXED(48)},
                             .childAlignment = {.x = CLAY_ALIGN_X_CENTER,
                                                .y = CLAY_ALIGN_Y_CENTER},
                         },
                     .backgroundColor = cal_accentBlue,

                 }) {
              CLAY_TEXT(dayNum, CLAY_TEXT_CONFIG({
                                    .fontId    = fontId,
                                    .fontSize  = 32,
                                    .textColor = cal_cream,
                                }));
            }
          } else {
            CLAY_TEXT(dayNum, CLAY_TEXT_CONFIG({
                                  .fontId    = fontId,
                                  .fontSize  = 32,
                                  .textColor = cal_primaryText,
                              }));
          }
        }
      }
    }

    // ── All-day row ──
    CLAY(CLAY_ID("AllDayRow"),
         {
             .layout =
                 {
                     .sizing = {.width = CLAY_SIZING_GROW(0),
                                .height = CLAY_SIZING_FIT(CAL_ALLDAY_HEIGHT)},
                 },
             .backgroundColor = cal_cream,
             .border = {.color = cal_borderColor, .width = {.bottom = 3}},
             .floating =
                 {
                     .attachTo = CLAY_ATTACH_TO_PARENT,
                     .attachPoints = {.element = CLAY_ATTACH_POINT_LEFT_TOP,
                                      .parent  = CLAY_ATTACH_POINT_LEFT_TOP},
                     .offset   = {0, CAL_HEADER_HEIGHT},
                     .zIndex   = 50,
                     .pointerCaptureMode = CLAY_POINTER_CAPTURE_MODE_PASSTHROUGH,
                 },
         }) {

      // Gutter spacer
      CLAY(CLAY_ID("AllDayGutter"),
           {
               .layout =
                   {
                       .sizing = {.width  = CLAY_SIZING_FIXED(CAL_GUTTER_WIDTH),
                                  .height = CLAY_SIZING_GROW(0)},
                   },
           }) {}

      // Day cells wrapper (so PERCENT sizing excludes the gutter)
      CLAY(CLAY_ID("AllDayCells"),
           {
               .layout =
                   {
                       .sizing = {.width  = CLAY_SIZING_GROW(0),
                                  .height = CLAY_SIZING_GROW(0)},
                   },
               .clip = {.horizontal = true},
           }) {
        for (int i = 0; i < 7; i++) {
          CLAY(CLAY_IDI("AllDayCell", i),
               {
                   .layout =
                       {
                           .sizing = {.width  = CLAY_SIZING_PERCENT(1.0f / 7.0f),
                                      .height = CLAY_SIZING_GROW(0)},
                           .layoutDirection = CLAY_TOP_TO_BOTTOM,
                           .childGap = 2,
                           .padding = {2, 2, 2, 2},
                       },
                   .clip = {.horizontal = true},
                   .border = {.color = cal_borderColor, .width = {.left = 2}},
               }) {
            for (int ae = 0; ae < alldayEventCount[i]; ae++) {
              const CalEvent *ev = &g_events[alldayEvents[i][ae]];
              EventColors ec = Calendar_ResolveEventColor(ev);
              Clay_String title = cal_make_string(ev->summary);

              int adId = i * CAL_MAX_EVENTS + ae;
              CLAY(CLAY_IDI("AllDayEvtClick", adId),
                   {
                       .layout =
                           {
                               .sizing = {.width  = CLAY_SIZING_GROW(0),
                                          .height = CLAY_SIZING_FIXED(20)},
                               .childAlignment = {.y = CLAY_ALIGN_Y_CENTER},
                               .padding = {4, 4, 0, 0},
                           },
                       .backgroundColor = ec.bg,
                       .border = {.color = cal_borderColor, .width = CLAY_BORDER_ALL(2)},

                   }) {
                CLAY_TEXT(title, CLAY_TEXT_CONFIG({
                                     .fontId    = fontId,
                                     .fontSize  = 22,
                                     .textColor = ec.text,
                                 }));
              }
            }
          }
        }
      }
    }

    // ── Scrollable Area ──
    CLAY(CLAY_ID("ScrollArea"),
         {
             .layout =
                 {
                     .sizing = {.width  = CLAY_SIZING_GROW(0),
                                .height = CLAY_SIZING_GROW(0)},
                 },
             .clip = {.vertical = true, .childOffset = Clay_GetScrollOffset()},
         }) {

      // TimeGrid: LEFT_TO_RIGHT, fixed height
      CLAY(CLAY_ID("TimeGrid"),
           {
               .layout =
                   {
                       .sizing = {.width  = CLAY_SIZING_GROW(0),
                                  .height = CLAY_SIZING_FIXED(CAL_GRID_TOTAL_HEIGHT)},
                   },
           }) {

        // ── Time Labels Column ──
        CLAY(
            CLAY_ID("TimeLabels"),
            {
                .layout =
                    {
                        .sizing = {.width  = CLAY_SIZING_FIXED(CAL_GUTTER_WIDTH),
                                   .height = CLAY_SIZING_GROW(0)},
                        .layoutDirection = CLAY_TOP_TO_BOTTOM,
                    },
            }) {
          for (int h = 0; h < 24; h++) {
            CLAY(CLAY_IDI("HourLabel", h),
                 {
                     .layout =
                         {
                             .sizing = {.width  = CLAY_SIZING_GROW(0),
                                        .height = CLAY_SIZING_FIXED(CAL_HOUR_HEIGHT)},
                             .childAlignment = {.x = CLAY_ALIGN_X_RIGHT},
                             .padding = {0, 8, 0, 0},
                         },
                 }) {
              Clay_String label = {.length = (int32_t)strlen(HOUR_LABELS[h]),
                                   .chars  = HOUR_LABELS[h]};
              CLAY_TEXT(label, CLAY_TEXT_CONFIG({
                                   .fontId    = fontId,
                                   .fontSize  = 18,
                                   .textColor = cal_secondaryText,
                               }));
            }
          }
        }

        // ── Day Columns Area ──
        CLAY(CLAY_ID("DayColumnsArea"),
             {
                 .layout =
                     {
                         .sizing = {.width  = CLAY_SIZING_GROW(0),
                                    .height = CLAY_SIZING_GROW(0)},
                     },
             }) {

          // 7 day columns
          for (int i = 0; i < 7; i++) {
            Clay_Color colBg = isToday[i] ? cal_todayTint : cal_cream;

            CLAY(CLAY_IDI("DayColumn", i),
                 {
                     .layout =
                         {
                             .sizing = {.width  = CLAY_SIZING_GROW(0),
                                        .height = CLAY_SIZING_GROW(0)},
                             .layoutDirection = CLAY_TOP_TO_BOTTOM,
                         },
                     .backgroundColor = colBg,
                     .border = {.color = cal_borderColor, .width = {.left = 2}},
                 }) {
              // 24 hour slots
              for (int h = 0; h < 24; h++) {
                CLAY(CLAY_IDI_LOCAL("HourSlot", h),
                     {
                         .layout =
                             {
                                 .sizing = {.width  = CLAY_SIZING_GROW(0),
                                            .height = CLAY_SIZING_FIXED(CAL_HOUR_HEIGHT)},
                             },
                         .border = {.color = cal_borderColor, .width = {.top = 1}},
                     }) {}
              }
            }

            // ── Timed event blocks for this column (floating) ──
            for (int ei = 0; ei < colEventCount[i]; ei++) {
              const CalEvent *ev = &g_events[colEvents[i][ei]];
              EventColors ec = Calendar_ResolveEventColor(ev);

              float startHour = timed_event_hour(ev->startTime);
              float endHour   = timed_event_hour(ev->endTime);
              float yTop      = startHour * CAL_HOUR_HEIGHT;
              float height    = (endHour - startHour) * CAL_HOUR_HEIGHT;
              if (height < 16.0f) height = 16.0f; // minimum tap target

              Clay_String title = cal_make_string(ev->summary);

              // Each event block is a floating element attached to DayColumnsArea.
              // We need its X offset too. We can't easily get the column's X
              // from Clay at layout time, so we use a trick: attach to the
              // DayColumnsArea and express X as a fraction by nesting.
              // Simpler approach: use CLAY_ATTACH_TO_PARENT (DayColumnsArea)
              // and rely on Clay knowing the column widths.
              // Since columns are SIZING_GROW equally, each is 1/7 of the area.
              // We'll use a floating element per event inside the DayColumnsArea.
              // We need the actual pixel width of the column — but we can
              // approximate by attaching to the parent element and computing
              // x as fraction * parent_width. Clay doesn't expose that directly,
              // so we use a simpler trick: attach to DayColumn[i] by ID and
              // float at (0, yTop) with SIZING_GROW width.

              // Use CLAY_ID_LOCAL won't work for floating with attach-to-element.
              // Use a combined index: col*64 + ei as the IDI numeric key.
              int evtId = i * CAL_MAX_EVENTS + ei;
              CLAY(CLAY_IDI("TimedEvt", evtId),
                   {
                       .layout =
                           {
                               .sizing = {.width  = CLAY_SIZING_GROW(0),
                                          .height = CLAY_SIZING_FIXED(height)},
                               .layoutDirection = CLAY_TOP_TO_BOTTOM,
                               .padding = {4, 4, 4, 4},
                               .childGap = 2,
                           },
                       .backgroundColor = ec.bg,
                       .border = {.color = cal_borderColor, .width = CLAY_BORDER_ALL(2)},
                       .floating =
                           {
                               .attachTo      = CLAY_ATTACH_TO_ELEMENT_WITH_ID,
                               .attachPoints  = {.element = CLAY_ATTACH_POINT_LEFT_TOP,
                                                 .parent  = CLAY_ATTACH_POINT_LEFT_TOP},
                               .parentId      = Clay_GetElementIdWithIndex(CLAY_STRING("DayColumn"), (uint32_t)i).id,
                               .offset        = {2, yTop},
                               .zIndex        = 10,
                               .pointerCaptureMode = CLAY_POINTER_CAPTURE_MODE_PASSTHROUGH,
                           },
                   }) {
                CLAY_TEXT(title, CLAY_TEXT_CONFIG({
                                     .fontId    = fontId,
                                     .fontSize  = 24,
                                     .textColor = ec.text,
                                 }));
              }
            }
          }

          // ── Current Time Line (floating) ──
          if (todayCol >= 0) {
            CLAY(CLAY_ID("CurrentTimeLine"),
                 {
                     .layout =
                         {
                             .sizing = {.width  = CLAY_SIZING_GROW(0),
                                        .height = CLAY_SIZING_FIXED(2)},
                         },
                     .backgroundColor = cal_redLine,
                     .floating =
                         {
                             .attachTo = CLAY_ATTACH_TO_PARENT,
                             .offset   = {0, timeLineY},
                             .pointerCaptureMode =
                                 CLAY_POINTER_CAPTURE_MODE_PASSTHROUGH,
                         },
                 }) {
              // Red dot on left edge
              CLAY(CLAY_ID("CurrentTimeDot"),
                   {
                       .layout =
                           {
                               .sizing = {.width  = CLAY_SIZING_FIXED(12),
                                          .height = CLAY_SIZING_FIXED(12)},
                           },
                       .backgroundColor = cal_redLine,
                       .floating =
                           {
                               .attachTo = CLAY_ATTACH_TO_PARENT,
                               .offset   = {-6, -5},
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
                     .sizing = {.width  = CLAY_SIZING_GROW(0),
                                .height = CLAY_SIZING_GROW(0)},
                 },
             .backgroundColor = {0, 0, 0, 120},
             .floating =
                 {
                     .attachTo = CLAY_ATTACH_TO_ROOT,
                     .zIndex   = 100,
                     .pointerCaptureMode = CLAY_POINTER_CAPTURE_MODE_CAPTURE,
                 },
         }) {}

    // Sidebar shadow
    CLAY(CLAY_ID("SidebarShadow"),
         {
             .layout =
                 {
                     .sizing = {.width  = CLAY_SIZING_FIXED(340),
                                .height = CLAY_SIZING_GROW(0)},
                 },
             .backgroundColor = cal_shadowColor,
             .floating =
                 {
                     .attachTo      = CLAY_ATTACH_TO_ELEMENT_WITH_ID,
                     .parentId      = Clay_GetElementId(CLAY_STRING("SidebarPanel")).id,
                     .attachPoints  = {.element = CLAY_ATTACH_POINT_LEFT_TOP,
                                       .parent  = CLAY_ATTACH_POINT_LEFT_TOP},
                     .offset        = {4, 4},
                     .zIndex        = 100,
                     .pointerCaptureMode = CLAY_POINTER_CAPTURE_MODE_PASSTHROUGH,
                 },
         }) {}

    // Sidebar panel
    CLAY(CLAY_ID("SidebarPanel"),
         {
             .layout =
                 {
                     .sizing = {.width  = CLAY_SIZING_FIXED(340),
                                .height = CLAY_SIZING_GROW(0)},
                     .layoutDirection = CLAY_TOP_TO_BOTTOM,
                     .padding = {0, 0, 8, 8},
                 },
             .backgroundColor = cal_cream,
             .border = {.color = cal_borderColor, .width = {.right = 3}},
             .floating =
                 {
                     .attachTo     = CLAY_ATTACH_TO_ROOT,
                     .attachPoints = {.element = CLAY_ATTACH_POINT_LEFT_TOP,
                                      .parent  = CLAY_ATTACH_POINT_LEFT_TOP},
                     .zIndex = 101,
                     .pointerCaptureMode = CLAY_POINTER_CAPTURE_MODE_CAPTURE,
                 },
         }) {

      // "My calendars" section header
      CLAY(CLAY_ID("CalSectionHeader"),
           {
               .layout =
                   {
                       .sizing = {.width  = CLAY_SIZING_GROW(0),
                                  .height = CLAY_SIZING_FIXED(56)},
                       .childAlignment = {.y = CLAY_ALIGN_Y_CENTER},
                       .padding = {16, 16, 0, 0},
                   },
           }) {
        CLAY_TEXT(CLAY_STRING("My calendars"), CLAY_TEXT_CONFIG({
                                  .fontId    = fontId,
                                  .fontSize  = 20,
                                  .textColor = cal_secondaryText,
                              }));
      }

      // Calendar rows with checkboxes
      for (int ci = 0; ci < g_calendarCount; ci++) {
        CalendarRow(ci, fontId);
      }

      // Divider between calendar list and menu items
      CLAY(CLAY_ID("CalDivider"),
           {
               .layout =
                   {
                       .sizing = {.width  = CLAY_SIZING_GROW(0),
                                  .height = CLAY_SIZING_FIXED(2)},
                   },
               .backgroundColor = cal_borderColor,
           }) {}

      // Menu items
      MenuItem(0, "Settings", fontId);
      MenuItem(1, "About", fontId);
    }
  }

  // ── Event Detail Popup (anchored to clicked event) ──
  if (selectedEvent >= 0 && selectedEvent < g_eventCount && selectedEventElId != 0) {
    EventDetail(&g_events[selectedEvent], fontId, selectedEventElId, selectedEventOnLeft);
  }

  } else if (g_currentPage == PAGE_SETTINGS) {
    SettingsPage_Render(fontId);
  } else if (g_currentPage == PAGE_ABOUT) {
    AboutPage_Render(fontId);
  }
}

#endif
