#pragma once

#include "./vk_device.hpp"

namespace keiki::render {

class vk_swapchain {
public:
  vk_swapchain(VkSwapchainKHR swapchain, VkFormat format, VkExtent2D extent,
               VkRenderPass renderpass, std::vector<VkImage>&& images,
               std::vector<VkImageView>&& image_views, std::vector<VkFramebuffer>&& framebuffers);

public:
  static fn create(const vk_device& device, vk_view<VkSurfaceKHR> surface, VkExtent2D extent,
                   vk_view<VkSwapchainKHR> old_swapchain,
                   ptr_view<const VkAllocationCallbacks> vkalloc) -> vk_sv_expect<vk_swapchain>;

  fn rebuild(const vk_device& device, vk_view<VkSurfaceKHR> surface, VkExtent2D extent,
             ptr_view<const VkAllocationCallbacks> vkalloc) -> vk_sv_expect<void>;
  fn destroy(const vk_device& device, ptr_view<const VkAllocationCallbacks> vkalloc) -> void;

public:
  fn swapchain() const -> vk_view<VkSwapchainKHR>;
  fn format() const -> VkFormat;
  fn extent() const -> VkExtent2D;
  fn renderpass() const -> vk_view<VkRenderPass>;
  fn images() const -> span<const VkImage>;
  fn image_views() const -> span<const VkImageView>;
  fn framebuffers() const -> span<const VkFramebuffer>;

public:
  operator VkSwapchainKHR() const { return swapchain(); }

  operator VkRenderPass() const { return renderpass(); }

private:
  VkSwapchainKHR _swapchain;
  VkFormat _format;
  VkExtent2D _extent;
  VkRenderPass _renderpass;
  std::vector<VkImage> _images;
  std::vector<VkImageView> _image_views;
  std::vector<VkFramebuffer> _framebuffers;
};

} // namespace keiki::render
