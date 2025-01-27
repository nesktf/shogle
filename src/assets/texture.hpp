#pragma once

#include "./fs.hpp"
#include "../render/render.hpp"
#include "../stl/allocator.hpp"
#include "../stl/function.hpp"
#include "../stl/expected.hpp"

#include <nlohmann/json.hpp>
#include <stb/stb_image.h>

namespace ntf {

using asset_error = error<void>;

template<typename T>
using asset_expected = expected<T, asset_error>;

template<typename T>
concept image_depth_type = same_as_any<T, uint8, uint16, float32>;

template<typename T>
struct stbi_image_delete {
  void operator()(T* data) {
    stbi_image_free(data);
  }
};

template<typename Alloc>
class alloc_image_delete {
  Alloc _alloc;
  size_t _n;

public:
  alloc_image_delete(size_t n, const Alloc& alloc = {}) :
    _alloc{alloc}, _n{n} {}

  void operator()(std::allocator_traits<Alloc>::pointer data) {
    data->~T();
    _alloc.deallocate(data, _n);
  }
};

template<image_depth_type T>
r_texture_format parse_image_format(uint32) {
  // TODO: Implement this
  return r_texture_format::rgb8n;
}

template<image_depth_type T, typename Deleter = stbi_image_delete<T>>
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

template<typename T>
asset_expected<image_data<T>> load_image(std::string_view path, bool flip_vertically = true) {
  using data_t = image_data<T>;
  std::string fpath{path.data()}; // Make sure that it is null terminated
  int w, h, ch;
  T* stbi_data;
  stbi_set_flip_vertically_on_load(flip_vertically);
  if constexpr (std::same_as<T, uint8>) {
    stbi_data = stbi_load(fpath.data(), &w, &h, &ch, 0);
  } else if constexpr (std::is_same_v<T, uint16>) {
    stbi_data = stbi_load_16(fpath.data(), &w, &h, &ch, 0);
  } else if constexpr (std::is_same_v<T, float32>) {
    stbi_data = stbi_loadf(fpath.data(), &w, &h, &ch, 0);
  }
  if (!stbi_data) {
    return unexpected{asset_error::format({"Failed to load image \"{}\": {}"},
                                          path, stbi_failure_reason())};
  }

  SHOGLE_LOG(verbose, "[ntf::load_image] Loaded image \"{}\"", path.data());
  return {data_t{
    typename data_t::uptr_type{stbi_data}, static_cast<size_t>(w*h*ch),
    uvec2{w, h}, parse_image_format<T>(static_cast<uint32>(ch))
  }};
}

template<typename T, typename Alloc>
auto load_image(std::string_view path, Alloc&& alloc, bool flip_vertically = true)
                                      -> asset_expected<image_data<T, alloc_image_delete<Alloc>>> {
  using alloc_t = alloc_image_delete<Alloc>;
  using data_t = image_data<T, alloc_t>;
  std::string fpath{path.data()}; // Make sure that it is null terminated
  int w, h, ch;
  T* stbi_data;
  stbi_set_flip_vertically_on_load(flip_vertically);
  if constexpr (std::is_same_v<T, uint8>) {
    stbi_data = stbi_load(fpath.data(), &w, &h, &ch, 0);
  } else if constexpr (std::is_same_v<T, uint16>) {
    stbi_data = stbi_load_16(fpath.data(), &w, &h, &ch, 0);
  } else if constexpr ( std::is_same_v<T, float32>) {
    stbi_data = stbi_loadf(fpath.data(), &w, &h, &ch, 0);
  }
  if (!stbi_data) {
    return unexpected{asset_error::format({"Failed to load image \"{}\": {}"},
                                          path, stbi_failure_reason())};
  }
  size_t alloc_count = static_cast<size_t>(w*h*ch);
  T* alloc_data = alloc.allocate(alloc_count*sizeof(T));
  if (!alloc_data) {
    stbi_image_free(stbi_data);
    return unexpected{asset_error::format({"Failed to load image \"{}\": Allocation failed"},
                                          path)};
  }
  std::memcpy(alloc_data, stbi_data, alloc_count*sizeof(T));
  stbi_image_free(stbi_data);

  SHOGLE_LOG(verbose, "[ntf::image_data::load] Loaded image \"{}\"", path.data());
  return {data_t{
    typename data_t::uptr_type{alloc_data, alloc_t{std::forward<Alloc>(alloc), alloc_count}},
    alloc_count, uvec2{w, h}, parse_image_format<T>(static_cast<uint32>(ch))
  }};
}

template<typename T>
image_data<T> load_image(unchecked_t, std::string_view path, bool flip_vertically = true) {
  using data_t = image_data<T>;
  int w, h, ch;
  T* stbi_data;
  stbi_set_flip_vertically_on_load(flip_vertically);
  if constexpr (std::same_as<T, uint8>) {
    stbi_data = stbi_load(path.data(), &w, &h, &ch, 0);
  } else if constexpr (std::is_same_v<T, uint16>) {
    stbi_data = stbi_load_16(path.data(), &w, &h, &ch, 0);
  } else if constexpr ( std::is_same_v<T, float32>) {
    stbi_data = stbi_loadf(path.data(), &w, &h, &ch, 0);
  }
  if (!stbi_data) {
    return data_t{};
  }

  SHOGLE_LOG(verbose, "[ntf::image_data::load] Loaded image \"{}\"", path.data());
  return data_t{
    typename data_t::uptr_type{stbi_data}, static_cast<size_t>(w*h*ch),
    uvec2{w, h}, parse_image_format<T>(static_cast<uint32>(ch))
  };
}

template<typename T, typename Alloc>
auto load_image(unchecked_t, std::string_view path, Alloc&& alloc,
                bool flip_vertically = true) -> image_data<T, alloc_image_delete<Alloc>>{
  using alloc_t = alloc_image_delete<Alloc>;
  using data_t = image_data<T, alloc_t>;
  int w, h, ch;
  T* stbi_data;
  stbi_set_flip_vertically_on_load(flip_vertically);
  if constexpr (std::is_same_v<T, uint8>) {
    stbi_data = stbi_load(path.data(), &w, &h, &ch, 0);
  } else if constexpr (std::is_same_v<T, uint16>) {
    stbi_data = stbi_load_16(path.data(), &w, &h, &ch, 0);
  } else if constexpr ( std::is_same_v<T, float32>) {
    stbi_data = stbi_loadf(path.data(), &w, &h, &ch, 0);
  }
  if (!stbi_data) {
    return data_t{alloc_image_delete<Alloc>{std::forward<Alloc>(alloc), 0}};
  }
  size_t alloc_count = static_cast<size_t>(w*h*ch);
  T* alloc_data = alloc.allocate(alloc_count*sizeof(T));
  if (!alloc_data) {
    stbi_image_free(stbi_data);
    return data_t{alloc_image_delete<Alloc>{std::forward<Alloc>(alloc), 0}};
  }
  std::memcpy(alloc_data, stbi_data, alloc_count*sizeof(T));
  stbi_image_free(stbi_data);

  SHOGLE_LOG(verbose, "[ntf::image_data::load] Loaded image \"{}\"", path.data());
  return data_t{
    typename data_t::uptr_type{alloc_data, alloc_t{std::forward<Alloc>(alloc), alloc_count}},
      alloc_count, uvec2{w, h}, parse_image_format<T>(static_cast<uint32>(ch))
  };
}

inline optional<std::pair<uvec2, uint>> texture_info(std::string_view path) {
  int w, h, c;
  if (!stbi_info(path.data(), &w, &h, &c)) {
    // SHOGLE_LOG(error, "[ntf::texture_info] Failed to open texture: {}", stbi_failure_reason());
    return nullopt;
  }
  return std::make_pair(uvec2{w, h}, static_cast<uint32>(c));
}

template<typename Alloc = std::allocator<uint8>>
class texture_data {
public:
  using allocator_type = Alloc;

public:
  texture_data()
  noexcept(std::is_nothrow_default_constructible_v<Alloc>) :
    _alloc{Alloc{}} {}

