#pragma once

#include "./types.hpp"
#include "./filesystem.hpp"

#include "../stl/function.hpp"

#include "../render/texture.hpp"

#include <nlohmann/json.hpp>

namespace ntf {

template<
  tex_depth_type DepthT,
  tex_dim_type DimT = extent2d,
  typename Deleter = allocator_delete<DepthT, std::allocator<DepthT>>
>
struct image_data {
public:
  using depth_type = DepthT;
  using dim_type = DimT;

  using array_type = unique_array<depth_type, Deleter>;

public:
  image_data(array_type&& texels_, dim_type dim_,
             r_texture_format format_, r_image_alignment alignment_) SHOGLE_ASSET_NOEXCEPT :
    texels{std::move(texels_)}, dim{dim_}, format{format_}, alignment{alignment_}
  {
    SHOGLE_ASSET_THROW_IF(!texels.has_data(), "No data in texel array");
  }

public:
  auto make_descriptor(
    const dim_type& offset = {}, uint32 level = 0, uint32 layer = 0
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

template<
  tex_depth_type DepthT,
  size_t array_size,
  tex_array_dim_type DimT = extent2d,
  typename Deleter = allocator_delete<DepthT, std::allocator<DepthT>>
>
struct image_data_array {
public:
  using depth_type = DepthT;
  using dim_type = DimT;

  using array_type = std::array<unique_array<depth_type, Deleter>, array_size>;
  
  template<typename T, typename It>
  struct iter_wrapper {
    using difference_type = typename It::difference_type;
    using value_type = T;

    iter_wrapper(It it_) noexcept :
      it{it_} {}

    value_type operator*() {
      return (*it).get();
    }
    iter_wrapper& operator++() {
      ++it;
      return *this;
    }
    iter_wrapper operator++(value_type) {
      auto tmp = *this;
      ++*this;
      return tmp;
    }
    iter_wrapper& operator--() {
      --it;
      return *this;
    }
    iter_wrapper operator--(value_type) {
      auto tmp = *this;
      --*this;
      return tmp;
    }
    bool operator==(const iter_wrapper& other) const {
      return it == other.it;
    }

    It it;
  };

  using iterator = iter_wrapper<depth_type*, typename array_type::iterator>;
  using const_iterator = iter_wrapper<const depth_type*, typename array_type::const_iterator>;

public:
  image_data_array(array_type&& texels_, dim_type dim_,
                   r_texture_format format_,
                   r_image_alignment alignment_) SHOGLE_ASSET_NOEXCEPT :
    texels{std::move(texels_)}, dim{dim_}, format{format_}, alignment{alignment_}
  {
    SHOGLE_ASSET_THROW_IF([this]() {
      for (const auto& tex : texels) {
        if (!tex.has_data()) {
          return true;
        }
      }
      return false;
    }(), "Invalid texel data");
  }

public:
  auto make_descriptor(
    const DimT& offset = {}, uint32 level = 0
  ) const noexcept -> std::array<r_image_data, array_size>
  {
    std::array<r_image_data, array_size> out;
    for (size_t i = 0; i < array_size; ++i) {
      out[i].texels = operator[](i);
      out[i].format = format;
      out[i].alignment = alignment;
      out[i].extent = tex_extent_cast(dim);
      out[i].offset = tex_array_offset(offset, i);
      out[i].layer = static_cast<uint32>(i);
      out[i].level = level;
    }
    return out;
  }

public:
  depth_type* operator[](size_t i) {
    return texels[i].get();
  }
  const depth_type* operator[](size_t i) const {
    return texels[i].get();
  }

  depth_type* at(size_t i) noexcept {
    if (i >= array_size) {
      return nullptr;
    }
    return operator[](i);
  }
  const depth_type* at(size_t i) const noexcept {
    if (i >= array_size) {
      return nullptr;
    }
    return operator[](i);
  }

public:
  constexpr size_t size() const noexcept { return array_size; }

  iterator begin() { return {texels.begin()}; }
  const_iterator begin() const { return {texels.begin()}; }
  const_iterator cbegin() const { return {texels.begin()}; }

  iterator end() { return {texels.end()}; }
  const_iterator end() const { return {texels.end()}; }
  const_iterator cend() const { return {texels.end()}; }

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
struct image_data_array<DepthT, 0u, DimT, Deleter> {
public:
  using depth_type = DepthT;
  using dim_type = DimT;

  using array_type = unique_array<depth_type, Deleter>;

  template<typename T>
  struct dense_iter {
    using difference_type = ptrdiff_t;
    using value_type = T;

    dense_iter(value_type image_ptr_, size_t stride_) noexcept :
      image_ptr{image_ptr_}, stride{stride_} {}

    value_type operator*() {
      return image_ptr;
    }
    dense_iter& operator++() {
      image_ptr += stride;
      return *this;
    }
    dense_iter operator++(value_type) {
      auto tmp = *this;
      ++*this;
      return tmp;
    }
    dense_iter& operator--() {
      image_ptr -= stride;
      return *this;
    }
    dense_iter operator--(value_type) {
      auto tmp = *this;
      --*this;
      return tmp;
    }
    bool operator==(const dense_iter& other) const {
      return image_ptr == other.image_ptr;
    }

    value_type image_ptr;
    const size_t stride;
  };

  using iterator = dense_iter<depth_type*>;
  using const_iterator = dense_iter<const depth_type*>;

public:
  image_data_array(array_type&& texels_, dim_type dim_, size_t array_size_,
                   r_texture_format format_,
                   r_image_alignment alignment_) SHOGLE_ASSET_NOEXCEPT :
    texels{std::move(texels_)}, dim{dim_}, array_size{array_size_},
    format{format_}, alignment{alignment_}
  {
    SHOGLE_ASSET_THROW_IF(!texels.has_data() || array_size == 0, "Invalid texel data");
  }

public:
  template<allocator_type<r_image_data> Alloc = std::allocator<r_image_data>>
  auto make_descriptor(
    const DimT& offset = {}, uint32 level = 0, Alloc&& alloc = {}
  ) const SHOGLE_ASSET_NOEXCEPT -> unique_array<r_image_data, allocator_delete<r_image_data,Alloc>>
  {
    auto out = unique_array<r_image_data>::from_allocator(::ntf::uninitialized,
                                                          array_size,
                                                          std::forward<Alloc>(alloc));
    SHOGLE_ASSET_THROW_IF(!out.has_data(), "Allocation failure");

    for (size_t i = 0; i < array_size; ++i) {
      std::construct_at(out.get()+i, r_image_data{
        .texels = operator[](i),
        .format = format,
        .alignment = alignment,
        .extent = tex_extent_cast(dim),
        .offset = tex_array_offset(offset, i),
        .layer = static_cast<uint32>(i),
        .level = level,
      });
    }

    return out;
  }

public:
  depth_type* operator[](size_t i) {
    return texels.get()+stride()+i;
  }
  const depth_type* operator[](size_t i) const {
    return texels.get()+stride()+i;
  }

  depth_type* at(size_t i) noexcept {
    if (i >= array_size) {
      return nullptr;
    }
    return operator[](i);
  }
  const depth_type* at(size_t i) const noexcept {
    if (i >= array_size) {
      return nullptr;
    }
    return operator[](i);
  }

public:
  size_t size() const noexcept { return array_size; }
  size_t stride() const noexcept { return tex_stride<DepthT>(dim); }

  iterator begin() { return {texels.begin(), stride()}; }
  const_iterator begin() const { return {texels.begin(), stride()}; }
  const_iterator cbegin() const { return {texels.begin(), stride()}; }

  iterator end() { return {texels.end(), stride()}; }
  const_iterator end() const { return {texels.end(), stride()}; }
  const_iterator cend() const { return {texels.end(), stride()}; }

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

enum class image_load_flags {
  none = 0,
  flip_y = 1 << 0,
  mark_normalized = 1 << 1,
  mark_nonlinear = 1 << 2,
};
NTF_DEFINE_ENUM_CLASS_FLAG_OPS(image_load_flags);

template<typename Loader, typename DepthT>
concept checked_image_loader_type = 
  requires(Loader& loader, tex_depth_traits<DepthT>::tag_type tag,
           const std::string& path, image_load_flags flags,
           decltype(loader.load(tag, path, flags)) data) {
    { loader.load(tag, path, flags) } -> expected_with_error<asset_error>;
    { loader.texels(*data) } -> std::convertible_to<
      unique_array<DepthT, typename Loader::template deleter<DepthT>>
    >;
    { loader.dimensions(*data) } -> tex_dim_type;
    { loader.format(*data) } -> std::convertible_to<r_texture_format>;
    { loader.alignment(*data) } -> std::convertible_to<r_image_alignment>;
  };

template<typename Loader, typename DepthT>
concept unchecked_image_loader_type =
  requires(Loader& loader, tex_depth_traits<DepthT>::tag_type tag,
           const std::string& path, image_load_flags flags,
           decltype(loader.load(tag, path, flags)) data) {
    { loader.load(tag, path, flags) } -> not_void;
    { loader.texels(data) } -> std::convertible_to<
      unique_array<DepthT, typename Loader::template deleter<DepthT>>
    >;
    { loader.dimensions(data) } -> tex_dim_type;
    { loader.format(data) } -> std::convertible_to<r_texture_format>;
    { loader.alignment(data) } -> std::convertible_to<r_image_alignment>;
  };

template<typename Loader, typename DepthT>
concept image_loader_type =
  checked_image_loader_type<Loader, DepthT> || unchecked_image_loader_type<Loader, DepthT>;

namespace impl {

template<
  tex_depth_type DepthT,
  tex_dim_type Dim,
  bool checked,
  image_loader_type<DepthT> Loader
>
auto load_image (
  const std::string& path, image_load_flags flags, Loader&& loader
) {
  constexpr auto tag = tex_depth_traits<DepthT>::tag;

  using image_data_t = image_data<DepthT, Dim, typename Loader::template deleter<DepthT>>;
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
      if constexpr (checked_image_loader_type<Loader, DepthT>) {
        return loader.load(tag, path, flags).and_then(make_data);
      } else {
        return make_data(loader.load(tag, path, flags));
      }
    });
  } else if constexpr (checked_image_loader_type<Loader, DepthT>) {
    return make_data(*loader.load(tag, path, flags));
  } else {
    return make_data(loader.load(tag, path, flags));
  }
}

} // namespace impl

class stb_image_loader {
public:
  template<typename T>
  struct deleter {
    void operator()(T* data, size_t) { stb_image_loader::_stbi_delete(data); }
  };

