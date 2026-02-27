#ifndef OAUTH_SERVER_H
#define OAUTH_SERVER_H

#include <signal.h>
#include <stdbool.h>
#include <stddef.h>

typedef enum {
  OAUTH_SERVER_IDLE,
  OAUTH_SERVER_LISTENING,
  OAUTH_SERVER_RECEIVED,
  OAUTH_SERVER_ERROR,
} OAuthServerStatus;

extern volatile sig_atomic_t g_oauthServerStatus;
extern char g_oauthReceivedCode[256];
extern int g_oauthServerPort;

bool OAuthServer_Start(void);
void OAuthServer_Stop(void);
void OAuthServer_GetRedirectUri(char *buf, size_t bufsize);

#endif
