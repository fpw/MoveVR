cmake_minimum_required(VERSION 3.9)
project(MoveVR VERSION 0.1.2 DESCRIPTION "MoveVR X-Plane plugin")

configure_file(
  "${PROJECT_SOURCE_DIR}/config.h.in"
  "${PROJECT_BINARY_DIR}/config.h"
)

add_definitions("-include ${PROJECT_BINARY_DIR}/config.h")
include_directories("${PROJECT_SOURCE_DIR}")

include_directories("/opt/XSDK/CHeaders")
link_directories("/opt/XSDK/Libraries/Win")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")

add_subdirectory(src)
