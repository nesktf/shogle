#pragma once

#include <ntfstl/optional.hpp>
#include <ntfstl/unique_array.hpp>

#include <shogle/core.hpp>

#include <filesystem>
#include <fstream>

namespace shogle {

namespace stdfs = std::filesystem;

using file_close_t = std::unique_ptr<std::FILE, decltype([](std::FILE* f) { std::fclose(f); })>;

ntf::expected<ntf::unique_array<u8>, std::error_code> read_entire_file(const char* path);
ntf::expected<std::string, std::error_code> read_entire_file_str(const char* path);
ntf::optional<std::string_view> split_file_dir(std::string_view);

} // namespace shogle
