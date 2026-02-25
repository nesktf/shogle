#include "./vk_pipeline.hpp"
#include "./vk_context.hpp"

namespace keiki::render {

fn vk_context::create_pipeline_layout(const vk_layout_info& info)
  -> vk_sv_expect<vk_pipeline_layout> {
  VkPipelineLayoutCreateInfo vkinfo{};
  vkinfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

  VkPipelineLayout layout;
  VK_ASSERT(vkCreatePipelineLayout(_device, &vkinfo, _mem.vkalloc, &layout));

  const auto handle = _pipeline_layouts.emplace(layout);
  return {ntf::in_place, handle.as_u64()};
}

fn vk_context::create_pipeline(const vk_pipeline_builder& builder) -> vk_sv_expect<vk_pipeline> {
  VkPipelineViewportStateCreateInfo viewport{};
  viewport.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
  viewport.pNext = nullptr;
  viewport.viewportCount = 1;
  viewport.scissorCount = 1;

  VkPipelineColorBlendStateCreateInfo blending{};
  blending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
  blending.pNext = nullptr;
  blending.logicOpEnable = VK_FALSE;
  blending.logicOp = VK_LOGIC_OP_COPY;
  blending.attachmentCount = 1;
  blending.pAttachments = &builder._color_blend_attachment;

  VkPipelineVertexInputStateCreateInfo vert_input{};
  vert_input.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

  const VkDynamicState dyn_state[] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
  VkPipelineDynamicStateCreateInfo dyn_info{};
  dyn_info.pDynamicStates = dyn_state;
  dyn_info.dynamicStateCount = sizeof(dyn_state) / sizeof(dyn_state[0]);

  VkGraphicsPipelineCreateInfo pipeline_info{};
  pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
  // connect the renderInfo to the pNext extension mechanism
  pipeline_info.pNext = &builder._render_info;

  pipeline_info.pVertexInputState = &vert_input;
  pipeline_info.pInputAssemblyState = &builder._input_assembly;
  pipeline_info.pViewportState = &viewport;
  pipeline_info.pRasterizationState = &builder._rasterizer;
  pipeline_info.pMultisampleState = &builder._multisampling;
  pipeline_info.pColorBlendState = &blending;
  pipeline_info.pDepthStencilState = &builder._depth_stencil;
  pipeline_info.pDynamicState = &dyn_info;

  auto layout = _pipeline_layouts.at_opt(ntf::freelist_handle::from_u64(builder._pipeline_layout));
  if (!layout) {
    return {ntf::unexpect, "Invalid pipeline layout", VK_ERROR_INVALID_EXTERNAL_HANDLE};
  }

  static constexpr auto shader_bits = std::to_array({
    VK_SHADER_STAGE_VERTEX_BIT,
    VK_SHADER_STAGE_FRAGMENT_BIT,
    VK_SHADER_STAGE_GEOMETRY_BIT,
    VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,
    VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT,
  });
  std::array<VkPipelineShaderStageCreateInfo, vk_pipeline_builder::MAX_GRAPHIC_STAGES> stages;
  u32 stage_count = 0u;
  for (u32 stage_idx = 0u; stage_idx < builder._stage_map.size(); ++stage_idx) {
    const auto stage = builder._stage_map[stage_idx];
    if (stage == vk_invalid_handle) {
      continue;
    }
    const auto handle = ntf::freelist_handle::from_u64(stage);
    const auto stage_module = _stage_modules.at_opt(handle);
    if (!stage_module) {
      continue;
    }
    stages[stage_count] = {};
    stages[stage_count].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    NTF_ASSERT(static_cast<u32>(stage_module->type) == stage_idx, "Stage type mismatch");
    stages[stage_count].stage = shader_bits[stage_idx];
    stages[stage_count].module = stage_module->stage;
    stages[stage_count].pName = "main";
    ++stage_count;
  }
  if (!stage_count) {
    return {ntf::unexpect, "Failed to parse pipeline stages", VK_ERROR_INVALID_EXTERNAL_HANDLE};
  }

  pipeline_info.stageCount = stage_count;
  pipeline_info.pStages = stages.data();
  pipeline_info.layout = *layout;

  VkPipeline pipeline;
  VkResult res =
    vkCreateGraphicsPipelines(_device, VK_NULL_HANDLE, 1, &pipeline_info, _mem.vkalloc, &pipeline);
  if (res != VK_SUCCESS) {
    return {ntf::unexpect, "Failed to create graphics pipeline", res};
  }
  const auto handle = _pipelines.emplace(pipeline);
  return {ntf::in_place, handle.as_u64()};
}

fn vk_context::destroy_pipeline(vk_handle pipeline) -> void {
  const auto handle = ntf::freelist_handle::from_u64(pipeline);
  const auto pip = _pipelines.at_opt(handle);
  if (!pip) {
    return;
  }
  vkDestroyPipeline(_device, *pip, _mem.vkalloc);
  _pipelines.remove(handle);
}

vk_pipeline_builder::vk_pipeline_builder(vk_context& ctx) : _ctx(&ctx) {
  clear();
}

fn vk_pipeline_builder::clear() -> void {
  _input_assembly = {.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO};
  _rasterizer = {.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO};
  _multisampling = {.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO};
  _depth_stencil = {.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO};
  _render_info = {.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO};
  std::memset(&_color_blend_attachment, 0, sizeof(_color_attachment_format));
  std::memset(&_color_attachment_format, 0, sizeof(_color_attachment_format));
  for (auto& stage : _stage_map) {
    stage = vk_invalid_handle;
  }
  _pipeline_layout = vk_invalid_handle;
}

fn vk_pipeline_builder::build() const -> vk_sv_expect<vk_pipeline> {
  return _ctx->create_pipeline(*this);
}

fn vk_pipeline_builder::set_layout(vk_pipeline_layout layout) -> vk_pipeline_builder& {
  NTF_ASSERT(layout != vk_invalid_handle, "Invalid layout handle");
  _pipeline_layout = layout;
  return *this;
}

fn vk_pipeline_builder::set_shader(vk_stage_type type, vk_pipeline_stage stage)
  -> vk_pipeline_builder& {
  const u32 index = static_cast<u32>(type);
  NTF_ASSERT(index < MAX_GRAPHIC_STAGES, "Invalid stage index");
  NTF_ASSERT(stage != vk_invalid_handle, "Invalid stage handle");
  _stage_map[index] = stage;
  return *this;
}

fn vk_pipeline_builder::set_topology(VkPrimitiveTopology topology) -> vk_pipeline_builder& {
  _input_assembly.topology = topology;
  _input_assembly.primitiveRestartEnable = VK_FALSE;
  return *this;
}

fn vk_pipeline_builder::set_polygon_mode(VkPolygonMode mode, f32 width) -> vk_pipeline_builder& {
  _rasterizer.polygonMode = mode;
  _rasterizer.lineWidth = width;
  return *this;
}

fn vk_pipeline_builder::set_cull_mode(VkCullModeFlags mode, VkFrontFace front)
  -> vk_pipeline_builder& {
  _rasterizer.cullMode = mode;
  _rasterizer.frontFace = front;
  return *this;
}

fn vk_pipeline_builder::set_multisampling_none() -> vk_pipeline_builder& {
  _multisampling.sampleShadingEnable = VK_FALSE;
  // multisampling defaulted to no multisampling (1 sample per pixel)
  _multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
  _multisampling.minSampleShading = 1.0f;
  _multisampling.pSampleMask = nullptr;
  // no alpha to coverage either
  _multisampling.alphaToCoverageEnable = VK_FALSE;
  _multisampling.alphaToOneEnable = VK_FALSE;
  return *this;
}

fn vk_pipeline_builder::disable_blending() -> vk_pipeline_builder& {
  _color_blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                                           VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
  _color_blend_attachment.blendEnable = VK_FALSE;
  return *this;
}

fn vk_pipeline_builder::set_color_attachment_format(VkFormat format) -> vk_pipeline_builder& {
  _color_attachment_format = format;
  _render_info.colorAttachmentCount = 1;
  _render_info.pColorAttachmentFormats = &_color_attachment_format;
  return *this;
}

fn vk_pipeline_builder::set_depth_format(VkFormat format) -> vk_pipeline_builder& {
  _render_info.depthAttachmentFormat = format;
  return *this;
}

fn vk_pipeline_builder::disable_depthtest() -> vk_pipeline_builder& {
  _depth_stencil.depthTestEnable = VK_FALSE;
  _depth_stencil.depthWriteEnable = VK_FALSE;
  _depth_stencil.depthCompareOp = VK_COMPARE_OP_NEVER;
  _depth_stencil.depthBoundsTestEnable = VK_FALSE;
  _depth_stencil.stencilTestEnable = VK_FALSE;
  _depth_stencil.front = {};
  _depth_stencil.back = {};
  _depth_stencil.minDepthBounds = 0.f;
  _depth_stencil.maxDepthBounds = 1.f;
  return *this;
}

} // namespace keiki::render