  template<typename T>
  friend struct deleter;

  template<typename T>
  using stbi_texels = unique_array<T, deleter<T>>;

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
  static void _stbi_delete(void* data);

private:
  template<tex_depth_type T>
  auto _pre_parse(
    const std::string& path, image_load_flags flags
  ) -> asset_expected<std::tuple<file_close_t, extent2d, r_texture_format, size_t>> {
    auto* file = std::fopen(path.c_str(), "rb");
    if (!file) {
      SHOGLE_LOG(error, "[ntf::stb_image_loader] Failed to open file \"{}\"", path);
      return unexpected{asset_error::format({"Failed to open file \"{}\""}, path)};
    }
    file_close_t fcloser{file};

    auto pre_parse = _stbi_preparse_file(file);
    if (!pre_parse) {
      SHOGLE_LOG(error, "[ntf::stb_image_loader] Failed to parse file \"{}\"", path);
      return unexpected{asset_error::format({"Failed to parse file \"{}\""}, path)};
    }

    auto& [dims, channels] = *pre_parse;
    if (+(flags & image_load_flags::mark_normalized)) {
      channels |= TEX_DEPTH_NORMALIZE_BIT;
    } else if (+(flags & image_load_flags::mark_nonlinear)) {
      channels |= TEX_DEPTH_NONLINEAR_BIT;
    }

    auto format = tex_depth_traits<T>::parse_channels(channels);
    if (!format) {
      SHOGLE_LOG(error, "[ntf::stb_image_loader] Invalid channels in file \"{}\"", path);
      return unexpected{asset_error::format({"Invalid channels in file \"{}\""}, path)};
    }

    size_t sz = static_cast<size_t>(dims.x*dims.y*channels);
    return std::make_tuple(std::move(fcloser), dims, *format, sz);
  }

