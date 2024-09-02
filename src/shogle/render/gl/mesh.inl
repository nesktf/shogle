#define SHOGLE_RENDER_MESH_INL
#include <shogle/render/gl/mesh.hpp>
#undef SHOGLE_RENDER_MESH_INL

namespace ntf {

template<typename T, is_shader_attribute... attribs>
gl_renderer::mesh::mesh(mesh_primitive primitive, T* vertices, size_t vert_sz,
                        mesh_buffer vert_buff, attribs... attrib) : _primitive(primitive) {
  static_assert(sizeof...(attribs) > 0, "Invalid attributes");
  glGenVertexArrays(1, &_vao);
  glGenBuffers(1, &_vbo);

  glBindVertexArray(_vao);
  glBindBuffer(GL_ARRAY_BUFFER, _vbo);
  glBufferData(GL_ARRAY_BUFFER, vert_sz, vertices, renderer_type::enumtogl(vert_buff));

  const constexpr auto stride = stride_sum<attribs...>::value;
  setup_vertex_attrib<stride>(attrib...);
  _draw_count = vert_sz / stride;
  _attrib_count = sizeof...(attribs);

  glBindVertexArray(0);

  SHOGLE_INTERNAL_LOG_FMT(verbose, "[SHOGLE][ntf::gl::mesh] Loaded (id: {})", _vao);
}

template<typename T, is_shader_attribute... attribs>
gl_renderer::mesh::mesh(mesh_primitive primitive, T* vertices, size_t vert_sz, mesh_buffer vert_buff,
                        uint* indices, size_t ind_sz, mesh_buffer ind_buff, attribs... attrib) :
  _primitive(primitive) {
  static_assert(sizeof...(attribs) > 0, "Invalid attributes");
  glGenVertexArrays(1, &_vao);
  glGenBuffers(1, &_vbo);
  glGenBuffers(1, &_ebo);

  glBindVertexArray(_vao);
  glBindBuffer(GL_ARRAY_BUFFER, _vbo);
  glBufferData(GL_ARRAY_BUFFER, vert_sz, vertices, renderer_type::enumtogl(vert_buff));
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, ind_sz, indices, renderer_type::enumtogl(ind_buff));

  const constexpr auto stride = stride_sum<attribs...>::value;
  _draw_count = ind_sz/sizeof(uint);
  _attrib_count = sizeof...(attribs);

  setup_vertex_attrib<stride>(attrib...);

  glBindVertexArray(0);

  SHOGLE_INTERNAL_LOG_FMT(verbose, "[SHOGLE][ntf::gl::mesh] Loaded (id: {})", _vao);
}

inline gl_renderer::mesh::~mesh() noexcept {
#ifdef SHOGLE_GL_RAII_UNLOAD
  unload();
#endif
}

inline gl_renderer::mesh::mesh(mesh&& m) noexcept :
  _vao(std::move(m._vao)), _vbo(std::move(m._vbo)), _ebo(std::move(m._ebo)),
  _draw_count(std::move(m._draw_count)), _attrib_count(std::move(m._attrib_count)),
  _primitive(std::move(m._primitive)) { m._vao = 0; m._vbo = 0; m._ebo = 0; }

inline auto gl_renderer::mesh::operator=(mesh&& m) noexcept -> mesh& {
  unload();

  _vao = std::move(m._vao);
  _vbo = std::move(m._vbo);
  _ebo = std::move(m._ebo);
  _draw_count = std::move(m._draw_count);
  _attrib_count = std::move(m._attrib_count);
  _primitive = std::move(m._primitive);

  m._vao = 0;
  m._vbo = 0;
  m._ebo = 0;

  return *this;
}

template<size_t total_size, size_t next_stride, typename curr_attrib, typename... attribs>
void gl_renderer::mesh::setup_vertex_attrib(curr_attrib, attribs... attrib) {
  constexpr auto curr_stride = next_stride;
  constexpr auto float_count = curr_attrib::stride / sizeof(float);

  glEnableVertexAttribArray(curr_attrib::index);
  glVertexAttribPointer(curr_attrib::index, float_count, GL_FLOAT, GL_FALSE, total_size, (void*)curr_stride);

  setup_vertex_attrib<total_size,curr_stride+curr_attrib::stride>(attrib...);
}

inline void gl_renderer::mesh::unload() {
  if (_vao) {
    SHOGLE_INTERNAL_LOG_FMT(verbose, "[SHOGLE][ntf::gl::mesh] Unloaded (id: {})", _vao);
    glBindVertexArray(_vao);
    for (size_t i = 0; i < _attrib_count; ++i) {
      glDisableVertexAttribArray(i); // ???
    }
    glBindVertexArray(0);

    glDeleteVertexArrays(1, &_vao);
    glDeleteBuffers(1, &_vbo);
    if (_ebo) {
      glDeleteBuffers(1, &_ebo);
    }
    _vao = 0;
    _vbo = 0;
    _ebo = 0;
  }
}

inline bool gl_renderer::mesh::draw() const {
  if (!valid()) {
    return false;
  }

  glBindVertexArray(_vao);
  const auto prim = renderer_type::enumtogl(primitive());
  if (has_indices()) {
    glDrawElements(prim, draw_count(), GL_UNSIGNED_INT, 0);
  } else {
    glDrawArrays(prim, 0, draw_count());
  }
  glBindVertexArray(0);

  return true;
}

} // namespace ntf
