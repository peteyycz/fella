#include "google_auth.h"
#include "cJSON.h"
#include "config_dir.h"

#include <curl/curl.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "config.h"
// GOOGLE_CLIENT_ID and GOOGLE_CLIENT_SECRET are defined in config.h (generated
// at build time)
static char s_redirectUri[128] = "";
static const char *GOOGLE_SCOPE =
    "https://www.googleapis.com/auth/calendar.readonly";

// ── Global state ─────────────────────────────────────────────────────────────
GoogleTokens g_googleTokens = {0};
GoogleAuthState g_authState = AUTH_READY;
char g_authErrorMsg[GOOGLE_AUTH_ERR_MAX] = {0};
char g_authUrl[GOOGLE_AUTH_URL_MAX] = {0};

// ── libcurl write callback ───────────────────────────────────────────────────
typedef struct {
  char *data;
  size_t size;
} CurlBuffer;

static size_t curl_write_cb(void *ptr, size_t size, size_t nmemb,
                            void *userdata) {
  size_t total = size * nmemb;
  CurlBuffer *buf = (CurlBuffer *)userdata;
  char *tmp = realloc(buf->data, buf->size + total + 1);
  if (!tmp)
    return 0;
  buf->data = tmp;
  memcpy(buf->data + buf->size, ptr, total);
  buf->size += total;
  buf->data[buf->size] = '\0';
  return total;
}

static void get_tokens_path(char *buf, size_t bufsize) {
  char dir[256];
  get_config_dir(dir, sizeof(dir));
  snprintf(buf, bufsize, "%s/tokens.json", dir);
}

// ── Token persistence ────────────────────────────────────────────────────────
static bool load_tokens(void) {
  char path[512];
  get_tokens_path(path, sizeof(path));

  FILE *f = fopen(path, "r");
  if (!f)
    return false;

  fseek(f, 0, SEEK_END);
  long fsize = ftell(f);
  fseek(f, 0, SEEK_SET);
  if (fsize <= 0) {
    fclose(f);
    return false;
  }

  char *buf = malloc(fsize + 1);
  if (!buf) {
    fclose(f);
    return false;
  }
  size_t nread = fread(buf, 1, fsize, f);
  buf[nread] = '\0';
  fclose(f);
  if (nread == 0) {
    free(buf);
    return false;
  }

  cJSON *root = cJSON_Parse(buf);
  free(buf);
  if (!root)
    return false;

  const cJSON *at = cJSON_GetObjectItemCaseSensitive(root, "access_token");
  const cJSON *rt = cJSON_GetObjectItemCaseSensitive(root, "refresh_token");
  const cJSON *ex = cJSON_GetObjectItemCaseSensitive(root, "expires_at");

  if (cJSON_IsString(at) && at->valuestring)
    strncpy(g_googleTokens.access_token, at->valuestring,
            sizeof(g_googleTokens.access_token) - 1);
  if (cJSON_IsString(rt) && rt->valuestring)
    strncpy(g_googleTokens.refresh_token, rt->valuestring,
            sizeof(g_googleTokens.refresh_token) - 1);
  if (cJSON_IsNumber(ex))
    g_googleTokens.expires_at = (time_t)ex->valuedouble;

  cJSON_Delete(root);
  return g_googleTokens.refresh_token[0] != '\0';
}

static void save_tokens(void) {
  ensure_config_dir();

  char path[512];
  get_tokens_path(path, sizeof(path));

  cJSON *root = cJSON_CreateObject();
  cJSON_AddStringToObject(root, "access_token", g_googleTokens.access_token);
  cJSON_AddStringToObject(root, "refresh_token", g_googleTokens.refresh_token);
  cJSON_AddNumberToObject(root, "expires_at",
                          (double)g_googleTokens.expires_at);

  char *json = cJSON_PrintUnformatted(root);
  cJSON_Delete(root);
  if (!json)
    return;

  int fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0600);
  if (fd >= 0) {
    FILE *f = fdopen(fd, "w");
    if (f) {
      fputs(json, f);
      fclose(f);
    } else {
      close(fd);
    }
  }
  free(json);
}

// ── Parse token response JSON ────────────────────────────────────────────────
static bool parse_token_response(const char *json) {
  cJSON *root = cJSON_Parse(json);
  if (!root) {
    snprintf(g_authErrorMsg, GOOGLE_AUTH_ERR_MAX,
             "Failed to parse token response");
    return false;
  }

  const cJSON *err = cJSON_GetObjectItemCaseSensitive(root, "error");
  if (cJSON_IsString(err) && err->valuestring) {
    const cJSON *desc =
        cJSON_GetObjectItemCaseSensitive(root, "error_description");
    snprintf(g_authErrorMsg, GOOGLE_AUTH_ERR_MAX, "%s: %s", err->valuestring,
             (cJSON_IsString(desc) && desc->valuestring) ? desc->valuestring
                                                         : "");
    cJSON_Delete(root);
    return false;
  }

  const cJSON *at = cJSON_GetObjectItemCaseSensitive(root, "access_token");
  if (cJSON_IsString(at) && at->valuestring)
    strncpy(g_googleTokens.access_token, at->valuestring,
            sizeof(g_googleTokens.access_token) - 1);

  const cJSON *rt = cJSON_GetObjectItemCaseSensitive(root, "refresh_token");
  if (cJSON_IsString(rt) && rt->valuestring)
    strncpy(g_googleTokens.refresh_token, rt->valuestring,
            sizeof(g_googleTokens.refresh_token) - 1);

  const cJSON *ei = cJSON_GetObjectItemCaseSensitive(root, "expires_in");
  if (cJSON_IsNumber(ei))
    g_googleTokens.expires_at = time(NULL) + (time_t)ei->valueint;

  cJSON_Delete(root);
  return true;
}