  template<tex_depth_type T, typename Loader>
  auto _make_parser(const std::string& path, image_load_flags flags, Loader&& load) {
    return
    [&path, flags, load=std::forward<Loader>(load)](auto&& pre) -> asset_expected<data_t<T>> {
      auto&& [fcloser, dims, format, sz] = std::forward<decltype(pre)>(pre);
      T* data = load(fcloser.get(), flags, 0); // TODO: Maybe pass type channels?
      if (!data) {
        const char* err = _stbi_get_err();
        SHOGLE_LOG(error, "[ntf::stb_image_loader] Failed to load file \"{}\", {}", path, err);
        return unexpected{asset_error::format({"Failed to load file \"{}\", {}"}, path, err)};
      }
      SHOGLE_LOG(debug, "[ntf::stb_image_loader] Loaded {} {}x{} image from file \"{}\"",
                 tex_depth_traits<T>::name, dims.x, dims.y, path);
      return data_t<T>{
        stbi_texels<T>{data, sz, deleter<T>{}}, dims, format
      };
    };
  }

public:
  asset_expected<data_t<uint8>> load(tex_depth_u8_t,
                                     const std::string& path, image_load_flags flags) noexcept {
    return _pre_parse<uint8>(path, flags)
      .and_then(_make_parser<uint8>(path, flags, _stbi_load_u8));
  }

