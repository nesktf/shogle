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
  static constexpr size_t attr_stride() { return sizeof(pnt_vertex); }
  static constexpr auto attr_descriptor(const std::array<uint32, 3>& binds = {0, 0, 0}) {
    return pnt_vertex::attrib_descriptor(binds);
  }

public:
  std::array<r_buffer_binding, 2u> bindings(optional<uint32> vbo_location = nullopt,
                                            optional<uint32> ebo_location = nullopt) const {
    return {{
      {.buffer = _vbo.handle(), .type = r_buffer_type::vertex, .location = vbo_location},
      {.buffer = _ebo.handle(), .type = r_buffer_type::index,  .location = ebo_location},
    }};
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
  static constexpr size_t attr_stride() { return sizeof(pnt_vertex); }
  static constexpr auto attr_descriptor(const std::array<uint32, 3>& binds = {0, 0, 0}) {
    return pnt_vertex::attrib_descriptor(binds);
  }

public:
  std::array<r_buffer_binding, 1u> bindings(optional<uint32> vbo_location = nullopt) const {
                                            // optional<uint32> ebo_location = nullopt) const {
    return {{
      {.buffer = _vbo.handle(), .type = r_buffer_type::vertex, .location = vbo_location},
      // {.buffer = _ebo.handle(), .type = r_buffer_type::index,  .location = ebo_location},
    }};
  }
  r_buffer_view vbo() const { return _vbo; }
  // r_buffer_view ebo() const { return _ebo;}

private:
  renderer_buffer _vbo;
  // renderer_buffer _ebo;
};

} // namespace ntf
