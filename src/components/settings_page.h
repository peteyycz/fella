#ifndef COMPONENT_SETTINGS_PAGE_H
#define COMPONENT_SETTINGS_PAGE_H

#include "cal_common.h"
#include "google_auth.h"
#include "oauth_server.h"
#include "raylib.h"

#include <stdio.h>

static void SettingsPage_Render(uint32_t fontId) {
  // Poll OAuth server status
  if (g_authState == AUTH_AWAITING_CODE &&
      g_oauthServerStatus == OAUTH_SERVER_RECEIVED) {
    if (GoogleAuth_ExchangeCode(g_oauthReceivedCode)) {
      Calendar_ReloadEvents();
    }
    OAuthServer_Stop();
  } else if (g_authState == AUTH_AWAITING_CODE &&
             g_oauthServerStatus == OAUTH_SERVER_ERROR) {
    snprintf(g_authErrorMsg, GOOGLE_AUTH_ERR_MAX,
             "Local OAuth server failed to start");
    g_authState = AUTH_ERROR;
    OAuthServer_Stop();
  }

  // Handle button clicks
  if (IsMouseButtonPressed(0)) {
    if (g_authState == AUTH_READY &&
        Clay_PointerOver(Clay_GetElementId(CLAY_STRING("GoogleConnectBtn")))) {
      if (OAuthServer_Start()) {
        char redirectUri[128];
        OAuthServer_GetRedirectUri(redirectUri, sizeof(redirectUri));
        GoogleAuth_BuildAuthUrlWithRedirect(redirectUri);
        OpenURL(g_authUrl);
        g_authState = AUTH_AWAITING_CODE;
      }
    }
    if (g_authState == AUTH_AUTHENTICATED &&
        Clay_PointerOver(
            Clay_GetElementId(CLAY_STRING("GoogleDisconnectBtn")))) {
      GoogleAuth_Disconnect();
      Calendar_ReloadEvents();
    }
    if (g_authState == AUTH_AUTHENTICATED &&
        Clay_PointerOver(Clay_GetElementId(CLAY_STRING("GoogleRefreshBtn")))) {
      Calendar_ReloadEvents();
    }
    if (g_authState == AUTH_ERROR &&
        Clay_PointerOver(Clay_GetElementId(CLAY_STRING("GoogleRetryBtn")))) {
      g_authState = AUTH_READY;
      g_authErrorMsg[0] = '\0';
    }
    if (g_authState == AUTH_AWAITING_CODE &&
        Clay_PointerOver(Clay_GetElementId(CLAY_STRING("GoogleCancelBtn")))) {
      OAuthServer_Stop();
      g_authState = AUTH_READY;
    }
  }

  CLAY(CLAY_ID("SettingsPage"),
       {
           .layout =
               {
                   .sizing = {.width = CLAY_SIZING_GROW(0),
                              .height = CLAY_SIZING_GROW(0)},
                   .layoutDirection = CLAY_TOP_TO_BOTTOM,
               },
           .backgroundColor = cal_cream,
       }) {

    // Top bar with back button
    CLAY(CLAY_ID("SettingsTopBar"),
         {
             .layout =
                 {
                     .sizing = {.width = CLAY_SIZING_GROW(0),
                                .height = CLAY_SIZING_FIXED(80)},
                     .childAlignment = {.y = CLAY_ALIGN_Y_CENTER},
                     .padding = {16, 16, 0, 0},
                     .childGap = 16,
                 },
             .backgroundColor = cal_cream,
             .border = {.color = cal_borderColor, .width = {.bottom = 3}},
         }) {
      CLAY(
          CLAY_ID("SettingsBackBtn"),
          {
              .layout =
                  {
                      .sizing = {.width = CLAY_SIZING_FIXED(48),
                                 .height = CLAY_SIZING_FIXED(48)},
                      .childAlignment = {.x = CLAY_ALIGN_X_CENTER,
                                         .y = CLAY_ALIGN_Y_CENTER},
                  },
              .backgroundColor =
                  Clay_Hovered() ? cal_hoverYellow : (Clay_Color){0, 0, 0, 0},
              .border = {.color = cal_borderColor, .width = CLAY_BORDER_ALL(2)},
          }) {
        CLAY_TEXT(CLAY_STRING("<"), CLAY_TEXT_CONFIG({
                                        .fontId = fontId,
                                        .fontSize = 24,
                                        .textColor = cal_primaryText,
                                    }));
      }
      CLAY_TEXT(CLAY_STRING("Settings"), CLAY_TEXT_CONFIG({
                                             .fontId = fontId,
                                             .fontSize = 28,
                                             .textColor = cal_primaryText,
                                         }));
    }

    // Content area
    CLAY(CLAY_ID("SettingsContent"),
         {
             .layout =
                 {
                     .sizing = {.width = CLAY_SIZING_GROW(0),
                                .height = CLAY_SIZING_GROW(0)},
                     .layoutDirection = CLAY_TOP_TO_BOTTOM,
                     .padding = {24, 24, 24, 24},
                     .childGap = 16,
                 },
         }) {

      // Section header
      CLAY_TEXT(CLAY_STRING("Google Calendar"),
                CLAY_TEXT_CONFIG({
                    .fontId = fontId,
                    .fontSize = 24,
                    .textColor = cal_primaryText,
                }));

      // ── AUTH_READY: Show connect button ──
      if (g_authState == AUTH_READY) {
        CLAY(CLAY_ID("GoogleConnectBtn"),
             {
                 .layout =
                     {
                         .sizing = {.width = CLAY_SIZING_FIXED(280),
                                    .height = CLAY_SIZING_FIXED(48)},
                         .childAlignment = {.x = CLAY_ALIGN_X_CENTER,
                                            .y = CLAY_ALIGN_Y_CENTER},
                     },
                 .backgroundColor =
                     Clay_Hovered() ? cal_hoverYellow : cal_accentBlue,
                 .border =
                     {.color = cal_borderColor,
                      .width = {.left = 3, .right = 6, .top = 3, .bottom = 6}},
             }) {
          CLAY_TEXT(
              CLAY_STRING("Connect Google Calendar"),
              CLAY_TEXT_CONFIG({
                  .fontId = fontId,
                  .fontSize = 20,
                  .textColor = Clay_Hovered() ? cal_primaryText : cal_cream,
              }));
        }
      }

      // ── AUTH_AWAITING_CODE: Waiting for browser authorization ──
      if (g_authState == AUTH_AWAITING_CODE) {
        CLAY_TEXT(
            CLAY_STRING("Waiting for authorization... Complete sign-in in your "
                        "browser."),
            CLAY_TEXT_CONFIG({
                .fontId = fontId,
                .fontSize = 18,
                .textColor = cal_secondaryText,
                .wrapMode = CLAY_TEXT_WRAP_WORDS,
            }));

        CLAY(CLAY_ID("GoogleCancelBtn"),
             {
                 .layout =
                     {
                         .sizing = {.width = CLAY_SIZING_FIXED(120),
                                    .height = CLAY_SIZING_FIXED(44)},
                         .childAlignment = {.x = CLAY_ALIGN_X_CENTER,
                                            .y = CLAY_ALIGN_Y_CENTER},
                     },
                 .backgroundColor = Clay_Hovered()
                                        ? cal_hoverYellow
                                        : (Clay_Color){200, 200, 200, 255},
                 .border = {.color = cal_borderColor,
                            .width = CLAY_BORDER_ALL(3)},
             }) {
          CLAY_TEXT(CLAY_STRING("Cancel"), CLAY_TEXT_CONFIG({
                                               .fontId = fontId,
                                               .fontSize = 20,
                                               .textColor = cal_primaryText,
                                           }));
        }
      }

      // ── AUTH_AUTHENTICATED: Connected status + buttons ──
      if (g_authState == AUTH_AUTHENTICATED) {
        CLAY(CLAY_ID("GoogleStatusRow"),
             {
                 .layout =
                     {
                         .sizing = {.width = CLAY_SIZING_GROW(0),
                                    .height = CLAY_SIZING_FIT(0)},
                         .childAlignment = {.y = CLAY_ALIGN_Y_CENTER},
                         .childGap = 8,
                     },
             }) {
          // Green dot
          CLAY(CLAY_ID("GoogleStatusDot"),
               {
                   .layout =
                       {
                           .sizing = {.width = CLAY_SIZING_FIXED(12),
                                      .height = CLAY_SIZING_FIXED(12)},
                       },
                   .backgroundColor = (Clay_Color){0, 180, 80, 255},
                   .border = {.color = cal_borderColor,
                              .width = CLAY_BORDER_ALL(1)},
               }) {}
          CLAY_TEXT(CLAY_STRING("Connected"),
                    CLAY_TEXT_CONFIG({
                        .fontId = fontId,
                        .fontSize = 20,
                        .textColor = (Clay_Color){0, 140, 60, 255},
                    }));
        }

        // Button row
        CLAY(CLAY_ID("GoogleAuthBtnRow"),
             {
                 .layout =
                     {
                         .sizing = {.width = CLAY_SIZING_GROW(0),
                                    .height = CLAY_SIZING_FIT(0)},
                         .childGap = 12,
                     },
             }) {
          CLAY(
              CLAY_ID("GoogleRefreshBtn"),
              {
                  .layout =
                      {
                          .sizing = {.width = CLAY_SIZING_FIXED(120),
                                     .height = CLAY_SIZING_FIXED(44)},
                          .childAlignment = {.x = CLAY_ALIGN_X_CENTER,
                                             .y = CLAY_ALIGN_Y_CENTER},
                      },
                  .backgroundColor =
                      Clay_Hovered() ? cal_hoverYellow : cal_accentBlue,
                  .border =
                      {.color = cal_borderColor,
                       .width = {.left = 3, .right = 6, .top = 3, .bottom = 6}},
              }) {
            CLAY_TEXT(
                CLAY_STRING("Refresh"),
                CLAY_TEXT_CONFIG({
                    .fontId = fontId,
                    .fontSize = 20,
                    .textColor = Clay_Hovered() ? cal_primaryText : cal_cream,
                }));
          }

          // Disconnect button
          CLAY(CLAY_ID("GoogleDisconnectBtn"),
               {
                   .layout =
                       {
                           .sizing = {.width = CLAY_SIZING_FIXED(140),
                                      .height = CLAY_SIZING_FIXED(44)},
                           .childAlignment = {.x = CLAY_ALIGN_X_CENTER,
                                              .y = CLAY_ALIGN_Y_CENTER},
                       },
                   .backgroundColor = Clay_Hovered()
                                          ? cal_hoverYellow
                                          : (Clay_Color){240, 60, 60, 255},
                   .border = {.color = cal_borderColor,
                              .width = CLAY_BORDER_ALL(3)},
               }) {
            CLAY_TEXT(
                CLAY_STRING("Disconnect"),
                CLAY_TEXT_CONFIG({
                    .fontId = fontId,
                    .fontSize = 20,
                    .textColor = Clay_Hovered() ? cal_primaryText : cal_cream,
                }));
          }
        }
      }

      // ── AUTH_ERROR: Error message + retry ──
      if (g_authState == AUTH_ERROR) {
        CLAY_TEXT(CLAY_STRING("Authentication failed:"),
                  CLAY_TEXT_CONFIG({
                      .fontId = fontId,
                      .fontSize = 20,
                      .textColor = (Clay_Color){220, 40, 40, 255},
                  }));

        if (g_authErrorMsg[0] != '\0') {
          CLAY_TEXT(cal_make_string(g_authErrorMsg),
                    CLAY_TEXT_CONFIG({
                        .fontId = fontId,
                        .fontSize = 16,
                        .textColor = cal_secondaryText,
                        .wrapMode = CLAY_TEXT_WRAP_WORDS,
                    }));
        }

        CLAY(CLAY_ID("GoogleRetryBtn"),
             {
                 .layout =
                     {
                         .sizing = {.width = CLAY_SIZING_FIXED(140),
                                    .height = CLAY_SIZING_FIXED(44)},
                         .childAlignment = {.x = CLAY_ALIGN_X_CENTER,
                                            .y = CLAY_ALIGN_Y_CENTER},
                     },
                 .backgroundColor =
                     Clay_Hovered() ? cal_hoverYellow : cal_accentBlue,
                 .border =
                     {.color = cal_borderColor,
                      .width = {.left = 3, .right = 6, .top = 3, .bottom = 6}},
             }) {
          CLAY_TEXT(
              CLAY_STRING("Try Again"),
              CLAY_TEXT_CONFIG({
                  .fontId = fontId,
                  .fontSize = 20,
                  .textColor = Clay_Hovered() ? cal_primaryText : cal_cream,
              }));
        }
      }
    }
  }
}

#endif
