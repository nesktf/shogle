#pragma once

#include "common.hpp"

namespace ntf {

class gl_buffer {
public:
  enum class buffer_type {
    none    = GL_NONE,
    vertex  = GL_ARRAY_BUFFER,
    index   = GL_ELEMENT_ARRAY_BUFFER,
    uniform = GL_UNIFORM_BUFFER,
  };

  enum class storage_type {
    none          = GL_NONE,
    static_draw   = GL_STATIC_DRAW,
    dynamic_draw  = GL_DYNAMIC_DRAW,
    stream_draw   = GL_STREAM_DRAW,
  };

public:
  gl_buffer() = default;

public:
  void load(void* data, std::size_t size, buffer_type buffer, storage_type storage);
  void update(void* data, std::size_t size, std::size_t offset);
  void destroy();

public:
  buffer_type buffer() const { return _buffer; }
  storage_type storage() const { return _storage; }
  GLuint id() const { return _id; }
  std::size_t size() const { return _size; }

private:
  buffer_type _buffer{buffer_type::none};
  storage_type _storage{storage_type::none};
  GLuint _id{0};
  std::size_t _size{0};
};

class gl_vertex_array {
public:
  gl_vertex_array() = default;

public:
  void load(gl_buffer& vertex, const std::vector<r_attrib_type>& attribs);
  void load(gl_buffer& vertex, gl_buffer& index, const std::vector<r_attrib_type>& attribs);
  void destroy();

private:
  void _setup_attribs(const std::vector<r_attrib_type>& attribs);

public:
  GLuint vertex() const { return _vertex; }
  GLuint index() const { return _index; }
  GLuint id() const { return _index; }

private:
  GLuint _vertex{0}, _index{0};
  GLuint _id{0};
};

} // namespace ntf
