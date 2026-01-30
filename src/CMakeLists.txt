cmake_minimum_required(VERSION 3.25)

file(GLOB_RECURSE SHOGLE_SOURCES "${SHOGLE_SOURCE_DIR}/**.cpp")
file(GLOB_RECURSE SHOGLE_INTERNAL_HEADERS "${SHOGLE_SOURCE_DIR}/**.hpp")
file(GLOB_RECURSE SHOGLE_HEADERS "${SHOGLE_INCLUDE_DIR}/**.hpp")
file(GLOB_RECURSE SHOGLE_INLINE "${SHOGLE_INCLUDE_DIR}/**.inl")

add_library(shogle ${SHOGLE_LIBRARY_TYPE}
                   ${SHOGLE_HEADERS} ${SHOGLE_INLINE}
                   ${SHOGLE_INTERNAL_HEADERS}
                   ${SHOGLE_SOURCES}
                   ${SHOGLE_EXTERN_INCLUDES}
                   ${SHOGLE_EXTERN_SOURCES})

configure_file("${SHOGLE_SOURCE_DIR}/version.hpp.in" "include/shogle/version.hpp" @ONLY)

target_include_directories(shogle PUBLIC
                           "$<BUILD_INTERFACE:${SHOGLE_SOURCE_DIR}/include>"
                           "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
                           ${SHOGLE_INC_DIR})

set_target_properties(shogle PROPERTIES LINKER_LANGUAGE CXX CXX_STANDARD 20)
target_link_libraries(shogle PUBLIC ${SHOGLE_EXTERN_LINK})
target_compile_definitions(${PROJECT_NAME} PUBLIC ${SHOGLE_DEFS})
