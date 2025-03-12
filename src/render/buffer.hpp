#pragma once

#include "./forward.hpp"

namespace ntf {

SHOGLE_DECLARE_RENDER_HANDLE(r_buffer_handle);

enum class r_buffer_type : uint8 {
  vertex = 0,
  index,
  texel,
  uniform,
  shader_storage,
};

enum class r_buffer_flag : uint8 {
  none              = 0,
  dynamic_storage   = 1 << 0,
  // read_mappable     = 1 << 1,
  // write_mappable    = 1 << 2,
  // rw_mappable       = (1<<1) | (1<<2),
};
NTF_DEFINE_ENUM_CLASS_FLAG_OPS(r_buffer_flag);

struct r_buffer_data {
  const void* data;
  size_t size;
  size_t offset;
};

struct r_buffer_descriptor {
  r_buffer_type type;
  r_buffer_flag flags;
  size_t size;

  weak_ref<r_buffer_data> data;
};

struct r_buffer_binding {
  r_buffer_handle buffer;
  r_buffer_type type;
  optional<uint32> location;
};

} // namespace ntf
