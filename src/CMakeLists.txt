cmake_minimum_required(VERSION 3.25)

set(SOURCES)
set(INCLUDES)

list(APPEND SOURCES
    "${SHOGLE_SOURCE_DIR}/util/filesystem.cpp"
    "${SHOGLE_SOURCE_DIR}/util/logger.cpp"
    "${SHOGLE_SOURCE_DIR}/util/memory.cpp"
    "${SHOGLE_SOURCE_DIR}/render/window.cpp")

list(APPEND INCLUDES
    "${SHOGLE_INCLUDE_DIR}/shogle/core.hpp"

    "${SHOGLE_INCLUDE_DIR}/shogle/util/concepts.hpp"
    "${SHOGLE_INCLUDE_DIR}/shogle/util/expected.hpp"
    "${SHOGLE_INCLUDE_DIR}/shogle/util/filesystem.hpp"
    "${SHOGLE_INCLUDE_DIR}/shogle/util/logger.hpp"
    "${SHOGLE_INCLUDE_DIR}/shogle/util/optional.hpp"
    "${SHOGLE_INCLUDE_DIR}/shogle/util/ptr.hpp"

    "${SHOGLE_INCLUDE_DIR}/shogle/math/common.hpp"
    "${SHOGLE_INCLUDE_DIR}/shogle/math/aabb.hpp"
    "${SHOGLE_INCLUDE_DIR}/shogle/math/calc.hpp"
    "${SHOGLE_INCLUDE_DIR}/shogle/math/matrix3x3.hpp"
    "${SHOGLE_INCLUDE_DIR}/shogle/math/matrix3x3.inl"
    "${SHOGLE_INCLUDE_DIR}/shogle/math/matrix4x4.hpp"
    "${SHOGLE_INCLUDE_DIR}/shogle/math/matrix4x4.inl"
    "${SHOGLE_INCLUDE_DIR}/shogle/math/quaternion.hpp"
    "${SHOGLE_INCLUDE_DIR}/shogle/math/quaternion.inl"
    "${SHOGLE_INCLUDE_DIR}/shogle/math/vector2.hpp"
    "${SHOGLE_INCLUDE_DIR}/shogle/math/vector2.inl"
    "${SHOGLE_INCLUDE_DIR}/shogle/math/vector3.hpp"
    "${SHOGLE_INCLUDE_DIR}/shogle/math/vector3.inl"
    "${SHOGLE_INCLUDE_DIR}/shogle/math/vector4.hpp"
    "${SHOGLE_INCLUDE_DIR}/shogle/math/vector4.inl"
    "${SHOGLE_INCLUDE_DIR}/shogle/math/transform.hpp"
    "${SHOGLE_INCLUDE_DIR}/shogle/math/camera.hpp"

    "${SHOGLE_INCLUDE_DIR}/shogle/render/common.hpp"
    "${SHOGLE_INCLUDE_DIR}/shogle/render/data.hpp"
    "${SHOGLE_INCLUDE_DIR}/shogle/render/window.hpp")

if (SHOGLE_ENABLE_OPENGL)
  list(APPEND SOURCES
      "${SHOGLE_SOURCE_DIR}/render/gl/loader.c"
      "${SHOGLE_SOURCE_DIR}/render/gl/context.cpp"
      "${SHOGLE_SOURCE_DIR}/render/gl/buffer.cpp"
      "${SHOGLE_SOURCE_DIR}/render/gl/framebuffer.cpp"
      "${SHOGLE_SOURCE_DIR}/render/gl/pipeline.cpp"
      "${SHOGLE_SOURCE_DIR}/render/gl/texture.cpp"
      "${SHOGLE_SOURCE_DIR}/render/gl/vertex.cpp")

  list(APPEND INCLUDES
      "${SHOGLE_INCLUDE_DIR}/shogle/render/gl/gldefs.h"
      "${SHOGLE_INCLUDE_DIR}/shogle/render/gl/loader.h"
      "${SHOGLE_INCLUDE_DIR}/shogle/render/gl/common.hpp"
      "${SHOGLE_INCLUDE_DIR}/shogle/render/gl/context.hpp"
      "${SHOGLE_INCLUDE_DIR}/shogle/render/gl/buffer.hpp"
      "${SHOGLE_INCLUDE_DIR}/shogle/render/gl/buffer.inl"
      "${SHOGLE_INCLUDE_DIR}/shogle/render/gl/framebuffer.hpp"
      "${SHOGLE_INCLUDE_DIR}/shogle/render/gl/pipeline.hpp"
      "${SHOGLE_INCLUDE_DIR}/shogle/render/gl/texture.hpp"
      "${SHOGLE_INCLUDE_DIR}/shogle/render/gl/texture.inl"
      "${SHOGLE_INCLUDE_DIR}/shogle/render/gl/vertex.hpp"
      "${SHOGLE_INCLUDE_DIR}/shogle/render/gl/vertex.inl"
      "${SHOGLE_INCLUDE_DIR}/shogle/render/opengl.hpp")
endif()

set(SHOGLE_SOURCES ${SOURCES} PARENT_SCOPE)
set(SHOGLE_INCLUDES ${INCLUDES} PARENT_SCOPE)
