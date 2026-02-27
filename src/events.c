#include "events.h"
#include "cJSON.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

CalEvent g_events[CAL_MAX_EVENTS];
int g_eventCount = 0;
bool g_eventsLoaded = false;

LinkedCalendar g_calendars[CAL_MAX_CALENDARS];
int g_calendarCount = 0;

// Parse "2026-02-27T09:00:00-05:00" -> time_t UTC
// or    "2026-02-27T09:00:00Z"      -> time_t UTC
static time_t parse_datetime(const char *s) {
  struct tm t = {0};
  int tzOffsetMinutes = 0;
  // "YYYY-MM-DDTHH:MM:SS"
  sscanf(s, "%d-%d-%dT%d:%d:%d", &t.tm_year, &t.tm_mon, &t.tm_mday, &t.tm_hour,
         &t.tm_min, &t.tm_sec);
  t.tm_year -= 1900;
  t.tm_mon -= 1;
  t.tm_isdst = -1;

  // Find timezone part after the seconds
  const char *tz = s + 19; // past "YYYY-MM-DDTHH:MM:SS"
  if (*tz == '.') {
    while (*tz && *tz != '+' && *tz != '-' && *tz != 'Z')
      tz++;
  }
  if (*tz == 'Z') {
    tzOffsetMinutes = 0;
  } else if (*tz == '+' || *tz == '-') {
    int sign = (*tz == '+') ? 1 : -1;
    int tzh = 0, tzm = 0;
    sscanf(tz + 1, "%d:%d", &tzh, &tzm);
    tzOffsetMinutes = sign * (tzh * 60 + tzm);
  }

  // timegm equivalent: interpret t as UTC then subtract tz offset
  // Set TZ to UTC temporarily.
  char *old_tz = getenv("TZ");
  char saved_tz[256] = "";
  if (old_tz)
    strncpy(saved_tz, old_tz, sizeof(saved_tz) - 1);
  setenv("TZ", "UTC", 1);
  tzset();
  time_t utc = mktime(&t);
  if (old_tz)
    setenv("TZ", saved_tz, 1);
  else
    unsetenv("TZ");
  tzset();

  // Subtract tz offset (offset means "local = UTC + offset")
  utc -= tzOffsetMinutes * 60;
  return utc;
}

// Parse "2026-02-28" -> year/mon/mday (1-indexed month)
static void parse_date(const char *s, int *year, int *mon, int *mday) {
  sscanf(s, "%d-%d-%d", year, mon, mday);
}

void Calendar_InitCalendars(void) {
  g_calendarCount = 0;

  // Private calendar — hot pink
  LinkedCalendar *priv = &g_calendars[g_calendarCount++];
  strncpy(priv->name, "Private", CAL_NAME_LEN - 1);
  strncpy(priv->filePath, "resources/private-entries.json", CAL_PATH_LEN - 1);
  priv->colorR = 255; priv->colorG = 60;  priv->colorB = 120; priv->colorA = 255;
  priv->visible = true;

  // Work calendar — electric blue
  LinkedCalendar *work = &g_calendars[g_calendarCount++];
  strncpy(work->name, "Work", CAL_NAME_LEN - 1);
  strncpy(work->filePath, "resources/work-entries.json", CAL_PATH_LEN - 1);
  work->colorR = 50;  work->colorG = 120; work->colorB = 255; work->colorA = 255;
  work->visible = true;
}

static void load_events_from_file(const char *path, int calIndex) {
  FILE *f = fopen(path, "r");
  if (!f) {
    fprintf(stderr, "Could not open %s\n", path);
    return;
  }

  fseek(f, 0, SEEK_END);
  long fsize = ftell(f);
  fseek(f, 0, SEEK_SET);
  char *buf = (char *)malloc(fsize + 1);
  if (!buf) {
    fclose(f);
    return;
  }
  (void)fread(buf, 1, fsize, f);
  buf[fsize] = '\0';
  fclose(f);

  cJSON *root = cJSON_Parse(buf);
  free(buf);
  if (!root)
    return;

  const cJSON *items = cJSON_GetObjectItemCaseSensitive(root, "items");
  if (!cJSON_IsArray(items)) {
    cJSON_Delete(root);
    return;
  }

  const cJSON *item = NULL;
  cJSON_ArrayForEach(item, items) {
    if (g_eventCount >= CAL_MAX_EVENTS)
      break;

    CalEvent ev = {0};
    ev.calendarIndex = calIndex;

    const cJSON *summary = cJSON_GetObjectItemCaseSensitive(item, "summary");
    if (cJSON_IsString(summary) && summary->valuestring) {
      strncpy(ev.summary, summary->valuestring, CAL_SUMMARY_LEN - 1);
    }

    const cJSON *desc = cJSON_GetObjectItemCaseSensitive(item, "description");
    if (cJSON_IsString(desc) && desc->valuestring) {
      strncpy(ev.description, desc->valuestring, CAL_DESC_LEN - 1);
    }

    const cJSON *loc = cJSON_GetObjectItemCaseSensitive(item, "location");
    if (cJSON_IsString(loc) && loc->valuestring) {
      strncpy(ev.location, loc->valuestring, CAL_LOC_LEN - 1);
    }

    const cJSON *colorId = cJSON_GetObjectItemCaseSensitive(item, "colorId");
    if (cJSON_IsString(colorId) && colorId->valuestring) {
      ev.colorId = atoi(colorId->valuestring);
    } else if (cJSON_IsNumber(colorId)) {
      ev.colorId = colorId->valueint;
    }

    const cJSON *start = cJSON_GetObjectItemCaseSensitive(item, "start");
    if (cJSON_IsObject(start)) {
      const cJSON *dt = cJSON_GetObjectItemCaseSensitive(start, "dateTime");
      if (cJSON_IsString(dt) && dt->valuestring) {
        ev.startTime = parse_datetime(dt->valuestring);
        ev.allDay = false;
      } else {
        const cJSON *d = cJSON_GetObjectItemCaseSensitive(start, "date");
        if (cJSON_IsString(d) && d->valuestring) {
          parse_date(d->valuestring, &ev.startYear, &ev.startMon,
                     &ev.startMday);
          ev.allDay = true;
        }
      }
    }

    const cJSON *end = cJSON_GetObjectItemCaseSensitive(item, "end");
    if (cJSON_IsObject(end)) {
      if (!ev.allDay) {
        const cJSON *dt = cJSON_GetObjectItemCaseSensitive(end, "dateTime");
        if (cJSON_IsString(dt) && dt->valuestring) {
          ev.endTime = parse_datetime(dt->valuestring);
        }
      } else {
        const cJSON *d = cJSON_GetObjectItemCaseSensitive(end, "date");
        if (cJSON_IsString(d) && d->valuestring) {
          parse_date(d->valuestring, &ev.endYear, &ev.endMon, &ev.endMday);
        }
      }
    }

    g_events[g_eventCount++] = ev;
  }

  cJSON_Delete(root);
}

void Calendar_LoadEvents(void) {
  if (g_eventsLoaded)
    return;
  g_eventsLoaded = true;
  g_eventCount = 0;

  if (g_calendarCount == 0)
    Calendar_InitCalendars();

  for (int i = 0; i < g_calendarCount; i++) {
    load_events_from_file(g_calendars[i].filePath, i);
  }
}
