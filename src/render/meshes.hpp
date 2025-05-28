#pragma once

#include "./vertex.hpp"
#include "./buffer.hpp"

namespace ntf {

class quad_mesh {
public:
  using attr_type = pnt_vertex;

private:
  quad_mesh(renderer_buffer vbo, renderer_buffer ebo, bool inverted) noexcept :
    _vbo{std::move(vbo)}, _ebo{std::move(ebo)}, _inverted_uvs{inverted} {}

public:
  r_expected<quad_mesh> static create(r_context_view ctx,
                                      bool inverted_uvs = false, bool dynamic_storage = false);

public:
  static constexpr auto attribute_binding() { return pnt_vertex::aos_binding(); }

public:
  r_buffer_binding bindings(cspan<r_shader_buffer> shader_buffers = {}) const {
    return { .vertex = _vbo.handle(), .index = _ebo.handle(), .shader = shader_buffers };
  }
  r_buffer_view vbo() const { return _vbo; }
  r_buffer_view ebo() const { return _ebo; }
  bool inverted_uvs() const { return _inverted_uvs; }

private:
  renderer_buffer _vbo, _ebo;
  bool _inverted_uvs;
};

class cube_mesh {
public:
  using attr_type = pnt_vertex;

private:
  cube_mesh(renderer_buffer vbo /*, renderer_buffer ebo*/) noexcept :
    _vbo{std::move(vbo)} /*, _ebo{std::move(ebo)} */{}

public:
  r_expected<cube_mesh> static create(r_context_view ctx, bool dynamic_storage = false);

public:
  static constexpr auto attribute_binding() { return pnt_vertex::aos_binding(); }

public:
  r_buffer_binding bindings(cspan<r_shader_buffer> shader_buffers = {}) const {
    return { .vertex = _vbo.handle(), .index = nullptr, .shader = shader_buffers };
  }
  r_buffer_view vbo() const { return _vbo; }
  // r_buffer_view ebo() const { return _ebo;}

private:
  renderer_buffer _vbo;
  // renderer_buffer _ebo;
};

} // namespace ntf
