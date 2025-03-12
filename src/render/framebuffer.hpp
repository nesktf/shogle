#pragma once

#include "./forward.hpp"
#include "./texture.hpp"

namespace ntf {

SHOGLE_DECLARE_RENDER_HANDLE(r_framebuffer_handle);

enum class r_test_buffer_format : uint8 {
  depth16u = 0,
  depth24u,
  depth32f,
  depth24u_stencil8u,
  depth32f_stencil8u,
};

enum class r_test_buffer_flag : uint8 {
  none    = 0,
  depth   = 1 << 0,
  stencil = 1 << 1,
  both    = (1<<0) | (1<<1),
};
NTF_DEFINE_ENUM_CLASS_FLAG_OPS(r_test_buffer_flag);

enum class r_clear_flag : uint8 {
  none          = 0,
  color         = 1 << 0,
  depth         = 1 << 1,
  stencil       = 1 << 2,
  color_depth   = (1<<0) | (1<<1),
  color_stencil = (1<<0) | (1<<2),
  all           = (1<<0) | (1<<1) | (1<<2),
};
NTF_DEFINE_ENUM_CLASS_FLAG_OPS(r_clear_flag);

struct r_framebuffer_attachment {
  r_texture_handle handle;
  uint32 layer;
  uint32 level;
};

struct r_framebuffer_descriptor {
  uvec2 extent;
  uvec4 viewport;
  color4 clear_color;
  r_clear_flag clear_flags;

  r_test_buffer_flag test_buffers;
  optional<r_test_buffer_format> test_buffer_format;

  span_view<r_framebuffer_attachment> attachments;
  optional<r_texture_format> color_buffer_format;
};

} // namespace ntf
