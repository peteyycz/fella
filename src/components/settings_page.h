#ifndef COMPONENT_SETTINGS_PAGE_H
#define COMPONENT_SETTINGS_PAGE_H

#include "cal_common.h"

static void SettingsPage_Render(uint32_t fontId) {
  CLAY(CLAY_ID("SettingsPage"),
       {
           .layout =
               {
                   .sizing = {.width = CLAY_SIZING_GROW(0),
                              .height = CLAY_SIZING_GROW(0)},
                   .layoutDirection = CLAY_TOP_TO_BOTTOM,
               },
           .backgroundColor = cal_cream,
       }) {

    // Top bar with back button
    CLAY(CLAY_ID("SettingsTopBar"),
         {
             .layout =
                 {
                     .sizing = {.width = CLAY_SIZING_GROW(0),
                                .height = CLAY_SIZING_FIXED(80)},
                     .childAlignment = {.y = CLAY_ALIGN_Y_CENTER},
                     .padding = {16, 16, 0, 0},
                     .childGap = 16,
                 },
             .backgroundColor = cal_cream,
             .border = {.color = cal_borderColor, .width = {.bottom = 3}},
         }) {
      CLAY(CLAY_ID("SettingsBackBtn"),
           {
               .layout =
                   {
                       .sizing = {.width = CLAY_SIZING_FIXED(48),
                                  .height = CLAY_SIZING_FIXED(48)},
                       .childAlignment = {.x = CLAY_ALIGN_X_CENTER,
                                          .y = CLAY_ALIGN_Y_CENTER},
                   },
               .backgroundColor = Clay_Hovered()
                                      ? cal_hoverYellow
                                      : (Clay_Color){0, 0, 0, 0},
               .border = {.color = cal_borderColor, .width = CLAY_BORDER_ALL(2)},
           }) {
        CLAY_TEXT(CLAY_STRING("<"), CLAY_TEXT_CONFIG({
                                        .fontId = fontId,
                                        .fontSize = 24,
                                        .textColor = cal_primaryText,
                                    }));
      }
      CLAY_TEXT(CLAY_STRING("Settings"), CLAY_TEXT_CONFIG({
                                              .fontId = fontId,
                                              .fontSize = 28,
                                              .textColor = cal_primaryText,
                                          }));
    }

    // Content area
    CLAY(CLAY_ID("SettingsContent"),
         {
             .layout =
                 {
                     .sizing = {.width = CLAY_SIZING_GROW(0),
                                .height = CLAY_SIZING_GROW(0)},
                     .layoutDirection = CLAY_TOP_TO_BOTTOM,
                     .padding = {24, 24, 24, 24},
                     .childGap = 16,
                 },
         }) {
      CLAY_TEXT(CLAY_STRING("Nothing here yet."),
                CLAY_TEXT_CONFIG({
                    .fontId = fontId,
                    .fontSize = 20,
                    .textColor = cal_secondaryText,
                }));
    }
  }
}

#endif
