#pragma once

#include <shogle/render/common.hpp>

#include <type_traits>
#include <vulkan/vulkan_core.h>

#include <vk_mem_alloc.h>

namespace shogle {

template<typename T>
using vk_view = T;

class vk_context;

enum class vk_buffer_type {
  vertex = 0,
  index,
  uniform,
};

using vk_handle = u64;
static constexpr vk_handle vk_invalid_handle = ntf::freelist_handle{}.as_u64();

struct vk_surface_provider {
  virtual ~vk_surface_provider() = default;
  virtual fn get_extensions(scratch_vec<const char*>& extensions) -> u32 = 0;
  virtual fn create_surface(VkInstance vk, VkSurfaceKHR& surface,
                            ptr_view<const VkAllocationCallbacks> vkalloc) -> bool = 0;
  virtual fn framebuffer_size() const -> std::pair<u32, u32> = 0;
};

fn vk_error_string(VkResult result) noexcept -> std::string_view;

class vk_error : public std::exception {
public:
  vk_error(std::string msg, VkResult err) : _msg(std::move(msg)), _err(err) {}

public:
  const char* what() const noexcept override { return _msg.c_str(); }

  VkResult code() const noexcept { return _err; }

  std::string_view code_msg() const noexcept { return vk_error_string(_err); }

private:
  std::string _msg;
  VkResult _err;
};

class vk_sv_error : public std::exception {
public:
  vk_sv_error(const char* msg, VkResult err) noexcept : _msg(msg), _err(err) {}

public:
  const char* what() const noexcept override { return _msg; }

  VkResult code() const noexcept { return _err; }

  std::string_view code_msg() const noexcept { return vk_error_string(_err); }

private:
  const char* _msg;
  VkResult _err;
};

template<typename T>
using vk_sv_expect = ntf::expected<T, vk_sv_error>;

template<typename T>
requires(std::is_arithmetic_v<T>)
constexpr fn to_vk_extent2d(const std::pair<T, T>& extent) -> VkExtent2D {
  const auto [width, height] = extent;
  return VkExtent2D{.width = static_cast<u32>(width), .height = static_cast<u32>(height)};
}

} // namespace shogle
