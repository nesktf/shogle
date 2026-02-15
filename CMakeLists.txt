cmake_minimum_required(VERSION 3.25)
include(FetchContent)

find_package(PkgConfig REQUIRED)

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

option(SHOGLE_BUILD_TESTS "Build tests" OFF)
option(SHOGLE_BUILD_DEMOS "Build demos" OFF)
option(SHOGLE_EMBED_GIT "Embed git info in version string" ON)
option(SHOGLE_DISABLE_INTERNAL_LOGS "Disable internal logs" OFF)
option(SHOGLE_ENABLE_IMGUI "Enable imgui support" OFF)
option(SHOGLE_SHARED_BUILD "Shared library build" OFF)
option(SHOGLE_DISABLE_EXCEPTIONS "Disable exceptions" OFF)

set(SHOGLE_ENABLE_GLFW ON)
set(SHOGLE_ENABLE_OPENGL ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

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
  set(SHOGLE_VERSION_STRING
      "${SHOGLE_VER}-${SHOGLE_BUILD_ID}-${SHOGLE_GIT_TAG} (${SHOGLE_GIT_BRANCH})")
else()
  set(SHOGLE_VERSION_STRING "${SHOGLE_VER}")
endif()

#set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_SOURCE_DIR}/cmake")
message(STATUS "ShOGLE: Version ${SHOGLE_VERSION_STRING}")
project(shogle VERSION ${SHOGLE_VER} LANGUAGES CXX C)

set(SHOGLE_EXTERN_SOURCES)
set(FETCHCONTENT_QUIET FALSE)

# libfmt
find_package(fmt)
list(APPEND SHOGLE_EXTERN_INCLUDES ${FMT_INCLUDE_DIRES})
list(APPEND SHOGLE_EXTERN_LINK fmt::fmt)

# glfw
if (SHOGLE_ENABLE_GLFW)
  pkg_search_module(GLFW REQUIRED glfw3)
  list(APPEND SHOGLE_EXTERN_INCLUDES ${GLFW_INCLUDE_DIRS})
  list(APPEND SHOGLE_EXTERN_LINK glfw)
endif()

# imgui
if (SHOGLE_ENABLE_IMGUI)
  if (NOT DEFINED IMGUI_TAG)
    set(IMGUI_TAG "v1.92.5")
  endif()

  message(STATUS "ShOGLE: Fetching dear imgui ${IMGUI_TAG}")

  FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG ${IMGUI_TAG}
    GIT_PROGRESS TRUE
  )
  FetchCOntent_GetProperties(imgui)
  if (NOT imgui_POPULATED)
    FetchContent_MakeAvailable(imgui)
    file(GLOB IMGUI_SOURCES "${imgui_SOURCE_DIR}/*.cpp")
    if (SHOGLE_ENABLE_GLFW)
      list(APPEND IMGUI_SOURCES "${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp")
    endif()
    if (SHOGLE_ENABLE_OPENGL)
      list(APPEND IMGUI_SOURCES "${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp")
    endif()

    list(APPEND SHOGLE_EXTERN_INCLUDES "${imgui_SOURCE_DIR}" "${imgui_SOURCE_DIR}/backends")
    list(APPEND SHOGLE_EXTERN_SOURCES ${IMGUI_SOURCES})
    configure_file("${imgui_SOURCE_DIR}/imgui.h" 
                   "${CMAKE_CURRENT_BINARY_DIR}/extern/shogle/extern/imgui.h"
                   COPYONLY)
    configure_file("${imgui_SOURCE_DIR}/imconfig.h" 
                   "${CMAKE_CURRENT_BINARY_DIR}/extern/shogle/extern/imconfig.h"
                   COPYONLY)
  endif()
endif()

add_subdirectory(${SHOGLE_SOURCE_DIR})
configure_file("${SHOGLE_SOURCE_DIR}/config.hpp.in"
               "${CMAKE_CURRENT_BINARY_DIR}/generated/shogle/config.hpp"
               @ONLY)

if (SHOGLE_SHARED_BUILD)
  set(SHOGLE_LIBRARY_TYPE SHARED)
else()
  set(SHOGLE_LIBRARY_TYPE STATIC)
endif()

add_library(${PROJECT_NAME} ${SHOGLE_LIBRARY_TYPE} ${SHOGLE_SOURCES} ${SHOGLE_EXTERN_SOURCES})

target_include_directories(${PROJECT_NAME} PUBLIC
                           "$<BUILD_INTERFACE:${SHOGLE_INCLUDE_DIR}>"
                           "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/generated>"
                           "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/extern>"
                           "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>")

target_include_directories(${PROJECT_NAME} PRIVATE
                           ${SHOGLE_SOURCE_DIR} ${SHOGLE_EXTERN_INCLUDES})

set_target_properties(${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE CXX CXX_STANDARD 20)
target_link_libraries(${PROJECT_NAME} PUBLIC ${SHOGLE_EXTERN_LINK})

install(TARGETS ${PROJECT_NAME}
        EXPORT ${PROJECT_NAME}-config
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})

install(FILES ${SHOGLE_INCLUDES}
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME})

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/generated/shogle/config.hpp"
        DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}")

if (SHOGLE_ENABLE_IMGUI)
  install(FILES "${CMAKE_CURRENT_BINARY_DIR}/extern/shogle/extern/imgui.h"
          DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}/extern")
  install(FILES "${CMAKE_CURRENT_BINARY_DIR}/extern/shogle/extern/imconfig.h"
          DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}/extern")
endif()

install(EXPORT ${PROJECT_NAME}-config
        NAMESPACE ${PROJECT_NAME}::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME})

#if (SHOGLE_BUILD_TESTS)
#  message(STATUS "ShOGLE: Building tests")
#  add_subdirectory(${SHOGLE_TESTS_DIR})
#endif()

if (SHOGLE_BUILD_DEMOS)
  message(STATUS "ShOGLE: Building demos")
  add_subdirectory(${SHOGLE_DEMOS_DIR})
endif()
