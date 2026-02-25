#pragma once

#include <shogle/render/vk/pipeline.hpp>
#include <shogle/render/vk/swapchain.hpp>

namespace shogle {

struct vk_layout_info {
  VkVertexInputBindingDescription bind;
  span<const VkVertexInputAttributeDescription> attr;
};

struct vk_indexed_draw_command {
  vk_handle pipeline;
  vk_handle vertex_buffer;
  vk_handle index_buffer;
  u32 indices;
};

class vk_context {
public:
  struct vk_memory {
    VmaAllocator vmalloc;
    ptr_view<const VkAllocationCallbacks> vkalloc;
  };

  static constexpr u32 MAX_FRAMES_IN_FLIGHT = 2u;

  template<u32 BuffCount>
  using buffer_array = std::array<VkCommandBuffer, BuffCount>;

  struct command_pool {
    VkCommandPool graphics;
    VkCommandPool transfer;
    buffer_array<MAX_FRAMES_IN_FLIGHT> graphics_command_buffers;
    VkCommandBuffer transfer_command_buffer;
  };

  struct sync_objects {
    std::array<VkSemaphore, MAX_FRAMES_IN_FLIGHT> image_avail_semaphores;
    std::array<VkSemaphore, MAX_FRAMES_IN_FLIGHT> render_finish_semaphores;
    std::array<VkFence, MAX_FRAMES_IN_FLIGHT> in_flight_fences;
  };

  enum frame_flags : u32 {
    FRAME_FLAG_NONE = 0,
    FRAME_FLAG_DIRTY_FRAMEBUFFER = 1 << 0,
  };

public:
  struct stage_module_data {
    vk_stage_type type;
    VkShaderModule stage;
  };

  static constexpr u32 MAX_SHADER_ATTRIBUTES = 16u;

  struct buffer_data {
    VkBuffer buffer;
    optional<VkDeviceAddress> address;
    VmaAllocation allocation;
    // VmaAllocationInfo info;
    VkBufferUsageFlags buffer_usage;
  };

  static constexpr u32 NULL_IMG_INDEX = std::numeric_limits<u32>::max();

private:
  struct create_t {};

  struct context_deleter {
    void operator()(vk_private* ptr) noexcept;
  };

  using context_data = std::unique_ptr<vk_private, context_deleter>;

public:
  vk_context(mem::scratch_arena&& arena, vk_memory mem, vk_surface_provider& surf_prov,
             VkInstance vk, VkSurfaceKHR surface, VkDebugUtilsMessengerEXT messenger,
             vk_device&& device, vk_swapchain&& swapchain, command_pool&& cmdpool,
             sync_objects&& sync);

public:
  static auto create(size_t arena_size, vk_surface_provider& surf_prov)
    -> vk_sv_expect<vk_context>;

public:
  auto start_frame() -> vk_sv_expect<void>;
  auto record_command(const vk_indexed_draw_command& cmd) -> void;
  auto end_frame() -> vk_sv_expect<void>;

  auto device_wait() -> void;
  auto flag_dirty_framebuffer() -> void;

public:
  auto create_pipeline_layout(const vk_layout_info& info) -> vk_sv_expect<vk_pipeline_layout>;
  auto destroy_pipeline_layout(vk_pipeline_layout layout) -> void;

  auto create_pipeline_stage(vk_stage_type stage, std::string_view src)
    -> vk_sv_expect<vk_pipeline_stage>;
  auto destroy_pipeline_stage(vk_pipeline_stage stage) -> void;

  auto create_pipeline(const vk_pipeline_builder& builder) -> vk_sv_expect<vk_handle>;
  auto destroy_pipeline(vk_handle pipeline) -> void;

  auto create_buffer(vk_buffer_type type, size_t size, u32 flags = 0u) -> vk_sv_expect<vk_handle>;
  auto upload_buffer_data(vk_handle buffer, const void* data, size_t size, size_t offset = 0u)
    -> vk_sv_expect<void>;
  auto destroy_buffer(vk_handle buffer) -> void;

  /*
private:
auto _rebuild_swapchain() -> vk_sv_expect<void>;
auto _current_command_buffer() -> VkCommandBuffer;

auto _allocate_buffer(VkBufferUsageFlags buffer_usage, VmaMemoryUsage mem_usage, size_t size,
                  u32 flags) -> vk_sv_expect<buffer_data>;
auto _deallocate_buffer(const buffer_data& data) -> void;

public:
auto swapchain_format() const -> VkFormat { return _swapchain.format(); }
  */

private:
  context_data _ctx;
  /*
scratch_arena _arena;
vk_memory _mem;
vk_surface_provider& _surf_prov;
VkInstance _vk;
VkSurfaceKHR _surface;
VkDebugUtilsMessengerEXT _messenger;
vk_device _device;
vk_swapchain _swapchain;
command_pool _cmdpool;
sync_objects _sync;

u32 _curr_frame;
u32 _img_index;
u32 _frame_flags;

ntf::freelist<buffer_data> _buffs;
ntf::freelist<stage_module_data> _stage_modules;
ntf::freelist<VkPipelineLayout> _pipeline_layouts;
ntf::freelist<VkPipeline> _pipelines;
  */
};

} // namespace shogle
