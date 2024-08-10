#pragma once

#include <shogle/render/render.hpp>

#include <shogle/core/log.hpp>

namespace ntf {

enum class mesh_buffer_type {
  static_draw = 0,
  dynamic_draw,
  stream_draw,
};

enum class mesh_primitive {
  triangles = 0,
  triangle_strip,
  triangle_fan,
  lines,
  line_strip,
  line_loop,
  points,
};

namespace impl {

constexpr GLint enumtogl(mesh_buffer_type type) {
  switch (type) {
    case mesh_buffer_type::static_draw:
      return GL_STATIC_DRAW;
    case mesh_buffer_type::dynamic_draw:
      return GL_DYNAMIC_DRAW;
    case mesh_buffer_type::stream_draw:
      return GL_STREAM_DRAW;
  }
  return 0; // shutup gcc
}

constexpr GLint enumtogl(mesh_primitive primitive) {
  switch (primitive) {
    case mesh_primitive::triangles:
      return GL_TRIANGLES;
    case mesh_primitive::triangle_strip:
      return GL_TRIANGLE_STRIP;
    case mesh_primitive::triangle_fan:
      return GL_TRIANGLE_FAN;
    case mesh_primitive::lines:
      return GL_LINES;
    case mesh_primitive::line_strip:
      return GL_LINE_STRIP;
    case mesh_primitive::line_loop:
      return GL_LINE_LOOP;
    case mesh_primitive::points:
      return GL_POINTS;
  }
  return 0; // shutup gcc
}

} // namespace impl

class mesh {
public:
  mesh() = default;
  inline mesh(mesh_primitive primitive, GLuint vao, GLuint vbo, GLuint ebo, 
              size_t draw_count, size_t att_count);
  inline mesh(mesh_primitive primitive, GLuint vao, GLuint vbo, 
              size_t draw_count, size_t att_count);

  template<typename T, is_shader_attribute... attribs>
  mesh(mesh_primitive primitive, T* vertices, size_t vert_sz, mesh_buffer_type vert_buff, attribs... attrib);

  template<typename T, is_shader_attribute... attribs>
  mesh(mesh_primitive primitive, T* vertices, size_t vert_sz, mesh_buffer_type vert_buff,
       uint* indices, size_t ind_sz, mesh_buffer_type ind_buff, attribs... attrib);

public:
  GLuint& vao() { return _vao; } // Not const
  GLuint& vbo() { return _vbo; } // Not const
  GLuint& ebo() { return _ebo; } // Not const
  size_t draw_count() const { return _draw_count; }
  mesh_primitive primitive() const { return _primitive; }
  bool has_indices() const { return _ebo != 0; }
  bool valid() const { return _vao != 0; }

public:
  inline ~mesh();
  inline mesh(mesh&&) noexcept;
  mesh(const mesh&) = delete;
  inline mesh& operator=(mesh&&) noexcept;
  mesh& operator=(const mesh&) = delete;

private:
  template<size_t total_size, size_t next_stride = 0, typename curr_attrib, typename... attribs>
  void setup_vertex_attrib(curr_attrib, attribs... attrib);

  template<size_t, size_t>
  void setup_vertex_attrib() {}

