cmake_minimum_required(VERSION 3.10)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -Wall -Wextra -Wpedantic -O0 -g2 -ggdb -Wno-psabi")
set(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -Wall -Wextra -Wpedantic -O3 -Wno-psabi")
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(hello_cirno CXX C)

set(LIBS_INCLUDE)
set(LIBS_LINK)

set(SHOGLE_LIB "${CMAKE_CURRENT_SOURCE_DIR}/../")
# add_subdirectory(${SHOGLE_LIB})
list(APPEND LIBS_INCLUDE "${SHOGLE_LIB}/include")
list(APPEND LIBS_LINK shogle)

# file(GLOB_RECURSE SOURCE_FILES "src/*.cpp")
add_executable(${PROJECT_NAME} "hello_cirno.cpp")
target_include_directories(${PROJECT_NAME} PUBLIC src ${LIBS_INCLUDE})
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 20)
target_link_libraries(${PROJECT_NAME} ${LIBS_LINK})
