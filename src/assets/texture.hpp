#pragma once

#include "./types.hpp"
#include "./filesystem.hpp"

#include "../stl/function.hpp"

#include "../render/texture.hpp"

#include <nlohmann/json.hpp>

namespace ntf {

template<
  tex_depth_type T,
  tex_dim_type Dim = extent2d,
  typename Deleter = allocator_delete<T, std::allocator<T>>
>
struct image_data {
public:
  using depth_type = T;
  using dim_type = Dim;
  using array_type = unique_array<typename T::underlying_type, Deleter>;

public:
  image_data(array_type&& texels_, dim_type dim_,
             r_texture_format format_, r_image_alignment alignment_) SHOGLE_ASSET_NOEXCEPT :
    texels{std::move(texels_)}, dim{dim_}, format{format_}, alignment{alignment_}
  {
    SHOGLE_ASSET_THROW_IF(!texels.has_data(), "No data in texel array");
  }

public:
  auto make_descriptor(
    const dim_type& offset = {},
    uint32 level = 0, uint32 layer = 0
  ) const noexcept -> r_image_data
  {
    return r_image_data{
      .texels = texels.get(),
      .format = format,
      .alignment = alignment,
      .extent = tex_extent_cast(dim),
      .offset = tex_offset_cast(offset),
      .layer = layer,
      .level = level,
    };
  };

public:
  array_type texels;
  dim_type dim;
  r_texture_format format;
  r_image_alignment alignment;
};

NTF_DEFINE_TEMPLATE_CHECKER(image_data);

namespace impl {

template<typename DepthT, typename DimT, uint32 array_size, bool dense>
struct image_array_desc {
  std::array<r_image_data, array_size> make_descriptor(
    const DimT& offset = {},
    uint32 level = 0
  ) const noexcept {
    std::array<r_image_data, array_size> out;
    for (size_t i = 0; i < array_size; ++i) {
      out[i].texels = [this, i]() -> typename DepthT::underlying_type* {
        if constexpr (dense) {
          return this->texels.get()+i*tex_stride<DepthT>(this->dim);
        } else {
          return this->texels[i].get();
        }
      }();
      out[i].format = this->format;
      out[i].alignment = this->alignment;
      out[i].extent = tex_extent_cast(this->dim);
      out[i].offset = tex_array_offset(offset, i);
      out[i].layer = static_cast<uint32>(i);
      out[i].level = level;
    }
    return out;
  };

protected:
  bool _check_texels(auto&& texels) const {
    if constexpr (dense) {
      return !texels.has_data();
    } else {
      for (const auto&& tex : texels) {
        if (!tex.has_data()) {
          return true;
        }
      }
      return false;
    }
  }
};

template<typename DepthT, typename DimT, bool dense>
struct image_array_desc<DepthT, DimT, 0u, dense> {
  template<allocator_type<r_image_data> Alloc = std::allocator<r_image_data>>
  unique_array<r_image_data, allocator_delete<r_image_data, Alloc>> make_descriptor(
    const DimT& offset = {},
    uint32 level = 0,
    Alloc&& alloc = {}
  ) const SHOGLE_ASSET_NOEXCEPT {
    auto out = unique_array<r_image_data>::from_allocator(::ntf::uninitialized,
                                                          this->array_size,
                                                          std::forward<Alloc>(alloc));
    SHOGLE_ASSET_THROW_IF(!out.has_data(), "Allocation failure");

    for (size_t i = 0; i < this->array_size; ++i) {
      std::construct_at(out.get()+i, r_image_data{
        .texels = [this, i]() -> typename DepthT::underlying_type* {
          if constexpr (dense) {
            return this->texels.get()+i*tex_stride<DepthT>(this->dim);
          } else {
            return this->texels[i].get();
          }
        }(),
        .format = this->format,
        .alignment = this->alignment,
        .extent = tex_extent_cast(this->dim),
        .offset = tex_array_offset(offset, i),
        .layer = static_cast<uint32>(i),
        .level = level,
      });
    }

    return out;
  }

protected:
  bool _check_texels(auto&& texels, size_t array_size) const {
    if constexpr (dense) {
      return !texels.has_data() || array_size == 0;
    } else {
      for (const auto&& tex : texels) {
        if (!tex.has_data()) {
          return true;
        }
      }
      return array_size == 0;
    }
  }
};

} // namespace impl

template<
  tex_depth_type DepthT,
  size_t array_size,
  tex_array_dim_type DimT = extent2d,
  typename Deleter = allocator_delete<DepthT, std::allocator<DepthT>>
>
struct image_data_array :
  public impl::image_array_desc<DepthT, DimT, array_size, false> {
public:
  using depth_type = DepthT;
  using dim_type = DimT;
  using array_type =
    std::array<unique_array<typename DepthT::underlying_type, Deleter>, array_size>;

public:
  image_data_array(array_type&& texels_, dim_type dim_,
                   r_texture_format format_, r_image_alignment alignment_) SHOGLE_ASSET_NOEXCEPT :
    texels{std::move(texels_)}, dim{dim_}, format{format_}, alignment{alignment_}
  {
    SHOGLE_ASSET_THROW_IF(this->_check_texels(texels), "Invalid texel data");
  }

public:
  array_type texels;
  dim_type dim;
  r_texture_format format;
  r_image_alignment alignment;
};

template<
  tex_depth_type DepthT,
  tex_array_dim_type DimT,
  typename Deleter
>
struct image_data_array<DepthT, 0u, DimT, Deleter> :
  public impl::image_array_desc<DepthT, DimT, 0u, false> {
public:
  using depth_type = DepthT;
  using dim_type = DimT;
  using array_type =
    unique_array<unique_array<typename DepthT::underlying_type, Deleter>>;

public:
  image_data_array(array_type&& texels_, dim_type dim_, size_t array_size_,
                   r_texture_format format_, r_image_alignment alignment_) SHOGLE_ASSET_NOEXCEPT :
    texels{std::move(texels_)}, dim{dim_}, array_size{array_size_},
    format{format_}, alignment{alignment_}
  {
    SHOGLE_ASSET_THROW_IF(this->_check_texels(texels, array_size), "Invalid texel data");
  }
  
public:
  array_type texels;
  dim_type dim;
  size_t array_size;
  r_texture_format format;
  r_image_alignment alignment;
};

NTF_DEFINE_TEMPLATE_CHECKER(image_data_array);

template<typename DepthT, typename Deleter = allocator_delete<DepthT, std::allocator<DepthT>>>
using cubemap_image_data = image_data_array<DepthT, 6u, uvec2, Deleter>;

template<
  tex_depth_type DepthT,
  uint32 array_size,
  tex_array_dim_type DimT = extent2d,
  typename Deleter = allocator_delete<DepthT, std::allocator<DepthT>>
>
struct dense_image_data_array :
  public impl::image_array_desc<DepthT, DimT, array_size, true> {
public:
  using depth_type = DepthT;
  using dim_type = DimT;
  using array_type = unique_array<typename DepthT::underlying_type, Deleter>;
  
public:
  dense_image_data_array(array_type&& texels_, dim_type dim_,
                         r_texture_format format_,
                         r_image_alignment alignment_) SHOGLE_ASSET_NOEXCEPT :
    texels{std::move(texels_)}, dim{dim_}, format{format_}, alignment{alignment_}
  {
    SHOGLE_ASSET_THROW_IF(this->_check_texels(texels), "Invalid texel data");
  }
  
public:
  array_type texels;
  dim_type dim;
  r_texture_format format;
  r_image_alignment alignment;
};

template<
  tex_depth_type DepthT,
  tex_array_dim_type DimT,
  typename Deleter
>
struct dense_image_data_array<DepthT, 0u, DimT, Deleter> :
  public impl::image_array_desc<DepthT, DimT, 0u, true> {
public:
  using depth_type = DepthT;
  using dim_type = DimT;
  using array_type = unique_array<typename DepthT::underlying_type, Deleter>;

public:
  dense_image_data_array(array_type&& texels_, dim_type dim_, size_t array_size_,
                         r_texture_format format_,
                         r_image_alignment alignment_) SHOGLE_ASSET_NOEXCEPT :
    texels{std::move(texels_)}, array_size{array_size_}, dim{dim_},
    format{format_}, alignment{alignment_}
  {
    SHOGLE_ASSET_THROW_IF(this->_check_texels(texels, array_size), "Invalid texel data");
  }

public:
  array_type texels;
  dim_type dim;
  size_t array_size;
  r_texture_format format;
  r_image_alignment alignment;
};

NTF_DEFINE_TEMPLATE_CHECKER(dense_image_data_array);

enum class image_load_flags {
  none = 0,
  flip_y = 1 << 0,
};
NTF_DEFINE_ENUM_CLASS_FLAG_OPS(image_load_flags);

template<typename Loader, typename DepthT>
concept checked_image_loader_type = 
  requires(Loader& loader, DepthT tag, const std::string& path, image_load_flags flags,
           decltype(loader.parse(tag, path, flags)) data) {
    { loader.parse(tag, path, flags) } -> expected_with_error<asset_error>;
    { loader.texels(*data) } -> std::convertible_to<unique_array<
      typename DepthT::underlying_type,
      typename Loader::template texel_deleter<typename DepthT::underlying_type>
    >>;
    { loader.dimensions(*data) } -> tex_dim_convertible;
    { loader.format(*data) } -> std::convertible_to<r_texture_format>;
    { loader.alignment(*data) } -> std::convertible_to<r_image_alignment>;
  };

template<typename Loader, typename DepthT>
concept unchecked_image_loader_type =
  requires(Loader& loader, DepthT tag, const std::string& path, image_load_flags flags,
           decltype(loader.parse(tag, path, flags)) data) {
    { loader.parse(tag, path, flags) } -> not_void;
    { loader.texels(data) } -> std::convertible_to<unique_array<
      typename DepthT::underlying_type,
      typename Loader::template texel_deleter<typename DepthT::underlying_type>
    >>;
    { loader.dimensions(data) } -> tex_dim_convertible;
    { loader.format(data) } -> std::convertible_to<r_texture_format>;
    { loader.alignment(data) } -> std::convertible_to<r_image_alignment>;
  };

template<typename Loader, typename DepthT>
concept image_loader_type =
  checked_image_loader_type<Loader, DepthT> || unchecked_image_loader_type<Loader, DepthT>;

namespace impl {

template<
  tex_depth_type T,
  tex_dim_type Dim,
  image_loader_type<T> Loader,
  bool checked
>
auto load_image (
  T&& tag,
  const std::string& path,
  image_load_flags flags,
  Loader&& loader
) {
  using image_data_t = image_data<T, Dim,
    typename Loader::template texel_deleter<typename T::underlying_type>
  >;
  using ret_t = std::conditional_t<checked, asset_expected<image_data_t>, image_data_t>;

  auto make_data = [&](auto&& data) -> ret_t {
    return image_data_t{
      std::move(loader.texels(data)),
      loader.dimensions(data),
      loader.format(data),
      loader.alignment(data)
    };
  };

  if constexpr (checked) {
    return asset_expected<image_data_t>::catch_error([&]() -> asset_expected<image_data_t> {
      if constexpr (checked_image_loader_type<Loader, T>) {
        return loader.parse(std::forward<T>(tag), path, flags).and_then(make_data);
      } else {
        return make_data(loader.parse(std::forward<T>(tag), path, flags));
      }
    });
  } else {
    if constexpr (checked_image_loader_type<Loader, T>) {
      return make_data(*loader.parse(std::forward<T>(tag), path, flags));
    } else {
      return make_data(loader.parse(std::forward<T>(tag), path, flags));
    }
  }
}

} // namespace impl

class stb_image_loader {
public:
  template<typename T>
  struct texel_deleter {
    void operator()(T* data, size_t) { stb_image_loader::stbi_delete(data); }
  };

