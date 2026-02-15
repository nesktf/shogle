cmake_minimum_required(VERSION 3.25)
include(ExternalProject)

find_package(PkgConfig REQUIRED)

# ntfstl
add_subdirectory("${SHOGLE_EXTERN_DIR}/ntfstl")
list(APPEND EXTERN_INCLUDE "${SHOGLE_EXTERN_DIR}/ntfstl/include")
list(APPEND EXTERN_LINK ntfstl)

# imgui
if (SHOGLE_ENABLE_IMGUI)
  message(STATUS "ShOGLE: Including dear imgui support...")

  set(IMGUI_DIR "${SHOGLE_EXTERN_DIR}/imgui")
  set(IMGUI_BACK "${IMGUI_DIR}/backends")

  file(GLOB IMGUI_SRC "${IMGUI_DIR}/imgui*.cpp")
  list(APPEND IMGUI_SRC "${IMGUI_BACK}/imgui_impl_glfw.cpp")
  list(APPEND IMGUI_SRC "${IMGUI_BACK}/imgui_impl_opengl3.cpp")

  list(APPEND EXTERN_SOURCE ${IMGUI_SRC})
  list(APPEND EXTERN_INCLUDE "${IMGUI_DIR}" "${IMGUI_BACK}")
endif()

# glfw
pkg_search_module(GLFW REQUIRED glfw3)
list(APPEND EXTERN_INCLUDE ${GLFW_INCLUDE_DIRS})
list(APPEND EXTERN_LINK glfw)

set(SHOGLE_EXTERN_SOURCE ${EXTERN_SOURCE} PARENT_SCOPE)
set(SHOGLE_EXTERN_INCLUDE ${EXTERN_INCLUDE} PARENT_SCOPE)
set(SHOGLE_EXTERN_LINK ${EXTERN_LINK} PARENT_SCOPE)
