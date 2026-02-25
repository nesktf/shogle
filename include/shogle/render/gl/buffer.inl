#define SHOGLE_RENDER_GL_BUFFER_INL
#include "./buffer.hpp"
#undef SHOGLE_RENDER_GL_BUFFER_INL

namespace shogle {

template<typename Cont>
auto gl_buffer::allocate_n(gl_context& gl, Cont&& cont, size_t count, size_t size,
                           gldefs::GLbitfield usage_flags, const void* data) -> n_err_return
requires(growable_buff_container<Cont>)
{
  auto texes = make_scratch_vec<gldefs::GLhandle>(::shogle::impl::gl_get_scratch_arena(gl));
  texes.resize(count);
  const auto allocated =
    ::shogle::gl_buffer::_allocate_span(gl, texes.data(), texes.size(), size, usage_flags, data);
  for (size_t i = 0; i < allocated.first; ++i) {
    if constexpr (emplace_buff_container<Cont>) {
      cont.emplace_back(create_t{}, gl, texes[i], usage_flags, size);
    } else {
      cont.push_back(gl_buffer{create_t{}, texes[i], usage_flags, size});
    }
  }
  return allocated;
}

} // namespace shogle
