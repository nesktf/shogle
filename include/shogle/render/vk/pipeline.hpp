#pragma once

#include <shogle/render/vk/common.hpp>

namespace shogle {

using vk_pipeline_layout = vk_handle;
using vk_pipeline_stage = vk_handle;

enum class vk_stage_type {
  vertex = 0,
  fragment = 1,
  geometry = 2,
  tess_ctrl = 3,
  tess_eval = 4,
  compute = 5,
};

class vk_pipeline_builder {
public:
  static constexpr u32 MAX_GRAPHIC_STAGES = 5;

public:
  vk_pipeline_builder(vk_context& ctx);

public:
  auto set_layout(vk_pipeline_layout layout) -> vk_pipeline_builder&;
  auto set_shader(vk_stage_type type, vk_pipeline_stage stage) -> vk_pipeline_builder&;
  auto set_topology(VkPrimitiveTopology topology) -> vk_pipeline_builder&;
  auto set_polygon_mode(VkPolygonMode mode, f32 width = 1.f) -> vk_pipeline_builder&;
  auto set_cull_mode(VkCullModeFlags mode, VkFrontFace front) -> vk_pipeline_builder&;
  auto set_multisampling_none() -> vk_pipeline_builder&;
  auto disable_blending() -> vk_pipeline_builder&;
  auto set_color_attachment_format(VkFormat format) -> vk_pipeline_builder&;
  auto set_depth_format(VkFormat format) -> vk_pipeline_builder&;
  auto disable_depthtest() -> vk_pipeline_builder&;

public:
  auto build() const -> vk_sv_expect<vk_pipeline>;
  auto clear() -> void;

private:
  ptr_view<vk_context> _ctx;
  std::array<vk_pipeline_stage, MAX_GRAPHIC_STAGES> _stage_map;
  VkPipelineInputAssemblyStateCreateInfo _input_assembly;
  VkPipelineRasterizationStateCreateInfo _rasterizer;
  VkPipelineColorBlendAttachmentState _color_blend_attachment;
  VkPipelineMultisampleStateCreateInfo _multisampling;
  vk_handle _pipeline_layout;
  VkPipelineDepthStencilStateCreateInfo _depth_stencil;
  VkPipelineRenderingCreateInfo _render_info;
  VkFormat _color_attachment_format;

private:
  friend class vk_context;
};

} // namespace shogle
