#pragma once

#include "./types.hpp"
#include <ntfstl/unique_array.hpp>
#include <ntfstl/optional.hpp>

#include <fstream>
#include <filesystem>

namespace shogle {

namespace stdfs = std::filesystem;

inline ntf::optional<std::string> file_contents(std::string_view path) {
  std::string out {};
  std::fstream fs{path.data()};

  if (!fs.is_open()) {
    return ntf::nullopt;
  }

  std::ostringstream ss;
  ss << fs.rdbuf();
  out = ss.str();

  fs.close();
  return out;
}

inline ntf::optional<std::string> file_dir(std::string_view path) {
  auto pos = path.find_last_of('/');
  if (pos == std::string::npos) {
    return ntf::nullopt;
  }
  return std::string{path.substr(0, pos)};
}

using file_close_t = std::unique_ptr<std::FILE, decltype([](std::FILE* f){ std::fclose(f); })>;

template<typename Alloc = std::allocator<uint8>>
requires(ntf::meta::allocator_type<std::remove_cvref_t<Alloc>, uint8>)
auto file_data(
  const std::string& path, Alloc&& alloc = {}
) noexcept -> asset_expect<
  ntf::unique_array<uint8, ntf::allocator_delete<uint8, std::remove_cvref_t<Alloc>>>
> {
  stdfs::path in{path};

  std::error_code ec;
  const size_t len = static_cast<size_t>(stdfs::file_size(in, ec));
  if (ec) {
    return ntf::unexpected{asset_error::format({"Failed to open '{}', {}"},
                                          path, ec.message())};
  }
  if (len == 0) {
    return ntf::unexpected{asset_error::format({"Failed to open '{}', empty file"},
                                          path)};
  }
  auto buffer = ntf::unique_array<uint8>::from_size(::ntf::uninitialized, len,
                                               std::forward<Alloc>(alloc));
  if (!buffer) {
    return ntf::unexpected{asset_error::format({"Failed to open '{}', allocation failed"},
                                          path)};
  }
  std::ifstream in_file{path, std::ios_base::binary};
  in_file.read(reinterpret_cast<char*>(buffer.get()), len);
  in_file.close();

  return buffer;
}

} // namespace shogle
