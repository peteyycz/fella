#ifndef THEME_H
#define THEME_H

#include "clay.h"
#include <stdbool.h>

typedef struct {
  Clay_Color base;
  Clay_Color surface;
  Clay_Color overlay;
  Clay_Color text;
  Clay_Color subtle;
  Clay_Color muted;
  Clay_Color love;
  Clay_Color gold;
  Clay_Color rose;
  Clay_Color pine;
  Clay_Color foam;
  Clay_Color iris;
  Clay_Color highlightLow;
  Clay_Color highlightMed;
  Clay_Color highlightHigh;
} CalTheme;

// Rose Pine Moon (dark)
#define THEME_MOON                                                             \
  (CalTheme) {                                                                 \
    .base = {35, 33, 54, 255}, .surface = {42, 39, 63, 255},                   \
    .overlay = {57, 53, 82, 255}, .text = {224, 222, 244, 255},                \
    .subtle = {144, 140, 170, 255}, .muted = {110, 106, 134, 255},             \
    .love = {235, 111, 146, 255}, .gold = {246, 193, 119, 255},                \
    .rose = {234, 154, 151, 255}, .pine = {62, 143, 176, 255},                 \
    .foam = {156, 207, 216, 255}, .iris = {196, 167, 231, 255},                \
    .highlightLow = {42, 40, 62, 255}, .highlightMed = {68, 65, 90, 255},      \
    .highlightHigh = {86, 82, 110, 255},                                       \
  }

// Rose Pine Dawn (light)
#define THEME_DAWN                                                             \
  (CalTheme) {                                                                 \
    .base = {250, 244, 237, 255}, .surface = {255, 250, 243, 255},             \
    .overlay = {242, 233, 225, 255}, .text = {87, 82, 121, 255},               \
    .subtle = {121, 117, 147, 255}, .muted = {152, 147, 165, 255},             \
    .love = {180, 99, 122, 255}, .gold = {234, 157, 52, 255},                  \
    .rose = {215, 130, 126, 255}, .pine = {40, 105, 131, 255},                 \
    .foam = {86, 148, 159, 255}, .iris = {144, 122, 169, 255},                 \
    .highlightLow = {244, 237, 232, 255},                                      \
    .highlightMed = {223, 218, 217, 255},                                      \
    .highlightHigh = {206, 202, 205, 255},                                     \
  }

static CalTheme g_theme = THEME_MOON;
static bool g_themeDark = true;

static Clay_Color cal_hover_adjust(Clay_Color base, int amount) {
  if (g_themeDark) {
    // Lighten in dark mode
    return (Clay_Color){(base.r + amount > 255) ? 255 : base.r + amount,
                        (base.g + amount > 255) ? 255 : base.g + amount,
                        (base.b + amount > 255) ? 255 : base.b + amount,
                        base.a};
  } else {
    // Darken in light mode
    return (Clay_Color){base.r > amount ? base.r - amount : 0,
                        base.g > amount ? base.g - amount : 0,
                        base.b > amount ? base.b - amount : 0, base.a};
  }
}

static void Theme_Set(bool dark) {
  g_themeDark = dark;
  g_theme = dark ? THEME_MOON : THEME_DAWN;
}

#endif
