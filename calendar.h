#ifndef CALENDAR_H
#define CALENDAR_H

#include <stdio.h>
#include <time.h>

static const char *CALENDAR_DAY_NAMES[] = {"Sun", "Mon", "Tue", "Wed",
                                           "Thu", "Fri", "Sat"};

static void Calendar_Render(uint32_t fontId) {
  time_t now = time(NULL);
  struct tm today_tm = *localtime(&now);
  int today_mday = today_tm.tm_mday;

  // Rewind to Sunday of this week
  struct tm sunday_tm = today_tm;
  sunday_tm.tm_mday -= today_tm.tm_wday;
  mktime(&sunday_tm);

  // Format header: "Week of Mon DD, YYYY"
  static char headerBuf[64];
  strftime(headerBuf, sizeof(headerBuf), "Week of %b %d, %Y", &sunday_tm);
  Clay_String headerStr = {.length = (int32_t)strlen(headerBuf),
                           .chars = headerBuf};

  // Pre-compute day number strings
  static char dayNumBufs[7][4];
  struct tm days[7];
  for (int i = 0; i < 7; i++) {
    days[i] = sunday_tm;
    days[i].tm_mday = sunday_tm.tm_mday + i;
    mktime(&days[i]);
    snprintf(dayNumBufs[i], sizeof(dayNumBufs[i]), "%d", days[i].tm_mday);
  }

  CLAY(CLAY_ID("CalendarContainer"),
       {
           .layout =
               {
                   .sizing = {.width = CLAY_SIZING_GROW(0),
                              .height = CLAY_SIZING_GROW(0)},
                   .layoutDirection = CLAY_TOP_TO_BOTTOM,
                   .padding = {24, 24, 24, 24},
                   .childGap = 16,
               },
           .backgroundColor = {240, 240, 240, 255},
       }) {

    // Header
    CLAY(CLAY_ID("CalendarHeader"),
         {
             .layout =
                 {
                     .sizing = {.width = CLAY_SIZING_GROW(0)},
                     .childAlignment = {.x = CLAY_ALIGN_X_CENTER},
                     .padding = {0, 0, 8, 8},
                 },
         }) {
      CLAY_TEXT(headerStr, CLAY_TEXT_CONFIG({
                               .fontId = fontId,
                               .fontSize = 24,
                               .textColor = {30, 30, 30, 255},
                           }));
    }

    // Day columns row
    CLAY(CLAY_ID("CalendarWeekRow"),
         {
             .layout =
                 {
                     .sizing = {.width = CLAY_SIZING_GROW(0),
                                .height = CLAY_SIZING_GROW(0)},
                     .childGap = 4,
                 },
         }) {
      for (int i = 0; i < 7; i++) {
        bool isToday = (days[i].tm_mday == today_mday &&
                        days[i].tm_mon == today_tm.tm_mon &&
                        days[i].tm_year == today_tm.tm_year);

        Clay_Color bgColor = isToday ? (Clay_Color){66, 133, 244, 255}
                                     : (Clay_Color){255, 255, 255, 255};
        Clay_Color textColor = isToday ? (Clay_Color){255, 255, 255, 255}
                                       : (Clay_Color){60, 60, 60, 255};

        Clay_String dayName = {.length = 3, .chars = CALENDAR_DAY_NAMES[i]};
        Clay_String dayNum = {.length = (int32_t)strlen(dayNumBufs[i]),
                              .chars = dayNumBufs[i]};

        CLAY(CLAY_IDI_LOCAL("DayCol", i),
             {
                 .layout =
                     {
                         .sizing = {.width = CLAY_SIZING_GROW(0),
                                    .height = CLAY_SIZING_GROW(0)},
                         .layoutDirection = CLAY_TOP_TO_BOTTOM,
                         .padding = {8, 8, 12, 12},
                         .childGap = 8,
                         .childAlignment = {.x = CLAY_ALIGN_X_CENTER},
                     },
                 .backgroundColor = bgColor,
                 .cornerRadius = CLAY_CORNER_RADIUS(8),
             }) {

          CLAY_TEXT(dayName, CLAY_TEXT_CONFIG({
                                 .fontId = fontId,
                                 .fontSize = 18,
                                 .textColor = textColor,
                             }));

          CLAY_TEXT(dayNum, CLAY_TEXT_CONFIG({
                                .fontId = fontId,
                                .fontSize = 24,
                                .textColor = textColor,
                            }));
        }
      }
    }
  }
}

#endif
