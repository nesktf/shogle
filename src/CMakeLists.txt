cmake_minimum_required(VERSION 3.10)

set(SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}")

file(GLOB_RECURSE SOURCE_FILES "${SRC_DIR}/*.cpp")

configure_file("${SRC_DIR}/version.hpp.in" "shogle_version.hpp")
set(SHOGLE_SOURCE ${SOURCE_FILES} PARENT_SCOPE)

# set(SHOGLE_INCLUDE "${SRC_DIR}/shogle.hpp" PARENT_SCOPE)
set(SHOGLE_INCLUDE ${SRC_DIR} PARENT_SCOPE)
