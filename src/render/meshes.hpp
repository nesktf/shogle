#pragma once

#include "./vertex.hpp"
#include "./buffer.hpp"

namespace ntf::render {

class quad_mesh {
public:
  using attribute_type = pnt_vertex;

private:
  quad_mesh(vertex_buffer&& vbo, index_buffer&& ebo, bool inv) noexcept :
    _vbo{std::move(vbo)}, _ebo{std::move(ebo)}, _inv{inv} {}

public:
  expect<quad_mesh> static create(context_view ctx,
                                  bool inverted_uvs = false, bool dynamic_storage = false);

public:
  static constexpr auto attribute_binding() { return pnt_vertex::aos_binding(); }

public:
  buffer_binding bindings(cspan<shader_buffer> shader_buffers = {}) const {
    return { .vertex = _vbo.get(), .index = _ebo.get(), .shader = shader_buffers };
  }
  vertex_buffer_view vertex() const { return _vbo; }
  index_buffer_view index() const { return _ebo; }
  bool inverted_uvs() const { return _inv; }

private:
  vertex_buffer _vbo;
  index_buffer _ebo;
  bool _inv;
};

class cube_mesh {
public:
  using attribute_type = pnt_vertex;

private:
  cube_mesh(vertex_buffer&& vbo) noexcept :
    _vbo{std::move(vbo)} {}

public:
  expect<cube_mesh> static create(context_view ctx, bool dynamic_storage = false);

public:
  static constexpr auto attribute_binding() { return pnt_vertex::aos_binding(); }

public:
  buffer_binding bindings(cspan<shader_buffer> shader_buffers = {}) const {
    return { .vertex = _vbo.get(), .index = nullptr, .shader = shader_buffers };
  }
  vertex_buffer_view vertex() const { return _vbo; }

private:
  vertex_buffer _vbo;
};

} // namespace ntf::render
