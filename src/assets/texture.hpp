#pragma once

#include "./types.hpp"

#include "../stl/function.hpp"

#include "../render/types.hpp"

#include <nlohmann/json.hpp>

namespace ntf {

template<typename Dim>
constexpr uvec3 image_dim_cast(const Dim& dim) {
  if constexpr(std::same_as<Dim, uvec3>) {
    return dim;
  } else if constexpr(std::same_as<Dim, uvec2>) {
    return uvec3{static_cast<uint32>(dim.x), static_cast<uint32>(dim.y), 0};
  } else {
    return uvec3{static_cast<uint32>(dim), 0, 0};
  }
}

template<
  image_depth_type T,
  image_dim_type Dim = uvec2,
  typename Deleter = allocator_delete<T, std::allocator<T>>
>
struct image_data {
public:
using depth_type = T;
  using dim_type = Dim;

  using texel_type = image_depth_traits<T>::underlying_t;
  using array_type = unique_array<texel_type, Deleter>;

public:
  image_data(array_type&& texels_, dim_type dim_, r_texture_format format_) NTF_ASSERT_NOEXCEPT :
    texels{std::move(texels_)}, dim{dim_}, format{format_}
  {
    NTF_ASSERT(texels.has_data());
  }

public:
  r_image_data make_descriptor(const dim_type& offset = {},
                               uint32 level = 0, uint32 layer = 0) const
  {
    return r_image_data{
      .texels = this->texels.get(),
      .format = this->format,
      .extent = image_dim_cast(this->dim),
      .offset = image_dim_cast(offset),
      .layer = layer,
      .level = level,
    };
  };

public:
  array_type texels;
  dim_type dim;
  r_texture_format format;
};

template<
  image_depth_type T,
  uint32 array_size,
  image_dim_type Dim = uvec2,
  typename Deleter = allocator_delete<T, std::allocator<T>>
>
struct image_data_array {
public:
  static_assert(array_size > 0, "Array size can't be 0");

  using depth_type = T;
  using dim_type = Dim;

  using texel_type = image_depth_traits<T>::underlying_t;
  using array_type = std::array<unique_array<texel_type, Deleter>, array_size>;

public:
  image_data_array(array_type&& texels_,
                   dim_type dim_, r_texture_format format_) NTF_ASSERT_NOEXCEPT :
  texels{std::move(texels_)}, dim{dim_}, format{format_}
  {
    NTF_ASSERT([this](){
      for (const auto& tex : texels) {
        if (!tex.has_data()) {
          return false;
        }
      }
      return true;
    }(), "No data in texel array");
  }

public:
  std::array<r_image_data, array_size> make_descriptor(const dim_type& offset = {},
                                                       uint32 level = 0) const
  {
    std::array<r_image_data, array_size> out;
    for (size_t i = 0; i < array_size; ++i) {
      out[i].texels = texels[i].get();
      out[i].format = format;
      out[i].extent = image_dim_cast(dim);
      out[i].offset = image_dim_cast(offset);
      out[i].layer = static_cast<uint32>(i);
      out[i].level = level;
    }
    return out;
  }

public:
  array_type texels;
  dim_type dim;
  r_texture_format format;
};

template<
  image_depth_type T,
  image_dim_type Dim = uvec2,
  typename Deleter = allocator_delete<T, std::allocator<T>>
>
struct dense_image_data_array {
public:
  using depth_type = T;
  using dim_type = Dim;

