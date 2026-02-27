#ifndef COMPONENT_SETTINGS_PAGE_H
#define COMPONENT_SETTINGS_PAGE_H

#include "cal_common.h"
#include "google_auth.h"
#include "raylib.h"

#include <stdio.h>
#include <string.h>

// ── Read system clipboard via wl-paste or xclip ─────────────────────────────
static void SettingsPage_PasteFromClipboard(void) {
  // Try Raylib first
  const char *clip = GetClipboardText();
  static char sysBuf[GOOGLE_AUTH_CODE_MAX];
  if (!clip || !clip[0]) {
    FILE *p = popen("wl-paste --no-newline 2>/dev/null || "
                    "xclip -selection clipboard -o 2>/dev/null",
                    "r");
    if (p) {
      size_t n = fread(sysBuf, 1, sizeof(sysBuf) - 1, p);
      sysBuf[n] = '\0';
      pclose(p);
      if (n > 0)
        clip = sysBuf;
    }
  }
  if (clip) {
    g_authCodeInputLen = 0;
    for (int i = 0; clip[i] && g_authCodeInputLen < GOOGLE_AUTH_CODE_MAX - 1;
         i++) {
      if (clip[i] >= 32 && clip[i] < 127) {
        g_authCodeInput[g_authCodeInputLen++] = clip[i];
      }
    }
    g_authCodeInput[g_authCodeInputLen] = '\0';
  }
}

// ── Text input handling (called each frame from the settings page) ───────────
static void SettingsPage_HandleTextInput(void) {
  if (g_authState != AUTH_AWAITING_CODE)
    return;

  // Keyboard character input
  int ch;
  while ((ch = GetCharPressed()) != 0) {
    if (g_authCodeInputLen < GOOGLE_AUTH_CODE_MAX - 1 && ch >= 32 && ch < 127) {
      g_authCodeInput[g_authCodeInputLen++] = (char)ch;
      g_authCodeInput[g_authCodeInputLen] = '\0';
    }
  }

  // Backspace
  if (IsKeyPressed(KEY_BACKSPACE) && g_authCodeInputLen > 0) {
    g_authCodeInput[--g_authCodeInputLen] = '\0';
  }

  // Ctrl+V paste
  if ((IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL)) &&
      IsKeyPressed(KEY_V)) {
    SettingsPage_PasteFromClipboard();
  }
}

