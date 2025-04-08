cmake_minimum_required(VERSION 3.10)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -Wall -Wextra -Wpedantic -O0 -g2 -ggdb -Wno-psabi")
set(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -Wall -Wextra -Wpedantic -O3 -Wno-psabi")
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(shogle_tests CXX C)

set(LIBS_INCLUDE)
set(LIBS_LINK)

set(SHOGLE_LIB "${CMAKE_CURRENT_SOURCE_DIR}/../")
# add_subdirectory(${SHOGLE_LIB})
list(APPEND LIBS_INCLUDE "${SHOGLE_LIB}/include")
list(APPEND LIBS_LINK shogle)

set(GTEST_LIB "${CMAKE_CURRENT_SOURCE_DIR}/../extern/googletest")
add_subdirectory(${GTEST_LIB} "googletest-build")
list(APPEND LIBS_LINK GTest::gtest_main)

enable_testing()

file(GLOB_RECURSE SOURCE_FILES "./*.cpp")
add_executable(${PROJECT_NAME} ${SOURCE_FILES})
target_include_directories(${PROJECT_NAME} PRIVATE src ${LIBS_INCLUDE})
# set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 20)
target_link_libraries(${PROJECT_NAME} PRIVATE ${LIBS_LINK})

include(GoogleTest)
gtest_discover_tests(${PROJECT_NAME})
