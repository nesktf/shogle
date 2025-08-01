#pragma once

#include <shogle/render/buffer.hpp>
#include <shogle/render/vertex.hpp>

namespace shogle {

class quad_mesh {
public:
  using attribute_type = pnt_vertex;

private:
  explicit quad_mesh(vertex_buffer&& vbo, index_buffer&& ebo, bool inv) noexcept :
    _vbo{std::move(vbo)}, _ebo{std::move(ebo)}, _inv{inv}
  {
    _bind[0].layout = 0u;
    _bind[0].buffer = _vbo.get();
    _bind[1].layout = 1u;
    _bind[1].buffer = _vbo.get();
    _bind[2].layout = 2u;
    _bind[2].buffer = _vbo.get();
  }

public:
  render_expect<quad_mesh> static create(context_view ctx,
                                  bool inverted_uvs = false, bool dynamic_storage = false);

public:
  static constexpr auto attribute_binding() { return pnt_vertex::aos_binding(); }

public:
  buffer_binding bindings(span<const shader_binding> shader_binds = {}) const {
    return { .vertex = _bind, .index = _ebo.get(), .shader = shader_binds };
  }
  vertex_buffer_view vertex() const { return _vbo; }
  index_buffer_view index() const { return _ebo; }
  bool inverted_uvs() const { return _inv; }

private:
  vertex_buffer _vbo;
  index_buffer _ebo;
  vertex_binding _bind[3u];
  bool _inv;
};

class cube_mesh {
public:
  using attribute_type = pnt_vertex;

private:
  cube_mesh(vertex_buffer&& vbo) noexcept :
    _vbo{std::move(vbo)}
  {
    _bind[0].layout = 0u;
    _bind[0].buffer = _vbo.get();
    _bind[1].layout = 1u;
    _bind[1].buffer = _vbo.get();
    _bind[2].layout = 2u;
    _bind[2].buffer = _vbo.get();
  }

public:
  render_expect<cube_mesh> static create(context_view ctx, bool dynamic_storage = false);

public:
  static constexpr auto attribute_binding() { return pnt_vertex::aos_binding(); }

public:
  buffer_binding bindings(span<const shader_binding> shader_binds = {}) const {
    return { .vertex = _bind, .index = nullptr, .shader = shader_binds };
  }
  vertex_buffer_view vertex() const { return _vbo; }

private:
  vertex_buffer _vbo;
  vertex_binding _bind[3u];
};

} // namespace shogle
