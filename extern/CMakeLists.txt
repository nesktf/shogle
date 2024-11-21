cmake_minimum_required(VERSION 3.10)

find_package(PkgConfig REQUIRED)

set(EXTERN_PATH ${CMAKE_CURRENT_SOURCE_DIR})

list(APPEND EXTERN_INCLUDE "${EXTERN_PATH}")

# stb_image
list(APPEND EXTERN_SOURCE "${EXTERN_PATH}/stb/stb_image.c")

# glad
list(APPEND EXTERN_SOURCE "${EXTERN_PATH}/glad/glad.c")

# imgui
if (SHOGLE_ENABLE_IMGUI)
  set(IMGUI_DIR "${EXTERN_PATH}/imgui")
  set(IMGUI_BACK "${IMGUI_DIR}/backends")

  file(GLOB IMGUI_SRC "${IMGUI_DIR}/imgui*.cpp")
  list(APPEND IMGUI_SRC "${IMGUI_BACK}/imgui_impl_glfw.cpp")
  list(APPEND IMGUI_SRC "${IMGUI_BACK}/imgui_impl_opengl3.cpp")

  list(APPEND EXTERN_SOURCE ${IMGUI_SRC})
  list(APPEND EXTERN_INCLUDE "${IMGUI_DIR}" "${IMGUI_BACK}")
endif()

# # glm
find_package(glm CONFIG REQUIRED)
list(APPEND EXTERN_INCLUDE ${GLM_INCLUDE_DIRS})
list(APPEND EXTERN_LINK glm)

# assimp
find_package(assimp REQUIRED)
list(APPEND EXTERN_INCLUDE ${ASSIMP_INCLUDE_DIRS})
list(APPEND EXTERN_LINK assimp)

# fmt
find_package(fmt REQUIRED)
list(APPEND EXTERN_INCLUDE ${FMT_INCLUDE_DIRS})
list(APPEND EXTERN_LINK fmt)

# glfw
pkg_search_module(GLFW REQUIRED glfw3)
list(APPEND EXTERN_INCLUDE ${GLFW_INCLUDE_DIRS})
list(APPEND EXTERN_LINK glfw)

# freetype
find_package(Freetype REQUIRED)
list(APPEND EXTERN_INCLUDE ${FREETYPE_INCLUDE_DIRS})
list(APPEND EXTERN_LINK freetype)

# # OpenAL
# find_package(OpenAL REQUIRED)
# list(APPEND EXTERN_INCLUDE ${OPENAL_INCLUDE_DIRS})
# list(APPEND EXTERN_LINK OpenAL::OpenAL)

set(EXTERN_SOURCE ${EXTERN_SOURCE} PARENT_SCOE)
set(EXTERN_INCLUDE ${EXTERN_INCLUDE} PARENT_SCOPE)
set(EXTERN_LINK ${EXTERN_LINK} PARENT_SCOPE)
