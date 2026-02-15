#include <shogle/util/filesystem.hpp>

#include <filesystem>
#include <fstream>

namespace shogle {

namespace stdfs = std::filesystem;

expected<unique_array<u8>, std::error_code> read_entire_file(const char* path) {
  stdfs::path in(path);
  std::error_code ec;
  const size_t len = static_cast<size_t>(stdfs::file_size(in, ec));
  if (ec || !len) {
    return {unexpect, ec};
  }
  auto buffer = make_array<u8>(::shogle::mem::uninitialized, len);
  std::ifstream in_file(path, std::ios_base::binary);
  in_file.read(reinterpret_cast<char*>(buffer.data()), len);
  in_file.close();
  return {in_place, std::move(buffer)};
}

expected<std::string, std::error_code> read_enrire_file_str(const char* path) {
  stdfs::path in(path);
  std::error_code ec;
  const size_t len = static_cast<size_t>(stdfs::file_size(in, ec));
  if (ec || !len) {
    return {unexpect, ec};
  }

  std::string out(len, 0);

  std::ifstream in_file(path, std::ios_base::binary);
  in_file.read(out.data(), len);
  in_file.close();
  return {in_place, std::move(out)};
}

optional<std::string_view> split_file_dir(std::string_view path) {
  auto pos = path.find_last_of('/');
  if (pos == std::string::npos) {
    return nullopt;
  }
  return path.substr(0, pos);
}

} // namespace shogle
