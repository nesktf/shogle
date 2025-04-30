#include "./common.hpp"

namespace ntf {

static auto _copy_pipeline_layout(
  const r_pipeline_descriptor& desc
) -> std::unique_ptr<vertex_layout> {
  auto layout = std::make_unique<vertex_layout>();
  layout->binding = desc.attrib_binding->binding;
  layout->stride = desc.attrib_binding->stride;
  layout->descriptors.resize(desc.attrib_desc.size());
  std::memcpy(
    layout->descriptors.data(), desc.attrib_desc.data(),
    desc.attrib_desc.size()*sizeof(r_attrib_descriptor)
  );
  return layout;
}

r_expected<r_pipeline> r_create_pipeline(r_context ctx, const r_pipeline_descriptor& desc) {
  // TODO: validation
  RET_ERROR_IF(!ctx,
               "[ntf::r_create_pipeline]",
               "Invalid context handle");

  std::unique_ptr<vertex_layout> layout = _copy_pipeline_layout(desc);
  uniform_map uniforms;

  r_platform_pipeline handle{};
  try {
    handle = ctx->platform->create_pipeline(desc, layout.get(), uniforms);
    RET_ERROR_IF(!handle,
                 "[ntf::r_create_pipeline]",
                 "Failed to create pipeline");
  }
  RET_ERROR_CATCH("[ntf::r_create_pipeline]",
                  "Failed to create pipeline");

  r_stages_flag stages{}; // TODO: parse stages
  [[maybe_unused]] auto [it, emplaced] = ctx->pipelines.try_emplace(
    handle, ctx, handle, desc, std::move(layout), std::move(uniforms), stages
  );
  NTF_ASSERT(emplaced);

  return &it->second;
}

r_pipeline r_create_pipeline(unchecked_t, r_context ctx, const r_pipeline_descriptor& desc) {
  auto layout = _copy_pipeline_layout(desc);
  uniform_map uniforms;

  auto handle = ctx->platform->create_pipeline(desc, layout.get(), uniforms);
  NTF_ASSERT(handle);

  r_stages_flag stages{}; // TODO: parse stages
  [[maybe_unused]] auto [it, emplaced] = ctx->pipelines.try_emplace(
    handle, ctx, handle, desc, std::move(layout), std::move(uniforms), stages
  );
  NTF_ASSERT(emplaced);

  return &it->second;
}

void r_destroy_pipeline(r_pipeline pip) {
  if (!pip) {
    return;
  }

  const auto handle = pip->handle;
  auto* ctx = pip->ctx;
  auto it = ctx->pipelines.find(handle);
  if (it == ctx->pipelines.end()) {
    return;
  }

  ctx->platform->destroy_pipeline(handle);
  ctx->pipelines.erase(it);
}

r_stages_flag r_pipeline_get_stages(r_pipeline pip) {
  NTF_ASSERT(pip);
  return pip->stages;
}

optional<r_uniform> r_pipeline_get_uniform(r_pipeline pip, std::string_view name) {
  if (!pip) {
    return nullopt;
  }

  auto unif_it = pip->uniforms.find(name.data());
  if (unif_it == pip->uniforms.end()) {
    return nullopt;
  }

  return unif_it->second;
}

r_uniform r_pipeline_get_uniform(unchecked_t, r_pipeline pip, std::string_view name) {
  NTF_ASSERT(pip);

  auto unif_it = pip->uniforms.find(name.data());
  NTF_ASSERT(unif_it != pip->uniforms.end());

  return unif_it->second;
}

r_context r_pipeline_get_ctx(r_pipeline pip) {
  NTF_ASSERT(pip);
  return pip->ctx;
}

r_platform_pipeline r_pipeline_get_handle(r_pipeline pip) {
  NTF_ASSERT(pip);
  return pip->handle;
}

} // namespace ntf
