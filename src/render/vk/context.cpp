#include "./vk_context.hpp"
#include <vulkan/vulkan_core.h>

#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

#include <sys/mman.h>

#define KEIKI_VULKAN_VERSION VK_API_VERSION_1_3
#define KEIKI_APP_VERSION    VK_MAKE_VERSION(KEIKI_VER_MAJ, KEIKI_VER_MIN, KEIKI_VER_REV)
#define KEIKI_ENGINE_NAME    KEIKI_APP_NAME " render engine"
#define KEIKI_ENGINE_VERSION VK_MAKE_VERSION(KEIKI_VER_MAJ, KEIKI_VER_MIN, KEIKI_VER_REV)

namespace keiki::render {

#define VKSTR(code) \
  case code:        \
    return #code;

fn vk_error_string(VkResult res) noexcept -> std::string_view {
  switch (res) {
    VKSTR(VK_NOT_READY);
    VKSTR(VK_TIMEOUT);
    VKSTR(VK_EVENT_SET);
    VKSTR(VK_EVENT_RESET);
    VKSTR(VK_INCOMPLETE);
    VKSTR(VK_ERROR_OUT_OF_HOST_MEMORY);
    VKSTR(VK_ERROR_OUT_OF_DEVICE_MEMORY);
    VKSTR(VK_ERROR_INITIALIZATION_FAILED);
    VKSTR(VK_ERROR_DEVICE_LOST);
    VKSTR(VK_ERROR_MEMORY_MAP_FAILED);
    VKSTR(VK_ERROR_LAYER_NOT_PRESENT);
    VKSTR(VK_ERROR_EXTENSION_NOT_PRESENT);
    VKSTR(VK_ERROR_FEATURE_NOT_PRESENT);
    VKSTR(VK_ERROR_INCOMPATIBLE_DRIVER);
    VKSTR(VK_ERROR_TOO_MANY_OBJECTS);
    VKSTR(VK_ERROR_FORMAT_NOT_SUPPORTED);
    VKSTR(VK_ERROR_SURFACE_LOST_KHR);
    VKSTR(VK_ERROR_NATIVE_WINDOW_IN_USE_KHR);
    VKSTR(VK_SUBOPTIMAL_KHR);
    VKSTR(VK_ERROR_OUT_OF_DATE_KHR);
    VKSTR(VK_ERROR_INCOMPATIBLE_DISPLAY_KHR);
    VKSTR(VK_ERROR_VALIDATION_FAILED_EXT);
    VKSTR(VK_ERROR_INVALID_SHADER_NV);
    default:
      return "UNKNOWN_ERROR";
  }
};

#undef VKSTR

namespace {

// Load vkCreateDebugUtilsMessengerEXT, since is an extension and is not loaded by default
VkResult CreateDebugUtilsMessengerEXT(VkInstance instance,
                                      const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
                                      const VkAllocationCallbacks* pAllocator,
                                      VkDebugUtilsMessengerEXT* pDebugMessenger) {
  auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
    instance, "vkCreateDebugUtilsMessengerEXT");
  if (func != nullptr) {
    return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
  } else {
    return VK_ERROR_EXTENSION_NOT_PRESENT;
  }
}

// Do the same with vkDestroyDebugUtilsMessengerEXT
void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger,
                                   const VkAllocationCallbacks* pAllocator) {
  auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
    instance, "vkDestroyDebugUtilsMessengerEXT");
  if (func) {
    func(instance, debugMessenger, pAllocator);
  }
}

VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(
  VkDebugUtilsMessageSeverityFlagBitsEXT msg_severity, VkDebugUtilsMessageTypeFlagsEXT msg_type,
  const VkDebugUtilsMessengerCallbackDataEXT* callback_data, void* user_data) {
  NTF_UNUSED(user_data);
  // auto& messenger = *static_cast<debug_messenger*>(user_data);

  const char* kind;
  switch (msg_type) {
    case VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT:
      kind = "GENERAL";
      break;
    case VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT:
      kind = "VALIDATION";
      break;
    case VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT:
      kind = "PERFORMANCE";
      break;
    default:
      kind = "UNKNOWN";
      break;
  }

  switch (msg_severity) {
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
      logger::verbose("[VK_LAYERS][{}] {}", kind, callback_data->pMessage);
      break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
      logger::info("[VK_LAYERS][{}] {}", kind, callback_data->pMessage);
      break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
      logger::warning("[VK_LAYERS][{}] {}", kind, callback_data->pMessage);
      break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
      logger::error("[VK_LAYERS][{}] {}", kind, callback_data->pMessage);
      break;
    default:
      logger::verbose("[VK_LAYERS][{}] {}", kind, callback_data->pMessage);
      break;
  }

  return VK_FALSE; // Should the callback abort the call that triggered the message?
}

fn create_instance(scratch_arena& arena, VkDebugUtilsMessengerEXT& messenger,
                   span<const char*> extensions, span<const char*> layers,
                   ptr_view<const VkAllocationCallbacks> vkalloc) -> vk_sv_expect<VkInstance> {
#ifndef NDEBUG
  fn check_layer_support = [&]() -> bool {
    u32 layer_count{};
    vkEnumerateInstanceLayerProperties(&layer_count, nullptr);

    auto avail = make_scratch_vec<VkLayerProperties>(arena);
    avail.resize(layer_count);
    vkEnumerateInstanceLayerProperties(&layer_count, avail.data());

    for (const char* layer : layers) {
      bool found{false};
      for (const auto& props : avail) {
        if (std::strcmp(layer, props.layerName) == 0) {
          found = true;
          break;
        }
      }
      if (!found) {
        return false;
      }
    }

    return true;
  };

  if (!check_layer_support()) {
    return {ntf::unexpect, "Validation layers not available", VK_ERROR_VALIDATION_FAILED_EXT};
  }
#endif

  VkApplicationInfo app_info{};
  app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO; // struct type
  // app_info.pNext = nullptr; // No extensions, not needed if default constructed
  app_info.pApplicationName = KEIKI_APP_NAME;
  app_info.applicationVersion = KEIKI_APP_VERSION;
  app_info.pEngineName = KEIKI_ENGINE_NAME;
  app_info.engineVersion = KEIKI_ENGINE_VERSION;
  app_info.apiVersion = KEIKI_VULKAN_VERSION;

  VkInstanceCreateInfo create_info{};
  create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  create_info.pApplicationInfo = &app_info;

  u32 ext_count{0};
  vkEnumerateInstanceExtensionProperties(nullptr, &ext_count, nullptr);
  auto exts = make_scratch_vec<VkExtensionProperties>(arena);
  exts.resize(ext_count);
  vkEnumerateInstanceExtensionProperties(nullptr, &ext_count, exts.data());
  logger::verbose("{} vulkan extensions available", ext_count);
  for (const auto& ext : exts) {
    logger::verbose(" - {}", ext.extensionName);
  }

  bool ext_avail{true};
  for (const char* extname : extensions) {
    bool found{false};
    for (const auto& props : exts) {
      if (std::strcmp(extname, props.extensionName) == 0) {
        found = true;
        break;
      }
    }
    if (!found) {
      ext_avail = false;
      break;
    }
  }

  if (!ext_avail) {
    return {ntf::unexpect, "Failed to find the required vulkan extensions",
            VK_ERROR_EXTENSION_NOT_PRESENT};
  }

  // Pass the windowing system extensions
  create_info.enabledExtensionCount = static_cast<u32>(extensions.size());
  create_info.ppEnabledExtensionNames = extensions.data();

#ifndef NDEBUG
  // Setup the debug messenger
  VkDebugUtilsMessengerCreateInfoEXT messenger_info{};
  messenger_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
  messenger_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                                   // VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
                                   VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                   VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
  messenger_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                               VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                               VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
  messenger_info.pfnUserCallback = debug_callback;
  messenger_info.pUserData = nullptr;

  // Which global validation layers to enable
  create_info.enabledLayerCount = static_cast<u32>(layers.size());
  create_info.ppEnabledLayerNames = layers.data();

  // Enable a debug messenger for vkCreateInstance and vkDestroyInstance
  create_info.pNext = static_cast<const void*>(&messenger_info);
#else
  NTF_UNUSED(messenger);
  create_info.enabledLayerCount = 0;
  create_info.pNext = nullptr;
#endif

  VkInstance instance;
  VkResult res = vkCreateInstance(&create_info, vkalloc, &instance);
  // Second param is a custom allocator callback, null for now
  if (res != VK_SUCCESS) {
    return {ntf::unexpect, "Failed to create vulkan instance", res};
  }

#ifndef NDEBUG
  // Create the debug messenger
  res = CreateDebugUtilsMessengerEXT(instance, &messenger_info, vkalloc, &messenger);
  if (res != VK_SUCCESS) {
    vkDestroyInstance(instance, vkalloc);
    return {ntf::unexpect, "Failed to setup debug menssenger", res};
  }
#endif

  logger::debug("Vulkan instance initialized");

  return {ntf::in_place, instance};
};

