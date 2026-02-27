#define CLAY_IMPLEMENTATION
#include "calendar.h"
#include "clay.h"
#include "clay_renderer_raylib.c"
#include "google_auth.h"
#include "oauth_server.h"
#include <curl/curl.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

const uint32_t FONT_ID_BODY_24 = 0;

void HandleClayErrors(Clay_ErrorData errorData) {
  fprintf(stderr, "Clay error: %s\n", errorData.errorText.chars);
}

Clay_RenderCommandArray CreateLayout(void) {
  Clay_BeginLayout();
  CLAY(CLAY_ID("Root"), {
                            .layout =
                                {
                                    .sizing = {.width = CLAY_SIZING_GROW(0),
                                               .height = CLAY_SIZING_GROW(0)},
                                },
                            .backgroundColor = {252, 250, 245, 255},
                        }) {
    Calendar_Render(FONT_ID_BODY_24);
  }
  return Clay_EndLayout();
}

int main(void) {
  curl_global_init(CURL_GLOBAL_DEFAULT);
  GoogleAuth_Init();

  uint64_t totalMemorySize = Clay_MinMemorySize();
  Clay_Arena clayMemory = Clay_CreateArenaWithCapacityAndMemory(
      totalMemorySize, malloc(totalMemorySize));
  Clay_Initialize(clayMemory, (Clay_Dimensions){1024, 768},
                  (Clay_ErrorHandler){HandleClayErrors, 0});
  Clay_Raylib_Initialize(1024, 768, "Calendar",
                         FLAG_VSYNC_HINT | FLAG_WINDOW_RESIZABLE |
                             FLAG_MSAA_4X_HINT);

  Font fonts[1];
  fonts[FONT_ID_BODY_24] =
      LoadFontEx("resources/Inter-Regular.ttf", 48, 0, 400);
  SetTextureFilter(fonts[FONT_ID_BODY_24].texture, TEXTURE_FILTER_BILINEAR);
  Clay_SetMeasureTextFunction(Raylib_MeasureText, fonts);

  bool scrollInitialized = false;

  while (!WindowShouldClose()) {
    Clay_SetPointerState(
        (Clay_Vector2){GetMousePosition().x, GetMousePosition().y},
        IsMouseButtonDown(0));
    Clay_SetLayoutDimensions(
        (Clay_Dimensions){(float)GetScreenWidth(), (float)GetScreenHeight()});
    Clay_UpdateScrollContainers(
        true, (Clay_Vector2){GetMouseWheelMoveV().x, GetMouseWheelMoveV().y},
        GetFrameTime());

    Clay_RenderCommandArray renderCommands = CreateLayout();

    // Auto-scroll to current time on first frame
    if (!scrollInitialized) {
      Clay_ScrollContainerData scrollData = Clay_GetScrollContainerData(
          Clay_GetElementId(CLAY_STRING("ScrollArea")));
      if (scrollData.found && scrollData.scrollPosition) {
        time_t now = time(NULL);
        struct tm lt = *localtime(&now);
        float currentTimeY =
            ((float)lt.tm_hour + (float)lt.tm_min / 60.0f) * CAL_HOUR_HEIGHT;
        float viewHeight = scrollData.scrollContainerDimensions.height;
        float headerOverlay = CAL_HEADER_HEIGHT + CAL_ALLDAY_HEIGHT;
        float targetScroll =
            -(currentTimeY - (viewHeight + headerOverlay) / 2.0f);
        float maxScroll = -(scrollData.contentDimensions.height - viewHeight);
        if (targetScroll > 0)
          targetScroll = 0;
        if (targetScroll < maxScroll)
          targetScroll = maxScroll;
        scrollData.scrollPosition->y = targetScroll;
        scrollInitialized = true;
      }
    }

    BeginDrawing();
    ClearBackground(BLACK);
    Clay_Raylib_Render(renderCommands, fonts);
    EndDrawing();
  }

  OAuthServer_Stop();
  Clay_Raylib_Close();
  curl_global_cleanup();
  return 0;
}
