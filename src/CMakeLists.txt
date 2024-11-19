cmake_minimum_required(VERSION 3.10)

project(shogle VERSION ${SHOGLE_VERSION} LANGUAGES CXX C)

set(SHOGLE_OPTS)
list(APPEND SHOGLE_OPTS -DSHOGLE_VERSION="${SHOGLE_VERSION}")

if (SHOGLE_ENABLE_IMGUI)
  message(STATUS "ShOGLE: Including dear imgui support")
  list(APPEND SHOGLE_OPTS -DSHOGLE_ENABLE_IMGUI="${SHOGLE_ENABLE_IMGUI}")
endif()

add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/extern")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/shogle")

add_library(${PROJECT_NAME} STATIC ${SHOGLE_SOURCE} ${EXTERN_SOURCE})
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 20)
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${EXTERN_INCLUDE})
target_link_libraries(${PROJECT_NAME} PUBLIC ${EXTERN_LINK})
target_compile_options(${PROJECT_NAME} PUBLIC ${SHOGLE_OPTS})