  inline void unload_mesh();

private: 
  GLuint _vao{}, _vbo{}, _ebo{};
  size_t _draw_count{}, _attrib_count{};
  mesh_primitive _primitive{};

private:
  friend void render_draw_mesh(const mesh& mesh);
};

mesh::mesh(mesh_primitive primitive, GLuint vao, GLuint vbo, GLuint ebo, size_t draw_count, size_t att_count) :
  _vao(vao), _vbo(vbo), _ebo(ebo),
  _draw_count(draw_count), _attrib_count(att_count), _primitive(primitive) {}

mesh::mesh(mesh_primitive primitive, GLuint vao, GLuint vbo, size_t draw_count, size_t att_count) :
  _vao(vao) , _vbo(vbo),
  _draw_count(draw_count), _attrib_count(att_count), _primitive(primitive) {}

void mesh::unload_mesh() {
  log::verbose("[shogle::mesh] Mesh Destroyed (id: {})", _vao);
  glBindVertexArray(_vao);
  for (size_t i = 0; i < _attrib_count; ++i) {
    glDisableVertexAttribArray(i);
  }
  glBindVertexArray(0);

  glDeleteVertexArrays(1, &_vao);
  glDeleteBuffers(1, &_vbo);
  if (_ebo) {
    glDeleteBuffers(1, &_ebo);
  }
}

mesh::~mesh() {
  if (_vao) {
    unload_mesh();
  }
}

mesh::mesh(mesh&& m) noexcept :
  _vao(std::move(m._vao)), _vbo(std::move(m._vbo)), _ebo(std::move(m._ebo)),
  _draw_count(std::move(m._draw_count)), _attrib_count(std::move(m._attrib_count)) { 
  m._vao = 0;
  m._vbo = 0;
  m._ebo = 0;
}

mesh& mesh::operator=(mesh&& m) noexcept {
  if (_vao) {
    unload_mesh();
  }

  _vao = std::move(m._vao);
  _vbo = std::move(m._vbo);
  _ebo = std::move(m._ebo);
  _draw_count = std::move(m._draw_count);
  _attrib_count = std::move(m._attrib_count);

  m._vao = 0;
  m._vbo = 0;
  m._ebo = 0;

  return *this;
}

template<typename T, is_shader_attribute... attribs>
mesh::mesh(mesh_primitive primitive, T* vertices, size_t vert_sz, 
           mesh_buffer_type vert_buff, attribs... attrib) : _primitive(primitive) {
  static_assert(sizeof...(attribs) > 0, "Invalid attributes");
  glGenVertexArrays(1, &_vao);
  glGenBuffers(1, &_vbo);

  glBindVertexArray(_vao);
  glBindBuffer(GL_ARRAY_BUFFER, _vbo);
  glBufferData(GL_ARRAY_BUFFER, vert_sz, vertices, impl::enumtogl(vert_buff));

  const constexpr auto stride = stride_sum<attribs...>::value;
  setup_vertex_attrib<stride>(attrib...);
  _draw_count = vert_sz / stride;
  _attrib_count = sizeof...(attribs);

  glBindVertexArray(0);

  log::verbose("[shogle::mesh] Mesh created (id: {})", _vao);
}

template<typename T, is_shader_attribute... attribs>
mesh::mesh(mesh_primitive primitive, T* vertices, size_t vert_sz, mesh_buffer_type vert_buff,
           uint* indices, size_t ind_sz, mesh_buffer_type ind_buff, attribs... attrib) : _primitive(primitive) {
  static_assert(sizeof...(attribs) > 0, "Invalid attributes");
  glGenVertexArrays(1, &_vao);
  glGenBuffers(1, &_vbo);
  glGenBuffers(1, &_ebo);

  glBindVertexArray(_vao);
  glBindBuffer(GL_ARRAY_BUFFER, _vbo);
  glBufferData(GL_ARRAY_BUFFER, vert_sz, vertices, impl::enumtogl(vert_buff));
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, ind_sz, indices, impl::enumtogl(ind_buff));

  const constexpr auto stride = stride_sum<attribs...>::value;
  _draw_count = ind_sz/sizeof(uint);
  _attrib_count = sizeof...(attribs);

  setup_vertex_attrib<stride>(attrib...);

  glBindVertexArray(0);
  log::verbose("[shogle::mesh] Mesh created (id: {})", _vao);
}

template<size_t total_size, size_t next_stride, typename curr_attrib, typename... attribs>
void mesh::setup_vertex_attrib(curr_attrib, attribs... attrib) {
  constexpr auto curr_stride = next_stride;
  constexpr auto float_count = curr_attrib::stride / sizeof(float);

  glEnableVertexAttribArray(curr_attrib::index);
  glVertexAttribPointer(curr_attrib::index, float_count, GL_FLOAT, GL_FALSE, total_size, (void*)curr_stride);

  setup_vertex_attrib<total_size,curr_stride+curr_attrib::stride>(attrib...);
}

inline void render_draw_mesh(const mesh& mesh) {
  assert(mesh.valid() && "Mesh has no data");

  glBindVertexArray(mesh._vao);
  const auto prim = impl::enumtogl(mesh.primitive());
  if (mesh.has_indices()) {
    glDrawElements(prim, mesh.draw_count(), GL_UNSIGNED_INT, 0);
  } else {
    glDrawArrays(prim, 0, mesh.draw_count());
  }
  glBindVertexArray(0);
}

} // namespace ntf
