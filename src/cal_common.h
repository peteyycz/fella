#ifndef CAL_COMMON_H
#define CAL_COMMON_H

#include "clay.h"
#include "events.h"

#include <string.h>

// ── Color constants ──────────────────────────────────────────────────────────
static const Clay_Color cal_borderGray    = {218, 220, 224, 255};
static const Clay_Color cal_primaryText   = {60,  64,  67,  255};
static const Clay_Color cal_secondaryText = {112, 117, 122, 255};
static const Clay_Color cal_todayBlue     = {26,  115, 232, 255};
static const Clay_Color cal_todayTint     = {232, 240, 254, 255};
static const Clay_Color cal_redLine       = {234, 67,  53,  255};
static const Clay_Color cal_white         = {255, 255, 255, 255};

// ── Static string buffers for Clay text ──────────────────────────────────────
// Clay_String.chars must remain valid for the frame; use static buffers.
#define CAL_MAX_EVT_TITLE_BUFS 64
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
  return (Clay_Color){26, 115, 232, 255}; // default blue
}

#endif
