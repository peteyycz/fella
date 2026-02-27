#include "google_calendar.h"
#include "google_auth.h"
#include "events.h"

#include <curl/curl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// ── libcurl write callback ───────────────────────────────────────────────────
typedef struct {
  char *data;
  size_t size;
} CurlBuffer;

static size_t curl_write_cb(void *ptr, size_t size, size_t nmemb, void *userdata) {
  size_t total = size * nmemb;
  CurlBuffer *buf = (CurlBuffer *)userdata;
  char *tmp = realloc(buf->data, buf->size + total + 1);
  if (!tmp) return 0;
  buf->data = tmp;
  memcpy(buf->data + buf->size, ptr, total);
  buf->size += total;
  buf->data[buf->size] = '\0';
  return total;
}

// Compute Monday 00:00 UTC and next Monday 00:00 UTC for the current week
static void get_week_bounds(char *timeMin, size_t minSize, char *timeMax, size_t maxSize) {
  time_t now = time(NULL);
  struct tm lt = *localtime(&now);

  // Rewind to Monday
  int days_since_monday = (lt.tm_wday + 6) % 7;
  lt.tm_mday -= days_since_monday;
  lt.tm_hour = 0;
  lt.tm_min = 0;
  lt.tm_sec = 0;
  mktime(&lt);

  // Monday start in RFC3339 UTC
  struct tm utc_start = lt;
  // Convert local Monday 00:00 to UTC by using gmtime of the mktime result
  time_t monday_local = mktime(&lt);
  struct tm monday_utc = *gmtime(&monday_local);
  strftime(timeMin, minSize, "%Y-%m-%dT%H:%M:%SZ", &monday_utc);

  // Next Monday
  lt.tm_mday += 7;
  time_t next_monday_local = mktime(&lt);
  struct tm next_monday_utc = *gmtime(&next_monday_local);
  strftime(timeMax, maxSize, "%Y-%m-%dT%H:%M:%SZ", &next_monday_utc);

  (void)utc_start;
}

void GoogleCalendar_FetchEvents(const char *calendarId, int calIndex) {
  if (!GoogleAuth_EnsureValidToken()) return;

  CURL *curl = curl_easy_init();
  if (!curl) return;

  char timeMin[64], timeMax[64];
  get_week_bounds(timeMin, sizeof(timeMin), timeMax, sizeof(timeMax));
  fprintf(stderr, "Fetching events: %s to %s\n", timeMin, timeMax);

  // URL-encode the calendar ID (handles @ in email addresses)
  char *escapedId = curl_easy_escape(curl, calendarId, 0);
  if (!escapedId) {
    curl_easy_cleanup(curl);
    return;
  }

  char url[1024];
  snprintf(url, sizeof(url),
    "https://www.googleapis.com/calendar/v3/calendars/%s/events"
    "?timeMin=%s&timeMax=%s&singleEvents=true&orderBy=startTime&maxResults=250",
    escapedId, timeMin, timeMax);
  curl_free(escapedId);

  // Authorization header
  char authHeader[2200];
  snprintf(authHeader, sizeof(authHeader), "Authorization: Bearer %s", g_googleTokens.access_token);
  struct curl_slist *headers = NULL;
  headers = curl_slist_append(headers, authHeader);

  CurlBuffer response = {0};

  curl_easy_setopt(curl, CURLOPT_URL, url);
  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_write_cb);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

  CURLcode res = curl_easy_perform(curl);

  long http_code = 0;
  curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);

  curl_slist_free_all(headers);
  curl_easy_cleanup(curl);

  if (res != CURLE_OK) {
    fprintf(stderr, "Google Calendar fetch failed: %s\n", curl_easy_strerror(res));
  } else {
    fprintf(stderr, "Google Calendar HTTP %ld, %zu bytes\n", http_code, response.size);
    if (http_code == 200 && response.data) {
      load_events_from_json(response.data, calIndex);
    } else if (response.data) {
      fprintf(stderr, "Google Calendar error: %.500s\n", response.data);
    }
  }

  free(response.data);
}
