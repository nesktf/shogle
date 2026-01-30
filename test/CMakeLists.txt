cmake_minimum_required(VERSION 3.25)
include(FetchContent)

project(shogle_test CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -Wall -Wextra -Wpedantic -O0 -g2 -ggdb -Wno-psabi")
set(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -Wall -Wextra -Wpedantic -O3 -Wno-psabi")

file(GLOB SHOGLE_TEST_SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
file(GLOB SHOGLE_TEST_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

message(STATUS "ShOGLE Test: Fetching Catch2 contents...")
FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG v3.8.1
)
FetchContent_MakeAvailable(Catch2)

message(STATUS "ShOGLE Test: Setting up...")
set(CATCH_BUILD_TESTING OFF)
enable_testing()

add_executable(${PROJECT_NAME} PUBLIC ${SHOGLE_TEST_SOURCE} ${SHOGLE_TEST_HEADERS})
target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../include/")
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 20)
target_link_libraries(${PROJECT_NAME} PRIVATE shogle Catch2::Catch2WithMain)

list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
include(CTest)
include(Catch)
catch_discover_tests(${PROJECT_NAME})
