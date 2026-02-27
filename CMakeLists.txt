cmake_minimum_required(VERSION 3.20)
project(fella C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Google OAuth credentials — set via environment variables
if(DEFINED ENV{GOOGLE_CLIENT_ID})
  set(GOOGLE_CLIENT_ID "$ENV{GOOGLE_CLIENT_ID}")
else()
  message(FATAL_ERROR "GOOGLE_CLIENT_ID environment variable not set")
endif()

if(DEFINED ENV{GOOGLE_CLIENT_SECRET})
  set(GOOGLE_CLIENT_SECRET "$ENV{GOOGLE_CLIENT_SECRET}")
else()
  message(FATAL_ERROR "GOOGLE_CLIENT_SECRET environment variable not set")
endif()

configure_file(src/config.h.in ${CMAKE_BINARY_DIR}/config.h @ONLY)

find_package(PkgConfig REQUIRED)
pkg_check_modules(RAYLIB REQUIRED raylib)
pkg_check_modules(CURL REQUIRED IMPORTED_TARGET libcurl)

add_executable(fella src/main.c src/events.c src/google_auth.c src/google_calendar.c vendor/cJSON.c)
target_include_directories(fella PRIVATE src vendor ${RAYLIB_INCLUDE_DIRS} ${CMAKE_BINARY_DIR})
target_link_libraries(fella ${RAYLIB_LIBRARIES} PkgConfig::CURL m pthread dl)
target_link_directories(fella PRIVATE ${RAYLIB_LIBRARY_DIRS})
target_compile_options(fella PRIVATE -Wall -Wextra -O2)
target_link_options(fella PRIVATE -Wl,--allow-shlib-undefined)

file(COPY resources DESTINATION ${CMAKE_BINARY_DIR})
