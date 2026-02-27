#ifndef COMPONENT_EVENT_DETAIL_H
#define COMPONENT_EVENT_DETAIL_H

#include "cal_common.h"

#include <stdio.h>
#include <time.h>

static void EventDetail(const CalEvent *sel, uint32_t fontId, uint32_t parentElId, bool onLeft) {
  Clay_Color calColor = Calendar_GetCalendarColor(sel->calendarIndex);

  // Format time string
  static char timeBuf[64];
  if (sel->allDay) {
    snprintf(timeBuf, sizeof(timeBuf), "%04d-%02d-%02d (All day)",
             sel->startYear, sel->startMon, sel->startMday);
  } else {
    struct tm st = *localtime(&sel->startTime);
    struct tm et = *localtime(&sel->endTime);
    snprintf(timeBuf, sizeof(timeBuf), "%02d:%02d - %02d:%02d",
             st.tm_hour, st.tm_min, et.tm_hour, et.tm_min);
  }

  const char *calName = (sel->calendarIndex >= 0 && sel->calendarIndex < g_calendarCount)
                        ? g_calendars[sel->calendarIndex].name : "";

  // Detail card floated next to the clicked event element
  // Show on the right if event is left of midline, on the left otherwise
  Clay_FloatingAttachPoints popupAttach = onLeft
      ? (Clay_FloatingAttachPoints){.element = CLAY_ATTACH_POINT_LEFT_TOP,
                                    .parent  = CLAY_ATTACH_POINT_RIGHT_TOP}
      : (Clay_FloatingAttachPoints){.element = CLAY_ATTACH_POINT_RIGHT_TOP,
                                    .parent  = CLAY_ATTACH_POINT_LEFT_TOP};
  Clay_Vector2 popupOffset = onLeft
      ? (Clay_Vector2){4, 0}
      : (Clay_Vector2){-4, 0};

  // Event detail shadow
  CLAY(CLAY_ID("EvtDetailShadow"),
       {
           .layout =
               {
                   .sizing = {.width  = CLAY_SIZING_FIXED(360),
                              .height = CLAY_SIZING_FIT(0)},
               },
           .backgroundColor = cal_shadowColor,
           .floating =
               {
                   .attachTo      = CLAY_ATTACH_TO_ELEMENT_WITH_ID,
                   .parentId      = Clay_GetElementId(CLAY_STRING("EvtDetailCard")).id,
                   .attachPoints  = {.element = CLAY_ATTACH_POINT_LEFT_TOP,
                                     .parent  = CLAY_ATTACH_POINT_LEFT_TOP},
                   .offset        = {4, 4},
                   .zIndex        = 299,
                   .pointerCaptureMode = CLAY_POINTER_CAPTURE_MODE_PASSTHROUGH,
               },
       }) {}

  CLAY(CLAY_ID("EvtDetailCard"),
       {
           .layout =
               {
                   .sizing = {.width  = CLAY_SIZING_FIXED(360),
                              .height = CLAY_SIZING_FIT(0)},
                   .layoutDirection = CLAY_TOP_TO_BOTTOM,
               },
           .backgroundColor = cal_cream,

           .border = {.color = cal_borderColor, .width = CLAY_BORDER_ALL(3)},
           .floating =
               {
                   .attachTo      = CLAY_ATTACH_TO_ELEMENT_WITH_ID,
                   .parentId      = parentElId,
                   .attachPoints  = popupAttach,
                   .offset        = popupOffset,
                   .zIndex        = 300,
                   .pointerCaptureMode = CLAY_POINTER_CAPTURE_MODE_CAPTURE,
               },
       }) {

    // Color bar at the top
    CLAY(CLAY_ID("EvtDetailColorBar"),
         {
             .layout =
                 {
                     .sizing = {.width  = CLAY_SIZING_GROW(0),
                                .height = CLAY_SIZING_FIXED(8)},
                 },
             .backgroundColor = calColor,

         }) {}

    // Content area
    CLAY(CLAY_ID("EvtDetailContent"),
         {
             .layout =
                 {
                     .sizing = {.width  = CLAY_SIZING_GROW(0),
                                .height = CLAY_SIZING_FIT(0)},
                     .layoutDirection = CLAY_TOP_TO_BOTTOM,
                     .padding = {16, 16, 12, 16},
                     .childGap = 8,
                 },
         }) {

      // Title
      CLAY_TEXT(cal_make_string(sel->summary), CLAY_TEXT_CONFIG({
                                   .fontId    = fontId,
                                   .fontSize  = 28,
                                   .textColor = cal_primaryText,
                               }));

      // Time
      CLAY_TEXT(cal_make_string(timeBuf), CLAY_TEXT_CONFIG({
                                   .fontId    = fontId,
                                   .fontSize  = 22,
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
                     },
             }) {
          CLAY_TEXT(CLAY_STRING("Location:"), CLAY_TEXT_CONFIG({
                                       .fontId    = fontId,
                                       .fontSize  = 22,
                                       .textColor = cal_secondaryText,
                                   }));
          CLAY_TEXT(cal_make_string(sel->location), CLAY_TEXT_CONFIG({
                                       .fontId    = fontId,
                                       .fontSize  = 22,
                                       .textColor = cal_primaryText,
                                   }));
        }
      }

      // Description (if present)
      if (sel->description[0] != '\0') {
        CLAY_TEXT(cal_make_string(sel->description), CLAY_TEXT_CONFIG({
                                     .fontId    = fontId,
                                     .fontSize  = 22,
                                     .textColor = cal_primaryText,
                                     .wrapMode  = CLAY_TEXT_WRAP_WORDS,
                                 }));
      }

      // Divider
      CLAY(CLAY_ID("EvtDetailDivider"),
           {
               .layout =
                   {
                       .sizing = {.width  = CLAY_SIZING_GROW(0),
                                  .height = CLAY_SIZING_FIXED(2)},
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
                         .sizing = {.width  = CLAY_SIZING_FIXED(12),
                                    .height = CLAY_SIZING_FIXED(12)},
                     },
                 .backgroundColor = calColor,
                 .border = {.color = cal_borderColor, .width = CLAY_BORDER_ALL(1)},

             }) {}
        CLAY_TEXT(cal_make_string(calName), CLAY_TEXT_CONFIG({
                                     .fontId    = fontId,
                                     .fontSize  = 22,
                                     .textColor = cal_secondaryText,
                                 }));
      }
    }
  }
}

#endif
