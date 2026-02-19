#define SHOGLE_GL_TEXTURE_INL
#include <shogle/render/gl/texture.hpp>
#undef SHOGLE_GL_TEXTURE_INL

namespace shogle {

template<typename Cont>
auto gl_texture::allocate1d_n(gl_context& gl, Cont&& cont, u32 count, texture_format format,
                              u32 extent, u32 levels, u32 layers) -> n_err_return
requires(growable_tex_container<Cont>)
{
  auto texes = make_scratch_vec<gldefs::GLenum>(::shogle::impl::gl_get_scratch_arena(gl));
  texes.resize(count);
  span<gldefs::GLenum> texspan{texes.data(), texes.size()};
  const allocate_args args{
    .extent = ::shogle::meta::extent_traits<u32>::extent_clamp3d(extent),
    .type = layers > 1 ? TEX_TYPE_1D_ARRAY : TEX_TYPE_1D,
    .format = format,
    .levels = std::min(levels, MAX_MIPMAP_LEVEL),
    .layers = layers,
    .multisampling = MULTISAMPLE_NONE,
  };
  const auto allocated = ::shogle::gl_texture::_allocate_span(gl, texspan, args);
  for (u32 i = 0; i < allocated.first; ++i) {
    if constexpr (emplace_tex_container<Cont>) {
      cont.emplace_back(create_t{}, gl, texes[i], args);
    } else {
      cont.push_back(gl_texture{create_t{}, texes[i], args});
    }
  }
  return allocated;
}

template<typename Cont>
auto gl_texture::allocate2d_n(gl_context& gl, Cont&& cont, u32 count, texture_format format,
                              const extent2d& extent, u32 levels, u32 layers,
                              multisample_opt multisampling) -> n_err_return
requires(growable_tex_container<Cont>)
{
  auto texes = make_scratch_vec<gldefs::GLenum>(::shogle::impl::gl_get_scratch_arena(gl));
  texes.resize(count);
  span<gldefs::GLenum> texspan{texes.data(), texes.size()};
  const allocate_args args{
    .extent = ::shogle::meta::extent_traits<extent2d>::extent_clamp3d(extent),
    .type = multisampling ? (layers > 1 ? TEX_TYPE_2D_MULTISAMPLE_ARRAY : TEX_TYPE_2D_MULTISAMPLE)
                          : (layers > 1 ? TEX_TYPE_2D_ARRAY : TEX_TYPE_2D),
    .format = format,
    .levels = std::min(levels, MAX_MIPMAP_LEVEL),
    .layers = std::max(layers, 1u),
    .multisampling = multisampling,
  };
  const auto allocated = ::shogle::gl_texture::_allocate_span(gl, texspan, args);
  for (u32 i = 0; i < allocated.first; ++i) {
    if constexpr (emplace_tex_container<Cont>) {
      cont.emplace_back(create_t{}, gl, texes[i], args);
    } else {
      cont.push_back(gl_texture{create_t{}, texes[i], args});
    }
  }
  return allocated;
}

template<typename Cont>
auto gl_texture::allocate_cubemap_n(gl_context& gl, Cont&& cont, u32 count, texture_format format,
                                    u32 extent, u32 levels) -> n_err_return
requires(growable_tex_container<Cont>)
{
  auto texes = make_scratch_vec<gldefs::GLenum>(::shogle::impl::gl_get_scratch_arena(gl));
  texes.resize(count);
  span<gldefs::GLenum> texspan{texes.data(), texes.size()};
  const allocate_args args{
    .extent = ::shogle::meta::extent_traits<extent2d>::extent_clamp3d(extent2d{extent, extent}),
    .type = TEX_TYPE_CUBEMAP,
    .format = format,
    .levels = std::min(levels, MAX_MIPMAP_LEVEL),
    .layers = 6u,
    .multisampling = MULTISAMPLE_NONE,
  };
  const auto allocated = ::shogle::gl_texture::_allocate_span(gl, texspan, args);
  for (u32 i = 0; i < allocated.first; ++i) {
    if constexpr (emplace_tex_container<Cont>) {
      cont.emplace_back(create_t{}, gl, texes[i], args);
    } else {
      cont.push_back(gl_texture{create_t{}, texes[i], args});
    }
  }
  return allocated;
}

template<typename Cont>
auto gl_texture::allocate3d_n(gl_context& gl, Cont&& cont, u32 count, texture_format format,
                              const extent3d& extent, u32 levels) -> n_err_return
requires(growable_tex_container<Cont>)
{
  auto texes = make_scratch_vec<gldefs::GLenum>(::shogle::impl::gl_get_scratch_arena(gl));
  texes.resize(count);
  span<gldefs::GLenum> texspan{texes.data(), texes.size()};
  const allocate_args args{
    .extent = ::shogle::meta::extent_traits<extent3d>::extent_clamp3d(extent),
    .type = TEX_TYPE_3D,
    .format = format,
    .levels = std::min(levels, MAX_MIPMAP_LEVEL),
    .layers = 1u,
    .multisampling = MULTISAMPLE_NONE,
  };
  const auto allocated = ::shogle::gl_texture::_allocate_span(gl, texspan, args);
  for (u32 i = 0; i < allocated.first; ++i) {
    if constexpr (emplace_tex_container<Cont>) {
      cont.emplace_back(create_t{}, gl, texes[i], args);
    } else {
      cont.push_back(gl_texture{create_t{}, texes[i], args});
    }
  }
  return allocated;
}

template<::shogle::meta::gl_data_type T, ::shogle::meta::extent_type Ext>
gl_expect<void> gl_texture::upload_image(gl_context& gl, span<const T> data, const Ext& extent,
                                         pixel_format format, const Ext& offset, u32 layer,
                                         u32 level, pixel_alignment alignment) {
  const image_data image{
    .data = static_cast<const void*>(data.data()),
    .extent = ::shogle::meta::extent_traits<Ext>::extent_clamp3d(extent),
    .format = format,
    .datatype = static_cast<pixel_data_type>(::shogle::meta::gl_data_traits<T>::gl_tag),
    .alignment = alignment,
  };
  const auto stride = ::shogle::meta::extent_traits<Ext>::template stride_of<T>(extent);
  NTF_ASSERT(stride <= data.size_bytes(), "Image stride out of span range");
  return this->upload_image(gl, image, ::shogle::meta::extent_traits<Ext>::offset_cast3d(offset),
                            level, layer);
}

} // namespace shogle