fn mmap_alloc(void* user_ptr, size_t size, size_t align) -> void* {
  NTF_UNUSED(user_ptr);
  NTF_UNUSED(align);
  void* ptr = mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, 0, 0);
  return ptr;
}

fn mmap_free(void* user_ptr, void* mem, size_t size) {
  NTF_UNUSED(user_ptr);
  munmap(mem, size);
}

const ntf::malloc_funcs mmap_funcs{
  .user_ptr = nullptr,
  .mem_alloc = mmap_alloc,
  .mem_free = mmap_free,
};

} // namespace

fn vk_context::create(size_t arena_size, vk_surface_provider& surf_prov)
  -> vk_sv_expect<vk_context> {

  auto arena = scratch_arena::from_extern(mmap_funcs, arena_size);
  if (!arena) {
    return {ntf::unexpect, "Failed to create scratch arena", VK_ERROR_OUT_OF_HOST_MEMORY};
  }
  const VkAllocationCallbacks* vkalloc = nullptr; // TODO: Get this from the user?

  const fn make_allocator = [&](VkInstance vk, const vk_device& device) {
    VmaAllocatorCreateInfo vma_info{};
    vma_info.instance = vk;
    vma_info.device = device;
    vma_info.physicalDevice = device.physical_device();
    vma_info.vulkanApiVersion = KEIKI_VULKAN_VERSION;
    vma_info.flags = VMA_ALLOCATOR_CREATE_EXTERNALLY_SYNCHRONIZED_BIT |
                     VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;

    VmaAllocator vmalloc;
    VK_ASSERT(vmaCreateAllocator(&vma_info, &vmalloc));
    return vmalloc;
  };

  const fn create_command_pool = [&](const vk_device& device) -> vk_context::command_pool {
    // Commands in vulkan are recorded in a command buffer
    // instead of being executed directly using function calls
    VkCommandPoolCreateInfo pool{};
    pool.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;

    const auto queue_families = device.queue_families();
    // The pool will be used for drawing commands only
    pool.queueFamilyIndex = queue_families.graphics;

    // All command buffers can be rerecorded individually
    pool.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    VkCommandPool graphics_pool;
    VK_ASSERT(vkCreateCommandPool(device, &pool, vkalloc, &graphics_pool));

    VkCommandPool transfer_pool;
    pool.queueFamilyIndex = queue_families.transfer;
    VK_ASSERT(vkCreateCommandPool(device, &pool, vkalloc, &transfer_pool));

    // Command buffer allocation
    buffer_array<MAX_FRAMES_IN_FLIGHT> graphics_command_buffers;

    VkCommandBufferAllocateInfo alloc{};
    alloc.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    alloc.commandPool = graphics_pool;
    alloc.commandBufferCount = MAX_FRAMES_IN_FLIGHT;

    // If the command buffer can be submited directly but not called from other buffers
    // or cannot be submitted directly but can be called from pirmary buffers
    alloc.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

    VK_ASSERT(vkAllocateCommandBuffers(device, &alloc, graphics_command_buffers.data()));

    VkCommandBuffer transfer_command_buffer;
    alloc.commandPool = transfer_pool;
    alloc.commandBufferCount = 1;
    VK_ASSERT(vkAllocateCommandBuffers(device, &alloc, &transfer_command_buffer));
    return {graphics_pool, transfer_pool, graphics_command_buffers, transfer_command_buffer};
  };

  const fn create_sync = [&](const vk_device& device) -> vk_context::sync_objects {
    // Steps for drawing a frame:
    // 1. Wait for the previous frame to finish
    // 2. Acquire an image from the swapchain
    // 3. Record a command buffer which draws an scene onto that image
    // 4. Submit the recorded command buffer (and execute it)
    // 5. Present the swapchain image

    // Steps 2, 4 and 5 hapen asynchronously on the GPU, so some
    // synchronization primitives are needed: semaphores and fences
    // - Semaphores are for synchronizing the GPU (Blocks between queue commands)
    // - Fences are for synchronizing the CPU (Blocks until some queue command finishes)

    // Semaphores should be used for swapchain operations (because they happen on the GPU)
    // Fences are used when we're waiting for the previous frame to end before drawing again, so
    // we don't overwrite the current contents of the command buffer while the GPU is using it

    VkSemaphoreCreateInfo semaphore{};
    semaphore.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fence{};
    fence.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

    // Create the fence signaled, since there is no previous frame on the first frame
    fence.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    vk_context::sync_objects objs;
    u32 i = 0;
    for (; i < vk_context::MAX_FRAMES_IN_FLIGHT; ++i) {
      auto& image_sem = objs.image_avail_semaphores[i];
      auto& render_sem = objs.render_finish_semaphores[i];
      auto& in_flight_fen = objs.in_flight_fences[i];
      VK_ASSERT(vkCreateSemaphore(device, &semaphore, vkalloc, &image_sem));
      VK_ASSERT(vkCreateSemaphore(device, &semaphore, vkalloc, &render_sem))
      VK_ASSERT(vkCreateFence(device, &fence, vkalloc, &in_flight_fen));
    }
    return objs;
  };

  static auto base_device_extensions = std::to_array<const char*>(
    {VK_KHR_SWAPCHAIN_EXTENSION_NAME, VK_KHR_DEVICE_GROUP_EXTENSION_NAME});

  auto extensions = make_scratch_vec<const char*>(*arena);
  const u32 provider_extensions = surf_prov.get_extensions(extensions);
  NTF_UNUSED(provider_extensions);

#ifndef NDEBUG
  static auto validation_layers = std::to_array<const char*>({
    "VK_LAYER_KHRONOS_validation",
  });
  const span<const char*> layers{validation_layers.data(), validation_layers.size()};
  extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME); // for setting up a debug messenger
