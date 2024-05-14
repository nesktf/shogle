#pragma once

#include <shogle/render/gl/render.hpp>

#include <vector>
#include <concepts>

namespace ntf::shogle::gl {

template<typename... T>
struct stride_sum { static constexpr size_t value = 0; };

template<typename T, typename... U >
struct stride_sum<T, U...> { static constexpr size_t value = T::stride + stride_sum<U...>::value; };

class mesh {
public:
  mesh();

public:
  template<typename T, typename... attrib_t>
  mesh& add_vertex_buffer(T* vertices, size_t vert_sz, attrib_t... attrib);

  template<typename T, typename... attrib_t>
  mesh& add_vertex_buffer(std::vector<T> vertices, attrib_t... attrib);

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

protected:
  template<size_t, size_t>
  void setup_vertex_attrib() {}

  template<size_t total_size, size_t next_stride = 0, typename T, typename... U>
  void setup_vertex_attrib(T t, U... u);

private: 
  GLuint _vao{0};
  GLuint _vbo{0}, _ebo{0};
  size_t _draw_count{0}, _attrib_count{0};
};

template<typename T, typename... attrib_t>
mesh& mesh::add_vertex_buffer(T* vertices, size_t vert_sz, attrib_t... attrib) {
  _draw_count = vert_sz/sizeof(T);
  _attrib_count = sizeof...(attrib_t);

  glGenBuffers(1, &_vbo);
  glBindVertexArray(_vao);

  glBindBuffer(GL_ARRAY_BUFFER, _vbo);
  glBufferData(GL_ARRAY_BUFFER, vert_sz, vertices, GL_STATIC_DRAW);

  setup_vertex_attrib<sizeof(T)>(attrib...);

  glBindVertexArray(0);
  return *this;
}

template<typename T, typename... attrib_t>
mesh& mesh::add_vertex_buffer(std::vector<T> vertices, attrib_t... attrib) {
  static_assert(sizeof...(attrib) > 0 && stride_sum<attrib_t...>::value == sizeof(T));
  _draw_count = vertices.size();
  _attrib_count = sizeof...(attrib_t);

  glGenBuffers(1, &_vbo);
  glBindVertexArray(_vao);

  glBindBuffer(GL_ARRAY_BUFFER, _vbo);
  glBufferData(GL_ARRAY_BUFFER, vertices.size()*sizeof(T), vertices, GL_STATIC_DRAW);

  setup_vertex_attrib<sizeof(T)>(attrib...);

  glBindVertexArray(0);
  return *this;
}

template<size_t total_size, size_t next_stride = 0, typename T, typename... U>
void setup_vertex_attrib(T t, U... u) {
  constexpr auto curr_stride = next_stride;
  constexpr auto float_count = T::stride / sizeof(float);

  glEnableVertexAttribArray(T::index);
  glVertexAttribPointer(T::index, float_count, GL_FLOAT, GL_FALSE, total_size, (void*)curr_stride);

  setup_vertex_attrib<total_size,curr_stride+T::stride>(u...);
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