  explicit texture_data(const Alloc& alloc)
  noexcept(std::is_nothrow_copy_constructible_v<Alloc>) :
    _alloc{alloc} {}

  explicit texture_data(std::string_view path) noexcept { load(path); }

  texture_data(std::string_view path, const Alloc& alloc)
  noexcept(std::is_nothrow_copy_constructible_v<Alloc>) :
    _alloc{alloc} { load(path); }

public:
  void load(std::string_view path) noexcept {
    NTF_ASSERT(!has_data());
    stbi_set_flip_vertically_on_load(true);
    int w, h, ch;
    uint8* stbi_data = stbi_load(path.data(), &w, &h, &ch, 0);
    if (!stbi_data) {
      SHOGLE_LOG(error, "[ntf::texture_data] Failed to load image \"{}\", {}",
                 path, stbi_failure_reason());
      return;
    }

    size_t alloc_sz = w*h*ch*sizeof(uint8);
    if constexpr (std::is_same_v<Alloc, std::allocator<uint8>>) {
      // stbi just mallocs, so no need to copy anything if not using a custom allocator
      _data = stbi_data;
    } else {
      uint8* alloc_data = _alloc.allocate(alloc_sz);
      if (!alloc_data) {
        stbi_image_free(stbi_data);
        return;
      }

      std::memcpy(alloc_data, stbi_data, alloc_sz);
      stbi_image_free(stbi_data);
      _data = alloc_data;
    }

    _dim = uvec2{w, h};
    _face_bytes = alloc_sz;
    _format = r_texture_format::rgb8u;
  }

