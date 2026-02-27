#ifndef CAL_COMMON_H
#define CAL_COMMON_H

#include "clay.h"
#include "events.h"

#include <string.h>

// ── Color constants (neobrutalist palette) ───────────────────────────────────
static const Clay_Color cal_borderColor   = {30,  30,  30,  255};
static const Clay_Color cal_primaryText   = {20,  20,  20,  255};
static const Clay_Color cal_secondaryText = {60,  60,  60,  255};
static const Clay_Color cal_accentBlue    = {0,   100, 255, 255};
static const Clay_Color cal_todayTint     = {210, 230, 255, 255};
static const Clay_Color cal_redLine       = {255, 50,  50,  255};
static const Clay_Color cal_cream         = {255, 250, 240, 255};
static const Clay_Color cal_hoverYellow   = {255, 230, 0,   255};
static const Clay_Color cal_shadowColor   = {30,  30,  30,  255};

// ── Page routing ─────────────────────────────────────────────────────────────
typedef enum { PAGE_CALENDAR, PAGE_SETTINGS, PAGE_ABOUT } AppPage;
static AppPage g_currentPage = PAGE_CALENDAR;

// ── Static string buffers for Clay text ──────────────────────────────────────
// Clay_String.chars must remain valid for the frame; use static buffers.
#define CAL_MAX_EVT_TITLE_BUFS 256
static char g_evtTitleBuf[CAL_MAX_EVT_TITLE_BUFS][CAL_SUMMARY_LEN];
static int  g_evtTitleBufIdx = 0;

static Clay_String cal_make_string(const char *s) {
  int idx = g_evtTitleBufIdx % CAL_MAX_EVT_TITLE_BUFS;
  strncpy(g_evtTitleBuf[idx], s, CAL_SUMMARY_LEN - 1);
  g_evtTitleBuf[idx][CAL_SUMMARY_LEN - 1] = '\0';
  g_evtTitleBufIdx++;
  return (Clay_String){.length = (int32_t)strlen(g_evtTitleBuf[idx]),
                       .chars  = g_evtTitleBuf[idx]};
}

static Clay_Color Calendar_GetCalendarColor(int calendarIndex) {
  if (calendarIndex >= 0 && calendarIndex < g_calendarCount) {
    LinkedCalendar *cal = &g_calendars[calendarIndex];
    return (Clay_Color){cal->colorR, cal->colorG, cal->colorB, cal->colorA};
  }
  return (Clay_Color){0, 100, 255, 255}; // default blue
}

#endif
