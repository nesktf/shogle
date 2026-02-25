#include "./vk_swapchain.hpp"

namespace keiki::render {

vk_swapchain::vk_swapchain(VkSwapchainKHR swapchain, VkFormat format, VkExtent2D extent,
                           VkRenderPass renderpass, std::vector<VkImage>&& images,
                           std::vector<VkImageView>&& image_views,
                           std::vector<VkFramebuffer>&& framebuffers) :
    _swapchain(swapchain), _format(format), _extent(extent), _renderpass(renderpass),
    _images(std::move(images)), _image_views(std::move(image_views)),
    _framebuffers(std::move(framebuffers)) {
  NTF_ASSERT(_swapchain != VK_NULL_HANDLE);
  NTF_ASSERT(_renderpass != VK_NULL_HANDLE);
  NTF_ASSERT(_extent.height > 0);
  NTF_ASSERT(_extent.width > 0);
  NTF_ASSERT(!_images.empty());
  NTF_ASSERT(!_image_views.empty());
  NTF_ASSERT(!_framebuffers.empty());
}

fn vk_swapchain::create(const vk_device& device, vk_view<VkSurfaceKHR> surface, VkExtent2D extent,
                        vk_view<VkSwapchainKHR> old_swapchain,
                        ptr_view<const VkAllocationCallbacks> vkalloc)
  -> vk_sv_expect<vk_swapchain> {
  const auto capabilities = device.swapchain_capabilities();

  // What is the format of the swapchain images?
  const fn find_format = [&]() -> VkSurfaceFormatKHR {
    // Try to get a swap surface with B8G8R8A8 pixel format or just use the first one
    // if there is not one available
    const auto formats = device.swapchain_formats();
    for (const auto& format : formats) {
      if (format.format == VK_FORMAT_B8G8R8A8_SRGB &&
          format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
        return format;
      }
    }
    return formats[0];
  };

  // How are the images in the swapchain presented?
  const fn find_present_mode = [&]() -> VkPresentModeKHR {
    // VK_PRESENT_MODE_IMMEDIATE_KHR -> Images submited are transferred to the screen immediately
    // VK_PRESENT_MODE_FIFO_KHR -> The swap chain is a FIFO queue of images (double buffering)
    // VK_PRESENT_MODE_FIFO_RELAXED_KHR -> Doesn't wait for next vblank if the queue was empty
    // VK_PRESENT_MODE_MAILBOX_KHR -> Triple buffering

    const auto modes = device.swapchain_present_modes();
    for (const auto& mode : modes) {
      if (mode == VK_PRESENT_MODE_MAILBOX_KHR) {
        return mode;
      }
    }

    return VK_PRESENT_MODE_FIFO_KHR; // Only one guaranteed to be available
  };

  // Resolution of the swapchain images (in pixels), usually the resolution of the window
  const fn find_extent = [&]() -> VkExtent2D {
    if (capabilities.currentExtent.width != std::numeric_limits<u32>::max()) {
      return capabilities.currentExtent;
    }

    VkExtent2D actual_extent = extent;
    actual_extent.height = std::clamp(actual_extent.width, capabilities.minImageExtent.width,
                                      capabilities.maxImageExtent.width);
    actual_extent.width = std::clamp(actual_extent.height, capabilities.minImageExtent.height,
                                     capabilities.maxImageExtent.height);
    return actual_extent;
  };

  const auto swapchain_format = find_format();
  const auto image_format = swapchain_format.format;
  const auto swapchain_extent = find_extent();

  // Images that will be stored in the swap chain
  // Add one to avoid waiting for the driverto complete operatons
  u32 image_count = capabilities.minImageCount + 1;
  if (capabilities.maxImageCount > 0 && image_count > capabilities.maxImageCount) {
    // Clamp
    image_count = capabilities.maxImageCount;
  }

  VkSwapchainCreateInfoKHR create_info{};
  create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  create_info.surface = surface;
  create_info.imageFormat = image_format;
  create_info.minImageCount = image_count;
  create_info.imageColorSpace = swapchain_format.colorSpace;
  create_info.imageExtent = swapchain_extent;
  create_info.imageArrayLayers = 1; // Layers for each image, 1 if not using stereoscopic 3D

  // Sets the operations for the swapchain, in this case we only render directly
  create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
  // create_info.imageUsage = VK_IMAGE_USAGE_TRANSFER_DST_BIT; // For postprocessing

  const auto queue_families = device.queue_families();
  const u32 queue_indices[] = {queue_families.graphics, queue_families.present};
  // imageSharingMode only refers to ownership transfer needs
  if (queue_families.graphics != queue_families.present) {
    create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT; // Shared across queue families
    create_info.queueFamilyIndexCount = 2;
    create_info.pQueueFamilyIndices = queue_indices;
  } else {
    create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE; // Owned by a single queue family
    // create_info.queueFamilyIndexCount = 0;
    // create_info.pQueueFamilyIndices = nullptr;
  }

  // I hate inverted framebuffers
  create_info.preTransform = capabilities.currentTransform;

  // For blending with other windows
  create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

  create_info.presentMode = find_present_mode();
  create_info.clipped = VK_TRUE;

  // Used when the swap chain has to be reconstructed
  create_info.oldSwapchain = old_swapchain;

  VkSwapchainKHR swapchain;
  VkResult res = vkCreateSwapchainKHR(device, &create_info, vkalloc, &swapchain);
  if (res != VK_SUCCESS) {
    return {ntf::unexpect, "Failed to create swapchain", res};
  }

  std::vector<VkImage> swapchain_images;
  vkGetSwapchainImagesKHR(device, swapchain, &image_count, nullptr);
  swapchain_images.resize(image_count);
  vkGetSwapchainImagesKHR(device, swapchain, &image_count, swapchain_images.data());

  std::vector<VkImageView> swapchain_image_views;
  swapchain_image_views.resize(swapchain_images.size());
  for (u32 i = 0; i < swapchain_images.size(); ++i) {
    VkImageViewCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    create_info.image = swapchain_images[i];

    create_info.viewType = VK_IMAGE_VIEW_TYPE_2D; // Treat the image as a 2D texture
    create_info.format = image_format;

    // Map the color channels to something, VK_COMPONENT_SWIZZLE_IDENTITY by default
    create_info.components = VkComponentMapping{
      .r = VK_COMPONENT_SWIZZLE_IDENTITY,
      .g = VK_COMPONENT_SWIZZLE_IDENTITY,
      .b = VK_COMPONENT_SWIZZLE_IDENTITY,
      .a = VK_COMPONENT_SWIZZLE_IDENTITY,
    };

    // Only one layer per image
    create_info.subresourceRange = VkImageSubresourceRange{
      .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
      .baseMipLevel = 0,
      .levelCount = 1,
      .baseArrayLayer = 0,
      .layerCount = 1,
    };

    VK_ASSERT(vkCreateImageView(device, &create_info, vkalloc, &swapchain_image_views[i]));
  }

  // Specify all the framebuffer attachments that will be used while rendering

  // A single color buffer attachment, represented by one of the images from the swap chain
  VkAttachmentDescription color_attachment{};
  color_attachment.format = image_format; // Same as the swapchain image format
  color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;

  // What will happen with the data in the attachment before and after rendering
  color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;   // Clear values at the start
  color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE; // Store rendered contents in memory

  // No stencil buffer for now
  color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

  // Layout for the framebuffer image (?) before and after the render pass
  color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

  // Things for subpasses, each one references one or more of the previous attachments
  VkAttachmentReference color_attachment_ref{};
  color_attachment_ref.attachment = 0;
  color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  // Only one subpass for the fragment shader out_color
  VkSubpassDescription subpass{};
  subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS; // It's a graphics subpass
  subpass.colorAttachmentCount = 1;

  // This maps to the location index in the shader:
  // layout(location = 0) out vec4 out_color;
  subpass.pColorAttachments = &color_attachment_ref;

  VkSubpassDependency dep{};
  dep.srcSubpass = VK_SUBPASS_EXTERNAL;
  dep.dstSubpass = 0;
  dep.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  dep.srcAccessMask = 0;
  dep.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  dep.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

  VkRenderPassCreateInfo render_pass_info{};
  render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  render_pass_info.attachmentCount = 1;
  render_pass_info.pAttachments = &color_attachment;
  render_pass_info.subpassCount = 1;
  render_pass_info.pSubpasses = &subpass;
  render_pass_info.dependencyCount = 1;
  render_pass_info.pDependencies = &dep;

  VkRenderPass renderpass;
  res = vkCreateRenderPass(device, &render_pass_info, vkalloc, &renderpass);
  if (res != VK_SUCCESS) {
    for (auto& view : swapchain_image_views) {
      vkDestroyImageView(device, view, vkalloc);
    }
    vkDestroySwapchainKHR(device, swapchain, vkalloc);
    return {ntf::unexpect, "Failed to create render pass", res};
  }

  std::vector<VkFramebuffer> framebuffers;
  framebuffers.resize(swapchain_image_views.size());
  for (u32 i = 0; i < swapchain_image_views.size(); ++i) {
    VkImageView attachments[] = {swapchain_image_views[i]};

    VkFramebufferCreateInfo framebuffer{};
    framebuffer.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebuffer.renderPass = renderpass;
    framebuffer.attachmentCount = 1;
    framebuffer.pAttachments = attachments;
    framebuffer.width = swapchain_extent.width;
    framebuffer.height = swapchain_extent.height;
    framebuffer.layers = 1; // Layers in the image arrays

    VK_ASSERT(vkCreateFramebuffer(device, &framebuffer, vkalloc, &framebuffers[i]));
  }

  return {ntf::in_place,
          swapchain,
          image_format,
          swapchain_extent,
          renderpass,
          std::move(swapchain_images),
          std::move(swapchain_image_views),
          std::move(framebuffers)};
}

fn vk_swapchain::rebuild(const vk_device& device, vk_view<VkSurfaceKHR> surf, VkExtent2D extent,
                         ptr_view<const VkAllocationCallbacks> vkalloc) -> vk_sv_expect<void> {
  return ::keiki::render::vk_swapchain::create(device, surf, extent, _swapchain, vkalloc)
    .transform([&](vk_swapchain&& new_swapchain) -> void {
      destroy(device, vkalloc);
      *this = std::move(new_swapchain);
    });
}

fn vk_swapchain::destroy(const vk_device& device, ptr_view<const VkAllocationCallbacks> vkalloc)
  -> void {
  NTF_ASSERT(_swapchain != VK_NULL_HANDLE, "vk_swapchain use after free");
  for (auto& fb : _framebuffers) {
    vkDestroyFramebuffer(device, fb, vkalloc);
  }
  vkDestroyRenderPass(device, _renderpass, vkalloc);
  for (auto& view : _image_views) {
    vkDestroyImageView(device, view, vkalloc);
  }
  vkDestroySwapchainKHR(device, _swapchain, vkalloc);
  _swapchain = VK_NULL_HANDLE;
}

fn vk_swapchain::swapchain() const -> vk_view<VkSwapchainKHR> {
  NTF_ASSERT(_swapchain != VK_NULL_HANDLE, "vk_swapchain use after free");
  return _swapchain;
}

fn vk_swapchain::renderpass() const -> vk_view<VkRenderPass> {
  NTF_ASSERT(_swapchain != VK_NULL_HANDLE, "vk_swapchain use after free");
  return _renderpass;
}

fn vk_swapchain::format() const -> VkFormat {
  NTF_ASSERT(_swapchain != VK_NULL_HANDLE, "vk_swapchain use after free");
  return _format;
}

fn vk_swapchain::extent() const -> VkExtent2D {
  NTF_ASSERT(_swapchain != VK_NULL_HANDLE, "vk_swapchain use after free");
  return _extent;
}

fn vk_swapchain::images() const -> span<const VkImage> {
  NTF_ASSERT(_swapchain != VK_NULL_HANDLE, "vk_swapchain use after free");
  NTF_ASSERT(!_images.empty(), "Empty image set");
  return to_cspan(_images);
}

fn vk_swapchain::image_views() const -> span<const VkImageView> {
  NTF_ASSERT(_swapchain != VK_NULL_HANDLE, "vk_swapchain use after free");
  NTF_ASSERT(!_image_views.empty(), "Empty image view set");
  return to_cspan(_image_views);
}

fn vk_swapchain::framebuffers() const -> span<const VkFramebuffer> {
  NTF_ASSERT(_swapchain != VK_NULL_HANDLE, "vk_swapchain use after free");
  return to_cspan(_framebuffers);
}

} // namespace keiki::render
