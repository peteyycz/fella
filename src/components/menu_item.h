#ifndef COMPONENT_MENU_ITEM_H
#define COMPONENT_MENU_ITEM_H

#include "cal_common.h"

static void MenuItem(int index, const char *text, uint32_t fontId) {
  CLAY(CLAY_IDI("MenuItem", index),
       {
           .layout =
               {
                   .sizing = {.width  = CLAY_SIZING_GROW(0),
                              .height = CLAY_SIZING_FIXED(44)},
                   .childAlignment = {.y = CLAY_ALIGN_Y_CENTER},
                   .padding = {16, 16, 0, 0},
               },
           .backgroundColor = Clay_Hovered()
                                  ? (Clay_Color){232, 240, 254, 255}
                                  : (Clay_Color){0, 0, 0, 0},

       }) {
    Clay_String itemText = {
        .length = (int32_t)strlen(text),
        .chars  = text,
    };
    CLAY_TEXT(itemText, CLAY_TEXT_CONFIG({
                            .fontId    = fontId,
                            .fontSize  = 14,
                            .textColor = cal_primaryText,
                        }));
  }
}

#endif
