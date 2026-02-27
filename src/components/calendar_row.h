#ifndef COMPONENT_CALENDAR_ROW_H
#define COMPONENT_CALENDAR_ROW_H

#include "cal_common.h"
#include "calendar_checkbox.h"

static void CalendarRow(int ci, uint32_t fontId) {
  Clay_Color calColor = Calendar_GetCalendarColor(ci);
  bool vis = g_calendars[ci].visible;

  CLAY(CLAY_IDI("CalRow", ci),
       {
           .layout =
               {
                   .sizing = {.width  = CLAY_SIZING_GROW(0),
                              .height = CLAY_SIZING_FIXED(64)},
                   .childAlignment = {.y = CLAY_ALIGN_Y_CENTER},
                   .padding = {16, 16, 0, 0},
                   .childGap = 10,
               },
           .backgroundColor = Clay_Hovered()
                                  ? cal_hoverBg
                                  : (Clay_Color){0, 0, 0, 0},
           .border = {.color = cal_borderColor, .width = {.bottom = 1}},

       }) {
    CalendarCheckbox(ci, vis, calColor);

    // Calendar name
    Clay_String calName = cal_make_string(g_calendars[ci].name);
    CLAY_TEXT(calName, CLAY_TEXT_CONFIG({
                            .fontId    = fontId,
                            .fontSize  = 22,
                            .textColor = cal_primaryText,
                        }));
  }
}

#endif