#else
  const span<const char*> layers{};
#endif

  VkDebugUtilsMessengerEXT messenger;
  auto vk = create_instance(*arena, messenger, to_span(extensions), layers, vkalloc);
  if (!vk) {
    return {ntf::unexpect, std::move(vk).error()};
  }
  arena->clear();

  VkSurfaceKHR surface;
  if (!surf_prov.create_surface(*vk, surface, vkalloc)) {
    vkDestroyInstance(*vk, vkalloc);
    return {ntf::unexpect, "Failed to create window surface",
            VK_ERROR_INVALID_EXTERNAL_HANDLE_KHR};
  }

  auto device =
    vk_device::create(*arena, *vk, surface, to_span(base_device_extensions), layers, vkalloc);
  if (!device) {
    vkDestroySurfaceKHR(*vk, surface, vkalloc);
    vkDestroyInstance(*vk, vkalloc);
    return {ntf::unexpect, std::move(device).error()};
  }
  arena->clear();

  const auto [swapchain_w, swapchain_h] = surf_prov.framebuffer_size();
  const VkExtent2D swapchain_extent{.width = swapchain_w, .height = swapchain_h};
  auto swapchain =
    vk_swapchain::create(*device, surface, swapchain_extent, VK_NULL_HANDLE, vkalloc);
  if (!swapchain) {
    device->destroy(vkalloc);
    vkDestroySurfaceKHR(*vk, surface, vkalloc);
    vkDestroyInstance(*vk, vkalloc);
    return {ntf::unexpect, std::move(swapchain).error()};
  }

  auto cmdpool = create_command_pool(*device);
  auto sync = create_sync(*device);
  vk_memory mem{
    .vmalloc = make_allocator(*vk, *device),
    .vkalloc = vkalloc,
  };

  const auto props = device->physical_device_props();
  logger::info("Vulkan device information:");
  logger::info(" - Name: {}", props.deviceName);
  logger::info(" - Device ID: {}", props.deviceID);
  logger::info(" - Vendor ID: {}", props.vendorID);
  logger::info(" - API version: {}", props.apiVersion);
  logger::info(" - Driver version: {}", props.driverVersion);

  return {ntf::in_place,
          *std::move(arena),
          std::move(mem),
          surf_prov,
          *vk,
          surface,
          messenger,
          *std::move(device),
          *std::move(swapchain),
          std::move(cmdpool),
          std::move(sync)};
}

