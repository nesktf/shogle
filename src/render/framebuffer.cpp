#include "./internal/common.hpp"

namespace ntf {

r_framebuffer_::r_framebuffer_(r_context ctx_, r_platform_fbo handle_,
                               extent2d extent_, r_test_buffer test_buffer_,
                               span<r_framebuffer_attachment> attachments_,
                               const rp_fbo_frame_data& fdata_) noexcept :
  rp_res_node<r_framebuffer_>{ctx_},
  handle{handle_},
  extent{extent_},
  test_buffer{test_buffer_},
  attachments{attachments_}, att_state{ATT_TEX},
  cmds{ctx_->alloc().make_adaptor<rp_draw_cmd>()},
  fdata{fdata_} {}

r_framebuffer_::r_framebuffer_(r_context ctx_, r_platform_fbo handle_,
                               extent2d extent_, r_test_buffer test_buffer_,
                               r_texture_format color_buffer_,
                               const rp_fbo_frame_data& fdata_) noexcept :
  rp_res_node<r_framebuffer_>{ctx_},
  handle{handle_},
  extent{extent_},
  test_buffer{test_buffer_},
  color_buffer{color_buffer_}, att_state{ATT_BUFF},
  cmds{ctx_->alloc().make_adaptor<rp_draw_cmd>()},
  fdata{fdata_} {}

r_framebuffer_::r_framebuffer_(r_context ctx_,
                               extent2d extent_, r_test_buffer test_buffer_,
                               const rp_fbo_frame_data& fdata_) noexcept :
  rp_res_node<r_framebuffer_>{ctx_},
  handle{DEFAULT_FBO_HANDLE},
  extent{extent_},
  test_buffer{test_buffer_},
  _dummy{}, att_state{ATT_NONE},
  cmds{ctx_->alloc().make_adaptor<rp_draw_cmd>()},
  fdata{fdata_} {}

r_framebuffer_::~r_framebuffer_() noexcept {
  if (att_state == ATT_TEX) {
    for (const auto& att : attachments) {
      r_destroy_texture(att.texture); // decreases refcount and maybe destroys
    }
    ctx->alloc().destroy(attachments.data());
  }
}

static rp_fbo_desc transform_desc(rp_alloc& alloc, const r_framebuffer_descriptor& desc) {
  rp_fbo_desc out;
  out.extent = desc.extent;
  out.test_buffer = desc.test_buffer;
  desc.attachments | ::ntf::overload{
    [&](cspan<r_framebuffer_attachment> atts) {
      auto* atts_out = alloc.arena_allocate_uninited<rp_fbo_att>(atts.size());
      if (!atts_out) {
        out.attachments = {};
        return;
      }
      for (size_t i = 0u; const auto& att : atts) {
        atts_out[i].layer = att.layer;
        atts_out[i].level = att.level;
        atts_out[i].texture = att.texture->handle;
      }

      out.attachments = {atts_out, atts.size()};
    },
    [&](r_texture_format color_buffer) {
      out.color_buffer = color_buffer;
    }
  };
  return out;
}

static r_expected<void> validate_desc(r_context ctx, const r_framebuffer_descriptor& desc) {
  NTF_ASSERT(ctx);
  if (desc.viewport.x+desc.viewport.z != desc.extent.x ||
      desc.viewport.y+desc.viewport.w != desc.extent.y) {
    RENDER_WARN_LOG("Mismatching viewport size");
  }

  if (std::holds_alternative<cspan<r_framebuffer_attachment>>(desc.attachments)) {
    auto attachments_in = std::get<cspan<r_framebuffer_attachment>>(desc.attachments);
    RET_ERROR_IF(attachments_in.empty(), "Invalid attachment span");
    for (uint32 i = 0; i < attachments_in.size(); ++i) {
      const auto& att = attachments_in[i];
      r_texture tex = att.texture;
      RET_ERROR_IF(!tex || tex->ctx != ctx, "Invalid texture handle at index {}", i);

      RET_ERROR_IF(att.layer > tex->layers, "Invalid texture layer at index {}", i);

      RET_ERROR_IF(att.level > tex->levels, "Invalid texture level at index {}", i);

      RET_ERROR_IF(tex->extent.x != desc.extent.x || tex->extent.y != desc.extent.y,
                   "Invalid texture extent at index {}", i);
    }
  }

  return {};
}

r_expected<r_framebuffer> r_create_framebuffer(r_context ctx,
                                               const r_framebuffer_descriptor& desc) {
  RET_ERROR_IF(!ctx, "Invalid context handle");
  auto& alloc = ctx->alloc();

  return validate_desc(ctx, desc)
  .and_then([&]() -> r_expected<rp_fbo_desc> {
    auto fbo_desc = transform_desc(alloc, desc);
    RET_ERROR_IF(!fbo_desc.color_buffer && fbo_desc.attachments.empty(),
                 "Failed to allocate attachment descriptors");
    return fbo_desc;
  })
  .and_then([&](rp_fbo_desc&& fbo_desc) -> r_expected<r_framebuffer> {
    r_platform_fbo handle;
    try {
      handle = ctx->renderer().create_framebuffer(fbo_desc);
      RET_ERROR_IF(!handle, "Failed to create framebuffer");
    }
    RET_ERROR_CATCH("Failed to create framebuffer");

    auto* fbo = alloc.allocate_uninited<r_framebuffer_>(1u);
    if (!fbo) {
      ctx->renderer().destroy_framebuffer(handle);
      RET_ERROR("Failed to allocate framebuffer");
    }
    const rp_fbo_frame_data fdata {
      .clear_color = desc.clear_color,
      .viewport = desc.viewport,
      .clear_flags = desc.clear_flags,
    };
    if (!fbo_desc.attachments.empty()) {
      auto* atts = alloc.allocate_uninited<r_framebuffer_attachment>(fbo_desc.attachments.size());
      if (!atts) {
        alloc.deallocate(fbo, sizeof(r_framebuffer_));
        ctx->renderer().destroy_framebuffer(handle),
        RET_ERROR("Failed to allocate framebuffer attachments");
      }
      for (size_t i = 0;
           const auto& att : std::get<cspan<r_framebuffer_attachment>>(desc.attachments)) {
        atts[i].level = att.level;
        atts[i].layer = att.layer;
        atts[i].texture = att.texture;
      }
      std::construct_at(fbo,
                        ctx, handle, fbo_desc.extent, fbo_desc.test_buffer,
                        span<r_framebuffer_attachment>{atts, fbo_desc.attachments.size()},
                        fdata);
    } else {
      std::construct_at(fbo,
                        ctx, handle, fbo_desc.extent, fbo_desc.test_buffer,
                        *fbo_desc.color_buffer, fdata);
    }
    ctx->insert_node(fbo);
    NTF_ASSERT(fbo->prev == nullptr);

    return fbo;
  });
}

r_framebuffer r_create_framebuffer(unchecked_t, r_context ctx,
                                   const r_framebuffer_descriptor& desc) {
  if (!ctx) {
    return nullptr;
  }

  auto& alloc = ctx->alloc();
  auto fbo_desc = transform_desc(alloc, desc);
  if (!fbo_desc.attachments.empty() || fbo_desc.color_buffer) {
    RENDER_ERROR_LOG("Failed to allocate attachment descriptors");
    return nullptr;
  }

  auto handle = ctx->renderer().create_framebuffer(fbo_desc);
  if (!handle) {
    RENDER_ERROR_LOG("Failed to create framebuffer");
    return nullptr;
  }

  auto* fbo = alloc.allocate_uninited<r_framebuffer_>(1u);
  if (!fbo) {
    RENDER_ERROR_LOG("Failed to allocate framebuffer");
    return nullptr;
  }

  const rp_fbo_frame_data fdata {
    .clear_color = desc.clear_color,
    .viewport = desc.viewport,
    .clear_flags = desc.clear_flags,
  };
  if (!fbo_desc.attachments.empty()) {
    auto* atts = alloc.allocate_uninited<r_framebuffer_attachment>(fbo_desc.attachments.size());
    if (!atts) {
      RENDER_ERROR_LOG("Failed to allocate attachments");
      alloc.deallocate(fbo, sizeof(r_framebuffer_));
      ctx->renderer().destroy_framebuffer(handle);
      return nullptr;
    }

    for (size_t i = 0;
         const auto& att : std::get<cspan<r_framebuffer_attachment>>(desc.attachments)) {
      atts[i].level = att.level;
      atts[i].layer = att.layer;
      atts[i].texture = att.texture;
    }
    std::construct_at(fbo,
                      ctx, handle, fbo_desc.extent, fbo_desc.test_buffer,
                      span<r_framebuffer_attachment>{atts, fbo_desc.attachments.size()},
                      fdata);
  } else {
    std::construct_at(fbo,
                      ctx, handle, fbo_desc.extent, fbo_desc.test_buffer,
                      *fbo_desc.color_buffer, fdata);
  }

  ctx->insert_node(fbo);
  NTF_ASSERT(fbo->prev == nullptr);

  return fbo;
}

void r_destroy_framebuffer(r_framebuffer fbo) {
  if (!fbo) {
    return;
  }
  const auto handle = fbo->handle;
  auto* ctx = fbo->ctx;

  ctx->remove_node(fbo);
  ctx->renderer().destroy_framebuffer(handle);
  ctx->alloc().destroy(fbo);
}

void r_framebuffer_set_clear(r_framebuffer fbo, r_clear_flag flags) {
  NTF_ASSERT(fbo);
  fbo->fdata.clear_flags = flags;
}

void r_framebuffer_set_viewport(r_framebuffer fbo, const uvec4& vp) {
  NTF_ASSERT(fbo);
  fbo->fdata.viewport = vp;
}

void r_framebuffer_set_color(r_framebuffer fbo, const color4& color) {
  NTF_ASSERT(fbo);
  fbo->fdata.clear_color = color;
}

r_clear_flag r_framebuffer_get_clear(r_framebuffer fbo) {
  NTF_ASSERT(fbo);
  return fbo->fdata.clear_flags;
}

uvec4 r_frambuffer_get_viewport(r_framebuffer fbo) {
  NTF_ASSERT(fbo);
  return fbo->fdata.viewport;
}

color4 r_framebuffer_get_color(r_framebuffer fbo) {
  NTF_ASSERT(fbo);
  return fbo->fdata.clear_color;
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
  return &ctx->default_fbo();
}

} // namespace ntf
