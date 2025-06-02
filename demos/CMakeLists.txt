cmake_minimum_required(VERSION 3.11)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -Wall -Wextra -Wpedantic -O0 -g2 -ggdb -Wno-psabi")
set(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -Wall -Wextra -Wpedantic -O3 -Wno-psabi")

set(DEMO_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(SHOGLE_INC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../include")
file(GLOB SHOGLE_DEMO_SRC "${DEMO_SRC_DIR}/*.cpp")
file(GLOB SHOGLE_DEMO_INC "${DEMO_SRC_DIR}/*.hpp")

foreach(DEMO_SRC ${SHOGLE_DEMO_SRC})
  get_filename_component(DEMO_NAME ${DEMO_SRC} NAME_WE)
  project(${DEMO_NAME} CXX)
  message(STATUS "ShOGLE: Setting up demo \"${DEMO_NAME}\"...")

  add_executable(${DEMO_NAME} ${DEMO_SRC})
  target_include_directories(${DEMO_NAME} PUBLIC ${DEMO_SRC_DIR} ${SHOGLE_INC_DIR})
  set_target_properties(${DEMO_NAME} PROPERTIES CXX_STANDARD 20)
  target_link_libraries(${DEMO_NAME} shogle)
endforeach()