vk_context::vk_context(scratch_arena&& arena, vk_memory mem, vk_surface_provider& surf_prov,
                       VkInstance vk, VkSurfaceKHR surface, VkDebugUtilsMessengerEXT messenger,
                       vk_device&& device, vk_swapchain&& swapchain, command_pool&& cmdpool,
                       sync_objects&& sync) :
    _arena(std::move(arena)), _mem(mem), _surf_prov(surf_prov), _vk(vk), _surface(surface),
    _messenger(messenger), _device(std::move(device)), _swapchain(std::move(swapchain)),
    _cmdpool(std::move(cmdpool)), _sync(std::move(sync)), _curr_frame(0),
    _img_index(NULL_IMG_INDEX), _frame_flags(FRAME_FLAG_NONE), _pips(), _buffs() {}

vk_context::~vk_context() noexcept {
  _swapchain.destroy(_device, _mem.vkalloc);

  for (auto& [_, handle] : _buffs) {
    destroy_buffer(handle.as_u64());
  }
  for (auto& [_, handle] : _pips) {
    destroy_pipeline(handle.as_u64());
  }
  for (u32 i = 0; i < vk_context::MAX_FRAMES_IN_FLIGHT; ++i) {
    auto& image_sem = _sync.image_avail_semaphores[i];
    auto& render_sem = _sync.render_finish_semaphores[i];
    auto& in_flight_fen = _sync.in_flight_fences[i];
    vkDestroyFence(_device, in_flight_fen, nullptr);
    vkDestroySemaphore(_device, render_sem, nullptr);
    vkDestroySemaphore(_device, image_sem, nullptr);
  }
  {
    vkFreeCommandBuffers(_device, _cmdpool.transfer, 1u, &_cmdpool.transfer_command_buffer);
    vkFreeCommandBuffers(_device, _cmdpool.graphics, MAX_FRAMES_IN_FLIGHT,
                         _cmdpool.graphics_command_buffers.data());
    vkDestroyCommandPool(_device, _cmdpool.transfer, _mem.vkalloc);
    vkDestroyCommandPool(_device, _cmdpool.graphics, _mem.vkalloc);
  }

  vkDestroySurfaceKHR(_vk, _surface, _mem.vkalloc);
  vmaDestroyAllocator(_mem.vmalloc);
  _device.destroy(_mem.vkalloc);
#ifndef NDEBUG
  DestroyDebugUtilsMessengerEXT(_vk, _messenger, nullptr);
#endif
  vkDestroyInstance(_vk, _mem.vkalloc);
}

fn vk_context::_rebuild_swapchain() -> vk_sv_expect<void> {
  vkDeviceWaitIdle(_device);
  const VkExtent2D new_swapchain_extent = to_vk_extent2d(_surf_prov.framebuffer_size());
  return _swapchain.rebuild(_device, _surface, new_swapchain_extent, _mem.vkalloc);
}

