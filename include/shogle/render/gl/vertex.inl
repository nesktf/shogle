#define SHOGLE_RENDER_GL_VERTEX_INL
#include "./vertex.hpp"
#undef SHOGLE_RENDER_GL_VERTEX_INL

namespace shogle {

template<meta::context_layout_type Layout>
gl_vertex_layout::gl_vertex_layout(gl_context& gl, const Layout& layout, gl_buffer vertex_buffer,
                                   size_t vertex_offset) :
    gl_vertex_layout(create(gl, layout, vertex_buffer, vertex_offset).value()) {}

template<meta::context_layout_type Layout>
gl_vertex_layout::gl_vertex_layout(gl_context& gl, const Layout& layout, gl_buffer vertex_buffer,
                                   size_t vertex_offset, gl_buffer index_buffer,
                                   index_format format, size_t index_offset) :
    gl_vertex_layout(
      create(gl, layout, vertex_buffer, vertex_offset, index_buffer, format, index_offset)
        .value()) {}

template<typename Layout>
gl_vertex_layout::gl_vertex_layout(gl_context& gl, vertex_arg<Layout>, gl_buffer vertex_buffer,
                                   size_t vertex_offset) :
    gl_vertex_layout(create<Layout>(gl, vertex_buffer, vertex_offset).value()) {}

template<typename Layout>
gl_vertex_layout::gl_vertex_layout(gl_context& gl, vertex_arg<Layout>, gl_buffer vertex_buffer,
                                   size_t vertex_offset, gl_buffer index_buffer,
                                   index_format format, size_t index_offset) :
    gl_vertex_layout(
      create<Layout>(gl, vertex_buffer, vertex_offset, index_buffer, format, index_offset)
        .value()) {}

template<meta::context_layout_type Layout>
gl_expect<gl_vertex_layout> gl_vertex_layout::create(gl_context& gl, const Layout& layout,
                                                     gl_buffer vertex_buffer,
                                                     size_t vertex_offset) {
  static_assert(Layout::attribute_count < MAX_ATTRIBUTE_BINDINGS, "Attribute count out of range");
  const auto attribs = layout.attributes();
  return _create(gl, attribs, vertex_buffer, vertex_offset, nullopt, INDEX_FORMAT_I8, 0).value();
}

template<meta::context_layout_type Layout>
gl_expect<gl_vertex_layout> gl_vertex_layout::create(gl_context& gl, const Layout& layout,
                                                     gl_buffer vertex_buffer, size_t vertex_offset,
                                                     gl_buffer index_buffer, index_format format,
                                                     size_t index_offset) {
  static_assert(Layout::attribute_count < MAX_ATTRIBUTE_BINDINGS, "Attribute count out of range");
  const auto attribs = layout.attributes();
  return _create(gl, attribs, vertex_buffer, vertex_offset, index_buffer, format, index_offset);
}

template<meta::static_layout_type Layout>
gl_expect<gl_vertex_layout> gl_vertex_layout::create(gl_context& gl, gl_buffer vertex_buffer,
                                                     size_t vertex_offset) {
  static_assert(Layout::attribute_count < MAX_ATTRIBUTE_BINDINGS, "Attribute count out of range");
  const auto attribs = Layout::attributes();
  return _create(gl, attribs, vertex_buffer, vertex_offset, nullopt, INDEX_FORMAT_I8, 0);
}

template<meta::static_layout_type Layout>
gl_expect<gl_vertex_layout> gl_vertex_layout::create(gl_context& gl, gl_buffer vertex_buffer,
                                                     size_t vertex_offset, gl_buffer index_buffer,
                                                     index_format format, size_t index_offset) {
  static_assert(Layout::attribute_count < MAX_ATTRIBUTE_BINDINGS, "Attribute count out of range");
  const auto attribs = Layout::attributes();
  return _create(gl, attribs, vertex_buffer, vertex_offset, index_buffer, format, index_offset);
}

} // namespace shogle
