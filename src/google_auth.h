#ifndef GOOGLE_AUTH_H
#define GOOGLE_AUTH_H

#include <stdbool.h>
#include <time.h>

typedef struct {
  char access_token[2048];
  char refresh_token[512];
  time_t expires_at;
} GoogleTokens;

typedef enum {
  AUTH_READY,
  AUTH_AWAITING_CODE,
  AUTH_AUTHENTICATED,
  AUTH_ERROR,
} GoogleAuthState;

#define GOOGLE_AUTH_CODE_MAX 256
#define GOOGLE_AUTH_URL_MAX  1024
#define GOOGLE_AUTH_ERR_MAX  256

extern GoogleTokens    g_googleTokens;
extern GoogleAuthState g_authState;
extern char            g_authErrorMsg[GOOGLE_AUTH_ERR_MAX];
extern char            g_authUrl[GOOGLE_AUTH_URL_MAX];
void GoogleAuth_Init(void);
void GoogleAuth_BuildAuthUrl(void);
void GoogleAuth_BuildAuthUrlWithRedirect(const char *redirect_uri);
bool GoogleAuth_ExchangeCode(const char *code);
bool GoogleAuth_RefreshAccessToken(void);
bool GoogleAuth_EnsureValidToken(void);
void GoogleAuth_Disconnect(void);

#endif
