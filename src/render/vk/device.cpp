#include "./vk_device.hpp"
#include <vulkan/vulkan_core.h>

namespace keiki::render {

vk_device::vk_device(VkPhysicalDevice physical_device, VkDevice device,
                     const queue_family_indices& indices, swapchain_caps&& caps) :
    _physical_device(physical_device), _device(device), _family_indices(indices),
    _swapchain_capabilities(std::move(caps)) {
  NTF_ASSERT(_physical_device != VK_NULL_HANDLE);
  NTF_ASSERT(_device != VK_NULL_HANDLE);
  NTF_ASSERT(!_swapchain_capabilities.formats.empty());
  NTF_ASSERT(!_swapchain_capabilities.present_modes.empty());
}

fn vk_device::create(scratch_arena& arena, vk_view<VkInstance> vk, vk_view<VkSurfaceKHR> surface,
                     span<const char*> device_extensions, span<const char*> layers,
                     ptr_view<const VkAllocationCallbacks> vkalloc) -> vk_sv_expect<vk_device> {
  u32 device_count{};
  VkResult res = vkEnumeratePhysicalDevices(vk, &device_count, nullptr);
  if (device_count == 0) {
    return {ntf::unexpect, "Failed to find a GPU with Vulkan support", res};
  }

  auto queue_families_cache = make_scratch_vec<VkQueueFamilyProperties>(arena);
  queue_family_indices indices;
  const fn find_queue_indices = [&](VkPhysicalDevice device) -> bool {
    queue_families_cache.clear();
    ntf::optional<u32> graphics_family;
    ntf::optional<u32> present_family;
    ntf::optional<u32> transfer_family;

    u32 queue_family_count{};
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, nullptr);
    queue_families_cache.resize(queue_family_count);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count,
                                             queue_families_cache.data());

    for (u32 i = 0; const auto& family : queue_families_cache) {
      // Require a device with a queue family that supports graphics commands
      if (family.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
        graphics_family.emplace(i);
      }

      // The same but for transfers
      // if (family.queueFlags & VK_QUEUE_TRANSFER_BIT) {
      //   indices.transfer_family = i;
      // }

      if ((family.queueFlags & VK_QUEUE_TRANSFER_BIT) &&
          !(family.queueFlags & VK_QUEUE_GRAPHICS_BIT)) {
        transfer_family.emplace(i);
      }

      // Require a device with a queue family that supports presentation
      // (might not be the same queue as the graphics one, so we store another index)
      VkBool32 present_support{false};
      vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &present_support);
      if (present_support) {
        present_family.emplace(i);
      }

      if (graphics_family.has_value() && present_family.has_value() &&
          transfer_family.has_value()) {
        indices.graphics = *graphics_family;
        indices.present = *present_family;
        indices.transfer = *transfer_family;
        return true;
      }

      ++i;
    }
    return false;
  };

  const fn check_extension_support = [&](VkPhysicalDevice device) -> bool {
    // Check if the physical device has all the extensions in device_extensions
    u32 ext_count{};
    vkEnumerateDeviceExtensionProperties(device, nullptr, &ext_count, nullptr);

    auto avail_ext = make_scratch_vec<VkExtensionProperties>(arena);
    avail_ext.resize(ext_count);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &ext_count, avail_ext.data());

    // auto required_extensions = make_scratch_set<std::string_view>(arena);
    std::unordered_set<std::string_view> required_extensions;
    required_extensions.insert(device_extensions.begin(), device_extensions.end());
    for (const auto& ext : avail_ext) {
      required_extensions.erase(ext.extensionName);
    }

    return required_extensions.empty();
  };

  swapchain_caps swapchain_detail;
  swapchain_detail.surface = surface;
  const fn query_swapchain_support = [&](VkPhysicalDevice device) -> bool {
    // Get supported formats and present modes from a device
    swapchain_detail.formats.clear();
    swapchain_detail.formats.clear();

    VkSurfaceCapabilitiesKHR caps;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &caps);

    u32 format_count{};
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &format_count, nullptr);
    if (format_count) {
      swapchain_detail.formats.resize(format_count);
      vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &format_count,
                                           swapchain_detail.formats.data());
    }

    u32 present_mode_count{};
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &present_mode_count, nullptr);
    if (present_mode_count) {
      swapchain_detail.present_modes.resize(present_mode_count);
      vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &present_mode_count,
                                                swapchain_detail.present_modes.data());
    }
    bool swapchain_adequate =
      !swapchain_detail.formats.empty() && !swapchain_detail.present_modes.empty();
    return swapchain_adequate;
  };

  auto devices = make_scratch_vec<VkPhysicalDevice>(arena);
  devices.resize(device_count);
  vkEnumeratePhysicalDevices(vk, &device_count, devices.data());

  VkPhysicalDevice physical_device = VK_NULL_HANDLE;
  for (const auto& dev : devices) {
    // Select the first suitable physical device found
    const bool has_queue_indices = find_queue_indices(dev);
    const bool ext_supported = check_extension_support(dev);
    const bool swapchain_adequate = query_swapchain_support(dev);
    if (has_queue_indices && ext_supported && swapchain_adequate) {
      physical_device = dev;
      break;
    }
  }
  if (physical_device == VK_NULL_HANDLE) {
    return {ntf::unexpect, "Failed to find a suitable GPU", VK_ERROR_DEVICE_LOST};
  }

  // Create a device to interface with the physical device
  std::array<VkDeviceQueueCreateInfo, vk_device::QUEUE_FAMILY_COUNT> queue_infos;
  u32 queue_idx = 0u;
  const float queue_priority{1.f}; // Priority for the scheduling of command buffer execution
  {
    // To avoid adding the same queue family more than once
    std::unordered_set<u32> unique_queue_families;
    // auto unique_queue_families = make_scratch_set<u32>(arena);
    unique_queue_families.emplace(indices.graphics);
    unique_queue_families.emplace(indices.present);
    unique_queue_families.emplace(indices.transfer);

    // How many queues we want for each queue family?
    for (u32 family : unique_queue_families) {
      VkDeviceQueueCreateInfo info{};
      info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
      info.queueFamilyIndex = family;
      info.queueCount = 1;
      info.pQueuePriorities = &queue_priority;
      queue_infos[queue_idx] = info;
      ++queue_idx;
    }
  }
  NTF_ASSERT(queue_idx > 0);

  VkPhysicalDeviceVulkan13Features vk13feats{};
  vk13feats.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
  vk13feats.dynamicRendering = true;
  vk13feats.synchronization2 = true;

  VkPhysicalDeviceVulkan12Features vk12feats{};
  vk12feats.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
  vk12feats.bufferDeviceAddress = true;
  vk12feats.descriptorIndexing = true;
  vk12feats.pNext = &vk13feats;

  // Which physical device features are we going to use?
  VkPhysicalDeviceFeatures features{}; // all VK_FALSE for now

  VkDeviceCreateInfo create_info{};
  create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  create_info.pQueueCreateInfos = queue_infos.data();
  create_info.queueCreateInfoCount = queue_idx;
  create_info.pEnabledFeatures = &features;
  create_info.pNext = &vk12feats;

  // Specify extensions and validation layers (device specific this time)
  create_info.enabledExtensionCount = static_cast<u32>(device_extensions.size());
  create_info.ppEnabledExtensionNames = device_extensions.data();