  using texel_type = image_depth_traits<T>::underlying_t;
  using array_type = unique_array<texel_type, Deleter>;
  
public:
  dense_image_data_array(array_type&& texels_, size_t size_,
                         dim_type dim_, r_texture_format format_) NTF_ASSERT_NOEXCEPT :
    texels{std::move(texels_)}, size{size_}, dim{dim_}, format{format_}
  {
    NTF_ASSERT(texels.has_data(), "No data in texel array");
    NTF_ASSERT(size > 0, "Array size can't be 0");
  }

public:
  template<standard_allocator_type<r_image_data> Alloc = std::allocator<r_image_data>>
  auto make_descriptor(
    const dim_type& offset = {},
    uint32 level = 0,
    Alloc&& alloc = {}
  ) const NTF_ASSERT_NOEXCEPT -> unique_array<r_image_data, allocator_delete<T, Alloc>>
  {
    auto out = unique_array<r_image_data>::from_allocator(::ntf::uninitialized, size, alloc);
    NTF_ASSERT(out.has_data(), "Allocation failure");

    const auto image_stride = sizeof(texel_type)*[this]() -> size_t {
      if constexpr(std::same_as<Dim, uvec3>) {
        return static_cast<size_t>(dim.x*dim.y*dim.z);
      } else if constexpr(std::same_as<Dim, uvec2>) {
        return static_cast<size_t>(dim.x*dim.y);
      } else {
        return static_cast<size_t>(dim);
      }
    }();
    for (size_t i = 0; i < size; ++i) {
      std::construct_at(out.get()+i, r_image_data{
        .texels = texels.get()+i*image_stride,
        .format = format,
        .extent = image_dim_cast(dim),
        .offset = image_dim_cast(offset),
        .layer = static_cast<uint32>(i),
        .level = level,
      });
    }
    return out;
  }
  
public:
  array_type texels;
  size_t size;
  dim_type dim;
  r_texture_format format;
};

template<typename T, typename Deleter = allocator_delete<T, std::allocator<T>>>
using cubemap_image_data = image_data_array<T, 6u, uvec2, Deleter>;

template<typename T>
struct image_data_check : public std::false_type {};

template<typename T, typename Dim, typename Deleter>
struct image_data_check<image_data<T, Dim, Deleter>> : public std::true_type {};

template<typename T>
constexpr bool image_data_check_v = image_data_check<T>::value;

template<typename T>
concept image_data_type = image_data_check_v<T>;

enum class image_load_flags {
  none = 0,
  flip_y = 1 << 0,
};
NTF_DEFINE_ENUM_CLASS_FLAG_OPS(image_load_flags);

template<typename Loader, typename T, typename Dim>
concept image_loader_type = requires(Loader loader,
                                     typename Loader::data_t& data,
                                     const std::string& path,
                                     image_load_flags flags) {
  { loader.template parse<T>(path, flags) } -> 
    same_as_any<typename Loader::data_t, asset_expected<typename Loader::data_t>>;
  { loader.texels(data) } -> std::convertible_to<unique_array<
      typename image_depth_traits<T>::underlying_t,
      typename Loader::template deleter<T>
    >>;
  { loader.dimensions(data) } -> std::convertible_to<Dim>;
  { loader.format(data) } -> std::convertible_to<r_texture_format>;
};


namespace impl {

constexpr std::array<const char*, 4> img_RGBA_str = { "R", "RG", "RGB", "RGBA", };

template<typename>
struct img_depth_str {
  static constexpr const char* value = "";
};

template<>
struct img_depth_str<uint8> {
  static constexpr const char* value = "8 bit unsigned";
};

template<>
struct img_depth_str<uint16> {
  static constexpr const char* value = "16 bit unsigned";
};

template<>
struct img_depth_str<float32> {
  static constexpr const char* value = "32 bit float";
};

template<typename T, typename Deleter, bool>
struct load_image_ret {
  using type = asset_expected<image_data<T, Deleter>>;
};

template<typename T, typename Deleter>
struct load_image_ret<T, Deleter, false> {
  using type = image_data<T, Deleter>;
};

template<image_depth_type T, image_loader_type<T> Loader, bool checked>
load_image_ret<T, typename Loader::template deleter<T>, checked>::type load_image(
  const std::string& path,
  image_load_flags flags,
  Loader&& loader
) {
  using image_data_t = image_data<T, typename Loader::template deleter<T>>;
  uint32 width = 0, height = 0, channels = 0;
  auto make_data = [&](auto ptr) {
    NTF_ASSERT(width && height && channels);
    return image_data{
      std::move(ptr),
      static_cast<size_t>(width*height*channels),
      uvec2{width, height},
      parse_image_format<T>(channels)
    };
  };

  if constexpr (checked) {
    return asset_expected<image_data_t>::catch_error([&]() -> asset_expected<image_data_t> {
      if constexpr (checked_image_loader_type<Loader, T>) {
        auto ret = loader.template load<T>(path, width, height, channels, flags);
        if (!ret) { // Check asset_expected
          return unexpected{std::move(ret.error())};
        }
        return make_data(std::move(*ret));
      } else {
        auto ret = loader.template load<T>(::ntf::unchecked, path, width, height, channels, flags);
        if (!ret) { // Check unique_ptr
          return unexpected{asset_error{"Image loader returned nullptr"}};
        }
        return make_data(std::move(*ret));
      }
    });
  } else if constexpr (checked_image_loader_type<Loader, T>) {
    auto ret = loader.template load<T>(path, width, height, channels, flags);
    NTF_ASSERT(ret); // Check asset_expected
    return make_data(std::move(*ret));
  } else {
    auto ret = loader.template load<T>(::ntf::unchecked, path, width, height, channels, flags);
    NTF_ASSERT(ret); // Check unique_ptr
    return make_data(std::move(ret));
  }
}

} // namespace impl

class stb_image_loader {
public:
  template<typename T>
  struct deleter {
    void operator()(T* data) { stb_image_loader::stbi_delete(data); }
  };

