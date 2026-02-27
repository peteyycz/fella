#ifndef EVENTS_H
#define EVENTS_H

#include <stdbool.h>
#include <stdint.h>
#include <time.h>

#define CAL_MAX_EVENTS 64
#define CAL_SUMMARY_LEN 64
#define CAL_DESC_LEN   256
#define CAL_LOC_LEN    128

#define CAL_MAX_CALENDARS 8
#define CAL_NAME_LEN      32
#define CAL_PATH_LEN     128

typedef struct {
  char    name[CAL_NAME_LEN];
  char    filePath[CAL_PATH_LEN];
  uint8_t colorR, colorG, colorB, colorA;
  bool    visible;
} LinkedCalendar;

typedef struct {
  char summary[CAL_SUMMARY_LEN];
  char description[CAL_DESC_LEN];
  char location[CAL_LOC_LEN];
  bool allDay;
  // For timed events: UTC epoch
  time_t startTime;
  time_t endTime;
  // For all-day events: year/mon/mday (1-indexed month)
  int startYear, startMon, startMday;
  int endYear,   endMon,   endMday;
  int colorId;  // 0 = default blue
  int calendarIndex;
} CalEvent;

extern CalEvent g_events[CAL_MAX_EVENTS];
extern int      g_eventCount;
extern bool     g_eventsLoaded;

extern LinkedCalendar g_calendars[CAL_MAX_CALENDARS];
extern int            g_calendarCount;

void Calendar_InitCalendars(void);
void Calendar_LoadEvents(void);

#endif
