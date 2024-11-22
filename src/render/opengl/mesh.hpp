#pragma once

#include "./common.hpp"

namespace ntf {

struct gl_indices_t {};
constexpr inline gl_indices_t gl_indices; // TODO: Find a way to avoid using this tag?

class gl_mesh {
public:
  using context_type = gl_context;

public:
  gl_mesh() = default;

  template<typename T, is_shader_attribute... Attribs>
  gl_mesh(const T* vert, std::size_t sz, mesh_buffer buff, Attribs... attr);

  template<typename T, is_shader_attribute... Attribs>
  gl_mesh(gl_indices_t, const T* vert, std::size_t vert_sz, mesh_buffer vert_buff,
          const uint* ind, std::size_t ind_sz, mesh_buffer ind_buff, Attribs... attr);

public:
  gl_mesh& indices(const uint* ind, std::size_t sz, mesh_buffer buff) &;
  gl_mesh&& indices(const uint* ind, std::size_t sz, mesh_buffer buff) &&;
  gl_mesh& indices(const uint* ind, std::size_t sz, std::size_t offset) &;

  template<typename T>
  gl_mesh& vertices(const T* vert, std::size_t sz, mesh_buffer buff) &;

  template<typename T>
  gl_mesh&& vertices(const T* vert, std::size_t sz, mesh_buffer buff) &&;

  template<typename T>
  gl_mesh& vertices(const T* vert, std::size_t sz, std::size_t offset) &;

  template<is_shader_attribute... Attribs>
  gl_mesh& attributes(Attribs... attr) &;

  template<is_shader_attribute... Attribs>
  gl_mesh&& attributes(Attribs... attr) &&;

  void unload();

public:
  GLuint vao() const { return _vao; }
  GLuint vbo() const { return _vbo; }
  GLuint ebo() const { return _ebo; }

  std::size_t elem_count() const { return _ebo ? _ebo_sz/sizeof(uint) : _vbo_sz/_stride_sz; }

  bool has_indices() const { return _ebo != 0; }
  bool valid() const { return _vao != 0 && _vbo != 0 && _attrib_count > 0; }

  explicit operator bool() const { return valid(); }

private:
  void _set_indices(const uint* indices, std::size_t sz, mesh_buffer buff);
  void _set_indices(const uint* indices, std::size_t sz, std::size_t offset);

  template<typename T>
  void _set_vertices(const T* vertices, std::size_t sz, mesh_buffer buff);

  template<typename T>
  void _set_vertices(const T* vertices, std::size_t sz, std::size_t offset);

  template<is_shader_attribute... Attribs>
  void _set_attributes(Attribs... attr);

  template<std::size_t total_size, std::size_t next_stride = 0,
    typename CurrAttrib, typename... Attribs>
  void _setup_vertex_attrib(CurrAttrib, Attribs... attrib);

  template<std::size_t, std::size_t>
  void _setup_vertex_attrib() {}

  void _reset();

private: 
  GLuint _vao{0}, _vbo{0}, _ebo{0};
  std::size_t _vbo_sz{0}, _ebo_sz{0}, _attrib_count{0}, _stride_sz{0};

public:
  NTF_DECLARE_MOVE_ONLY(gl_mesh);
};

} // namespace ntf

#ifndef SHOGLE_RENDER_OPENGL_MESH_INL
#include "./mesh.inl"
#endif
