#pragma once

#include "../stl/optional.hpp"

namespace ntf {

template<typename T, typename Loader>
class resource_data {
public:
  using loader_type = Loader;
  using resource_type = T;

  static_assert(std::is_same_v<T, typename loader_type::resource_type>);
  using data_type = loader_type::data_type;

public:
  resource_data() = default;

  template<typename... Args>
  resource_data(Args&&... args);

  template<typename... Args>
  resource_data& load(Args&&... args) &;

  template<typename... Args>
  resource_data&& load(Args&&... args) &&;

  std::optional<resource_type> make_resource() const;

  void unload();

public:
  const data_type& data() const { return _loader.data(); }
  data_type& data() { return _loader.data(); }

  bool has_data() const { return _has_data; }
  explicit operator bool() { return has_data(); }

private:
  template<typename... Args>
  void _load(Args&&... args);

  void _reset();

private:
  loader_type _loader;
  bool _has_data{false};

public:
  NTF_DECLARE_MOVE_ONLY(resource_data);
};

std::optional<std::string> file_contents(std::string_view path);
std::optional<std::string> file_dir(std::string_view path);

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