static void SettingsPage_Render(uint32_t fontId) {
  // Handle text input each frame
  SettingsPage_HandleTextInput();

  // Handle button clicks
  if (IsMouseButtonPressed(0)) {
    if (g_authState == AUTH_READY &&
        Clay_PointerOver(Clay_GetElementId(CLAY_STRING("GoogleConnectBtn")))) {
      GoogleAuth_BuildAuthUrl();
      OpenURL(g_authUrl);
      g_authState = AUTH_AWAITING_CODE;
      g_authCodeInput[0] = '\0';
      g_authCodeInputLen = 0;
    }
    if (g_authState == AUTH_AWAITING_CODE &&
        Clay_PointerOver(Clay_GetElementId(CLAY_STRING("GoogleSubmitBtn")))) {
      if (g_authCodeInputLen > 0) {
        // Strip whitespace from pasted code
        char code[GOOGLE_AUTH_CODE_MAX];
        int j = 0;
        for (int i = 0; i < g_authCodeInputLen; i++) {
          if (g_authCodeInput[i] != ' ' && g_authCodeInput[i] != '\n' &&
              g_authCodeInput[i] != '\r' && g_authCodeInput[i] != '\t') {
            code[j++] = g_authCodeInput[i];
          }
        }
        code[j] = '\0';
        if (GoogleAuth_ExchangeCode(code)) {
          Calendar_ReloadEvents();
        }
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
      g_authState = AUTH_READY;
      g_authCodeInput[0] = '\0';
      g_authCodeInputLen = 0;
    }
    if (g_authState == AUTH_AWAITING_CODE &&
        Clay_PointerOver(Clay_GetElementId(CLAY_STRING("GooglePasteBtn")))) {
      SettingsPage_PasteFromClipboard();
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

      // ── AUTH_AWAITING_CODE: Instructions + text input + submit ──
      if (g_authState == AUTH_AWAITING_CODE) {
        CLAY_TEXT(
            CLAY_STRING("A browser window has opened. Sign in with Google,"),
            CLAY_TEXT_CONFIG({
                .fontId = fontId,
                .fontSize = 18,
                .textColor = cal_secondaryText,
                .wrapMode = CLAY_TEXT_WRAP_WORDS,
            }));
        CLAY_TEXT(
            CLAY_STRING(
                "then copy the code from the URL bar and paste it below:"),
            CLAY_TEXT_CONFIG({
                .fontId = fontId,
                .fontSize = 18,
                .textColor = cal_secondaryText,
                .wrapMode = CLAY_TEXT_WRAP_WORDS,
            }));

        // Text input row: box + paste button
        CLAY(CLAY_ID("GoogleCodeInputRow"),
             {
                 .layout =
                     {
                         .sizing = {.width = CLAY_SIZING_GROW(.max = 620),
                                    .height = CLAY_SIZING_FIT(0)},
                         .childGap = 8,
                         .childAlignment = {.y = CLAY_ALIGN_Y_CENTER},
                     },
             }) {
          CLAY(CLAY_ID("GoogleCodeInputBox"),
               {
                   .layout =
                       {
                           .sizing = {.width = CLAY_SIZING_GROW(.max = 500),
                                      .height = CLAY_SIZING_FIXED(44)},
                           .childAlignment = {.y = CLAY_ALIGN_Y_CENTER},
                           .padding = {8, 8, 0, 0},
                       },
                   .backgroundColor = (Clay_Color){255, 255, 255, 255},
                   .border = {.color = cal_borderColor,
                              .width = CLAY_BORDER_ALL(3)},
               }) {
            if (g_authCodeInputLen > 0) {
              CLAY_TEXT(cal_make_string(g_authCodeInput),
                        CLAY_TEXT_CONFIG({
                            .fontId = fontId,
                            .fontSize = 18,
                            .textColor = cal_primaryText,
                        }));
            } else {
              CLAY_TEXT(CLAY_STRING("Paste authorization code here..."),
                        CLAY_TEXT_CONFIG({
                            .fontId = fontId,
                            .fontSize = 18,
                            .textColor = (Clay_Color){150, 150, 150, 255},
                        }));
            }
          }

          // Paste button
          CLAY(CLAY_ID("GooglePasteBtn"),
               {
                   .layout =
                       {
                           .sizing = {.width = CLAY_SIZING_FIXED(80),
                                      .height = CLAY_SIZING_FIXED(44)},
                           .childAlignment = {.x = CLAY_ALIGN_X_CENTER,
                                              .y = CLAY_ALIGN_Y_CENTER},
                       },
                   .backgroundColor = Clay_Hovered()
                                          ? cal_hoverYellow
                                          : (Clay_Color){220, 220, 220, 255},
                   .border = {.color = cal_borderColor,
                              .width = CLAY_BORDER_ALL(3)},
               }) {
            CLAY_TEXT(CLAY_STRING("Paste"), CLAY_TEXT_CONFIG({
                                                .fontId = fontId,
                                                .fontSize = 18,
                                                .textColor = cal_primaryText,
                                            }));
          }
        } // close GoogleCodeInputRow

        // Button row
        CLAY(CLAY_ID("GoogleCodeBtnRow"),
             {
                 .layout =
                     {
                         .sizing = {.width = CLAY_SIZING_GROW(0),
                                    .height = CLAY_SIZING_FIT(0)},
                         .childGap = 12,
                     },
             }) {
          CLAY(
              CLAY_ID("GoogleSubmitBtn"),
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
                                         : (Clay_Color){0, 180, 80, 255},
                  .border =
                      {.color = cal_borderColor,
                       .width = {.left = 3, .right = 6, .top = 3, .bottom = 6}},
              }) {
            CLAY_TEXT(
                CLAY_STRING("Submit"),
                CLAY_TEXT_CONFIG({
                    .fontId = fontId,
                    .fontSize = 20,
                    .textColor = Clay_Hovered() ? cal_primaryText : cal_cream,
                }));
          }

          // Cancel button
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
