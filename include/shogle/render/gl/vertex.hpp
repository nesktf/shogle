#pragma once

#include <shogle/render/gl/common.hpp>

namespace shogle {

class gl_vertex_layout {
public:
  using context_type = gl_context;
  using deleter_type = gl_deleter<gl_vertex_layout>;

public:
  // I'm too lazy to manage a growing array here
  static constexpr u32 MAX_ATTRIBUTE_BINDINGS = 16;

  using attribute_array = std::array<vertex_attribute, MAX_ATTRIBUTE_BINDINGS>;

  enum layout_type {
    TYPE_AOS_LAYOUT = 0,
    TYPE_SOA_LAYOUT,
  };

private:
  struct create_t {};

public:
  gl_vertex_layout(create_t, gldefs::GLhandle vao, attribute_array attributes, u32 attribute_count,
                   size_t stride);

  gl_vertex_layout(gl_context& gl, size_t stride, const ::shogle::vertex_attribute* attribs,
                   u32 attrib_count);

  gl_vertex_layout(gl_context& gl, size_t stride, span<const ::shogle::vertex_attribute> attribs);

  template<size_t AttribCount>
  gl_vertex_layout(gl_context& gl, size_t stride,
                   span<const ::shogle::vertex_attribute, AttribCount> attribs)
  requires(AttribCount != dynamic_extent && AttribCount <= MAX_ATTRIBUTE_BINDINGS);

  template<typename T>
  gl_vertex_layout(gl_context& gl, soa_vertex_arg<T>)
  requires(T::attribute_count <= MAX_ATTRIBUTE_BINDINGS);

  template<typename T>
  gl_vertex_layout(gl_context& gl, aos_vertex_arg<T>)
  requires(T::attribute_count <= MAX_ATTRIBUTE_BINDINGS);

public:
  static gl_expect<gl_vertex_layout> create(gl_context& gl, size_t stride,
                                            const ::shogle::vertex_attribute* attribs,
                                            u32 attrib_count);

  static gl_expect<gl_vertex_layout> create(gl_context& gl, size_t stride,
                                            span<const ::shogle::vertex_attribute> attribs);

  template<size_t AttribCount>
  static gl_expect<gl_vertex_layout>
  create(gl_context& gl, size_t stride,
         span<const ::shogle::vertex_attribute, AttribCount> attribs)
  requires(AttribCount != dynamic_extent && AttribCount <= MAX_ATTRIBUTE_BINDINGS);

  template<::shogle::meta::vertex_type T>
  static gl_expect<gl_vertex_layout> from_soa_vertex(gl_context& gl)
  requires(T::attribute_count <= MAX_ATTRIBUTE_BINDINGS);

  template<::shogle::meta::vertex_type T>
  static gl_expect<gl_vertex_layout> from_aos_vertex(gl_context& gl)
  requires(T::attribute_count <= MAX_ATTRIBUTE_BINDINGS);

  static void destroy(gl_context& gl, gl_vertex_layout& layout) noexcept;
  static void destroy_n(gl_context& gl, gl_vertex_layout* layouts, size_t count) noexcept;
  static void destroy_n(gl_context& gl, span<gl_vertex_layout> layouts) noexcept;

public:
  gldefs::GLhandle vao() const;
  span<const vertex_attribute> attributes() const;
  size_t stride() const;
  layout_type type() const;

  bool invalidated() const noexcept;

public:
  explicit operator bool() const noexcept { return invalidated(); }

private:
  attribute_array _attributes;
  size_t _stride;
  gldefs::GLhandle _vao;
  u32 _attribute_count;
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

} // namespace shogle

#ifndef SHOGLE_RENDER_GL_VERTEX
#include <shogle/render/gl/vertex.inl>
#endif
