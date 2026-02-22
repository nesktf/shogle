cmake_minimum_required(VERSION 3.25)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -Wall -Wextra -Wpedantic -O0 -g2 -ggdb -Wno-psabi")
set(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -Wall -Wextra -Wpedantic -O3 -Wno-psabi")

set(RES_FOLDER "${SHOGLE_DIR}/res")
set(EXAMPLE_LIST textured_quad)

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

foreach(EXAMPLE_NAME ${EXAMPLE_LIST})
  project(${EXAMPLE_NAME} CXX)

  add_executable(${EXAMPLE_NAME} ${EXAMPLE_NAME}.cpp)
  target_include_directories(${EXAMPLE_NAME} PRIVATE ${SHOGLE_INCLUDE_DIR} ${SHOGLE_EXTERN_INCLUDE})
  set_target_properties(${EXAMPLE_NAME} PROPERTIES CXX_STANDARD 20)
  target_link_libraries(${EXAMPLE_NAME} PRIVATE shogle chimatools)
  target_compile_definitions(${EXAMPLE_NAME} PRIVATE RES_FOLDER=\"${RES_FOLDER}\")
endforeach()