fn vk_context::_current_command_buffer() -> VkCommandBuffer {
  const auto& graphics_buffers = _cmdpool.graphics_command_buffers;
  NTF_ASSERT(_curr_frame < MAX_FRAMES_IN_FLIGHT);
  return graphics_buffers[_curr_frame];
}

fn vk_context::start_frame() -> vk_sv_expect<void> {
  // Wait for 1 fence without timeout
  vkWaitForFences(_device, 1, &_sync.in_flight_fences[_curr_frame], VK_TRUE, UINT64_MAX);

  // Reset 1 fence
  vkResetFences(_device, 1, &_sync.in_flight_fences[_curr_frame]);

  // Acquire the next image without a timeout, pass a semaphore only
  _img_index = NULL_IMG_INDEX;
  VkResult result =
    vkAcquireNextImageKHR(_device, _swapchain, UINT64_MAX,
                          _sync.image_avail_semaphores[_curr_frame], VK_NULL_HANDLE, &_img_index);
  if (result == VK_ERROR_OUT_OF_DATE_KHR) {
    if (auto res = _rebuild_swapchain(); !res) {
      return {ntf::unexpect, "Failed to rebuild swapchain", res.error().code()};
    }
  } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
    return {ntf::unexpect, "Failed to acquire swapchain image", result};
  }
  NTF_ASSERT(_img_index != NULL_IMG_INDEX);

  vkResetFences(_device, 1, &_sync.in_flight_fences[_curr_frame]);

  const auto buffer = _current_command_buffer();
  // Make sure the buffer is able to be recorded, the second arg is some flag
  VK_ASSERT(vkResetCommandBuffer(buffer, 0));
  VkCommandBufferBeginInfo begin_info{};
  begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  // begin_info.flags = 0;
  // begin_info.pInheritanceInfo = nullptr;

  // If the buffer was recorded once, a new call will reset it instead
  VK_ASSERT(vkBeginCommandBuffer(buffer, &begin_info));

  const auto swapchain_extent = _swapchain.extent();
  const auto framebuffers = _swapchain.framebuffers();

  VkRenderPassBeginInfo render_pass{};
  render_pass.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  render_pass.renderPass = _swapchain.renderpass();
  render_pass.framebuffer = framebuffers[_img_index];
  render_pass.renderArea.offset = {0, 0};
  render_pass.renderArea.extent = swapchain_extent;

  VkClearValue clear_color{{{.2f, .2f, .2f, 1.f}}};
  render_pass.clearValueCount = 1;
  render_pass.pClearValues = &clear_color;

  // Set the dynamic states
  VkViewport viewport{};
  viewport.x = 0.f;
  viewport.y = 0.f;
  viewport.width = static_cast<float>(swapchain_extent.width);
  viewport.height = static_cast<float>(swapchain_extent.height);
  viewport.minDepth = 0.f;
  viewport.maxDepth = 1.f;
  vkCmdSetViewport(buffer, 0, 1, &viewport); // firstViewport, viewportCount

  VkRect2D scissor{};
  scissor.offset = {0, 0};
  scissor.extent = swapchain_extent;
  vkCmdSetScissor(buffer, 0, 1, &scissor); // firstScissor, scissorCount

  // Begin renderpass
  // VK_SUBPASS_CONTENTS_INLINE specifies that no secondary buffers will be executed
  vkCmdBeginRenderPass(buffer, &render_pass, VK_SUBPASS_CONTENTS_INLINE);

  return {};
}

