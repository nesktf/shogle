#pragma once

#include "./forward.hpp"
#include "./shader.hpp"
#include "./attribute.hpp"

namespace ntf {

SHOGLE_DECLARE_RENDER_HANDLE(r_pipeline_handle);

enum class r_primitive : uint8 {
  triangles = 0,
  triangle_strip,
  triangle_fan,
  lines,
  line_strip,
  points,
};

enum class r_polygon_mode : uint8 {
  fill = 0,
  point,
  line,
};

enum class r_compare_op : uint8 {
  never = 0,
  always,
  less,
  greater,
  equal,
  lequal,
  gequal,
  nequal,
};

enum class r_front_face : uint8 {
  clockwise = 0,
  counter_clockwise,
};

enum class r_cull_mode : uint8 {
  front = 0,
  back,
  front_back,
};

enum class r_pipeline_test : uint8 {
  none          = 0,
  depth         = 1 << 0,
  stencil       = 1 << 1,
  scissor       = 1 << 2,
  depth_stencil = (1<<0) | (1<<1),
  all           = (1<<0) | (1<<1) | (1<<2),
};
NTF_DEFINE_ENUM_CLASS_FLAG_OPS(r_pipeline_test);

struct r_pipeline_descriptor {
  span_view<r_shader_handle> stages;

  weak_ref<r_attrib_binding> attrib_binding;
  span_view<r_attrib_descriptor> attrib_desc;

  r_primitive primitive;
  r_polygon_mode poly_mode;
  r_front_face front_face;
  r_cull_mode cull_mode;

  r_pipeline_test tests;
  optional<r_compare_op> depth_compare_op;
  optional<r_compare_op> stencil_compare_op;
};

SHOGLE_DECLARE_RENDER_HANDLE(r_uniform);

// struct r_uniform_descriptor {
//   r_attrib_type type;
//   r_uniform location;
//   const void* data;
// };

} // namespace ntf
