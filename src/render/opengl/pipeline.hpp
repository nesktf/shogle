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
            const r_attrib_descriptor* attribs, uint32 attrib_count);
  void unload();

private:
  static void push_uniform(uint32 location, r_attrib_type type, const void* data);

public:
  r_shader_type enabled_shaders() const { return _enabled_shaders; }
  std::optional<uint32> uniform_location(std::string_view name) const;

private:
  bool complete() const { return _program_id != 0; }

private:
  gl_context& _ctx;

  GLuint _program_id{0};
  r_shader_type _enabled_shaders{r_shader_type::none};
  std::vector<r_attrib_descriptor> _attribs;
  size_t _stride{0};

private:
  friend class gl_context;
};

} // namespace ntf
