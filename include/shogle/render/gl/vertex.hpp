#pragma once

#include <shogle/render/gl/common.hpp>

namespace shogle {

class gl_vertex_layout {
public:
  // I'm too lazy to manage a growing array here
  static constexpr u32 MAX_ATTRIBUTE_BINDINGS = 16;

  struct attribute_binding {
    attribute_type type;
    u32 location;
    size_t offset;
  };

  using attribute_array = std::array<attribute_binding, MAX_ATTRIBUTE_BINDINGS>;

private:
  struct create_t {};

public:
  gl_vertex_layout(create_t, gl_context& gl, gldefs::GLhandle vao, attribute_array attributes,
                   u32 attribute_count, size_t stride);

  gl_vertex_layout(gl_context& gl, size_t stride, const ::shogle::vertex_attribute* attribs,
                   u32 attrib_count);

  gl_vertex_layout(gl_context& gl, size_t stride, span<const ::shogle::vertex_attribute> attribs);

  template<size_t AttribCount>
  gl_vertex_layout(gl_context& gl, size_t stride,
                   span<const ::shogle::vertex_attribute, AttribCount> attribs)
  requires(AttribCount != ntf::dynamic_extent && AttribCount <= MAX_ATTRIBUTE_BINDINGS);

  template<::shogle::meta::vertex_layout_type Layout>
  gl_vertex_layout(gl_context& gl, Layout&& layout)
  requires(std::remove_cvref_t<Layout>::attribute_count <= MAX_ATTRIBUTE_BINDINGS);

  template<typename T>
  gl_vertex_layout(gl_context& gl, vertex_arg<T>)
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
  requires(AttribCount != ntf::dynamic_extent && AttribCount <= MAX_ATTRIBUTE_BINDINGS);

  template<::shogle::meta::vertex_layout_type Layout>
  static gl_expect<gl_vertex_layout> from_layout(gl_context& gl, Layout&& layout)
  requires(std::remove_cvref_t<Layout>::attribute_count <= MAX_ATTRIBUTE_BINDINGS);

  template<::shogle::meta::vertex_type T>
  static gl_expect<gl_vertex_layout> from_vertex(gl_context& gl)
  requires(T::attribute_count <= MAX_ATTRIBUTE_BINDINGS);

  static void destroy(gl_context& gl, gl_vertex_layout& layout);

public:
  gldefs::GLhandle id() const;
  span<const attribute_binding> attributes() const;
  size_t stride() const;
  bool invalidated() const noexcept;

public:
  explicit operator bool() const noexcept { return invalidated(); }

private:
  attribute_array _attributes;
  size_t _stride;
  gldefs::GLhandle _vao;
  u32 _attribute_count;
};

} // namespace shogle
