cmake_minimum_required(VERSION 3.25)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -Wall -Wextra -Wpedantic -O0 -g2 -ggdb -Wno-psabi")
set(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -Wall -Wextra -Wpedantic -O3 -Wno-psabi")

set(DEMO_DIRS)
set(DEMO_DIRS hello_quad)

# freetype
#find_package(Freetype REQUIRED)
#list(APPEND EXTERN_INCLUDE ${FREETYPE_INCLUDE_DIRS})
#list(APPEND EXTERN_LINK freetype)

# assimp
#find_package(assimp REQUIRED)
#list(APPEND EXTERN_INCLUDE ${ASSIMP_INCLUDE_DIRS})
#list(APPEND EXTERN_LINK assimp)

# chimatools
message(STATUS "ShOGLE: Fetching chimatools")
FetchContent_Declare(
  chimatools
  GIT_REPOSITORY https://github.com/nesktf/chimatools.git
  GIT_TAG 64086978def2957458dbca1f0e28ec3efb326e9f # 1.0.0
  GIT_PROGRESS TRUE
  FIND_PACKAGE_ARGS NAMES chimatools
)
FetchContent_MakeAvailable(chimatools)

foreach(DEMO_NAME ${DEMO_DIRS})
  #get_filename_component(DEMO_NAME ${DEMO_DIR} NAME_WE)
  project(${DEMO_NAME} CXX)
  message(STATUS "ShOGLE: Setting up demo \"${DEMO_NAME}\"...")

  file(GLOB DEMO_SOURCES "${SHOGLE_DEMOS_DIR}/${DEMO_NAME}*.cpp")
  file(GLOB DEMO_INCLUDES "${SHOGLE_DEMOS_DIR}/${DEMO_NAME}*.hpp")

  add_executable(${DEMO_NAME} ${DEMO_SOURCES} ${DEMO_INCLUDES})
  target_include_directories(${DEMO_NAME} PRIVATE ${SHOGLE_INCLUDE_DIR} ${SHOGLE_EXTERN_INCLUDE})
  set_target_properties(${DEMO_NAME} PROPERTIES CXX_STANDARD 20)
  target_link_libraries(${DEMO_NAME} PRIVATE shogle chimatools)
endforeach()
