cmake_minimum_required(VERSION 3.20)
project(fella C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(PkgConfig REQUIRED)
pkg_check_modules(RAYLIB REQUIRED raylib)

add_executable(fella main.c events.c cJSON.c)
target_include_directories(fella PRIVATE ${CMAKE_SOURCE_DIR} ${RAYLIB_INCLUDE_DIRS})
target_link_libraries(fella ${RAYLIB_LIBRARIES} m pthread dl)
target_link_directories(fella PRIVATE ${RAYLIB_LIBRARY_DIRS})
target_compile_options(fella PRIVATE -Wall -Wextra -O2)

file(COPY resources DESTINATION ${CMAKE_BINARY_DIR})
