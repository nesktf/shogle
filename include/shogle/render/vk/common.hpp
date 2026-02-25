#pragma once

#include <shogle/render/common.hpp>
#include <shogle/util/expected.hpp>
#include <shogle/util/ptr.hpp>

#include <vulkan/vulkan_core.h>

#include <type_traits>

namespace shogle {

template<typename T>
using vk_view = ptr_view<std::remove_pointer_t<T>>;

using VmaAllocator = void*;
using VmaAllocation = void*;

class vk_pipeline;
class vk_private;
class vk_context;

enum class vk_buffer_type {
  vertex = 0,
  index,
  uniform,
};

using vk_handle = u64;
static constexpr vk_handle vk_invalid_handle = std::numeric_limits<vk_handle>::max();

std::string_view vk_error_string(VkResult result) noexcept;

template<typename T>
concept vk_provider_type = requires(T prov, const char** names, VkInstance vk,
                                    VkSurfaceKHR& surface, const VkAllocationCallbacks* vkalloc) {
  { prov.vk_get_extension_count() } -> std::convertible_to<size_t>;
  { prov.vk_get_extensions(names) } -> std::same_as<void>;
  { prov.vk_create_surface(vk, surface, vkalloc) } -> std::convertible_to<bool>;
  { prov.surface_extent() } -> std::same_as<extent2d>;
  requires noexcept(prov.vk_get_extension_count());
  requires noexcept(prov.vk_get_extensions(names));
  requires noexcept(prov.vk_create_sruface(vk, surface, vkalloc));
  requires noexcept(prov.surface_extent());
};

struct vk_version {
  u32 major;
  u32 minor;
};

class vk_surface_provider {
private:
  struct vtbl_t {
    size_t (*vk_get_extension_count)(void* user);
    void (*vk_get_extensions)(void* user, const char** names);
    bool (*vk_create_surface)(void* user, VkInstance vk, VkSurfaceKHR& surface,
                              const VkAllocationCallbacks* vkalloc);
    extent2d (*surface_extent)(void* user);
  };

  template<typename T>
  static constexpr vtbl_t vtbl_for{
    .vk_get_extension_count = +[](void* user) noexcept -> size_t {
      return (*static_cast<T>(user)).vk_get_extension_count();
    },
    .vk_get_extensions = +[](void* user, const char** names) noexcept -> void {
      (*static_cast<T>(user)).vk_get_extensions(names);
    },
    .vk_create_surface = +[](void* user, VkInstance vk, VkSurfaceKHR& surface,
                             const VkAllocationCallbacks* vkalloc) noexcept -> bool {
      return (*static_cast<T>(user)).vk_create_surface(vk, surface, vkalloc);
    },
    .surface_extent =
      +[](void* user) noexcept -> extent2d { return (*static_cast<T*>(user)).surface_extent(); },
  };

public:
  template<vk_provider_type T>
  vk_surface_provider(T& provider) noexcept :
      _provider(std::addressof(provider)), _vtbl(&vtbl_for<T>) {}

public:
  size_t extension_count() const noexcept { return _vtbl->vk_get_extension_count(_provider); }

  void get_extensions(const char** names) const noexcept {
    _vtbl->vk_get_extensions(_provider, names);
  }

  bool create_surface(VkInstance vk, VkSurfaceKHR& surface,
                      const VkAllocationCallbacks* vkalloc) const noexcept {
    return _vtbl->vk_create_surface(_provider, vk, surface, vkalloc);
  }

  extent2d surface_extent() const noexcept { return _vtbl->surface_extent(_provider); }

public:
  void* get_ptr() const noexcept { return _provider; }

private:
  void* _provider;
  const vtbl_t* _vtbl;
};

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
using vk_sv_expect = ::shogle::expected<T, vk_sv_error>;

template<typename T>
requires(std::is_arithmetic_v<T>)
constexpr VkExtent2D to_vk_extent2d(const extent2d& extent) {
  const auto [width, height] = extent;
  return VkExtent2D{.width = static_cast<u32>(width), .height = static_cast<u32>(height)};
}

} // namespace shogle