  template<typename T>
  using stbi_ptr = std::unique_ptr<T, deleter<T>>;

public:
  template<image_depth_type T>
  auto load(const std::string& path,
            uint32& w, uint32& h, uint32& ch,
            image_load_flags flags) -> asset_expected<stbi_ptr<T>> {
    if constexpr (std::same_as<T, uint8>) {
      return load_rgb8u(path, w, h, ch, flags);
    } else if constexpr (std::same_as<T, uint16>) {
      return load_rgb16u(path, w, h, ch, flags);
    } else if constexpr (std::same_as<T, float32>) {
      return load_rgb32f(path, w, h, ch, flags);
    } else {
      NTF_UNREACHABLE();
    }
  }

  template<image_depth_type T>
  auto load(unchecked_t,
            const std::string& path,
            uint32& w, uint32& h, uint32& ch,
            image_load_flags flags) -> stbi_ptr<T> {
    if constexpr (std::same_as<T, uint8>) {
      return load_rgb8u(unchecked, path, w, h, ch, flags);
    } else if constexpr (std::same_as<T, uint16>) {
      return load_rgb16u(unchecked, path, w, h, ch, flags);
    } else if constexpr (std::same_as<T, float32>) {
      return load_rgb32f(unchecked, path, w, h, ch, flags);
    } else {
      NTF_UNREACHABLE();
    }
  }

  static optional<texture_meta> parse_meta(const std::string& path);

private:
  static void stbi_delete(void* data);

  auto load_rgb8u(const std::string& path,
                  uint32& w, uint32& h, uint32& ch,
                  image_load_flags flags) -> asset_expected<stbi_ptr<uint8>>;

  auto load_rgb8u(unchecked_t,
                  const std::string& path,
                  uint32& w, uint32& h, uint32& ch,
                  image_load_flags flags) -> stbi_ptr<uint8>;

  auto load_rgb16u(const std::string& path,
                   uint32& w, uint32& h, uint32& ch,
                   image_load_flags flags) -> asset_expected<stbi_ptr<uint16>>;

  auto load_rgb16u(unchecked_t,
                   const std::string& path,
                   uint32& w, uint32& h, uint32& ch,
                   image_load_flags flags) -> stbi_ptr<uint16>;

  auto load_rgb32f(const std::string& path,
                   uint32& w, uint32& h, uint32& ch,
                   image_load_flags flags) -> asset_expected<stbi_ptr<float32>>;