fn vk_context::record_command(const vk_indexed_draw_command& cmd) -> void {
  // Draw something in an image
  NTF_ASSERT(cmd.indices > 0);
  const auto& pip = _pips.at(ntf::freelist_handle::from_u64(cmd.pipeline));
  const auto& vertbuf = _buffs.at(ntf::freelist_handle::from_u64(cmd.vertex_buffer));
  const auto& indxbuf = _buffs.at(ntf::freelist_handle::from_u64(cmd.index_buffer));

  // Write commands to a command buffer
  const auto buffer = _current_command_buffer();
  // VK_PIPELINE_BIND_POINT_GRAPHICS specifies that is a graphics pipeline (not a compute one)
  vkCmdBindPipeline(buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pip.pipeline);

  const VkBuffer vert_buffers[] = {vertbuf.buffer};
  VkDeviceSize offsets[] = {0};
  vkCmdBindVertexBuffers(buffer, 0, 1, vert_buffers, offsets);
  vkCmdBindIndexBuffer(buffer, indxbuf.buffer, 0, VK_INDEX_TYPE_UINT16);
  vkCmdDrawIndexed(buffer, cmd.indices, 1, 0, 0, 0);
  // vkCmdDraw(buffer, static_cast<u32>(vertices.size()), 1, 0, 0);
  // vkCmdDraw(buffer, 3, 1, 0, 0); // vertexCount, instanceCount, firstVertex, firstInstance
}

fn vk_context::end_frame() -> vk_sv_expect<void> {
  // End command buffer things
  const auto buffer = _current_command_buffer();
  vkCmdEndRenderPass(buffer);
  VK_ASSERT(vkEndCommandBuffer(buffer));

  // Now to submit the queue
  VkSubmitInfo submit{};
  submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

  // Wait with ONLY writing colors to the image until it's available
  // This means that the implementation COULD start executing the vertex shader for example
  const VkPipelineStageFlags wait_stages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
  const VkSemaphore wait_semaphores[] = {_sync.image_avail_semaphores[_curr_frame]};
  submit.waitSemaphoreCount = 1;
  submit.pWaitSemaphores = wait_semaphores;
  submit.pWaitDstStageMask = wait_stages;

  // Which semaphores to signal when the command buffer finishes execution
  const auto& graphics_buffers = _cmdpool.graphics_command_buffers;
  const VkSemaphore signal_semaphores[] = {_sync.render_finish_semaphores[_curr_frame]};
  submit.commandBufferCount = 1;
  submit.pCommandBuffers = &graphics_buffers[_curr_frame];
  submit.signalSemaphoreCount = 1;
  submit.pSignalSemaphores = signal_semaphores;

  const VkQueue graphics_queue = _device.get_queue(vk_device::QUEUE_FAMILY_GRAPHICS);
  VkResult res = vkQueueSubmit(graphics_queue, 1, &submit, _sync.in_flight_fences[_curr_frame]);
  if (res != VK_SUCCESS) {
    return {ntf::unexpect, "Failed to submit draw command buffer", res};
  }

  VkPresentInfoKHR present{};
  present.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
  present.waitSemaphoreCount = 1;
  present.pWaitSemaphores = signal_semaphores;

  NTF_ASSERT(_img_index != NULL_IMG_INDEX);
  const VkSwapchainKHR swapchains[] = {_swapchain.swapchain()};
  present.swapchainCount = 1;
  present.pSwapchains = swapchains;
  present.pImageIndices = &_img_index;
  // present.pResults = nullptr;

  const VkQueue present_queue = _device.get_queue(vk_device::QUEUE_FAMILY_PRESENT);
  const bool framebuffer_resized = _frame_flags & FRAME_FLAG_DIRTY_FRAMEBUFFER;

  VkResult result = vkQueuePresentKHR(present_queue, &present);
  if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebuffer_resized) {
    _frame_flags = _frame_flags & ~FRAME_FLAG_DIRTY_FRAMEBUFFER;
    if (auto ret = _rebuild_swapchain(); !ret) {
      return {ntf::unexpect, "Failed to rebuild swapchain", ret.error().code()};
    }
  } else if (result != VK_SUCCESS) {
    return {ntf::unexpect, "Failed to present swapchain image", result};
  }

  _curr_frame = (_curr_frame + 1) % vk_context::MAX_FRAMES_IN_FLIGHT;
  return {};
}

fn vk_context::flag_dirty_framebuffer() -> void {
  _frame_flags |= FRAME_FLAG_DIRTY_FRAMEBUFFER;
}

void vk_context::device_wait() {
  vkDeviceWaitIdle(_device);
}

