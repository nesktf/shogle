#pragma once

#include <shogle/render/gl.hpp>

namespace ntf {

class gl::mesh {
public:
  using gl = gl;

public:
  mesh() = default;

  mesh(mesh_primitive primitive, GLuint vao, GLuint vbo, GLuint ebo, size_t draw_count, size_t att_count) :
    _vao(vao), _vbo(vbo), _ebo(ebo),
    _draw_count(draw_count), _attrib_count(att_count), _primitive(primitive) {}
  mesh(mesh_primitive primitive, GLuint vao, GLuint vbo, size_t draw_count, size_t att_count) :
    _vao(vao) , _vbo(vbo),
    _draw_count(draw_count), _attrib_count(att_count), _primitive(primitive) {}

  template<typename T, is_shader_attribute... attribs>
  mesh(mesh_primitive primitive, T* vertices, size_t vert_sz, mesh_buffer vert_buff, attribs... attrib);

  template<typename T, is_shader_attribute... attribs>
  mesh(mesh_primitive primitive, T* vertices, size_t vert_sz, mesh_buffer vert_buff,
       uint* indices, size_t ind_sz, mesh_buffer ind_buff, attribs... attrib);

public:
  bool draw() const;
  void unload();

public:
  GLuint& vao() { return _vao; } // Not const
  GLuint& vbo() { return _vbo; } // Not const
  GLuint& ebo() { return _ebo; } // Not const
  size_t draw_count() const { return _draw_count; }
  mesh_primitive primitive() const { return _primitive; }
  bool has_indices() const { return _ebo != 0; }
  bool valid() const { return _vao != 0; }

  operator bool() const { return valid(); }

private:
  template<size_t total_size, size_t next_stride = 0, typename curr_attrib, typename... attribs>
  void setup_vertex_attrib(curr_attrib, attribs... attrib);

  template<size_t, size_t>
  void setup_vertex_attrib() {}

private: 
  GLuint _vao{}, _vbo{}, _ebo{};
  size_t _draw_count{}, _attrib_count{};
  mesh_primitive _primitive{};

public:
  NTF_DECLARE_MOVE_ONLY(mesh);
};

} // namespace ntf

#ifndef SHOGLE_RENDER_MESH_INL
#include <shogle/render/gl/mesh.inl>
#endif
