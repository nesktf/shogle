cmake_minimum_required(VERSION 3.5)

## CMake opts
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

## Project things
project(shogle)
file(GLOB_RECURSE PROJ_SRC "src/*.cpp")
set(PROJ_RESOURCES res)

## Libs
list(APPEND PROJ_INCLUDES lib)

# glad
add_library(glad OBJECT lib/glad/glad.c)
target_include_directories(glad PRIVATE lib)
list(APPEND PROJ_LIBRARIES glad)

# stb image
add_library(stbimage OBJECT lib/stb/stb_image.c)
list(APPEND PROJ_LIBRARIES stbimage)

# glm
find_package(glm CONFIG REQUIRED)
list(APPEND PROJ_INCLUDES ${GLM_INCLUDE_DIRS})
list(APPEND PROJ_LIBRARIES glm)

# assimp
find_package(assimp REQUIRED)
list(APPEND PROJ_INCLUDES ${ASSIMP_INCLUDE_DIRS})
list(APPEND PROJ_LIBRARIES assimp)

# fmt
find_package(fmt REQUIRED)
list(APPEND PROJ_INCLUDES ${FMT_INCLUDE_DIRS})
list(APPEND PROJ_LIBRARIES fmt)

# glfw
find_package(PkgConfig REQUIRED)
pkg_search_module(GLFW REQUIRED glfw3)
list(APPEND PROJ_INCLUDES ${GLFW_INCLUDE_DIRS})
list(APPEND PROJ_LIBRARIES glfw)

# imgui
if (ENABLE_IMGUI)
  set(IMGUI_DIR lib/imgui)

  file(GLOB IMGUI_SRC "${IMGUI_DIR}/imgui*.cpp")
  list(APPEND IMGUI_SRC ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp)
  list(APPEND IMGUI_SRC ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp)
  add_library(imgui OBJECT ${IMGUI_SRC})
  target_include_directories(imgui PRIVATE ${IMGUI_DIR})

  list(APPEND PROJ_INCLUDES ${IMGUI_DIR})
  list(APPEND PROJ_LIBRARIES imgui)
endif()

## Target things
add_executable(${PROJECT_NAME} ${PROJ_SRC})
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 17)

target_include_directories(${PROJECT_NAME} PRIVATE ${PROJ_INCLUDES} include)
target_compile_options(${PROJECT_NAME} PRIVATE -Wpedantic -Wall -Wextra -Wshadow -Wunused -Wnon-virtual-dtor)
target_link_libraries(${PROJECT_NAME} PRIVATE ${PROJ_LIBRARIES})
