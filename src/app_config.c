#include "app_config.h"
#include "cJSON.h"
#include "config_dir.h"
#include "theme.h"

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static void get_config_path(char *buf, size_t bufsize) {
  char dir[256];
  get_config_dir(dir, sizeof(dir));
  snprintf(buf, bufsize, "%s/config.json", dir);
}

void AppConfig_Load(void) {
  char path[512];
  get_config_path(path, sizeof(path));

  FILE *f = fopen(path, "r");
  if (!f)
    return;

  fseek(f, 0, SEEK_END);
  long fsize = ftell(f);
  fseek(f, 0, SEEK_SET);
  if (fsize <= 0) {
    fclose(f);
    return;
  }

  char *buf = malloc(fsize + 1);
  if (!buf) {
    fclose(f);
    return;
  }
  size_t nread = fread(buf, 1, fsize, f);
  buf[nread] = '\0';
  fclose(f);
  if (nread == 0) {
    free(buf);
    return;
  }

  cJSON *root = cJSON_Parse(buf);
  free(buf);
  if (!root)
    return;

  const cJSON *theme = cJSON_GetObjectItemCaseSensitive(root, "theme");
  if (cJSON_IsString(theme) && theme->valuestring) {
    if (strcmp(theme->valuestring, "dawn") == 0) {
      Theme_Set(false);
    } else {
      Theme_Set(true);
    }
  }

  cJSON_Delete(root);
}

void AppConfig_Save(void) {
  ensure_config_dir();

  char path[512];
  get_config_path(path, sizeof(path));

  cJSON *root = cJSON_CreateObject();
  cJSON_AddStringToObject(root, "theme", g_themeDark ? "moon" : "dawn");

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
