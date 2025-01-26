cmake_minimum_required(VERSION 3.10)

set(SHOGLE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
set(SRC_DIR "${SHOGLE_DIR}/src")
set(EXTERN_DIR "${SHOGLE_DIR}/extern")
set(DEMOS_DIR "${SHOGLE_DIR}/demos")
set(TESTS_DIR "${SHOGLE_DIR}/tests")

set(SHOGLE_VER_MAJ 0)
set(SHOGLE_VER_MIN 2)
set(SHOGLE_VER_REV 1)
set(SHOGLE_VERSION "${SHOGLE_VER_MAJ}.${SHOGLE_VER_MIN}.${SHOGLE_VER_REV}")

execute_process(COMMAND git rev-parse --short HEAD
  WORKING_DIRECTORY "${SHOGLE_DIR}"
  OUTPUT_VARIABLE SHOGLE_VER_GIT_TAG
  OUTPUT_STRIP_TRAILING_WHITESPACE
)
execute_process(COMMAND git rev-list --count HEAD
  WORKING_DIRECTORY "${SHOGLE_DIR}"
  OUTPUT_VARIABLE SHOGLE_VER_BUILD
  OUTPUT_STRIP_TRAILING_WHITESPACE
)
execute_process(COMMAND git rev-parse --abbrev-ref HEAD
  WORKING_DIRECTORY "${SHOGLE_DIR}"
  OUTPUT_VARIABLE SHOGLE_VER_GIT_BRANCH
  OUTPUT_STRIP_TRAILING_WHITESPACE
)
set(SHOGLE_VERSION_STRING
  "${SHOGLE_VERSION}-${SHOGLE_VER_BUILD}-${SHOGLE_VER_GIT_TAG} (${SHOGLE_VER_GIT_BRANCH})")


message(STATUS "ShOGLE: Version ${SHOGLE_VERSION_STRING}")
project(shogle VERSION ${SHOGLE_VERSION} LANGUAGES CXX C)


set(SHOGLE_DEFS)
if (SHOGLE_ENABLE_IMGUI)
  message(STATUS "ShOGLE: Including dear imgui support")
  list(APPEND SHOGLE_DEFS SHOGLE_ENABLE_IMGUI=1)
endif()


message(STATUS "ShOGLE: Configuring dependencies")
add_subdirectory(${EXTERN_DIR})

message(STATUS "ShOGLE: Configuring source files")
add_subdirectory(${SRC_DIR})


add_library(${PROJECT_NAME} STATIC ${SHOGLE_SOURCE} ${EXTERN_SOURCE})
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 20)
target_include_directories(${PROJECT_NAME} PUBLIC
  ${SHOGLE_INCLUDE} ${EXTERN_INCLUDE} "${PROJECT_BINARY_DIR}/src")
target_link_libraries(${PROJECT_NAME} PRIVATE ${EXTERN_LINK})
target_compile_definitions(${PROJECT_NAME} PUBLIC ${SHOGLE_DEFS})


# if (SHOGLE_BUILD_TESTS)
#   message(STATUS "ShOGLE: Building tests")
#   add_subdirectory(${TESTS_DIR})
# endif()

if (SHOGLE_BUILD_DEMOS)
  message(STATUS "ShOGLE: Building demos")
  add_subdirectory(${DEMOS_DIR})
endif()
