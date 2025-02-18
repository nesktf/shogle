#pragma once

#include "./asset.hpp"

#include "../render/render.hpp"
#include "../stl/function.hpp"

#include <nlohmann/json.hpp>

namespace ntf {

template<typename T>
concept image_depth_type = same_as_any<T, uint8, uint16, float32>;

template<image_depth_type T>
r_texture_format parse_image_format(uint32) {
  // TODO: Implement this
  return r_texture_format::rgb8n;
}

template<image_depth_type T, typename Deleter = std::default_delete<T>>
class image_data {
public:
  using uptr_type = std::unique_ptr<T, Deleter>;

public:
  image_data() noexcept :
    _data{nullptr}, _size{}, _dim{}, _format{} {}

  explicit image_data(const Deleter& del) noexcept :
    _data{nullptr, del}, _size{}, _dim{}, _format{} {}

  image_data(uptr_type data, size_t size, uvec2 dim, r_texture_format format) noexcept :
    _data{std::move(data)}, _size{size}, _dim{dim}, _format{format} {}

public:
  void unload() { _data.reset(); }

public:
  const T* data() const { return _data.get(); }
  T* data() { return _data.get(); }

  size_t size() const { return _size; }
  size_t bytes() const { return sizeof(T)*size(); }
  uvec2 dim() const { return _dim; }
  r_texture_format format() const { return _format; }

  bool has_data() const { return _data.get() != nullptr; }
  explicit operator bool() const { return has_data(); }

  r_image_data descriptor(uvec3 offset = {0, 0, 0}, uint32 layer = 0, uint32 level = 0) const {
    return r_image_data{
      .texels = data(),
      .format = format(),
      .extent = {dim(), 0},
      .offset = offset,
      .layer = layer,
      .level = level,
    };
  }

private:
  uptr_type _data;
  size_t _size;
  uvec2 _dim;
  r_texture_format _format;
};

enum class image_load_flags {
  none = 0,
  flip_vertically = 1 << 0,
};
NTF_DEFINE_ENUM_CLASS_FLAG_OPS(image_load_flags);

template<typename Loader, typename T>
concept checked_image_loader_type = requires(Loader loader,
                                             const std::string& path,
                                             uint32& w, uint32& h, uint32& ch,
                                             image_load_flags flags) {
  { loader.template load<T>(path, w, h, ch, flags) }
    -> std::same_as<asset_expected<std::unique_ptr<T, typename Loader::template deleter<T>>>>;
};

template<typename Loader, typename T>
concept unchecked_image_loader_type = requires(Loader loader,
                                               const std::string& path,
                                               uint32& w, uint32& h, uint32& ch,
                                               image_load_flags flags) {
  { loader.template load<T>(unchecked_t{}, path, w, h, ch, flags) }
    -> std::same_as<std::unique_ptr<T, typename Loader::template deleter<T>>>;
};

template<typename Loader, typename T>
concept image_loader_type =
  checked_image_loader_type<Loader, T> || unchecked_image_loader_type<Loader, T>;

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
    try {
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
    } catch (asset_error& err) {
      return unexpected{std::move(err)};
    } catch (const std::exception& err) {
      return unexpected{asset_error::format({"{}"}, err.what())};
    } catch (...) {
      return unexpected{asset_error{"Unknown error"}};
    }
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