  template<typename T>
  friend struct texel_deleter;

  template<typename T>
  using stbi_texels = unique_array<T, texel_deleter<T>>;

  template<typename T>
  struct data_t {
    stbi_texels<T> texels;
    extent2d dimensions;
    r_texture_format format;
  };

private: 
  static optional<std::pair<extent2d, uint32>> _stbi_preparse_file(std::FILE* f);
  static const char* _stbi_get_err();
  static uint8* _stbi_load_u8(std::FILE* f, image_load_flags flags, int32 desired);
  static uint16* _stbi_load_u16(std::FILE* f, image_load_flags flags, int32 desired);
  static float32* _stbi_load_f32(std::FILE* f, image_load_flags flags, int32 desired);
  static void stbi_delete(void* data);

private:
  template<typename Parser>
  auto _pre_parse(
    const std::string& path,
    Parser&& parser
  ) -> asset_expected<std::tuple<file_close_t, extent2d, r_texture_format, size_t>> {
    auto* file = std::fopen(path.c_str(), "rb");
    if (!file) {
      SHOGLE_LOG(error, "[ntf::stb_image_loader] Failed to open file \"{}\"", path);
      return unexpected{asset_error::format({"Failed to open file \"{}\""}, path)};
    }
    file_close_t fcloser{file};

    auto pre_parse = _stbi_preparse_file(file);
    if (!pre_parse) {
      SHOGLE_LOG(error, "[ntf::stb_image_loader] Failed to preparse file \"{}\"", path);
      return unexpected{asset_error::format({"Failed to preparse file \"{}\""}, path)};
    }

    auto& [dims, channels] = *pre_parse;
    size_t sz = static_cast<size_t>(dims.x*dims.y*channels);
    auto format = parser(channels);
    if constexpr (optional_type<decltype(format)>) {
      if (!format) {
        SHOGLE_LOG(error, "[ntf::stb_image_loader] Invalid channels in file \"{}\"", path);
        return unexpected{asset_error::format({"Invalid channels in file \"{}\""}, path)};
      }
      return std::make_tuple(std::move(fcloser), dims, *format, sz);
    } else {
      return std::make_tuple(std::move(fcloser), dims, format, sz);
    }
  }

