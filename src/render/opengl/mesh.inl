#define SHOGLE_RENDER_OPENGL_MESH_INL
#include "./mesh.hpp"
#undef SHOGLE_RENDER_OPENGL_MESH_INL

namespace ntf {

template<typename T, is_shader_attribute... Attribs>
gl_mesh::gl_mesh(const T* vert, std::size_t sz, mesh_buffer buff, Attribs... attr) {
  _set_vertices(vert, sz, buff);
  _set_attributes(attr...);
}

template<typename T, is_shader_attribute... Attribs>
gl_mesh::gl_mesh(gl_indices_t, const T* vert, std::size_t vert_sz, mesh_buffer vert_buff,
          const uint* ind, std::size_t ind_sz, mesh_buffer ind_buff, Attribs... attr) {
  _set_vertices(vert, vert_sz, vert_buff);
  _set_indices(ind, ind_sz, ind_buff);
  _set_attributes(attr...);
}


template<typename T>
gl_mesh& gl_mesh::vertices(const T* vert, std::size_t sz, mesh_buffer buff) & {
  _set_vertices(vert, sz, buff);
  return *this;
}

template<typename T>
gl_mesh&& gl_mesh::vertices(const T* vert, std::size_t sz, mesh_buffer buff) && {
  _set_vertices(vert, sz, buff);
  return std::move(*this);
}

template<typename T>
gl_mesh& gl_mesh::vertices(const T* vert, std::size_t sz, std::size_t offset) & {
  _set_vertices(vert, sz, offset);
  return *this;
}

template<is_shader_attribute... Attribs>
gl_mesh& gl_mesh::attributes(Attribs... attr) & {
  _set_attributes(attr...);
  return *this;
}

template<is_shader_attribute... Attribs>
gl_mesh&& gl_mesh::attributes(Attribs... attr) && {
  _set_attributes(attr...);
  return std::move(*this);
}


template<typename T>
void gl_mesh::_set_vertices(const T* vertices, std::size_t sz, mesh_buffer buff) {
  if (!_vao) {
    glGenVertexArrays(1, &_vao);
  }
  if (!_vbo) {
    glGenBuffers(1, &_vbo);
  }

  glBindVertexArray(_vao);
  glBindBuffer(GL_ARRAY_BUFFER, _vbo);
  glBufferData(GL_ARRAY_BUFFER, sz, vertices, enumtogl(buff));
  glBindVertexArray(0);
  _vbo_sz = sz;

  if (valid()) {
    SHOGLE_LOG(verbose, "[ntf::gl_mesh] Mesh created (id: {})", _vao);
  }
}

template<typename T>
void gl_mesh::_set_vertices(const T* vertices, std::size_t sz, std::size_t offset) {
  NTF_ASSERT(_vbo);
  NTF_ASSERT(offset+sz <= _vbo_sz);

  glBindVertexArray(_vao);
  glBindBuffer(GL_ARRAY_BUFFER, _ebo);
  glBufferSubData(GL_ARRAY_BUFFER, offset, sz, vertices);
  glBindVertexArray(0);
}

template<is_shader_attribute... Attribs>
void gl_mesh::_set_attributes(Attribs... attr) {
  static_assert(sizeof...(Attribs) > 0);
  if (!_vao) {
    glGenVertexArrays(1, &_vao);
  }

  glBindVertexArray(_vao);
  if (_attrib_count > 0) {
    // Will overwrite attributes
    for (std::size_t i = 0; i < _attrib_count; ++i) {
      glDisableVertexAttribArray(i);
    }
  }

  constexpr auto stride = stride_sum<Attribs...>::value;
  _setup_vertex_attrib<stride>(attr...);
  _attrib_count = sizeof...(Attribs);
  _stride_sz = stride;
  glBindVertexArray(0);

  if (valid()) {
    SHOGLE_LOG(verbose, "[ntf::gl_mesh] Mesh created (id: {})", _vao);
  }
}

template<std::size_t total_size, std::size_t next_stride, typename CurrAttrib, typename... Attribs>
void gl_mesh::_setup_vertex_attrib(CurrAttrib, Attribs... attrib) {
  glEnableVertexAttribArray(CurrAttrib::index);
  glVertexAttribPointer(CurrAttrib::index, CurrAttrib::stride / sizeof(float), GL_FLOAT,
                        GL_FALSE, total_size, reinterpret_cast<void*>(next_stride));

  _setup_vertex_attrib<total_size, next_stride+CurrAttrib::stride>(attrib...);
}

} // namespace ntf
