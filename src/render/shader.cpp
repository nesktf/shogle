#include "./common.hpp"

namespace ntf {

r_expected<r_shader> r_create_shader(r_context ctx, const r_shader_descriptor& desc) {
  r_platform_shader handle{};
  try {
    handle = ctx->platform->create_shader(desc);
    RET_ERROR_IF(!handle,
                 "[ntf::r_create_shader]",
                 "Failed to create shader");
  }
  RET_ERROR_CATCH("[ntf::r_create_shader]",
                  "Failed to create shader");

  [[maybe_unused]] auto [it, emplaced] = ctx->shaders.try_emplace(
    handle, ctx, handle, desc
  );
  NTF_ASSERT(emplaced);

  return &it->second;
}

r_shader r_create_shader(unchecked_t, r_context ctx, const r_shader_descriptor& desc) {
  NTF_ASSERT(ctx);
  auto handle = ctx->platform->create_shader(desc);
  NTF_ASSERT(handle);

  [[maybe_unused]] auto [it, emplaced] = ctx->shaders.try_emplace(
    handle, ctx, handle, desc
  );
  NTF_ASSERT(emplaced);

  return &it->second;
}

void r_destroy_shader(r_shader shader) {
  if (!shader) {
    return;
  }

  const auto handle = shader->handle;
  auto* ctx = shader->ctx;
  auto it = ctx->shaders.find(handle);
  if (it == ctx->shaders.end()) {
    return;
  }

  ctx->platform->destroy_shader(handle);
  ctx->shaders.erase(it);
}

r_shader_type r_shader_get_type(r_shader shader) {
  NTF_ASSERT(shader);
  return shader->type;
}

r_context r_shader_get_ctx(r_shader shader) {
  NTF_ASSERT(shader);
  return shader->ctx;
}

r_platform_shader r_shader_get_handle(r_shader shader) {
  NTF_ASSERT(shader);
  return shader->handle;
}

} // namespace ntf