  template<tex_depth_type T, typename Loader>
  auto _make_parser(const std::string& path, image_load_flags flags, Loader&& load) {
    using und_t = T::underlying_type;
    return
    [&path, flags, load=std::forward<Loader>(load)](auto&& pre) -> asset_expected<data_t<und_t>> {
      auto&& [fcloser, dims, format, sz] = std::forward<decltype(pre)>(pre);
      und_t* data = load(fcloser.get(), flags, 0); // TODO: Maybe pass type channels?
      if (!data) {
        const char* err = _stbi_get_err();
        SHOGLE_LOG(error, "[ntf::stb_image_loader] Failed to parse file \"{}\", {}", path, err);
        return unexpected{asset_error::format({"Failed to parse file \"{}\", {}"}, path, err)};
      }
      return data_t<und_t>{
        stbi_texels<und_t>{data, sz, texel_deleter<und_t>{}}, dims, format
      };
    };
  }

public:
  asset_expected<data_t<uint8>> parse(tex_depth_u8,
                                      const std::string& path, image_load_flags flags) noexcept {
    return _pre_parse(path, tex_depth_u8::parse_channels)
      .and_then(_make_parser<tex_depth_u8>(path, flags, _stbi_load_u8));
  }

  asset_expected<data_t<uint8>> parse(tex_depth_u8n,
                                      const std::string& path, image_load_flags flags) noexcept {
    return _pre_parse(path, tex_depth_u8n::parse_channels)
      .and_then(_make_parser<tex_depth_u8n>(path, flags, _stbi_load_u8));
  }

