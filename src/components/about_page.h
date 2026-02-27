#ifndef COMPONENT_ABOUT_PAGE_H
#define COMPONENT_ABOUT_PAGE_H

#include "cal_common.h"

static void AboutPage_Render(uint32_t fontId) {
  CLAY(CLAY_ID("AboutPage"),
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
    CLAY(CLAY_ID("AboutTopBar"),
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
      CLAY(CLAY_ID("AboutBackBtn"),
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
      CLAY_TEXT(CLAY_STRING("About"), CLAY_TEXT_CONFIG({
                                            .fontId = fontId,
                                            .fontSize = 28,
                                            .textColor = cal_primaryText,
                                        }));
    }

    // Content area
    CLAY(CLAY_ID("AboutContent"),
         {
             .layout =
                 {
                     .sizing = {.width = CLAY_SIZING_GROW(0),
                                .height = CLAY_SIZING_GROW(0)},
                     .layoutDirection = CLAY_TOP_TO_BOTTOM,
                     .padding = {24, 24, 24, 24},
                     .childGap = 12,
                 },
         }) {
      CLAY_TEXT(CLAY_STRING("Fella Calendar"),
                CLAY_TEXT_CONFIG({
                    .fontId = fontId,
                    .fontSize = 24,
                    .textColor = cal_primaryText,
                }));
      CLAY_TEXT(
          CLAY_STRING("A neobrutalist calendar app built with Clay and Raylib."),
          CLAY_TEXT_CONFIG({
              .fontId = fontId,
              .fontSize = 18,
              .textColor = cal_secondaryText,
              .wrapMode = CLAY_TEXT_WRAP_WORDS,
          }));
    }
  }
}

#endif