  auto load_rgb32f(unchecked_t,
                   const std::string& path,
                   uint32& w, uint32& h, uint32& ch,
                   image_load_flags flags) -> stbi_ptr<float32>;
};
static_assert(checked_image_loader_type<stb_image_loader, uint8>);
static_assert(checked_image_loader_type<stb_image_loader, uint16>);
static_assert(checked_image_loader_type<stb_image_loader, float32>);
static_assert(unchecked_image_loader_type<stb_image_loader, uint8>);
static_assert(unchecked_image_loader_type<stb_image_loader, uint16>);
static_assert(unchecked_image_loader_type<stb_image_loader, float32>);


template<image_depth_type T, checked_image_loader_type<T> Loader = stb_image_loader>
asset_expected<image_data<T, typename Loader::template deleter<T>>> load_image(
  const std::string& path,
  image_load_flags flags = image_load_flags::none,
  Loader&& loader = {}
) {
  return impl::load_image<T, Loader, true>(path, flags, std::forward<Loader>(loader));
}

template<image_depth_type T, unchecked_image_loader_type<T> Loader = stb_image_loader>
image_data<T, typename Loader::template deleter<T>> load_image(
  unchecked_t,
  const std::string& path,
  image_load_flags flags = image_load_flags::none,
  Loader&& loader = {}
) {
  return impl::load_image<T, Loader, false>(path, flags, std::forward<Loader>(loader));
}

// template<typename Alloc = std::allocator<uint8>>
// class cubemap_data {
// private:
//   static constexpr size_t face_count = 6;
//
// public:
//   using allocator_type = Alloc;
//
// public:
//   cubemap_data() noexcept(noexcept(Alloc{})) :
//     _alloc(Alloc{}) { _zero_array(); }
//   explicit cubemap_data(const Alloc& alloc) noexcept :
//     _alloc(alloc) { _zero_array(); }
//
//   explicit cubemap_data(std::array<std::string_view, face_count> paths) noexcept { 
//     _zero_array();
//     load(paths);
//   }
//   cubemap_data(std::array<std::string_view, face_count> paths, const Alloc& alloc) noexcept :
//     _alloc(alloc) {
//     _zero_array();
//     load(paths);
//   }
//
// public:
//   void load(std::array<std::string_view, face_count> paths) noexcept {
//     NTF_ASSERT(!has_data());
//     int w, h, ch;
//     uint8* stbi_data[face_count] = {0};
//     for (uint32 i = 0; i < face_count; ++i) {
//       uint8* curr_data = stbi_load(paths[i].data(), &w, &h, &ch, 0);
//       if (!curr_data) {
//         for (uint32 j = i-1; j >= 0; --j) {
//           stbi_image_free(stbi_data[j]);
//         }
//         return;
//       }
//       stbi_data[i] = curr_data;
//     }
//
//     size_t alloc_sz = w*h*ch*sizeof(uint8);
//     if constexpr (std::is_same_v<Alloc, std::allocator<uint8>>) {
//       std::memcpy(_data.data(), stbi_data, sizeof(uint8*)*face_count);
//     } else {
//       uint8* alloc_data = _alloc.allocate(alloc_sz*face_count);
//       if (!alloc_data) {
//         for (uint32 i = 0; i < face_count; ++i) {
//           stbi_image_free(stbi_data[i]);
//         }
//         return;
//       }
//
//       size_t offset = 0;
//       for (uint32 i = 0; i < face_count; ++i) {
//         uint8* pos = reinterpret_cast<uint8*>(impl::ptr_add(alloc_data, offset));
//         std::memcpy(pos, stbi_data[i], alloc_sz);
//         _data[i] = pos;
//         offset += alloc_sz;
//       }
//     }
//
//     _dim = uvec2{w, h};
//     _face_bytes = alloc_sz;
//     _format = r_texture_format::rgb8u;
//   }
//
//   void load(std::string_view json_path) noexcept {
//     NTF_ASSERT(!has_data());
//     auto dir = file_dir(json_path);
//     if (!dir) {
//       return;
//     }
//
//     using json = nlohmann::json;
//     std::ifstream f{json_path.data()};
//     json data = json::parse(f);
//     auto content = data["content"];
//     if (content.size() != face_count) {
//       return;
//     }
//
//     std::string paths[face_count];
//     std::array<std::string_view, face_count> load_paths;
//     for (uint32 i = 0; i < face_count; ++i) {
//       auto& curr = content[i];
//       paths[i] = dir.value()+"/"+curr["path"].get<std::string>();
//       load_paths[i] = paths[i];
//     }
//
//     load(load_paths);
//   }
//
//   void unload() noexcept {
//     NTF_ASSERT(has_data());
//     if constexpr (std::is_same_v<Alloc, std::allocator<uint8>>) {
//       for (uint i = 0; i < face_count; ++i) {
//         stbi_image_free(_data[i]);
//       }
//     } else {
//       _alloc.deallocate(_data[0], face_count*_face_bytes);
//     }
//
//     _zero_array();
//     _face_bytes = 0;
//   }
//
// private:
//   void _zero_array() { std::fill(_data.data(), _data.data()+face_count, nullptr); }
//
// public:
//   const uint8* const* data() const { return _data.data(); }
//   uint8** data() { return _data.data(); }
//   constexpr size_t count() const { return face_count; }
//   size_t size() const { return _face_bytes; }
//   uvec2 dim() const { return _dim; }
//   r_texture_format format() const { return _format; }
//
//   bool has_data() const { return _face_bytes > 0; }
//   explicit operator bool() const { return has_data(); }
//
// private:
//   [[maybe_unused]] Alloc _alloc;
//   std::array<uint8*, face_count> _data;
//   uvec2 _dim{0, 0};
//   size_t _face_bytes{0};
//   r_texture_format _format;
//
// public:
//   ~cubemap_data() noexcept {
//     if (has_data()) {
//       unload();
//     }
//   }
//
//   // use case for copying?
//   cubemap_data(const cubemap_data&) = delete;
//   cubemap_data& operator=(const cubemap_data&) = delete;
//   
//   cubemap_data(cubemap_data&& c) noexcept :
//     _alloc(std::move(c._alloc)),
//     _data(std::move(c._data)),
//     _dim(std::move(c._dim)),
//     _face_bytes(std::move(c._face_bytes)),
//     _format(std::move(c._format)) { c._zero_array(); c._face_bytes = 0; }
//   cubemap_data& operator=(cubemap_data&& c) noexcept {
//     if (std::addressof(c) == this) {
//       return *this;
//     }
//
//     if (has_data()) {
//       unload();
//     }
//
//     _alloc = std::move(c._alloc);
//     _data = std::move(c._data);
//     _dim = std::move(c._dim);
//     _face_bytes = std::move(c._face_bytes);
//     _format = std::move(c._format);
//
//     c._zero_array();
//     c._face_bytes = 0;
//
//     return *this;
//   }
// };
//
} // namespace ntf
