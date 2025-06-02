cmake_minimum_required(VERSION 3.11)

set(SHOGLE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
set(SHOGLE_SRC_DIR "${SHOGLE_DIR}/src")
set(SHOGLE_INC_DIR "${SHOGLE_DIR}/include")
set(SHOGLE_EXTERN_DIR "${SHOGLE_DIR}/extern")
set(SHOGLE_DEMOS_DIR "${SHOGLE_DIR}/demos")
set(SHOGLE_TESTS_DIR "${SHOGLE_DIR}/tests")

set(SHOGLE_VER_MAJ 0)
set(SHOGLE_VER_MIN 3)
set(SHOGLE_VER_REV 0)
set(SHOGLE_VER "${SHOGLE_VER_MAJ}.${SHOGLE_VER_MIN}.${SHOGLE_VER_REV}")

project(shogle VERSION ${SHOGLE_VER} LANGUAGES CXX C)

option(SHOGLE_BUILD_TESTS "Build tests" OFF)
option(SHOGLE_BUILD_DEMOS "Build demos" OFF)
option(SHOGLE_EMBED_GIT "Embed git info in version string" ON)

option(SHOGLE_ENABLE_INTERNAL_LOGS "Enable internal logs" ON)
option(SHOGLE_ENABLE_IMGUI "Enable imgui support" ON)
option(SHOGLE_ENABLE_GLFW "Use glfw as the windowing system" ON)

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

message(STATUS "ShOGLE: Setting up version ${SHOGLE_VERSION}...")

file(GLOB_RECURSE SHOGLE_SOURCES "${SHOGLE_SRC_DIR}/**.cpp")
file(GLOB_RECURSE SHOGLE_PRIV_HEADERS "${SHOGLE_SRC_DIR}/**.hpp")
file(GLOB_RECURSE SHOGLE_PUB_HEADERS "${SHOGLE_INC_DIR}/**.hpp")
configure_file("${SHOGLE_SRC_DIR}/version.hpp.in" "include/shogle/version.hpp" @ONLY)
list(APPEND SHOGLE_INCLUDE "${SHOGLE_INC_DIR}")
list(APPEND SHOGLE_INCLUDE "${CMAKE_CURRENT_BINARY_DIR}/include")

set (CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(CommonDefs)

message(STATUS "ShOGLE: Setting up dependencies...")
add_subdirectory(${SHOGLE_EXTERN_DIR})

add_library(${PROJECT_NAME} STATIC ${SHOGLE_SOURCES} ${SHOGLE_EXTERN_SOURCE} ${SHOGLE_PRIV_HEADERS} ${SHOGLE_PUB_HEADERS})
set_target_properties(${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE CXX CXX_STANDARD 20)

if (SHOGLE_ENABLE_INTERNAL_LOGS)
  list(APPEND SHOGLE_DEFS SHOGLE_ENABLE_INTERNAL_LOGS=1)
endif()

if (SHOGLE_BUILD_TESTS)
  message(STATUS "ShOGLE: Tests enabled")
  add_subdirectory(${SHOGLE_TESTS_DIR})
endif()

if (SHOGLE_BUILD_DEMOS)
  message(STATUS "ShOGLE: Demos enabled")
  add_subdirectory(${SHOGLE_DEMOS_DIR})
endif()

target_include_directories(${PROJECT_NAME} PUBLIC ${SHOGLE_INCLUDE} ${SHOGLE_EXTERN_INCLUDE})
target_link_libraries(${PROJECT_NAME} PUBLIC ${SHOGLE_EXTERN_LINK})
target_compile_definitions(${PROJECT_NAME} PUBLIC ${SHOGLE_DEFS})
