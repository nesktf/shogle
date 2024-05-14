#pragma once

#include <shogle/render/gl/render.hpp>

#include <concepts>

namespace ntf::shogle::gl {

template<typename T, typename... TRes>
concept same_as_any = (... or std::same_as<T, TRes>);

template<typename T>
concept vertex_type = (same_as_any<T, vec2, vec3, vec4>);

class mesh {
public:
  mesh();

public:
  template<typename T, typename... attrib_t>
  mesh& add_vertex_buffer(T* vertices, size_t vert_sz, attrib_t... attrib);

  mesh& add_index_buffer(uint* indices, size_t ind_sz);
  void draw() const;

public:
  GLuint id() const { return _vao; }
  bool has_indices() const { return _ebo != 0; }
  size_t draw_count() const { return _draw_count; }

public:
  ~mesh();
  mesh(mesh&&) noexcept;
  mesh(const mesh&) = delete;
  mesh& operator=(mesh&&) noexcept;
  mesh& operator=(const mesh&) = delete;

private: 
  GLuint _vao{0};
  GLuint _vbo{0}, _ebo{0};
  size_t _draw_count{0}, _attrib_count{0};
};

template<typename T, typename... attrib_t>
mesh& mesh::add_vertex_buffer(T* vertices, size_t vert_sz, attrib_t... attrib) {
  _draw_count = vert_sz;
  _attrib_count = sizeof...(attrib_t);

  glGenBuffers(1, &_vbo);
  glBindVertexArray(_vao);

  glBindBuffer(GL_ARRAY_BUFFER, _vbo);
  glBufferData(GL_ARRAY_BUFFER, vert_sz*sizeof(T), vertices, GL_STATIC_DRAW);

  glBindVertexArray(0);
  return *this;
}

// struct vertex2d {
//   vec2 coord;
//   vec2 tex_coord;
// };
//
// struct vertex3d {
//   vec3 coord;
//   vec3 normal;
//   vec2 tex_coord;
// };
//
// struct vertex3dn {
//   vec3 coord;
//   vec3 normal;
// };

} // namespace ntf::shogle::gl
