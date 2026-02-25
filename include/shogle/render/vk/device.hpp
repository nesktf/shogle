#pragma once

#include <shogle/render/vk/common.hpp>

#include <shogle/util/memory.hpp>

namespace shogle {

class vk_device {
public:
  struct swapchain_caps {
    vk_view<VkSurfaceKHR> surface;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> present_modes;
  };

  enum queue_family {
    QUEUE_FAMILY_GRAPHICS = 0,
    QUEUE_FAMILY_PRESENT,
    QUEUE_FAMILY_TRANSFER,

    QUEUE_FAMILY_COUNT,
  };

  struct queue_family_indices {
    u32 graphics;
    u32 present;
    u32 transfer;
  };

public:
  vk_device(VkPhysicalDevice physical_device, VkDevice device, const queue_family_indices& indices,
            swapchain_caps&& caps);

public:
  static auto create(mem::scratch_arena& arena, vk_view<VkInstance> vk,
                     vk_view<VkSurfaceKHR> surface, span<const char*> device_extensions,
                     span<const char*> layers, ptr_view<const VkAllocationCallbacks> vkalloc)
    -> vk_sv_expect<vk_device>;

  auto destroy(ptr_view<const VkAllocationCallbacks> vkalloc) -> void;

public:
  auto device() const -> vk_view<VkDevice>;
  auto physical_device() const -> vk_view<VkPhysicalDevice>;

  auto swapchain_formats() const -> span<const VkSurfaceFormatKHR>;
  auto swapchain_present_modes() const -> span<const VkPresentModeKHR>;
  auto swapchain_capabilities() const -> VkSurfaceCapabilitiesKHR;

  auto queue_families() const -> queue_family_indices;

  auto get_queue(queue_family family, u32 queue_index = 0u) const -> vk_view<VkQueue>;

  auto physical_device_props() const -> VkPhysicalDeviceProperties;

public:
  operator VkDevice() const { return _device; }

  operator VkPhysicalDevice() const { return _physical_device; }

private:
  VkPhysicalDevice _physical_device;
  VkDevice _device;
  queue_family_indices _family_indices;
  swapchain_caps _swapchain_capabilities;
};

} // namespace shogle
