#ifndef CAL_COMMON_H
#define CAL_COMMON_H

#include "clay.h"
#include "theme.h"
#include "events.h"

#include <stdio.h>
#include <string.h>
#include <time.h>

// ── Theme-aware color aliases (drop-in replacements for old constants) ───────
#define cal_borderColor   g_theme.highlightHigh
#define cal_primaryText   g_theme.text
#define cal_secondaryText g_theme.subtle
#define cal_accentBlue    g_theme.rose
#define cal_todayTint     g_theme.highlightLow
#define cal_redLine       g_theme.love
#define cal_cream         g_theme.base
#define cal_hoverBg       g_theme.surface
#define cal_cardBg        g_theme.surface

#define cal_hover_darken(base, amount) cal_hover_adjust((base), (amount))

// ── Page routing ─────────────────────────────────────────────────────────────
typedef enum { PAGE_CALENDAR, PAGE_SETTINGS, PAGE_ABOUT } AppPage;
static AppPage g_currentPage = PAGE_CALENDAR;

// ── Static string buffers for Clay text ──────────────────────────────────────
// Clay_String.chars must remain valid for the frame; use static buffers.
#define CAL_MAX_EVT_TITLE_BUFS 256
#define CAL_STR_BUF_LEN CAL_DESC_LEN
static char g_evtTitleBuf[CAL_MAX_EVT_TITLE_BUFS][CAL_STR_BUF_LEN];
static int  g_evtTitleBufIdx = 0;

static Clay_String cal_make_string(const char *s) {
  int idx = g_evtTitleBufIdx % CAL_MAX_EVT_TITLE_BUFS;
  strncpy(g_evtTitleBuf[idx], s, CAL_STR_BUF_LEN - 1);
  g_evtTitleBuf[idx][CAL_STR_BUF_LEN - 1] = '\0';
  g_evtTitleBufIdx++;
  return (Clay_String){.length = (int32_t)strlen(g_evtTitleBuf[idx]),
                       .chars  = g_evtTitleBuf[idx]};
}

// Format an event's time range into buf. Returns buf.
static const char *cal_format_event_time(const CalEvent *ev, char *buf,
                                         size_t buflen) {
  if (ev->allDay) {
    snprintf(buf, buflen, "%04d-%02d-%02d (All day)", ev->startYear,
             ev->startMon, ev->startMday);
  } else {
    struct tm st = *localtime(&ev->startTime);
    struct tm et = *localtime(&ev->endTime);
    snprintf(buf, buflen, "%02d:%02d - %02d:%02d", st.tm_hour, st.tm_min,
             et.tm_hour, et.tm_min);
  }
  return buf;
}

static Clay_Color Calendar_GetCalendarColor(int calendarIndex) {
  if (calendarIndex >= 0 && calendarIndex < g_calendarCount) {
    LinkedCalendar *cal = &g_calendars[calendarIndex];
    return (Clay_Color){cal->colorR, cal->colorG, cal->colorB, cal->colorA};
  }
  return g_theme.pine; // default blue
}

#endif
