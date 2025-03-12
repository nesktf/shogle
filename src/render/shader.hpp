#pragma once

#include "./forward.hpp"

namespace ntf {

SHOGLE_DECLARE_RENDER_HANDLE(r_shader_handle);

enum class r_shader_type : uint8 {
  vertex = 0,
  fragment,
  geometry,
  tesselation_eval,
  tesselation_control,
  compute,
};

enum class r_stages_flag : uint8 {
  none                = 0,
  vertex              = 1 << static_cast<uint8>(r_shader_type::vertex),
  fragment            = 1 << static_cast<uint8>(r_shader_type::fragment),
  geometry            = 1 << static_cast<uint8>(r_shader_type::geometry),
  tesselation_eval    = 1 << static_cast<uint8>(r_shader_type::tesselation_eval),
  tesselation_control = 1 << static_cast<uint8>(r_shader_type::tesselation_control),
  compute             = 1 << static_cast<uint8>(r_shader_type::compute),
};
NTF_DEFINE_ENUM_CLASS_FLAG_OPS(r_stages_flag);
constexpr r_stages_flag r_required_render_stages = r_stages_flag::vertex | r_stages_flag::fragment;

struct r_shader_descriptor {
  r_shader_type type;
  span_view<std::string_view> source;
};


} // namespace ntf
