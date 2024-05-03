message(STATUS "ShOGLE: Building examples")

function(shogle_build_example NAME)
  set(TARGET_NAME example-${NAME})
  add_executable(${TARGET_NAME} ${NAME}.cpp)
  set_target_properties(${TARGET_NAME} PROPERTIES CXX_STANDARD 20)
  target_link_libraries(${TARGET_NAME} PRIVATE shogle)
endfunction()

shogle_build_example(test_async)
# shogle_build_example(test_event)
shogle_build_example(test_imgui)
shogle_build_example(test_framebuffer)