  void unload() noexcept {
    NTF_ASSERT(has_data());

    if constexpr (std::is_same_v<Alloc, std::allocator<uint8>>) {
      stbi_image_free(_data);
    } else {
      _alloc.deallocate(_data, _face_bytes);
    }

    _data = nullptr;
  }

public:
  const uint8* data() const { return _data; }
  uint8* data() { return _data; }
  constexpr size_t count() const { return 1; }
  size_t size() const { return _face_bytes; }
  uvec2 dim() const { return _dim; }
  r_texture_format format() const { return _format; }

  bool has_data() const { return _data != nullptr; }
  explicit operator bool() const { return has_data(); }

private:
  [[maybe_unused]] Alloc _alloc;
  uint8* _data{nullptr};
  size_t _face_bytes{0};
  uvec2 _dim{0, 0};
  r_texture_format _format;

public:
  ~texture_data() noexcept {
    if (has_data()) {
      unload();
    }
  }

  // use case for copying?
  texture_data(const texture_data&) = delete;
  texture_data& operator=(const texture_data&) = delete;

  texture_data(texture_data&& t) noexcept :
    // _alloc(std::move(t._alloc)),
    _data(std::move(t._data)),
    _face_bytes(std::move(t._face_bytes)),
    _dim(std::move(t._dim)),
    _format(std::move(t._format)) { t._face_bytes = 0; t._data = nullptr; }
  texture_data& operator=(texture_data&& t) noexcept {
    if (std::addressof(t) == this) {
      return *this;
    }

    if (has_data()) {
      unload();
    }

    // _alloc = std::move(t._alloc);
    _data = std::move(t._data);
    _face_bytes = std::move(t._face_bytes);
    _dim = std::move(t._dim);
    _format = std::move(t._format);

    t._data = nullptr;
    t._face_bytes = 0;

    return *this;
  }
};

template<typename Alloc = std::allocator<uint8>>
class cubemap_data {
private:
  static constexpr size_t face_count = 6;

public:
  using allocator_type = Alloc;

public:
  cubemap_data() noexcept(noexcept(Alloc{})) :
    _alloc(Alloc{}) { _zero_array(); }
  explicit cubemap_data(const Alloc& alloc) noexcept :
    _alloc(alloc) { _zero_array(); }

