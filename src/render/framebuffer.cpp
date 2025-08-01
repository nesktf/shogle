#include "./framebuffer.hpp"
#include "./context.hpp"

namespace shogle {

framebuffer_t_::framebuffer_t_(context_t ctx_,
                               extent2d extent_, fbo_buffer test_buffer_,
                               const ctx_render_data::fbo_data_t& fdata_) noexcept :
  ctx_res_node<framebuffer_t_>{ctx_},
  handle{DEFAULT_FBO_HANDLE},
  extent{extent_},
  test_buffer{test_buffer_},
  attachments{ctx_->alloc().make_deleter<fbo_image>()},
  cmds{ctx_->alloc().make_adaptor<ctx_render_cmd>()},
  fdata{fdata_} {}

framebuffer_t_::framebuffer_t_(context_t ctx_, ctx_fbo handle_,
                               extent2d extent_, fbo_buffer test_buffer_,
                               ctx_alloc::uarray_t<fbo_image>&& attachments,
                               const ctx_render_data::fbo_data_t& fdata_) noexcept :
  ctx_res_node<framebuffer_t_>{ctx_},
  handle{handle_},
  extent{extent_},
  test_buffer{test_buffer_},
  attachments{std::move(attachments)},
  cmds{ctx_->alloc().make_adaptor<ctx_render_cmd>()},
  fdata{fdata_} {}

framebuffer_t_::~framebuffer_t_() noexcept {
  for (const auto& att : attachments) {
    destroy_texture(att.texture); // decreases refcount and maybe destroys
  }
}

static ctx_fbo_desc transform_desc(ctx_alloc& alloc, const fbo_image_desc& desc) {
  auto ctx_atts = alloc.arena_span<ctx_fbo_desc::tex_att_t>(desc.images.size());
  NTF_ASSERT(!ctx_atts.empty());
  auto atts = alloc.make_uninited_array<fbo_image>(desc.images.size());
  NTF_ASSERT(!atts.empty());
  for (size_t i = 0u; const auto& att : desc.images) {
    ctx_atts[i].layer = att.layer;
    ctx_atts[i].level = att.level;
    ctx_atts[i].texture = att.texture->handle;
    std::construct_at(atts.get()+i, att);
    ++i;
  }

  return {
    .extent = desc.extent,
    .test_buffer = desc.test_buffer,
    .ctx_attachments = ctx_atts,
    .attachments = std::move(atts),
  };
}

static render_expect<void> validate_desc(context_t ctx, const fbo_image_desc& desc) {
  NTF_ASSERT(ctx);
  if (desc.viewport.x+desc.viewport.z != desc.extent.x ||
      desc.viewport.y+desc.viewport.w != desc.extent.y) {
    SHOGLE_LOG(warning, "Mismatching viewport size");
  }
  if (desc.images.empty()) {
    return {ntf::unexpect, render_error::tex_no_images};
  }

  for (const auto& att : desc.images) {
    auto tex = att.texture;
    if (!tex || tex->ctx != ctx) {
      return {ntf::unexpect, render_error::invalid_handle};
    }

    if (att.layer > tex->layers) {
      return {ntf::unexpect, render_error::tex_invalid_layer};
    }

    if (att.level > tex->levels) {
      return {ntf::unexpect, render_error::tex_invalid_level};
    }

    if (tex->extent.x != desc.extent.x || tex->extent.y != desc.extent.y) {
      return {ntf::unexpect, render_error::tex_invalid_extent};
    }
  }

  return {};
}

static const char* fbo_buffer_str(fbo_buffer buff) {
  switch (buff) {
    case fbo_buffer::depth16u: return "DEPTH16U";
    case fbo_buffer::depth24u: return "DEPTH24U";
    case fbo_buffer::depth32f: return "DEPTH32F";
    case fbo_buffer::depth24u_stencil8u: return "DEPTH24U_STENCIL8U";
    case fbo_buffer::depth32f_stencil8u: return "DEPTH32F_STENCIL8U";
    case fbo_buffer::none: return "NONE";
  }
  NTF_UNREACHABLE();
}

render_expect<framebuffer_t> create_framebuffer(context_t ctx, const fbo_image_desc& desc) {
  if (!ctx) {
    return {ntf::unexpect, render_error::invalid_handle};
  }

  try {
    auto& alloc = ctx->alloc();
    return validate_desc(ctx, desc)
    .and_then([&]() -> render_expect<ctx_fbo_desc> { return transform_desc(alloc, desc); })
    .and_then([&](ctx_fbo_desc&& fbo_desc) -> render_expect<framebuffer_t> {
      auto* fbo = alloc.allocate_uninited<framebuffer_t_>();
      NTF_ASSERT(fbo);

      ctx_fbo handle = CTX_HANDLE_TOMB;
      const auto ret = ctx->renderer().create_framebuffer(handle, fbo_desc);
      if (ret != render_error::no_error){
        alloc.deallocate(fbo, sizeof(framebuffer_t_));
        return {ntf::unexpect, ret};
      }

      NTF_ASSERT(check_handle(handle));
      const ctx_render_data::fbo_data_t fdata {
        .clear_color = desc.clear_color,
        .viewport = desc.viewport,
        .clear_flags = desc.clear_flags,
      };
      std::construct_at(fbo,
                        ctx, handle, fbo_desc.extent, fbo_desc.test_buffer,
                        std::move(fbo_desc.attachments), fdata);
      ctx->insert_node(fbo);
      NTF_ASSERT(fbo->prev == nullptr);
      SHOGLE_LOG(verbose, "Framebuffer created ({}) [ext: {}x{}, buf: {}, atts: {}]",
                     fbo->handle, fbo->extent.x, fbo->extent.y,
                     fbo_buffer_str(fbo->test_buffer), fbo->attachments.size());
      return fbo;
    });
  } RET_ERROR_CATCH("Failed to create framebuffer");
}

void destroy_framebuffer(framebuffer_t fbo) noexcept {
  if (!fbo) {
    return;
  }
  const auto handle = fbo->handle;
  auto* ctx = fbo->ctx;

  SHOGLE_LOG(verbose, "Framebuffer destroyed ({}) [ext: {}x{}, buf: {}, atts: {}]",
                 fbo->handle, fbo->extent.x, fbo->extent.y,
                 fbo_buffer_str(fbo->test_buffer), fbo->attachments.size());

  ctx->remove_node(fbo);
  ctx->renderer().destroy_framebuffer(handle);
  ctx->alloc().destroy(fbo);
}

void framebuffer_set_clear_flags(framebuffer_t fbo, clear_flag flags) {
  NTF_ASSERT(fbo);
  fbo->fdata.clear_flags = flags;
}

void framebuffer_set_viewport(framebuffer_t fbo, const uvec4& vp) {
  NTF_ASSERT(fbo);
  fbo->fdata.viewport = vp;
}

void framebuffer_set_clear_color(framebuffer_t fbo, const color4& color) {
  NTF_ASSERT(fbo);
  fbo->fdata.clear_color = color;
}

clear_flag framebuffer_get_clear_flags(framebuffer_t fbo) {
  NTF_ASSERT(fbo);
  return fbo->fdata.clear_flags;
}

uvec4 framebuffer_get_viewport(framebuffer_t fbo) {
  NTF_ASSERT(fbo);
  return fbo->fdata.viewport;
}

color4 framebuffer_get_clear_color(framebuffer_t fbo) {
  NTF_ASSERT(fbo);
  return fbo->fdata.clear_color;
}

context_t framebuffer_get_ctx(framebuffer_t fbo) {
  NTF_ASSERT(fbo);
  return fbo->ctx;
}

ctx_handle framebuffer_get_id(framebuffer_t fbo) {
  NTF_ASSERT(fbo);
  return fbo->handle;
}

framebuffer_t get_default_framebuffer(context_t ctx) {
  NTF_ASSERT(ctx);
  return ctx->default_fbo();
}

} // namespace shogle
