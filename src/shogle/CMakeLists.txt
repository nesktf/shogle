cmake_minimum_required(VERSION 3.10)

set(SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}")

file(GLOB GL_SOURCE "${SRC_DIR}/render/opengl/*.cpp")

set(SHOGLE_SOURCE ${GL_SOURCE} PARENT_SCOPE)