#ifndef NDEBUG
  create_info.enabledLayerCount = static_cast<u32>(layers.size());
  create_info.ppEnabledLayerNames = layers.data();
#else
  create_info.enabledLayerCount = 0;
#endif

  VkDevice device;
  res = vkCreateDevice(physical_device, &create_info, vkalloc, &device);
  if (res != VK_SUCCESS) {
    return {ntf::unexpect, "Failed to create logical device", res};
  }

  return {ntf::in_place, physical_device, device, indices, std::move(swapchain_detail)};
}

fn vk_device::destroy(ptr_view<const VkAllocationCallbacks> vkalloc) -> void {
  NTF_ASSERT(_device != VK_NULL_HANDLE, "vk_device use after free");
  vkDestroyDevice(_device, vkalloc);
}

fn vk_device::device() const -> vk_view<VkDevice> {
  NTF_ASSERT(_device != VK_NULL_HANDLE, "vk_device use after free");
  return _device;
}

fn vk_device::physical_device() const -> vk_view<VkPhysicalDevice> {
  NTF_ASSERT(_device != VK_NULL_HANDLE, "vk_device use after free");
  return _physical_device;
}

fn vk_device::physical_device_props() const -> VkPhysicalDeviceProperties {
  NTF_ASSERT(_device != VK_NULL_HANDLE, "vk_device use after free");
  VkPhysicalDeviceProperties props;
  vkGetPhysicalDeviceProperties(_physical_device, &props);
  return props;
}

fn vk_device::swapchain_formats() const -> span<const VkSurfaceFormatKHR> {
  NTF_ASSERT(_device != VK_NULL_HANDLE, "vk_device use after free");
  NTF_ASSERT(!_swapchain_capabilities.formats.empty(), "No swapchain formats in vk_device");
  return to_cspan(_swapchain_capabilities.formats);
}

fn vk_device::swapchain_present_modes() const -> span<const VkPresentModeKHR> {
  NTF_ASSERT(_device != VK_NULL_HANDLE, "vk_device use after free");
  NTF_ASSERT(!_swapchain_capabilities.present_modes.empty(),
             "No swapchain present modes in vk_device");
  return to_cspan(_swapchain_capabilities.present_modes);
}

fn vk_device::swapchain_capabilities() const -> VkSurfaceCapabilitiesKHR {
  NTF_ASSERT(_device != VK_NULL_HANDLE, "vk_device use after free");
  VkSurfaceCapabilitiesKHR caps;
  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(_physical_device, _swapchain_capabilities.surface,
                                            &caps);
  return caps;
}

fn vk_device::queue_families() const -> queue_family_indices {
  NTF_ASSERT(_device != VK_NULL_HANDLE, "vk_device use after free");
  return _family_indices;
}

fn vk_device::get_queue(queue_family family, u32 queue_index) const -> vk_view<VkQueue> {
  NTF_ASSERT(_device != VK_NULL_HANDLE, "vk_device use after free");
  NTF_ASSERT(family < QUEUE_FAMILY_COUNT, "Invalid queue family");
  const auto families = std::to_array<u32>({
    _family_indices.graphics,
    _family_indices.present,
    _family_indices.transfer,
  });
  VkQueue queue = VK_NULL_HANDLE;
  vkGetDeviceQueue(_device, families[family], queue_index, &queue);
  NTF_ASSERT(queue != VK_NULL_HANDLE, "Can't find queue with id {}", (u32)family);
  return queue;
}

} // namespace keiki::render
