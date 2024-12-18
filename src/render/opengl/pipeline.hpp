#pragma once

#include "./shader.hpp"

#include <optional>

namespace ntf {

class gl_pipeline {
private:
  gl_pipeline(gl_context& ctx) :
    _ctx(ctx) {}

private:
  void load(const gl_shader** shaders, uint32 shader_count,
            const r_attrib_info* attribs, uint32 attrib_ccount);
  void unload();

private:
  void bind(bool with_program);
  void uniform(uint32 location, float32 value);
  void uniform(uint32 location, vec2 value);
  void uniform(uint32 location, vec3 value);
  void uniform(uint32 location, vec4 value);
  void uniform(uint32 location, mat3 value);
  void uniform(uint32 location, mat4 value);
  void uniform(uint32 location, int32 value);
  void uniform(uint32 location, float64 value);

public:
  r_shader_type enabled_shaders() const { return _enabled_shaders; }
  std::optional<uint32> uniform_location(std::string_view name) const;

private:
  gl_context& _ctx;

  GLuint _program_id{0};
  r_shader_type _enabled_shaders{r_shader_type::none};
  std::vector<r_attrib_info> _attribs;
  size_t _attrib_stride{0};

public:
  NTF_DISABLE_MOVE_COPY(gl_pipeline);

private:
  friend class gl_context;
};

} // namespace ntf
