#ifndef EVENTS_H
#define EVENTS_H

#include <stdbool.h>
#include <time.h>

#define CAL_MAX_EVENTS 64
#define CAL_SUMMARY_LEN 64

typedef struct {
  char summary[CAL_SUMMARY_LEN];
  bool allDay;
  // For timed events: UTC epoch
  time_t startTime;
  time_t endTime;
  // For all-day events: year/mon/mday (1-indexed month)
  int startYear, startMon, startMday;
  int endYear,   endMon,   endMday;
  int colorId;  // 0 = default blue
} CalEvent;

extern CalEvent g_events[CAL_MAX_EVENTS];
extern int      g_eventCount;
extern bool     g_eventsLoaded;

void Calendar_LoadEvents(void);

#endif
