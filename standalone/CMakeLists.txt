cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

project(tinyprogStandalone LANGUAGES CXX)

# --- Import tools ----

include(../cmake/tools.cmake)

# ---- Dependencies ----

include(../cmake/CPM.cmake)

CPMAddPackage(NAME tinyprog SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/..)

# ---- Create standalone executable ----

file(GLOB sources CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp)

add_executable(tinyprogStandalone ${sources})

set_target_properties(tinyprogStandalone PROPERTIES CXX_STANDARD 17 OUTPUT_NAME "tinyprog")

target_link_libraries(tinyprogStandalone tinyprog)

set_target_properties(tinyprogStandalone PROPERTIES CXX_STANDARD 17)