  asset_expected<data_t<uint16>> parse(tex_depth_u16,
                                       const std::string& path, image_load_flags flags) noexcept {
    return _pre_parse(path, tex_depth_u16::parse_channels)
      .and_then(_make_parser<tex_depth_u16>(path, flags, _stbi_load_u16));
  }

  asset_expected<data_t<float32>> parse(tex_depth_f32,
                                        const std::string& path, image_load_flags flags) noexcept {
    return _pre_parse(path, tex_depth_f32::parse_channels)
      .and_then(_make_parser<tex_depth_f32>(path, flags, _stbi_load_f32));
  } 

  static optional<std::pair<extent2d, uint32>> parse_meta(const std::string& path) {
    auto* file = std::fopen(path.c_str(), "rb");
    if (!file) {
      SHOGLE_LOG(error, "[ntf::stb_image_loader] Failed to open file \"{}\"", path);
      return nullopt;
    }
    file_close_t fcloser{file};
    return _stbi_preparse_file(file);
  }

public:
  template<typename T>
  stbi_texels<T>&& texels(data_t<T>& data) {
    return std::move(data.texels);
  }
  template<typename T>
  stbi_texels<T>&& texels(data_t<T>&& data) {
    return std::move(data.texels);
  }

  template<typename T>
  extent2d&& dimensions(data_t<T>& data) {
    return std::move(data.dimensions);
  }
  template<typename T>
  extent2d&& dimensions(data_t<T>&& data) {
    return std::move(data.dimensions);
  }

  template<typename T>
  r_texture_format format(data_t<T>& data) {
    return data.format;
  }
  template<typename T>
  r_texture_format format(data_t<T>&& data) {
    return data.format;
  }

  template<typename T>
  r_image_alignment alignment(T&&) {
    return r_image_alignment::bytes4;
  }
};
static_assert(checked_image_loader_type<stb_image_loader, tex_depth_u8>);
static_assert(checked_image_loader_type<stb_image_loader, tex_depth_u8n>);
static_assert(checked_image_loader_type<stb_image_loader, tex_depth_u16>);
static_assert(checked_image_loader_type<stb_image_loader, tex_depth_f32>);


template<
  tex_depth_type T,
  tex_dim_type Dim,
  image_loader_type<T> Loader = stb_image_loader
>
auto load_image(
  unchecked_t,
  const std::string& path,
  image_load_flags flags = image_load_flags::none,
  Loader&& loader = {}
) -> image_data<T, Dim, typename Loader::template texel_deleter<typename T::underlying_type>> {
  return impl::load_image<T, Dim, Loader, false>(T{}, path, flags, std::forward<Loader>(loader));
}

template<
  tex_depth_type T,
  tex_dim_type Dim,
  image_loader_type<T> Loader = stb_image_loader
>
auto load_image(
  const std::string& path,
  image_load_flags flags = image_load_flags::none,
  Loader&& loader = {}
) -> asset_expected<
    image_data<T, Dim, typename Loader::template texel_deleter<typename T::underlying_type>>
  >
{
  return impl::load_image<T, Dim, Loader, true>(T{}, path, flags, std::forward<Loader>(loader));
}

} // namespace ntf
