#define SHOGLE_ASSETS_INL
#include "./assets.hpp"
#undef SHOGLE_ASSETS_INL

namespace ntf {

template<typename T, typename Loader>
template<typename... Args>
resource_data<T, Loader>::resource_data(Args&&... args) {
  _load(std::forward<Args>(args)...);
}

template<typename T, typename Loader>
template<typename... Args>
auto resource_data<T, Loader>::load(Args&&... args) & -> resource_data& {
  _load(std::forward<Args>(args)...);
  return *this;
}

template<typename T, typename Loader>
template<typename... Args>
auto resource_data<T, Loader>::load(Args&&... args) && -> resource_data&& {
  _load(std::forward<Args>(args)...);
  return std::move(*this);
}

template<typename T, typename Loader>
auto resource_data<T, Loader>::make_resource() const -> std::optional<resource_type> {
  if (!has_data()) {
    return std::nullopt;
  }
  return _loader.make_resource();
}

template<typename T, typename Loader>
void resource_data<T, Loader>::unload() {
  if (!has_data()) {
    return;
  }

  _loader.resource_unload(false);

  _reset();
}


template<typename T, typename Loader>
template<typename... Args>
void resource_data<T, Loader>::_load(Args&&... args) {
  if (has_data()) {
    _loader.resource_unload(true);
    _has_data = false;
  }
  _has_data = _loader.resource_load(std::forward<Args>(args)...);
}

template<typename T, typename Loader>
void resource_data<T, Loader>::_reset() {
  _has_data = false;
}


template<typename T, typename Loader>
resource_data<T, Loader>::~resource_data() noexcept { unload(); }

template<typename T, typename Loader>
resource_data<T, Loader>::resource_data(resource_data&& d) noexcept :
  _loader(std::move(d._loader)), _has_data(std::move(d._has_data)) { d._reset(); }

template<typename T, typename Loader>
auto resource_data<T, Loader>::operator=(resource_data&& d) noexcept -> resource_data& {
  unload();

  _loader = std::move(d._loader);
  _has_data = std::move(d._has_data);

  d._reset();

  return *this;
}


inline std::optional<std::string> file_contents(std::string_view path) {
  std::string out {};
  std::fstream fs{path.data()};

  if (!fs.is_open()) {
    return std::nullopt;
  }

  std::ostringstream ss;
  ss << fs.rdbuf();
  out = ss.str();

  fs.close();
  return out;
}

inline std::optional<std::string> file_dir(std::string_view path) {
  auto pos = path.find_last_of('/');
  if (pos == std::string::npos) {
    return std::nullopt;
  }
  return std::string{path.substr(0, pos)};
}

} // namespace ntf
