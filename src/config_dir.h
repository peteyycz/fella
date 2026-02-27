#ifndef CONFIG_DIR_H
#define CONFIG_DIR_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

static void get_config_dir(char *buf, size_t bufsize) {
  const char *xdg = getenv("XDG_CONFIG_HOME");
  if (xdg && xdg[0]) {
    snprintf(buf, bufsize, "%s/fella", xdg);
  } else {
    const char *home = getenv("HOME");
    if (!home)
      home = "/tmp";
    snprintf(buf, bufsize, "%s/.config/fella", home);
  }
}

static void ensure_config_dir(void) {
  char dir[256];
  get_config_dir(dir, sizeof(dir));

  char parent[256];
  const char *xdg = getenv("XDG_CONFIG_HOME");
  if (xdg && xdg[0]) {
    strncpy(parent, xdg, sizeof(parent) - 1);
    parent[sizeof(parent) - 1] = '\0';
  } else {
    const char *home = getenv("HOME");
    if (!home)
      home = "/tmp";
    snprintf(parent, sizeof(parent), "%s/.config", home);
  }
  mkdir(parent, 0755);
  mkdir(dir, 0755);
}

#endif
