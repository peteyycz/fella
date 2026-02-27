#include "oauth_server.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>

volatile sig_atomic_t g_oauthServerStatus = OAUTH_SERVER_IDLE;
char g_oauthReceivedCode[256] = {0};
int g_oauthServerPort = 0;

static int s_listenFd = -1;
static pthread_t s_thread;
static volatile sig_atomic_t s_stopFlag = 0;

static const char *RESPONSE_SUCCESS =
    "HTTP/1.1 200 OK\r\n"
    "Content-Type: text/html\r\n"
    "Connection: close\r\n"
    "\r\n"
    "<!DOCTYPE html><html><body style=\"font-family:sans-serif;"
    "display:flex;justify-content:center;align-items:center;height:100vh;"
    "margin:0;background:#f5ebd8\">"
    "<h1>Authorized! You can close this tab.</h1>"
    "</body></html>";

static const char *RESPONSE_BAD_REQUEST = "HTTP/1.1 400 Bad Request\r\n"
                                          "Content-Type: text/plain\r\n"
                                          "Connection: close\r\n"
                                          "\r\n"
                                          "Missing code parameter\n";

static bool extract_code(const char *request, char *code, size_t codesize) {
  const char *qmark = strchr(request, '?');
  if (!qmark)
    return false;

  const char *end = strchr(qmark, ' ');
  if (!end)
    end = qmark + strlen(qmark);

  const char *p = qmark + 1;
  while (p < end) {
    if (strncmp(p, "code=", 5) == 0) {
      p += 5;
      size_t i = 0;
      while (p < end && *p != '&' && i < codesize - 1) {
        code[i++] = *p++;
      }
      code[i] = '\0';
      return i > 0;
    }
    // Skip to next parameter
    while (p < end && *p != '&')
      p++;
    if (p < end)
      p++; // skip '&'
  }
  return false;
}

static void *server_thread(void *arg) {
  (void)arg;

  while (!s_stopFlag) {
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(s_listenFd, &readfds);

    struct timeval tv = {.tv_sec = 1, .tv_usec = 0};
    int ret = select(s_listenFd + 1, &readfds, NULL, NULL, &tv);

    if (ret <= 0)
      continue;

    int clientFd = accept(s_listenFd, NULL, NULL);
    if (clientFd < 0)
      continue;

    char buf[4096];
    ssize_t n = read(clientFd, buf, sizeof(buf) - 1);
    if (n <= 0) {
      close(clientFd);
      continue;
    }
    buf[n] = '\0';

    char code[256];
    if (extract_code(buf, code, sizeof(code))) {
      if (write(clientFd, RESPONSE_SUCCESS, strlen(RESPONSE_SUCCESS)) < 0) {
        // Best-effort response to browser; nothing to do on failure
      }
      close(clientFd);

      strncpy(g_oauthReceivedCode, code, sizeof(g_oauthReceivedCode) - 1);
      g_oauthReceivedCode[sizeof(g_oauthReceivedCode) - 1] = '\0';
      g_oauthServerStatus = OAUTH_SERVER_RECEIVED;
      break;
    } else {
      if (write(clientFd, RESPONSE_BAD_REQUEST, strlen(RESPONSE_BAD_REQUEST)) <
          0) {
        // Best-effort; ignore
      }
      close(clientFd);
    }
  }

  return NULL;
}

bool OAuthServer_Start(void) {
  s_stopFlag = 0;
  g_oauthReceivedCode[0] = '\0';
  g_oauthServerStatus = OAUTH_SERVER_IDLE;

  s_listenFd = socket(AF_INET, SOCK_STREAM, 0);
  if (s_listenFd < 0) {
    g_oauthServerStatus = OAUTH_SERVER_ERROR;
    return false;
  }

  int opt = 1;
  setsockopt(s_listenFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

  struct sockaddr_in addr = {
      .sin_family = AF_INET,
      .sin_addr.s_addr = htonl(INADDR_LOOPBACK),
      .sin_port = 0, // OS-assigned port
  };

  if (bind(s_listenFd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
    close(s_listenFd);
    s_listenFd = -1;
    g_oauthServerStatus = OAUTH_SERVER_ERROR;
    return false;
  }

  if (listen(s_listenFd, 1) < 0) {
    close(s_listenFd);
    s_listenFd = -1;
    g_oauthServerStatus = OAUTH_SERVER_ERROR;
    return false;
  }

  // Get the assigned port
  socklen_t addrlen = sizeof(addr);
  getsockname(s_listenFd, (struct sockaddr *)&addr, &addrlen);
  g_oauthServerPort = ntohs(addr.sin_port);

  if (pthread_create(&s_thread, NULL, server_thread, NULL) != 0) {
    close(s_listenFd);
    s_listenFd = -1;
    g_oauthServerStatus = OAUTH_SERVER_ERROR;
    return false;
  }

  g_oauthServerStatus = OAUTH_SERVER_LISTENING;
  return true;
}

void OAuthServer_Stop(void) {
  s_stopFlag = 1;

  if (s_listenFd >= 0) {
    close(s_listenFd);
    s_listenFd = -1;
  }

  if (g_oauthServerStatus != OAUTH_SERVER_IDLE) {
    pthread_join(s_thread, NULL);
  }

  g_oauthServerStatus = OAUTH_SERVER_IDLE;
  g_oauthReceivedCode[0] = '\0';
  g_oauthServerPort = 0;
}

void OAuthServer_GetRedirectUri(char *buf, size_t bufsize) {
  snprintf(buf, bufsize, "http://127.0.0.1:%d", g_oauthServerPort);
}
