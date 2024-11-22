#pragma once

#include "../stl/common.hpp"

#include <fstream>
#include <sstream>
#include <optional>

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

} // namespace ntf

#ifndef SHOGLE_ASSETS_INL
#include "./assets.inl"
#endif
