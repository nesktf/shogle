#pragma once

#include "../util.hpp"
#include "./vk_common.hpp"

namespace keiki::render {

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
  static fn create(scratch_arena& arena, vk_view<VkInstance> vk, vk_view<VkSurfaceKHR> surface,
                   span<const char*> device_extensions, span<const char*> layers,
                   ptr_view<const VkAllocationCallbacks> vkalloc) -> vk_sv_expect<vk_device>;

  fn destroy(ptr_view<const VkAllocationCallbacks> vkalloc) -> void;

public:
  fn device() const -> vk_view<VkDevice>;
  fn physical_device() const -> vk_view<VkPhysicalDevice>;

  fn swapchain_formats() const -> span<const VkSurfaceFormatKHR>;
  fn swapchain_present_modes() const -> span<const VkPresentModeKHR>;
  fn swapchain_capabilities() const -> VkSurfaceCapabilitiesKHR;

  fn queue_families() const -> queue_family_indices;

  fn get_queue(queue_family family, u32 queue_index = 0u) const -> vk_view<VkQueue>;

  fn physical_device_props() const -> VkPhysicalDeviceProperties;

public:
  operator VkDevice() const { return device(); }

  operator VkPhysicalDevice() const { return physical_device(); }

private:
  VkPhysicalDevice _physical_device;
  VkDevice _device;
  queue_family_indices _family_indices;
  swapchain_caps _swapchain_capabilities;
};

} // namespace keiki::render
