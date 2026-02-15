cmake_minimum_required(VERSION 3.25)

set(SHOGLE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
set(SHOGLE_SOURCE_DIR "${SHOGLE_DIR}/src")
set(SHOGLE_INCLUDE_DIR "${SHOGLE_DIR}/include")
set(SHOGLE_EXTERN_DIR "${SHOGLE_DIR}/extern")
set(SHOGLE_DEMOS_DIR "${SHOGLE_DIR}/demos")
set(SHOGLE_TESTS_DIR "${SHOGLE_DIR}/test")

set(SHOGLE_VER_MAJ 0)
set(SHOGLE_VER_MIN 4)
set(SHOGLE_VER_REV 0)
set(SHOGLE_VER "${SHOGLE_VER_MAJ}.${SHOGLE_VER_MIN}.${SHOGLE_VER_REV}")

set(SHOGLE_LIBRARY_TYPE STATIC)

option(SHOGLE_BUILD_TESTS "Build tests" OFF)
option(SHOGLE_BUILD_DEMOS "Build demos" OFF)
option(SHOGLE_EMBED_GIT "Embed git info in version string" ON)

option(SHOGLE_DISABLE_INTERNAL_LOGS "Disable internal logs" OFF)
option(SHOGLE_DISABLE_GLFW "Don't use glfw as the windowing system" ON)
option(SHOGLE_ENABLE_IMGUI "Enable imgui support" ON)

if (SHOGLE_EMBED_GIT)
  execute_process(COMMAND git rev-parse --short HEAD
    WORKING_DIRECTORY "${SHOGLE_DIR}"
    OUTPUT_VARIABLE SHOGLE_GIT_TAG
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )
  execute_process(COMMAND git rev-list --count HEAD
    WORKING_DIRECTORY "${SHOGLE_DIR}"
    OUTPUT_VARIABLE SHOGLE_BUILD_ID
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )
  execute_process(COMMAND git rev-parse --abbrev-ref HEAD
    WORKING_DIRECTORY "${SHOGLE_DIR}"
    OUTPUT_VARIABLE SHOGLE_GIT_BRANCH
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )
  set(SHOGLE_VERSION "${SHOGLE_VER}-${SHOGLE_BUILD_ID}-${SHOGLE_GIT_TAG} (${SHOGLE_GIT_BRANCH})")
else()
  set(SHOGLE_VERSION "${SHOGLE_VER}")
endif()

set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_SOURCE_DIR}/cmake")
set(SHOGLE_DEFINITIONS)
include(CommonDefs)

message(STATUS "ShOGLE: Setting up version ${SHOGLE_VERSION}...")
project(shogle VERSION ${SHOGLE_VER} LANGUAGES CXX C)

message(STATUS "ShOGLE: Setting up dependencies...")
add_subdirectory(${SHOGLE_EXTERN_DIR})

message(STATUS "ShOGLE: Setting up sources...")
add_subdirectory(${SHOGLE_SOURCE_DIR})

#if (SHOGLE_BUILD_TESTS)
#  message(STATUS "ShOGLE: Tests enabled")
#  add_subdirectory(${SHOGLE_TESTS_DIR})
#endif()

if (SHOGLE_BUILD_DEMOS)
  message(STATUS "ShOGLE: Demos enabled")
  add_subdirectory(${SHOGLE_DEMOS_DIR})
endif()