  asset_expected<data_t<uint16>> load(tex_depth_u16_t,
                                      const std::string& path, image_load_flags flags) noexcept {
    return _pre_parse<uint16>(path, flags)
      .and_then(_make_parser<uint16>(path, flags, _stbi_load_u16));
  }

  asset_expected<data_t<float32>> load(tex_depth_f32_t,
                                       const std::string& path, image_load_flags flags) noexcept {
    return _pre_parse<float32>(path, flags)
      .and_then(_make_parser<float32>(path, flags, _stbi_load_f32));
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
static_assert(checked_image_loader_type<stb_image_loader, uint8>);
static_assert(checked_image_loader_type<stb_image_loader, uint16>);
static_assert(checked_image_loader_type<stb_image_loader, float32>);

template<
  tex_depth_type DepthT,
  tex_dim_type Dim = extent2d,
  image_loader_type<DepthT> Loader = stb_image_loader
>
auto load_image(
  unchecked_t, const std::string& path, image_load_flags flags = image_load_flags::mark_normalized,
  Loader&& loader = {}
) -> image_data<DepthT, Dim, typename Loader::template deleter<DepthT>> {
  return impl::load_image<DepthT, Dim, false>(path, flags, std::forward<Loader>(loader));
}

template<
  tex_depth_type DepthT,
  tex_dim_type Dim = extent2d,
  image_loader_type<DepthT> Loader = stb_image_loader
>
auto load_image(
  const std::string& path, image_load_flags flags = image_load_flags::mark_normalized,
  Loader&& loader = {}
) -> asset_expected<image_data<DepthT, Dim, typename Loader::template deleter<DepthT>>>
{
  return impl::load_image<DepthT, Dim, true>(path, flags, std::forward<Loader>(loader));
}

} // namespace ntf
