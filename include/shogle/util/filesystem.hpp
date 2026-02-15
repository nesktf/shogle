#pragma once

#include <shogle/util/expected.hpp>
#include <shogle/util/memory.hpp>

namespace shogle {

using file_close_t = std::unique_ptr<std::FILE, decltype([](std::FILE* f) { std::fclose(f); })>;

expected<unique_array<u8>, std::error_code> read_entire_file(const char* path);
expected<std::string, std::error_code> read_entire_file_str(const char* path);
optional<std::string_view> split_file_dir(std::string_view);

} // namespace shogle
