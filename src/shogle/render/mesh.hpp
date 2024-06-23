#pragma once

#include <shogle/render/render.hpp>

namespace ntf::shogle {

class mesh {
public:
  mesh();

public:
  template<typename T, typename... attribs>
  mesh& add_vertex_buffer(T* vertices, size_t vert_sz, attribs... attrib);

  mesh& add_index_buffer(uint* indices, size_t ind_sz);

public:
  GLuint id() const { return _vao; }

public:
  ~mesh();
  mesh(mesh&&) noexcept;
  mesh(const mesh&) = delete;
  mesh& operator=(mesh&&) noexcept;
  mesh& operator=(const mesh&) = delete;

private:
  template<size_t total_size, size_t next_stride = 0, typename curr_attrib, typename... attribs>
  void setup_vertex_attrib(curr_attrib, attribs... attrib);

  template<size_t, size_t>
  void setup_vertex_attrib() {}

private: 
  GLuint _vao{0};
  GLuint _vbo{0}, _ebo{0};
  size_t _draw_count{0}, _attrib_count{0};

private:
  friend void render_draw_mesh(const mesh& mesh);
};

void render_draw_mesh(const mesh& mesh);

} // namespace ntf::shogle

#ifndef SHOGLE_MESH_INL_HPP
#include <shogle/render/mesh.inl.hpp>
#endif
