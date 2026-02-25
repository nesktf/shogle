#pragma once

#include <shogle/render/vk/device.hpp>

namespace shogle {

class vk_swapchain {
public:
  vk_swapchain(VkSwapchainKHR swapchain, VkFormat format, VkExtent2D extent,
               VkRenderPass renderpass, std::vector<VkImage>&& images,
               std::vector<VkImageView>&& image_views, std::vector<VkFramebuffer>&& framebuffers);

public:
  static auto create(const vk_device& device, vk_view<VkSurfaceKHR> surface, VkExtent2D extent,
                     vk_view<VkSwapchainKHR> old_swapchain,
                     ptr_view<const VkAllocationCallbacks> vkalloc) -> vk_sv_expect<vk_swapchain>;

  auto rebuild(const vk_device& device, vk_view<VkSurfaceKHR> surface, VkExtent2D extent,
               ptr_view<const VkAllocationCallbacks> vkalloc) -> vk_sv_expect<void>;
  auto destroy(const vk_device& device, ptr_view<const VkAllocationCallbacks> vkalloc) -> void;

public:
  auto swapchain() const -> vk_view<VkSwapchainKHR>;
  auto format() const -> VkFormat;
  auto extent() const -> VkExtent2D;
  auto renderpass() const -> vk_view<VkRenderPass>;
  auto images() const -> span<const VkImage>;
  auto image_views() const -> span<const VkImageView>;
  auto framebuffers() const -> span<const VkFramebuffer>;

public:
  operator VkSwapchainKHR() const { return _swapchain; }

  operator VkRenderPass() const { return _renderpass; }

private:
  VkSwapchainKHR _swapchain;
  VkFormat _format;
  VkExtent2D _extent;
  VkRenderPass _renderpass;
  std::vector<VkImage> _images;
  std::vector<VkImageView> _image_views;
  std::vector<VkFramebuffer> _framebuffers;
};

} // namespace shogle
