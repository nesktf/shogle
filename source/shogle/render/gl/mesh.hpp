#pragma once

#include <shogle/render/gl/shader_program.hpp>

#include <vector>
#include <concepts>

namespace ntf::shogle::gl {

template<typename... T>
struct stride_sum { static constexpr size_t value = 0; };

template<typename T, typename... U >
struct stride_sum<T, U...> { static constexpr size_t value = T::stride + stride_sum<U...>::value; };

template<class T>
concept is_attribute = requires(T x) { 
  { gl::shader_attribute{x} } -> std::same_as<T>;
};

class mesh {
public:
  mesh();

public:
  template<typename T, typename... attribs>
  requires(is_attribute<attribs> && ...)
  mesh& add_vertex_buffer(T* vertices, size_t vert_sz, attribs... attrib);

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

  template<size_t total_size, size_t next_stride = 0, typename curr_attrib, typename... attribs>
  void setup_vertex_attrib(curr_attrib, attribs... attrib);

private: 
  GLuint _vao{0};
  GLuint _vbo{0}, _ebo{0};
  size_t _draw_count{0}, _attrib_count{0};
};

template<typename T, typename... attribs>
requires(is_attribute<attribs> && ...)
mesh& mesh::add_vertex_buffer(T* vertices, size_t vert_sz, attribs... attrib) {
  static_assert(sizeof...(attribs) > 0);
  constexpr auto stride = stride_sum<attribs...>::value;

  _draw_count = vert_sz / stride;
  _attrib_count = sizeof...(attribs);

  glGenBuffers(1, &_vbo);
  glBindVertexArray(_vao);

  glBindBuffer(GL_ARRAY_BUFFER, _vbo);
  glBufferData(GL_ARRAY_BUFFER, vert_sz, vertices, GL_STATIC_DRAW);

  setup_vertex_attrib<stride>(attrib...); // TODO: Test on float arrays?

  glBindVertexArray(0);
  return *this;
}

template<size_t total_size, size_t next_stride, typename curr_attrib, typename... attribs>
void mesh::setup_vertex_attrib(curr_attrib, attribs... attrib) {
  constexpr auto curr_stride = next_stride;
  constexpr auto float_count = curr_attrib::stride / sizeof(float);

  glEnableVertexAttribArray(curr_attrib::index);
  glVertexAttribPointer(curr_attrib::index, float_count, GL_FLOAT, GL_FALSE, total_size, (void*)curr_stride);

  setup_vertex_attrib<total_size,curr_stride+curr_attrib::stride>(attrib...);
}

} // namespace ntf::shogle::gl
