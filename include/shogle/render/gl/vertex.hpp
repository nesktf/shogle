#pragma once

#include <shogle/render/gl/buffer.hpp>

namespace shogle {

class gl_vertex_layout {
public:
  using context_type = gl_context;
  using deleter_type = gl_deleter<gl_vertex_layout>;

  enum index_format : gldefs::GLenum {
    INDEX_FORMAT_I8 = 0, // GL_BYTE
    INDEX_FORMAT_U8,     // GL_UNSIGNED_BYTE
    INDEX_FORMAT_I16,    // GL_SHORT
    INDEX_FORMAT_U16,    // GL_UNSIGNED_SHORT
    INDEX_FORMAT_I32,    // GL_INT
    INDEX_FORMAT_U32,    // GL_UNSIGNED_INT
  };

public:
  // I'm too lazy to manage a growing array here
  static constexpr size_t MAX_ATTRIBUTE_BINDINGS = 16;

  using attribute_array = std::array<vertex_attribute, MAX_ATTRIBUTE_BINDINGS>;

private:
  struct create_t {};

public:
  gl_vertex_layout(create_t, attribute_array attributes, u32 attribute_count, gldefs::GLhandle vao,
                   optional<gl_buffer> vertex, optional<gl_buffer> index, size_t vertex_offset,
                   size_t index_offset);

  template<meta::context_layout_type Layout>
  gl_vertex_layout(gl_context& gl, const Layout& layout, ptr_view<gl_buffer> vertex_buffer,
                   ptr_view<gl_buffer> index_buffer, index_format format, size_t vertex_offset,
                   size_t index_offset);

  template<typename Layout>
  gl_vertex_layout(gl_context& gl, vertex_arg<Layout>, ptr_view<gl_buffer> vertex_buffer,
                   ptr_view<gl_buffer> index_buffer, index_format format, size_t vertex_offset,
                   size_t index_offset);

private:
  static gl_expect<gl_vertex_layout> _create(gl_context& gl, span<const vertex_attribute> attribs,
                                             ptr_view<gl_buffer> vertex_buffer,
                                             ptr_view<gl_buffer> index_buffer, index_format format,
                                             size_t vertex_offset, size_t index_offset);

public:
  template<meta::context_layout_type Layout>
  static gl_expect<gl_vertex_layout> create(gl_context& gl, const Layout& layout,
                                            ptr_view<gl_buffer> vertex_buffer,
                                            ptr_view<gl_buffer> index_buffer, index_format format,
                                            size_t vertex_offset, size_t index_offset);

  template<meta::static_layout_type Layout>
  static gl_expect<gl_vertex_layout> create(gl_context& gl, ptr_view<gl_buffer> vertex_buffer,
                                            ptr_view<gl_buffer> index_buffer, index_format format,
                                            size_t vertex_offset, size_t index_offset);

  static void destroy(gl_context& gl, gl_vertex_layout& layout) noexcept;
  static void destroy_n(gl_context& gl, gl_vertex_layout* layouts, size_t count) noexcept;
  static void destroy_n(gl_context& gl, span<gl_vertex_layout> layouts) noexcept;

public:
  gldefs::GLhandle vao() const;
  span<const vertex_attribute> attributes() const;

  const gl_buffer* vertex_buffer() const;
  size_t vertex_offset() const;

  const gl_buffer* index_buffer() const;
  size_t index_offset() const;

private:
  attribute_array _attributes;
  u32 _attribute_count;
  gldefs::GLhandle _vao;
  optional<gl_buffer> _vertex;
  optional<gl_buffer> _index;
  size_t _vertex_offset;
  size_t _index_offset;
};

static_assert(::shogle::meta::renderer_object_type<gl_vertex_layout>);

template<>
struct gl_deleter<gl_vertex_layout> {
public:
  gl_deleter(gl_context& gl) noexcept : _gl(&gl) {}

public:
  void operator()(gl_vertex_layout* layouts, size_t count) const noexcept {
    gl_vertex_layout::destroy_n(*_gl, layouts, count);
  }

  void operator()(gl_vertex_layout& layout) const noexcept {
    gl_vertex_layout::destroy(*_gl, layout);
  }

private:
  gl_context* _gl;
};

class gl_layout_builder {
public:
  gl_layout_builder() noexcept;

public:
  gl_layout_builder& set_vertex_buffer(gl_buffer& buffer);
  gl_layout_builder& set_index_buffer(gl_buffer& buffer, gl_vertex_layout::index_format format);

  gl_layout_builder& set_vertex_offset(size_t offset);
  gl_layout_builder& set_index_offset(size_t offset);

  template<meta::context_layout_type Layout>
  gl_expect<gl_vertex_layout> build(gl_context& gl, const Layout& layout) const;

  template<meta::static_layout_type Layout>
  gl_expect<gl_vertex_layout> build(gl_context& gl) const;

private:
  ptr_view<gl_buffer> _vert;
  ptr_view<gl_buffer> _ind;
  gl_vertex_layout::index_format _ind_format;
  size_t _vert_offset;
  size_t _ind_offset;
};

} // namespace shogle

#ifndef SHOGLE_RENDER_GL_VERTEX_INL
#include "./vertex.inl"
#endif
