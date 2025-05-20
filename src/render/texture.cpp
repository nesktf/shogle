#include "./internal/common.hpp"

namespace ntf {

r_texture_::r_texture_(r_context ctx_, r_platform_texture handle_,
                       const rp_tex_desc& desc) noexcept :
  rp_res_node<r_texture_>{ctx_},
  handle{handle_},
  refcount{1},
  type{desc.type}, format{desc.format},
  extent{desc.extent},
  levels{desc.levels}, layers{desc.layers},
  addressing{desc.addressing}, sampler{desc.sampler} {}

r_texture_::~r_texture_() noexcept {}


static const char* tex_type_str(r_texture_type type) {
  switch (type) {
    case r_texture_type::cubemap: return "CUBEMAP";
    case r_texture_type::texture1d: return "TEX1D";
    case r_texture_type::texture2d: return "TEX2D";
    case r_texture_type::texture3d: return "TEX3D";
  }
  NTF_UNREACHABLE();
}

static const char* tex_add_str(r_texture_address add) {
  switch (add) {
    case r_texture_address::repeat: return "REPEAT";
    case r_texture_address::repeat_mirrored: return "REPEAT_MIRROR";
    case r_texture_address::clamp_edge: return "CLAMP_EDGE";
    case r_texture_address::clamp_border: return "CLAMP_BORDER";
    case r_texture_address::clamp_edge_mirrored: return "CLAMP_EDGE_MIRROR";
  }
  NTF_UNREACHABLE();
}

static const char* tex_smpl_str(r_texture_sampler smpl) {
  switch (smpl) {
    case r_texture_sampler::linear: return "LINEAR";
    case r_texture_sampler::nearest: return "NEAREST";
  }
  NTF_UNREACHABLE();
}

static rp_tex_desc transform_desc(const r_texture_descriptor& desc) {
  return {
    .type = desc.type,
    .format = desc.format,
    .extent = desc.extent,
    .layers = desc.layers,
    .levels = desc.levels,
    .initial_data = desc.images,
    .gen_mipmaps = desc.gen_mipmaps,
    .sampler = desc.sampler,
    .addressing = desc.addressing,
  };
};

static r_expected<void> validate_images(cspan<r_image_data> images, const extent3d& max_extent,
                                        uint32 max_layers, uint32 max_levels,
                                        r_texture_type type)
{
  for (size_t i = 0u; const auto& img : images) {
    RET_ERROR_IF(!img.texels, "No image data at index {}", i)
    RET_ERROR_IF(img.layer > max_layers, "Invalid image layer at index {}", i);
    RET_ERROR_IF(img.level > max_levels, "Invalid image level index {}", i);

    const uint32 texels = img.extent.x*img.extent.y*img.extent.z;
    RET_ERROR_IF(texels == 0, "Invalid texture extent at {}", i);

    const extent3d upload_extent = img.offset+img.extent;
    switch (type) {
      case r_texture_type::texture1d: {
        RET_ERROR_IF(upload_extent.x > max_extent.x,
                     "Invalid image extent at index {}", i);
        break;
      }
      case r_texture_type::cubemap: [[fallthrough]];
      case r_texture_type::texture2d: {
        RET_ERROR_IF(upload_extent.x > max_extent.x ||
                     upload_extent.y > max_extent.y,
                     "Invalid image extent at index {}", i);
        break;
      }
      case r_texture_type::texture3d: {
        RET_ERROR_IF(upload_extent.x > max_extent.x ||
                     upload_extent.y > max_extent.y ||
                     upload_extent.z > max_extent.z,
                     "Invalid image extent at index {}", i);
        break;
      }
    }
    ++i;
  }
  return {};
}

static r_expected<void> validate_desc(r_context ctx, const r_texture_descriptor& desc) {
  NTF_ASSERT(ctx);
  rp_platform_meta ctx_meta{};
  ctx->renderer().get_meta(ctx_meta);

  RET_ERROR_IF(desc.layers > ctx_meta.tex_max_layers,
               "Texture layers to high ({} > {})",
               desc.layers, ctx_meta.tex_max_layers);

  RET_ERROR_IF(desc.type == r_texture_type::cubemap && desc.extent.x != desc.extent.y,
               "Invalid cubemap extent");

  RET_ERROR_IF(desc.type == r_texture_type::texture3d && desc.layers > 1,
               "Invalid layers for texture3d");

  RET_ERROR_IF(desc.type == r_texture_type::cubemap && desc.layers != 6,
               "Invalid layers for cubemap");

  RET_ERROR_IF(desc.levels > 7 || desc.levels == 0,
               "Invalid texture level \"{}\"",
               desc.levels);

  if (desc.gen_mipmaps) {
    if (!desc.images) {
      RENDER_WARN_LOG("Ignoring mipmap generation for texture with no image data");
    }
    if (desc.levels == 1) {
      RENDER_WARN_LOG("Ignoring mipmap generation for texture with level 1");
    }
  }

  switch (desc.type) {
    case r_texture_type::texture1d: {
      const auto max_ext = ctx_meta.tex_max_extent;
      RET_ERROR_IF(desc.extent.x > max_ext,
                   "Requested texture is too big ({} > {})",
                   desc.extent.x, ctx_meta.tex_max_extent);
      break;
    }
    case r_texture_type::cubemap: [[fallthrough]];
    case r_texture_type::texture2d: {
      const auto max_ext = ctx_meta.tex_max_extent;
      RET_ERROR_IF(desc.extent.x > max_ext ||
                   desc.extent.y > max_ext,
                   "Requested texture is too big ({}x{} > {}x{})",
                   desc.extent.x, desc.extent.y, max_ext, max_ext);
      break;
    }
    case r_texture_type::texture3d: {
      const auto max_ext = ctx_meta.tex_max_extent3d;
      RET_ERROR_IF(desc.extent.x > max_ext ||
                   desc.extent.y > max_ext ||
                   desc.extent.z > max_ext,
                   "Requested texture is too big ({}x{}x{} > {}x{}x{})",
                   desc.extent.x, desc.extent.y, desc.extent.z,
                   max_ext, max_ext, max_ext);
      break;
    }
  }

  return validate_images(desc.images, desc.extent, desc.layers, desc.levels, desc.type);
}

r_expected<r_texture> r_create_texture(r_context ctx, const r_texture_descriptor& desc) {
  RET_ERROR_IF(!ctx, "Invalid context handle");
  auto& alloc = ctx->alloc();
  try {
  return validate_desc(ctx, desc)
    .transform([&]() -> rp_tex_desc { return transform_desc(desc); })
    .and_then([&](rp_tex_desc&& tex_desc)-> r_expected<r_texture> {
      r_platform_texture handle = ctx->renderer().create_texture(tex_desc);
      RET_ERROR_IF(!handle, "Failed to create texture handle");

      auto* tex = alloc.allocate_uninited<r_texture_>(1u);
      if (!tex) {
        ctx->renderer().destroy_texture(handle);
        RET_ERROR("Failed to allocate texture");
      }
      std::construct_at(tex,
                        ctx, handle, tex_desc);
      ctx->insert_node(tex);
      NTF_ASSERT(tex->prev == nullptr);
      SHOGLE_LOG(debug, "[ntf::r_create_texture] {} {}x{}x{} (lvl: {}, lyr: {}, a: {}, s: {})",
                 tex_type_str(tex->type), tex->extent.x, tex->extent.y, tex->extent.z,
                 tex->levels, tex->layers,
                 tex_add_str(tex->addressing), tex_smpl_str(tex->sampler));

      return tex;
    });
  } RET_ERROR_CATCH("Failed to create texture");
}

void r_destroy_texture(r_texture tex) noexcept {
  if (!tex) {
    return;
  }
  const auto handle = tex->handle;
  auto* ctx = tex->ctx;

  if (--tex->refcount > 0) {
    return;
  }
  SHOGLE_LOG(debug, "[ntf::r_destroy_texture] {} {}x{}x{} (lvl: {}, lyr: {}, a: {}, s: {})",
             tex_type_str(tex->type), tex->extent.x, tex->extent.y, tex->extent.z,
             tex->levels, tex->layers,
             tex_add_str(tex->addressing), tex_smpl_str(tex->sampler));

  ctx->remove_node(tex);
  ctx->renderer().destroy_texture(handle);
  ctx->alloc().destroy(tex);
}

static bool update_texture_images(r_texture tex, cspan<r_image_data> images, bool regen_mips) {
  size_t image_count = images.size();
  rp_tex_image_data* images_out = nullptr;

  if (!images.empty()) {
    auto& alloc = tex->ctx->alloc();
    auto* images_out = alloc.arena_allocate_uninited<rp_tex_image_data>(images.size());
    if (!images_out) {
      return false;
    }
    for (size_t i = 0u; const auto& image_in : images) {
      images_out[i].texels = image_in.texels;
      images_out[i].format = image_in.format;
      images_out[i].alignment = image_in.alignment;
      images_out[i].extent = image_in.extent;
      images_out[i].offset = image_in.offset;
      images_out[i].layer = image_in.layer;
      images_out[i].level = image_in.level;
      ++i;
    }
  }
  tex->ctx->renderer().upload_texture_images(tex->handle,
                                             cspan<rp_tex_image_data>{images_out, image_count},
                                             regen_mips);
  return true;
}

void update_texture_opts(r_texture tex,
                         optional<r_texture_sampler> sampler,
                         optional<r_texture_address> addressing)
{
  rp_tex_opts opts;
  opts.addressing = addressing.value_or(tex->addressing);
  opts.sampler = sampler.value_or(tex->sampler);
  tex->ctx->renderer().update_texture_options(tex->handle, opts);

  if (addressing) {
    tex->addressing = *addressing;
  }
  if (sampler) {
    tex->sampler = *sampler;
  }
}

r_expected<void> r_texture_upload(r_texture tex, const r_texture_data& data) {
  RET_ERROR_IF(!tex, "Invalid handle");
  try {
    return validate_images(data.images, tex->extent,
                           tex->layers, tex->levels, tex->type)
    .and_then([&]() -> r_expected<void> {
      RET_ERROR_IF(!update_texture_images(tex, data.images, data.gen_mipmaps),
                   "Failed to allocate image descriptors");
      update_texture_opts(tex, data.sampler, data.addressing);
      return {};
    });
  }
  RET_ERROR_CATCH("Failed to update texture");

  return {};
}

r_expected<void> r_texture_upload(r_texture tex,
                                  cspan<r_image_data> images, bool regen_mips) {
  RET_ERROR_IF(!tex, "Invalid handle");
  try {
    return validate_images(images, tex->extent,
                           tex->layers, tex->levels, tex->type)
    .and_then([&]() -> r_expected<void> {
      RET_ERROR_IF(!update_texture_images(tex, images, regen_mips),
                   "Failed to allocate image descriptors");
      return {};
    });
  }
  RET_ERROR_CATCH("Failed to update texture");

  return {};
}

void r_texture_set_sampler(r_texture tex, r_texture_sampler sampler) {
  if (!tex) {
    RENDER_ERROR_LOG("Invalid texture handle");
    return;
  }
  try {
    update_texture_opts(tex, sampler, nullopt);
  }
  RENDER_ERROR_LOG_CATCH("Failed to update sampler");
}

void r_texture_set_addressing(r_texture tex, r_texture_address addressing) {
  if (!tex) {
    RENDER_ERROR_LOG("Invalid texture handle");
    return;
  }
  try {
    update_texture_opts(tex, nullopt, addressing);
  }
  RENDER_ERROR_LOG_CATCH("Failed to upadte addressing");
}

r_texture_type r_texture_get_type(r_texture tex) {
  NTF_ASSERT(tex);
  return tex->type;
}

r_texture_format r_texture_get_format(r_texture tex) {
  NTF_ASSERT(tex);
  return tex->format;
}

r_texture_sampler r_texture_get_sampler(r_texture tex) {
  NTF_ASSERT(tex);
  return tex->sampler;
}

r_texture_address r_texture_get_addressing(r_texture tex) {
  NTF_ASSERT(tex);
  return tex->addressing;
}

extent3d r_texture_get_extent(r_texture tex) {
  NTF_ASSERT(tex);
  return tex->extent;
}

uint32 r_texture_get_layers(r_texture tex) {
  NTF_ASSERT(tex);
  return tex->layers;
}

uint32 r_texture_get_levels(r_texture tex) {
  NTF_ASSERT(tex);
  return tex->levels;
}

r_context r_texture_get_ctx(r_texture tex) {
  NTF_ASSERT(tex);
  return tex->ctx;
}

r_platform_texture r_texture_get_handle(r_texture tex) {
  NTF_ASSERT(tex);
  return tex->handle;
}

} // namespace ntf
