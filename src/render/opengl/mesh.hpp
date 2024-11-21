#pragma once

#include "./common.hpp"

namespace ntf {

class gl_mesh {
public:
  using context_type = gl_context;

public:
  gl_mesh() = default;

  gl_mesh(mesh_primitive primitive, GLuint vao, GLuint vbo, GLuint ebo,
          std::size_t draw_count, std::size_t att_count) :
    _vao(vao), _vbo(vbo), _ebo(ebo),
    _draw_count(draw_count), _attrib_count(att_count), _primitive(primitive) {}

  gl_mesh(mesh_primitive primitive, GLuint vao, GLuint vbo,
          std::size_t draw_count, std::size_t att_count) :
    _vao(vao) , _vbo(vbo),
    _draw_count(draw_count), _attrib_count(att_count), _primitive(primitive) {}

  template<typename T, is_shader_attribute... Attribs>
  gl_mesh(mesh_primitive primitive,
          T* vertices, std::size_t vert_sz, mesh_buffer vert_buff,
          Attribs... attrib)
    { load(primitive, vertices, vert_sz, vert_buff, attrib...); }

  template<typename T, is_shader_attribute... attribs>
  gl_mesh(mesh_primitive primitive, 
          T* vertices, std::size_t vert_sz, mesh_buffer vert_buff,
          unsigned int* indices, std::size_t ind_sz, mesh_buffer ind_buff,
          attribs... attrib)
    { load(primitive, vertices, vert_sz, vert_buff, indices, ind_sz, ind_buff, attrib...); }

public:
  template<typename T, is_shader_attribute... Attribs>
  void load(mesh_primitive primitive,
            T* vertices, std::size_t vert_sz, mesh_buffer vert_buff,
            Attribs... attrib);

  template<typename T, is_shader_attribute... Attribs>
  void load(mesh_primitive primitive,
            T* vertices, std::size_t vert_sz, mesh_buffer vert_buff,
            unsigned int* indices, std::size_t ind_sz, mesh_buffer ind_buf,
            Attribs... attrib);

  void unload();

public:
  bool draw() const;

public:
  GLuint& vao() { return _vao; } // Not const
  GLuint& vbo() { return _vbo; } // Not const
  GLuint& ebo() { return _ebo; } // Not const
  std::size_t draw_count() const { return _draw_count; }
  mesh_primitive primitive() const { return _primitive; }
  bool has_indices() const { return _ebo != 0; }
  bool valid() const { return _vao != 0 && _vbo != 0; }

  explicit operator bool() const { return valid(); }

private:
  template<std::size_t total_size, std::size_t next_stride = 0,
    typename CurrAttrib, typename... Attribs>
  void _setup_vertex_attrib(CurrAttrib, Attribs... attrib);

  template<std::size_t, std::size_t>
  void _setup_vertex_attrib() {}

private: 
  GLuint _vao{0}, _vbo{0}, _ebo{0};
  std::size_t _draw_count{}, _attrib_count{};
  mesh_primitive _primitive{};

public:
  NTF_DECLARE_MOVE_ONLY(gl_mesh);
};

} // namespace ntf

#ifndef SHOGLE_RENDER_OPENGL_MESH_INL
#include "./mesh.inl"
#endif
