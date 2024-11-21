#define SHOGLE_RENDER_OPENGL_MESH_INL
#include "./mesh.hpp"
#undef SHOGLE_RENDER_OPENGL_MESH_INL

namespace ntf {

template<typename T, is_shader_attribute... Attribs>
void gl_mesh::load(mesh_primitive primitive, T* vertices, std::size_t vert_sz,
                   mesh_buffer vert_buff, Attribs... attrib) {
  static_assert(sizeof...(Attribs) > 0, "Invalid attributes");
  NTF_ASSERT(!valid(), "gl_mesh already initialized");
  _primitive = primitive;

  glGenVertexArrays(1, &_vao);
  glGenBuffers(1, &_vbo);

  glBindVertexArray(_vao);
  glBindBuffer(GL_ARRAY_BUFFER, _vbo);
  glBufferData(GL_ARRAY_BUFFER, vert_sz, vertices, enumtogl(vert_buff));

  const constexpr auto stride = stride_sum<Attribs...>::value;
  _setup_vertex_attrib<stride>(attrib...);
  _draw_count = vert_sz / stride;
  _attrib_count = sizeof...(Attribs);

  glBindVertexArray(0);

  SHOGLE_LOG(verbose, "[ntf::gl_mesh] Mesh loaded (id: {})", _vao);
}

template<typename T, is_shader_attribute... Attribs>
void gl_mesh::load(mesh_primitive primitive, T* vertices, std::size_t vert_sz,
                   mesh_buffer vert_buff, uint* indices, std::size_t ind_sz,
                   mesh_buffer ind_buff, Attribs... attrib) {
  static_assert(sizeof...(Attribs) > 0, "Invalid attributes");
  NTF_ASSERT(!valid(), "gl_mesh already initialized");
  _primitive = primitive;

  glGenVertexArrays(1, &_vao);
  glGenBuffers(1, &_vbo);
  glGenBuffers(1, &_ebo);

  glBindVertexArray(_vao);
  glBindBuffer(GL_ARRAY_BUFFER, _vbo);
  glBufferData(GL_ARRAY_BUFFER, vert_sz, vertices, enumtogl(vert_buff));
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, ind_sz, indices, enumtogl(ind_buff));

  const constexpr auto stride = stride_sum<Attribs...>::value;
  _draw_count = ind_sz/sizeof(uint);
  _attrib_count = sizeof...(Attribs);

  _setup_vertex_attrib<stride>(attrib...);

  glBindVertexArray(0);

  SHOGLE_LOG(verbose, "[ntf::gl_mesh] Mesh loaded (id: {})", _vao);
}

template<std::size_t total_size, std::size_t next_stride,
  typename CurrAttrib, typename... Attribs>
void gl_mesh::_setup_vertex_attrib(CurrAttrib, Attribs... attrib) {
  constexpr auto curr_stride = next_stride;
  constexpr auto float_count = CurrAttrib::stride / sizeof(float);

  glEnableVertexAttribArray(CurrAttrib::index);
  glVertexAttribPointer(CurrAttrib::index, float_count, GL_FLOAT, GL_FALSE,
                        total_size, (void*)curr_stride);

  _setup_vertex_attrib<total_size,curr_stride+CurrAttrib::stride>(attrib...);
}

} // namespace ntf
