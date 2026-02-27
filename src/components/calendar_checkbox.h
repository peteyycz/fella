#ifndef COMPONENT_CALENDAR_CHECKBOX_H
#define COMPONENT_CALENDAR_CHECKBOX_H

#include "cal_common.h"

static void CalendarCheckbox(int ci, bool visible, Clay_Color calColor) {
  if (visible) {
    // Filled square with calendar color + white inner square
    CLAY(CLAY_IDI("CalCheck", ci),
         {
             .layout =
                 {
                     .sizing = {.width  = CLAY_SIZING_FIXED(18),
                                .height = CLAY_SIZING_FIXED(18)},
                     .childAlignment = {.x = CLAY_ALIGN_X_CENTER,
                                        .y = CLAY_ALIGN_Y_CENTER},
                 },
             .backgroundColor = calColor,

         }) {
      CLAY(CLAY_IDI("CalCheckInner", ci),
           {
               .layout =
                   {
                       .sizing = {.width  = CLAY_SIZING_FIXED(8),
                                  .height = CLAY_SIZING_FIXED(8)},
                   },
               .backgroundColor = cal_white,

           }) {}
    }
  } else {
    // Empty bordered square
    CLAY(CLAY_IDI("CalCheck", ci),
         {
             .layout =
                 {
                     .sizing = {.width  = CLAY_SIZING_FIXED(18),
                                .height = CLAY_SIZING_FIXED(18)},
                 },
             .border = {.color = calColor, .width = CLAY_BORDER_ALL(2)},

         }) {}
  }
}

#endif
