#include "./common.hpp"

namespace ntf {

static auto transform_descriptor(
  r_context ctx, const r_framebuffer_descriptor& desc
) -> r_fbo_desc {

  if (std::holds_alternative<cspan<r_framebuffer_attachment>>(desc.attachments)) {
    auto attachments_in = std::get<cspan<r_framebuffer_attachment>>(desc.attachments);
    std::vector<r_framebuffer_attachment> attachments;
    attachments.reserve(attachments_in.size());
  }

};

static auto check_and_transform_descriptor(
  r_context ctx, const r_framebuffer_descriptor& desc
) -> r_expected<r_fbo_desc> {
  RET_ERROR_IF(!ctx,
               "[ntf::r_create_framebuffer]",
               "Invalid context handle");

  if (desc.viewport.x+desc.viewport.z != desc.extent.x ||
      desc.viewport.y+desc.viewport.w != desc.extent.y) {
    SHOGLE_LOG(warning, "[ntf::r_create_framebuffer] Mismatching viewport size");
  }

  if (std::holds_alternative<cspan<r_framebuffer_attachment>>(desc.attachments)) {
    auto attachments_in = std::get<cspan<r_framebuffer_attachment>>(desc.attachments);
    RET_ERROR_IF(attachments_in.empty(),
                 "[ntf::r_create_framebuffer]",
                 "Invalid attachment span");
    for (uint32 i = 0; i < attachments_in.size(); ++i) {
      const auto& att = attachments_in[i];
      r_texture tex = att.handle;
      RET_ERROR_IF(!tex || tex->ctx != ctx,
                   "[ntf::r_create_framebuffer]",
                   "Invalid texture handle at index {}",
                   i);

      RET_ERROR_IF(att.layer > tex->layers,
                   "[ntf::r_create_framebuffer]",
                   "Invalid texture layer at index {}",
                   i);

      RET_ERROR_IF(att.level > tex->levels,
                   "[ntf::r_create_framebuffer]",
                   "Invalid texture level at index {}",
                   i);

      RET_ERROR_IF(tex->extent.x != desc.extent.x || tex->extent.y != desc.extent.y,
                   "[ntf::r_create_framebuffer]",
                   "Invalid texture extent at index {}",
                   i);
    }
  } else {
    RET_ERROR("[ntf::r_create_framebuffer]",
              "Color buffer not implemented");
  }

  return transform_descriptor(ctx, desc);
}

r_expected<r_framebuffer> r_create_framebuffer(r_context ctx,
                                               const r_framebuffer_descriptor& desc) {

  return check_and_transform_descriptor(ctx, desc)
  .and_then([&](r_fbo_desc&& fbo) -> r_expected<r_framebuffer> {
    r_platform_fbo handle{};
    try {
      handle = ctx->platform->create_framebuffer(desc);
      RET_ERROR_IF(!handle,
                   "[ntf::r_create_framebuffer]",
                   "Failed to create framebuffer");
    }
    RET_ERROR_CATCH("[ntf::r_create_framebuffer]",
                    "Failed to create framebuffer");
  });
  for (size_t i = 0; i < desc.attachments.size(); ++i) {
    attachments.push_back(desc.attachments[i]);
    desc.attachments[i].handle->refcount++;
  }

  [[maybe_unused]] auto [it, emplaced] = ctx->framebuffers.try_emplace(
    handle, ctx, handle, desc, std::move(attachments)
  );
  NTF_ASSERT(emplaced);

  ctx->draw_lists.try_emplace(handle);
  auto& list = ctx->draw_lists.at(handle);
  list.viewport = desc.viewport;
  list.color = desc.clear_color;
  list.clear = desc.clear_flags;

  return &it->second;
}

r_framebuffer r_create_framebuffer(unchecked_t, r_context ctx,
                                   const r_framebuffer_descriptor& desc) {
  NTF_ASSERT(ctx);
  std::vector<r_framebuffer_attachment> attachments;
  attachments.reserve(desc.attachments.size());

  auto handle = ctx->platform->create_framebuffer(desc);

  for (size_t i = 0; i < desc.attachments.size(); ++i) {
    attachments.push_back(desc.attachments[i]);
    desc.attachments[i].handle->refcount++;
  }

  [[maybe_unused]] auto [it, emplaced] = ctx->framebuffers.try_emplace(
    handle, ctx, handle, desc, std::move(attachments)
  );
  NTF_ASSERT(emplaced);

  ctx->draw_lists.try_emplace(handle);
  auto& list = ctx->draw_lists.at(handle);
  list.viewport = desc.viewport;
  list.color = desc.clear_color;
  list.clear = desc.clear_flags;

  return &it->second;
}

void r_destroy_framebuffer(r_framebuffer fbo) {
  if (!fbo) {
    return;
  }

  auto* ctx = fbo->ctx;
  if (fbo == &ctx->default_fbo) {
    return;
  }

  const auto handle = fbo->handle;
  auto it = ctx->framebuffers.find(handle);
  if (it == ctx->framebuffers.end()) {
    return;
  }

  ctx->platform->destroy_framebuffer(handle);
  for (auto att : it->second.attachments) {
    r_destroy_texture(att.handle); // decreases refcount and maybe destroys
  }
  ctx->framebuffers.erase(it);
}

void r_framebuffer_set_clear(r_framebuffer fbo, r_clear_flag flags) {
  NTF_ASSERT(fbo);
  fbo->ctx->draw_lists.at(fbo->handle).clear = flags;
}

void r_framebuffer_set_viewport(r_framebuffer fbo, const uvec4& vp) {
  NTF_ASSERT(fbo);
  fbo->ctx->draw_lists.at(fbo->handle).viewport = vp;
}

void r_framebuffer_set_color(r_framebuffer fbo, const color4& color) {
  NTF_ASSERT(fbo);
  fbo->ctx->draw_lists.at(fbo->handle).color = color;
}

r_clear_flag r_framebuffer_get_clear(r_framebuffer fbo) {
  NTF_ASSERT(fbo);
  return fbo->ctx->draw_lists.at(fbo->handle).clear;
}

uvec4 r_frambuffer_get_viewport(r_framebuffer fbo) {
  NTF_ASSERT(fbo);
  return fbo->ctx->draw_lists.at(fbo->handle).viewport;
}

color4 r_framebuffer_get_color(r_framebuffer fbo) {
  NTF_ASSERT(fbo);
  return fbo->ctx->draw_lists.at(fbo->handle).color;
}

r_context r_framebuffer_get_ctx(r_framebuffer fbo) {
  NTF_ASSERT(fbo);
  return fbo->ctx;
}

r_platform_fbo r_framebuffer_get_handle(r_framebuffer fbo) {
  NTF_ASSERT(fbo);
  return fbo->handle;
}

r_framebuffer r_get_default_framebuffer(r_context ctx) {
  return &ctx->default_fbo;
}

} // namespace ntf