// ── Public API ───────────────────────────────────────────────────────────────

void GoogleAuth_Init(void) {
  memset(&g_googleTokens, 0, sizeof(g_googleTokens));
  g_authState = AUTH_READY;
  g_authErrorMsg[0] = '\0';
  if (load_tokens()) {
    g_authState = AUTH_AUTHENTICATED;
  }
}

void GoogleAuth_BuildAuthUrlWithRedirect(const char *redirect_uri) {
  strncpy(s_redirectUri, redirect_uri, sizeof(s_redirectUri) - 1);
  s_redirectUri[sizeof(s_redirectUri) - 1] = '\0';

  snprintf(g_authUrl, GOOGLE_AUTH_URL_MAX,
           "https://accounts.google.com/o/oauth2/v2/auth"
           "?client_id=%s"
           "&redirect_uri=%s"
           "&response_type=code"
           "&scope=%s"
           "&access_type=offline"
           "&prompt=consent",
           GOOGLE_CLIENT_ID, s_redirectUri, GOOGLE_SCOPE);
}

void GoogleAuth_BuildAuthUrl(void) {
  GoogleAuth_BuildAuthUrlWithRedirect(s_redirectUri);
}

bool GoogleAuth_ExchangeCode(const char *code) {
  CURL *curl = curl_easy_init();
  if (!curl) {
    snprintf(g_authErrorMsg, GOOGLE_AUTH_ERR_MAX, "Failed to initialize curl");
    g_authState = AUTH_ERROR;
    return false;
  }

  char postfields[2048];
  snprintf(postfields, sizeof(postfields),
           "code=%s"
           "&client_id=%s"
           "&client_secret=%s"
           "&redirect_uri=%s"
           "&grant_type=authorization_code",
           code, GOOGLE_CLIENT_ID, GOOGLE_CLIENT_SECRET, s_redirectUri);

  CurlBuffer response = {0};

  curl_easy_setopt(curl, CURLOPT_URL, "https://oauth2.googleapis.com/token");
  curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postfields);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_write_cb);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

  CURLcode res = curl_easy_perform(curl);
  curl_easy_cleanup(curl);

  if (res != CURLE_OK) {
    snprintf(g_authErrorMsg, GOOGLE_AUTH_ERR_MAX, "HTTP error: %s",
             curl_easy_strerror(res));
    free(response.data);
    g_authState = AUTH_ERROR;
    return false;
  }

  bool ok = parse_token_response(response.data);
  free(response.data);

  if (ok) {
    save_tokens();
    g_authState = AUTH_AUTHENTICATED;
  } else {
    g_authState = AUTH_ERROR;
  }
  return ok;
}

bool GoogleAuth_RefreshAccessToken(void) {
  if (g_googleTokens.refresh_token[0] == '\0') {
    g_authState = AUTH_READY;
    return false;
  }

  CURL *curl = curl_easy_init();
  if (!curl)
    return false;

  char postfields[2048];
  snprintf(postfields, sizeof(postfields),
           "refresh_token=%s"
           "&client_id=%s"
           "&client_secret=%s"
           "&grant_type=refresh_token",
           g_googleTokens.refresh_token, GOOGLE_CLIENT_ID,
           GOOGLE_CLIENT_SECRET);

  CurlBuffer response = {0};

  curl_easy_setopt(curl, CURLOPT_URL, "https://oauth2.googleapis.com/token");
  curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postfields);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_write_cb);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

  CURLcode res = curl_easy_perform(curl);
  curl_easy_cleanup(curl);

  if (res != CURLE_OK) {
    free(response.data);
    return false;
  }

  bool ok = parse_token_response(response.data);
  free(response.data);

  if (ok) {
    save_tokens();
  }
  return ok;
}

bool GoogleAuth_EnsureValidToken(void) {
  if (g_authState != AUTH_AUTHENTICATED)
    return false;
  if (g_googleTokens.access_token[0] == '\0')
    return GoogleAuth_RefreshAccessToken();
  if (time(NULL) >= g_googleTokens.expires_at - 60)
    return GoogleAuth_RefreshAccessToken();
  return true;
}

void GoogleAuth_Disconnect(void) {
  memset(&g_googleTokens, 0, sizeof(g_googleTokens));
  g_authState = AUTH_READY;
  g_authErrorMsg[0] = '\0';

  char path[512];
  get_tokens_path(path, sizeof(path));
  remove(path);
}
