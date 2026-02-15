#define SHOGLE_RENDER_GL_VERTEX
#include <shogle/render/gl/vertex.hpp>
#undef SHOGLE_RENDER_GL_VERTEX

namespace shogle {

inline gl_vertex_layout::gl_vertex_layout(gl_context& gl, size_t stride,
                                          span<const ::shogle::vertex_attribute> attribs) :
    gl_vertex_layout(gl, stride, attribs.data(), attribs.size()) {}

inline gl_vertex_layout::gl_vertex_layout(gl_context& gl, size_t stride,
                                          const ::shogle::vertex_attribute* attribs,
                                          u32 attrib_count) :
    gl_vertex_layout(
      ::shogle::gl_vertex_layout::create(gl, stride, attribs, attrib_count).value()) {}

template<size_t AttribCount>
gl_vertex_layout::gl_vertex_layout(gl_context& gl, size_t stride,
                                   span<const ::shogle::vertex_attribute, AttribCount> attribs)
requires(AttribCount != dynamic_extent && AttribCount <= MAX_ATTRIBUTE_BINDINGS)
    :
    gl_vertex_layout(
      ::shogle::gl_vertex_layout::create(gl, stride, attribs.data(), AttribCount).value()) {}

template<typename T>
gl_vertex_layout::gl_vertex_layout(gl_context& gl, soa_vertex_arg<T>)
requires(T::attribute_count <= MAX_ATTRIBUTE_BINDINGS)
    : gl_vertex_layout(::shogle::gl_vertex_layout::from_soa_vertex<T>(gl).value()) {}

template<typename T>
gl_vertex_layout::gl_vertex_layout(gl_context& gl, aos_vertex_arg<T>)
requires(T::attribute_count <= MAX_ATTRIBUTE_BINDINGS)
    : gl_vertex_layout(::shogle::gl_vertex_layout::from_aos_vertex<T>(gl).value()) {}

inline gl_expect<gl_vertex_layout>
gl_vertex_layout::create(gl_context& gl, size_t stride,
                         span<const ::shogle::vertex_attribute> attribs) {
  return ::shogle::gl_vertex_layout::create(gl, stride, attribs.data(), attribs.size());
}

template<size_t AttribCount>
gl_expect<gl_vertex_layout>
gl_vertex_layout::create(gl_context& gl, size_t stride,
                         span<const ::shogle::vertex_attribute, AttribCount> attribs)
requires(AttribCount != dynamic_extent && AttribCount <= MAX_ATTRIBUTE_BINDINGS)
{
  return ::shogle::gl_vertex_layout::create(gl, stride, attribs.data(), AttribCount);
}

template<::shogle::meta::vertex_type T>
gl_expect<gl_vertex_layout> gl_vertex_layout::from_soa_vertex(gl_context& gl)
requires(T::attribute_count <= MAX_ATTRIBUTE_BINDINGS)
{
  static constexpr auto attribs = T::attributes();
  return ::shogle::gl_vertex_layout::create(gl, soa_vertex_arg<T>::vertex_stride, attribs.data(),
                                            T::attribute_count);
}

template<::shogle::meta::vertex_type T>
gl_expect<gl_vertex_layout> gl_vertex_layout::from_aos_vertex(gl_context& gl)
requires(T::attribute_count <= MAX_ATTRIBUTE_BINDINGS)
{
  static constexpr auto attribs = T::attributes();
  return ::shogle::gl_vertex_layout::create(gl, aos_vertex_arg<T>::vertex_stride, attribs.data(),
                                            T::attribute_count);
}

} // namespace shogle
