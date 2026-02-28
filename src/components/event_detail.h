#ifndef COMPONENT_EVENT_DETAIL_H
#define COMPONENT_EVENT_DETAIL_H

#include "cal_common.h"

#include <stdio.h>
#include <time.h>

static void EventDetail(const CalEvent *sel, uint32_t fontId,
                        uint32_t parentElId, bool onLeft) {
  Clay_Color calColor = Calendar_GetCalendarColor(sel->calendarIndex);

  // Format time string
  static char timeBuf[64];
  if (sel->allDay) {
    snprintf(timeBuf, sizeof(timeBuf), "%04d-%02d-%02d (All day)",
             sel->startYear, sel->startMon, sel->startMday);
  } else {
    struct tm st = *localtime(&sel->startTime);
    struct tm et = *localtime(&sel->endTime);
    snprintf(timeBuf, sizeof(timeBuf), "%02d:%02d - %02d:%02d", st.tm_hour,
             st.tm_min, et.tm_hour, et.tm_min);
  }

  const char *calName =
      (sel->calendarIndex >= 0 && sel->calendarIndex < g_calendarCount)
          ? g_calendars[sel->calendarIndex].name
          : "";

  Clay_FloatingAttachPoints popupAttach =
      onLeft
          ? (Clay_FloatingAttachPoints){.element = CLAY_ATTACH_POINT_LEFT_TOP,
                                        .parent = CLAY_ATTACH_POINT_RIGHT_TOP}
          : (Clay_FloatingAttachPoints){.element = CLAY_ATTACH_POINT_RIGHT_TOP,
                                        .parent = CLAY_ATTACH_POINT_LEFT_TOP};
  Clay_Vector2 popupOffset =
      onLeft ? (Clay_Vector2){4, 0} : (Clay_Vector2){-4, 0};

  CLAY(CLAY_ID("EvtDetailCard"),
       {
           .layout =
               {
                   .sizing = {.width = CLAY_SIZING_FIXED(360),
                              .height = CLAY_SIZING_FIT(0)},
                   .layoutDirection = CLAY_TOP_TO_BOTTOM,
               },
           .backgroundColor = cal_cardBg,
           .cornerRadius = CLAY_CORNER_RADIUS(12),
           .floating =
               {
                   .attachTo = CLAY_ATTACH_TO_ELEMENT_WITH_ID,
                   .parentId = parentElId,
                   .attachPoints = popupAttach,
                   .offset = popupOffset,
                   .zIndex = 300,
                   .pointerCaptureMode = CLAY_POINTER_CAPTURE_MODE_CAPTURE,
               },
       }) {

    // Content area — use FIXED width so text wrapping resolves correctly
    CLAY(CLAY_ID("EvtDetailContent"),
         {
             .layout =
                 {
                     .sizing = {.width = CLAY_SIZING_FIXED(380),
                                .height = CLAY_SIZING_FIT(0)},
                     .layoutDirection = CLAY_TOP_TO_BOTTOM,
                     .padding = {16, 16, 12, 16},
                     .childGap = 8,
                 },
         }) {

      // Title
      CLAY_TEXT(cal_make_string(sel->summary),
                CLAY_TEXT_CONFIG({
                    .fontId = fontId,
                    .fontSize = 20,
                    .textColor = cal_primaryText,
                    .wrapMode = CLAY_TEXT_WRAP_WORDS,
                }));

      // Time
      CLAY_TEXT(cal_make_string(timeBuf), CLAY_TEXT_CONFIG({
                                              .fontId = fontId,
                                              .fontSize = 18,
                                              .textColor = cal_secondaryText,
                                          }));

      // Location (if present)
      if (sel->location[0] != '\0') {
        CLAY(CLAY_ID("EvtDetailLocRow"),
             {
                 .layout =
                     {
                         .sizing = {.width = CLAY_SIZING_GROW(0),
                                    .height = CLAY_SIZING_FIT(0)},
                         .childGap = 6,
                         .layoutDirection = CLAY_TOP_TO_BOTTOM,
                     },
             }) {
          CLAY_TEXT(CLAY_STRING("Location:"),
                    CLAY_TEXT_CONFIG({
                        .fontId = fontId,
                        .fontSize = 18,
                        .textColor = cal_secondaryText,
                    }));
          CLAY_TEXT(cal_make_string(sel->location),
                    CLAY_TEXT_CONFIG({
                        .fontId = fontId,
                        .fontSize = 16,
                        .textColor = cal_primaryText,
                        .wrapMode = CLAY_TEXT_WRAP_WORDS,
                    }));
        }
      }

      // Description (if present)
      if (sel->description[0] != '\0') {
        CLAY_TEXT(cal_make_string(sel->description),
                  CLAY_TEXT_CONFIG({
                      .fontId = fontId,
                      .fontSize = 18,
                      .textColor = cal_primaryText,
                      .wrapMode = CLAY_TEXT_WRAP_WORDS,
                  }));
      }

      // Divider
      CLAY(CLAY_ID("EvtDetailDivider"),
           {
               .layout =
                   {
                       .sizing = {.width = CLAY_SIZING_GROW(0),
                                  .height = CLAY_SIZING_FIXED(1)},
                   },
               .backgroundColor = cal_borderColor,
           }) {}

      // Calendar name with colored dot
      CLAY(CLAY_ID("EvtDetailCalRow"),
           {
               .layout =
                   {
                       .sizing = {.width = CLAY_SIZING_GROW(0),
                                  .height = CLAY_SIZING_FIT(0)},
                       .childAlignment = {.y = CLAY_ALIGN_Y_CENTER},
                       .childGap = 8,
                   },
           }) {
        CLAY(CLAY_ID("EvtDetailCalDot"),
             {
                 .layout =
                     {
                         .sizing = {.width = CLAY_SIZING_FIXED(12),
                                    .height = CLAY_SIZING_FIXED(12)},
                     },
                 .backgroundColor = calColor,
                 .cornerRadius = CLAY_CORNER_RADIUS(6),
                 .border = {.color = cal_borderColor,
                            .width = CLAY_BORDER_ALL(1)},

             }) {}
        CLAY_TEXT(cal_make_string(calName), CLAY_TEXT_CONFIG({
                                                .fontId = fontId,
                                                .fontSize = 18,
                                                .textColor = cal_secondaryText,
                                            }));
      }
    }
  }
}

#endif