fn vk_context::_allocate_buffer(VkBufferUsageFlags buffer_usage, VmaMemoryUsage mem_usage,
                                size_t size, u32 flags) -> vk_sv_expect<buffer_data> {
  NTF_UNUSED(flags);

  VkBufferCreateInfo buff_info{};
  buff_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  buff_info.size = size;
  buff_info.usage = buffer_usage;

  VmaAllocationCreateInfo vma_info{};
  vma_info.usage = mem_usage;
  vma_info.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT; // map the buffer pointer automatically

  VkBuffer buff;
  VmaAllocation allocation;
  VmaAllocationInfo info;
  VkResult res = vmaCreateBuffer(_mem.vmalloc, &buff_info, &vma_info, &buff, &allocation, &info);
  if (res != VK_SUCCESS) {
    return {ntf::unexpect, "Failed to allocate buffer", res};
  }

  ntf::optional<VkDeviceAddress> address;
  if (vma_info.flags & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT) {
    VkBufferDeviceAddressInfo address_info{};
    address_info.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
    address.emplace(vkGetBufferDeviceAddress(_device, &address_info));
  }
  return {ntf::in_place, buff, address, allocation, info, buffer_usage};
}

fn vk_context::_deallocate_buffer(const buffer_data& data) -> void {
  vmaDestroyBuffer(_mem.vmalloc, data.buffer, data.allocation);
}

fn vk_context::create_buffer(vk_buffer_type type, size_t size, u32 flags)
  -> vk_sv_expect<vk_handle> {
  const fn parse_type = [&](vk_buffer_type buff_type) -> VkBufferUsageFlags {
    static constexpr auto base = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    switch (buff_type) {
      case vk_buffer_type::vertex: {
        return base | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
               VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
      } break;
      case vk_buffer_type::index: {
        return base | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
      } break;
      case vk_buffer_type::uniform: {
        return base | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
      } break;
    }
    NTF_UNREACHABLE();
  };

  return _allocate_buffer(parse_type(type), VMA_MEMORY_USAGE_GPU_ONLY, size, flags)
    .transform([&](buffer_data&& data) -> vk_handle {
      const auto handle = _buffs.emplace(std::move(data));
      return handle.as_u64();
    });
}

fn vk_context::destroy_buffer(vk_handle buffer) -> void {
  const auto handle = ntf::freelist_handle::from_u64(buffer);
  const auto& buff = _buffs.at(handle);
  _deallocate_buffer(buff);
  _buffs.remove(handle);
}

fn vk_context::upload_buffer_data(vk_handle buffer, const void* data, size_t size, size_t offset)
  -> vk_sv_expect<void> {
  const auto& buff = _buffs.at(ntf::freelist_handle::from_u64(buffer));

  const auto staging =
    _allocate_buffer(VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY, size, 0u);
  if (!staging) {
    return {ntf::unexpect, "Failed to allocate staging buffer", staging.error().code()};
  }

  const auto transfer_buff = _cmdpool.transfer_command_buffer;
  const auto transfer_queue = _device.get_queue(vk_device::QUEUE_FAMILY_TRANSFER);
  const fn copy_buffer = [&](VkBuffer src, VkBuffer dst) {
    VkCommandBufferBeginInfo begin_info{};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(transfer_buff, &begin_info);

    VkBufferCopy copy_region{};
    copy_region.srcOffset = 0;
    copy_region.dstOffset = offset;
    copy_region.size = size;
    vkCmdCopyBuffer(transfer_buff, src, dst, 1, &copy_region);

    vkEndCommandBuffer(transfer_buff);

    VkSubmitInfo submit{};
    submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit.commandBufferCount = 1;
    submit.pCommandBuffers = &transfer_buff;

    vkQueueSubmit(transfer_queue, 1, &submit, VK_NULL_HANDLE);
    vkQueueWaitIdle(transfer_queue);
  };

  void* mapping = staging->allocation->GetMappedData();
  std::memcpy(mapping, data, size);
  copy_buffer(staging->buffer, buff.buffer);

  _deallocate_buffer(*staging);
  return {};
}

} // namespace keiki::render
