cmake_minimum_required(VERSION 3.10)

message(STATUS "ShOGLE: Configuring version ${SHOGLE_VERSION}")

## Vars ##
set(SHOGLE_PATH ${CMAKE_CURRENT_SOURCE_DIR})
set(EXTERN_PATH ${SHOGLE_PATH}/extern)

set(SHOGLE_SOURCE)
set(SHOGLE_INCLUDE)
set(SHOGLE_LIB_LINK)
set(SHOGLE_LIB_SOURCE)
set(SHOGLE_LIB_INCLUDE)

## Compiled libs ##
list(APPEND SHOGLE_LIB_INCLUDE ${EXTERN_PATH})

# stb_image
list(APPEND SHOGLE_LIB_SOURCE ${EXTERN_PATH}/stb/stb_image.c)

# glad
list(APPEND SHOGLE_LIB_SOURCE ${EXTERN_PATH}/glad/glad.c)

# imgui
set(IMGUI_DIR ${EXTERN_PATH}/imgui)
file(GLOB IMGUI_SRC "${IMGUI_DIR}/imgui*.cpp")
list(APPEND IMGUI_SRC ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp)
list(APPEND IMGUI_SRC ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp)
list(APPEND SHOGLE_LIB_SOURCE ${IMGUI_SRC})
list(APPEND SHOGLE_LIB_INCLUDE ${IMGUI_DIR} "${IMGUI_DIR}/backends")

## Packaged libs ##
find_package(PkgConfig REQUIRED)

# glm
find_package(glm CONFIG REQUIRED)
list(APPEND SHOGLE_LIB_INCLUDE ${GLM_INCLUDE_DIRS})
list(APPEND SHOGLE_LIB_LINK glm::glm)

# assimp
find_package(assimp REQUIRED)
list(APPEND SHOGLE_LIB_INCLUDE ${ASSIMP_INCLUDE_DIRS})
list(APPEND SHOGLE_LIB_LINK assimp)

# fmt
find_package(fmt REQUIRED)
list(APPEND SHOGLE_LIB_INCLUDE ${FMT_INCLUDE_DIRS})
list(APPEND SHOGLE_LIB_LINK fmt)

# glfw
pkg_search_module(GLFW REQUIRED glfw3)
list(APPEND SHOGLE_LIB_INCLUDE ${GLFW_INCLUDE_DIRS})
list(APPEND SHOGLE_LIB_LINK glfw)

# lua
pkg_search_module(Lua REQUIRED lua5.3)
list(APPEND SHOGLE_LIB_INCLUDE /usr/include/lua5.3)
list(APPEND SHOGLE_LIB_LINK lua5.3)


## Lib files ##
file(GLOB_RECURSE SHOGLE_SOURCE "${SHOGLE_PATH}/src/*.cpp")
set(SHOGLE_INCLUDE "${SHOGLE_PATH}/include")

add_library(shogle STATIC ${SHOGLE_SOURCE} ${SHOGLE_LIB_SOURCE})
set_target_properties(shogle PROPERTIES CXX_STANDARD 20)
target_include_directories(shogle PUBLIC ${SHOGLE_INCLUDE} ${SHOGLE_LIB_INCLUDE})
target_link_libraries(shogle ${SHOGLE_LIB_LINK})
target_compile_options(shogle PUBLIC
  -DSHOGLE_RESOURCES="${SHOGLE_RESOURCES_PATH}"
  -DSHOGLE_VERSION="${SHOGLE_VERSION}"
)
