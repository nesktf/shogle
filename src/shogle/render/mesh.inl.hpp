#define SHOGLE_MESH_INL_HPP
#include <shogle/render/mesh.hpp>
#undef SHOGLE_MESH_INL_HPP

namespace ntf::shogle {

template<typename T, typename... attribs>
mesh& mesh::add_vertex_buffer(T* vertices, size_t vert_sz, attribs... attrib) {
  static_assert(sizeof...(attribs) > 0 && (... && is_shader_attribute<attribs>), "Invalid attributes");
  constexpr auto stride = stride_sum<attribs...>::value;

  _draw_count = vert_sz / stride;
  _attrib_count = sizeof...(attribs);

  glGenBuffers(1, &_vbo);
  glBindVertexArray(_vao);

  glBindBuffer(GL_ARRAY_BUFFER, _vbo);
  glBufferData(GL_ARRAY_BUFFER, vert_sz, vertices, GL_STATIC_DRAW);

  setup_vertex_attrib<stride>(attrib...);

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

} // namespace ntf::shogle