  explicit cubemap_data(std::array<std::string_view, face_count> paths) noexcept { 
    _zero_array();
    load(paths);
  }
  cubemap_data(std::array<std::string_view, face_count> paths, const Alloc& alloc) noexcept :
    _alloc(alloc) {
    _zero_array();
    load(paths);
  }

public:
  void load(std::array<std::string_view, face_count> paths) noexcept {
    NTF_ASSERT(!has_data());
    int w, h, ch;
    uint8* stbi_data[face_count] = {0};
    for (uint32 i = 0; i < face_count; ++i) {
      uint8* curr_data = stbi_load(paths[i].data(), &w, &h, &ch, 0);
      if (!curr_data) {
        for (uint32 j = i-1; j >= 0; --j) {
          stbi_image_free(stbi_data[j]);
        }
        return;
      }
      stbi_data[i] = curr_data;
    }

    size_t alloc_sz = w*h*ch*sizeof(uint8);
    if constexpr (std::is_same_v<Alloc, std::allocator<uint8>>) {
      std::memcpy(_data.data(), stbi_data, sizeof(uint8*)*face_count);
    } else {
      uint8* alloc_data = _alloc.allocate(alloc_sz*face_count);
      if (!alloc_data) {
        for (uint32 i = 0; i < face_count; ++i) {
          stbi_image_free(stbi_data[i]);
        }
        return;
      }

      size_t offset = 0;
      for (uint32 i = 0; i < face_count; ++i) {
        uint8* pos = reinterpret_cast<uint8*>(impl::ptr_add(alloc_data, offset));
        std::memcpy(pos, stbi_data[i], alloc_sz);
        _data[i] = pos;
        offset += alloc_sz;
      }
    }

    _dim = uvec2{w, h};
    _face_bytes = alloc_sz;
    _format = r_texture_format::rgb8u;
  }

  void load(std::string_view json_path) noexcept {
    NTF_ASSERT(!has_data());
    auto dir = file_dir(json_path);
    if (!dir) {
      return;
    }

    using json = nlohmann::json;
    std::ifstream f{json_path.data()};
    json data = json::parse(f);
    auto content = data["content"];
    if (content.size() != face_count) {
      return;
    }

    std::string paths[face_count];
    std::array<std::string_view, face_count> load_paths;
    for (uint32 i = 0; i < face_count; ++i) {
      auto& curr = content[i];
      paths[i] = dir.value()+"/"+curr["path"].get<std::string>();
      load_paths[i] = paths[i];
    }

    load(load_paths);
  }

  void unload() noexcept {
    NTF_ASSERT(has_data());
    if constexpr (std::is_same_v<Alloc, std::allocator<uint8>>) {
      for (uint i = 0; i < face_count; ++i) {
        stbi_image_free(_data[i]);
      }
    } else {
      _alloc.deallocate(_data[0], face_count*_face_bytes);
    }

    _zero_array();
    _face_bytes = 0;
  }

private:
  void _zero_array() { std::fill(_data.data(), _data.data()+face_count, nullptr); }

public:
  const uint8* const* data() const { return _data.data(); }
  uint8** data() { return _data.data(); }
  constexpr size_t count() const { return face_count; }
  size_t size() const { return _face_bytes; }
  uvec2 dim() const { return _dim; }
  r_texture_format format() const { return _format; }

  bool has_data() const { return _face_bytes > 0; }
  explicit operator bool() const { return has_data(); }

private:
  [[maybe_unused]] Alloc _alloc;
  std::array<uint8*, face_count> _data;
  uvec2 _dim{0, 0};
  size_t _face_bytes{0};
  r_texture_format _format;

public:
  ~cubemap_data() noexcept {
    if (has_data()) {
      unload();
    }
  }

  // use case for copying?
  cubemap_data(const cubemap_data&) = delete;
  cubemap_data& operator=(const cubemap_data&) = delete;
  
  cubemap_data(cubemap_data&& c) noexcept :
    _alloc(std::move(c._alloc)),
    _data(std::move(c._data)),
    _dim(std::move(c._dim)),
    _face_bytes(std::move(c._face_bytes)),
    _format(std::move(c._format)) { c._zero_array(); c._face_bytes = 0; }
  cubemap_data& operator=(cubemap_data&& c) noexcept {
    if (std::addressof(c) == this) {
      return *this;
    }

    if (has_data()) {
      unload();
    }

    _alloc = std::move(c._alloc);
    _data = std::move(c._data);
    _dim = std::move(c._dim);
    _face_bytes = std::move(c._face_bytes);
    _format = std::move(c._format);

    c._zero_array();
    c._face_bytes = 0;

    return *this;
  }
};

} // namespace ntf
