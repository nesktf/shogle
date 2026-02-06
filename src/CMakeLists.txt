cmake_minimum_required(VERSION 3.25)

file(GLOB_RECURSE SHOGLE_RENDER_GL_SOURCES "${SHOGLE_SOURCE_DIR}/render/gl/**.cpp")
list(APPEND SHOGLE_RENDER_GL_SOURCES "${SHOGLE_SOURCE_DIR}/render/gl/loader.c")
set(SHOGLE_RENDER_SOURCES "${SHOGLE_SOURCE_DIR}/render/window.cpp" ${SHOGLE_RENDER_GL_SOURCES})

file(GLOB_RECURSE SHOGLE_UTIL_SOURCES "${SHOGLE_SOURCE_DIR}/util/**.cpp")

set(SHOGLE_SOURCES ${SHOGLE_UTIL_SOURCES} ${SHOGLE_RENDER_SOURCES})

file(GLOB_RECURSE SHOGLE_INTERNAL_HEADERS "${SHOGLE_SOURCE_DIR}/**.hpp")

file(GLOB_RECURSE SHOGLE_CPP_HEADERS "${SHOGLE_INCLUDE_DIR}/**.hpp")
file(GLOB_RECURSE SHOGLE_CPP_INLINE "${SHOGLE_INCLUDE_DIR}/**.inl")
file(GLOB_RECURSE SHOGLE_C_HEADERS "${SHOGLE_INCLUDE_DIR}/**.h")

add_library(shogle ${SHOGLE_LIBRARY_TYPE}
									 ${SHOGLE_C_HEADERS}
                   ${SHOGLE_CPP_HEADERS} ${SHOGLE_CPP_INLINE}
                   ${SHOGLE_INTERNAL_HEADERS}
                   ${SHOGLE_SOURCES}
                   ${SHOGLE_EXTERN_INCLUDES}
                   ${SHOGLE_EXTERN_SOURCES})

configure_file("${SHOGLE_SOURCE_DIR}/version.hpp.in" "include/shogle/version.hpp" @ONLY)

target_include_directories(shogle PUBLIC
                           "$<BUILD_INTERFACE:${SHOGLE_INCLUDE_DIR}>"
                           "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>")

set_target_properties(shogle PROPERTIES LINKER_LANGUAGE CXX CXX_STANDARD 20)
target_link_libraries(shogle PUBLIC ${SHOGLE_EXTERN_LINK})
target_compile_definitions(${PROJECT_NAME} PUBLIC ${SHOGLE_DEFS})
