#pragma once

#include "./shader.hpp"

#include <optional>

namespace ntf {

class gl_pipeline {
private:
  gl_pipeline(gl_context& ctx) :
    _ctx(ctx) {}

private:
  void load(const gl_shader** shaders, uint32 shader_count, r_resource_handle attrib);
  void unload();

private:
  static void push_uniform(uint32 location, r_attrib_type type, const void* data);

public:
  r_shader_type enabled_shaders() const { return _enabled_shaders; }
  std::optional<uint32> uniform_location(std::string_view name) const;

private:
  gl_context& _ctx;

  GLuint _program_id{0};
  r_shader_type _enabled_shaders{r_shader_type::none};
  r_resource_handle _attrib_handle{r_resource_tombstone};

public:
  NTF_DISABLE_MOVE_COPY(gl_pipeline);

private:
  friend class gl_context;
};

} // namespace ntf
