#define SHOGLE_RENDER_GL_VERTEX_INL
#include "./vertex.hpp"
#undef SHOGLE_RENDER_GL_VERTEX_INL

namespace shogle {

template<meta::context_layout_type Layout>
gl_vertex_layout::gl_vertex_layout(gl_context& gl, const Layout& layout,
                                   ptr_view<gl_buffer> vertex_buffer,
                                   ptr_view<gl_buffer> index_buffer, index_format format,
                                   size_t vertex_offset, size_t index_offset) :
    gl_vertex_layout(
      create(gl, layout, vertex_buffer, index_buffer, format, vertex_offset, index_offset)
        .value()) {}

template<typename Layout>
gl_vertex_layout::gl_vertex_layout(gl_context& gl, vertex_arg<Layout>,
                                   ptr_view<gl_buffer> vertex_buffer,
                                   ptr_view<gl_buffer> index_buffer, index_format format,
                                   size_t vertex_offset, size_t index_offset) :
    gl_vertex_layout(
      create<Layout>(gl, vertex_buffer, index_buffer, format, vertex_offset, index_offset)
        .value()) {}

template<meta::context_layout_type Layout>
gl_expect<gl_vertex_layout>
gl_vertex_layout::create(gl_context& gl, const Layout& layout, ptr_view<gl_buffer> vertex_buffer,
                         ptr_view<gl_buffer> index_buffer, index_format format,
                         size_t index_offset, size_t vertex_offset) {
  const auto attribs = layout.attributes();
  return _create(gl, attribs, vertex_buffer, index_buffer, format, index_offset, vertex_offset);
}

template<meta::static_layout_type Layout>
gl_expect<gl_vertex_layout>
gl_vertex_layout::create(gl_context& gl, ptr_view<gl_buffer> vertex_buffer,
                         ptr_view<gl_buffer> index_buffer, index_format format,
                         size_t index_offset, size_t vertex_offset) {
  const auto attribs = Layout::attributes();
  return _create(gl, attribs, vertex_buffer, index_buffer, format, index_offset, vertex_offset);
}

template<meta::context_layout_type Layout>
gl_expect<gl_vertex_layout> gl_layout_builder::build(gl_context& gl, const Layout& layout) const {
  return gl_vertex_layout::create(gl, layout, _vert, _ind, _ind_format, _vert_offset, _ind_offset);
}

template<meta::static_layout_type Layout>
gl_expect<gl_vertex_layout> gl_layout_builder::build(gl_context& gl) const {
  return gl_vertex_layout::create<Layout>(gl, _vert, _ind, _ind_format, _vert_offset, _ind_offset);
}

} // namespace shogle
