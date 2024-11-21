cmake_minimum_required(VERSION 3.10)

set(SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}")

file(GLOB GL_SOURCE "${SRC_DIR}/render/opengl/*.cpp")

configure_file("${SRC_DIR}/version.hpp.in" "version.hpp")
set(SHOGLE_SOURCE ${GL_SOURCE} PARENT_SCOPE)

# set(SHOGLE_INCLUDE "${SRC_DIR}/shogle.hpp" PARENT_SCOPE)
set(SHOGLE_INCLUDE ${SRC_DIR} PARENT_SCOPE)
