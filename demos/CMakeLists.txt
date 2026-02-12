cmake_minimum_required(VERSION 3.25)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -Wall -Wextra -Wpedantic -O0 -g2 -ggdb -Wno-psabi")
set(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -Wall -Wextra -Wpedantic -O3 -Wno-psabi")

set(DEMO_DIRS)
set(DEMO_DIRS hello_quad)

foreach(DEMO_NAME ${DEMO_DIRS})
  #get_filename_component(DEMO_NAME ${DEMO_DIR} NAME_WE)
  project(${DEMO_NAME} CXX)
  message(STATUS "ShOGLE: Setting up demo \"${DEMO_NAME}\"...")

  file(GLOB DEMO_SOURCES "${SHOGLE_DEMOS_DIR}/${DEMO_NAME}*.cpp")
  file(GLOB DEMO_INCLUDES "${SHOGLE_DEMOS_DIR}/${DEMO_NAME}*.hpp")

  add_executable(${DEMO_NAME} ${DEMO_SOURCES} ${DEMO_INCLUDES})
  target_include_directories(${DEMO_NAME} PRIVATE ${SHOGLE_INCLUDE_DIR} ${SHOGLE_EXTERN_INCLUDE})
  set_target_properties(${DEMO_NAME} PROPERTIES CXX_STANDARD 20)
  target_link_libraries(${DEMO_NAME} PRIVATE shogle)
endforeach()
