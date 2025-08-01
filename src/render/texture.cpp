#include "./texture.hpp"
#include "./context.hpp"

namespace shogle {

texture_t_::texture_t_(context_t ctx_, ctx_tex handle_, const ctx_tex_desc& desc) noexcept :
  ctx_res_node<texture_t_>{ctx_},
  handle{handle_},
  refcount{1},
  type{desc.type}, format{desc.format},
  sampler{desc.sampler}, addressing{desc.addressing},
  extent{desc.extent},
  levels{desc.levels}, layers{desc.layers} {}

texture_t_::~texture_t_() noexcept {}

static ctx_tex_desc transform_desc(const texture_desc& desc) {
  return {
    .type = desc.type,
    .format = desc.format,
    .sampler = desc.sampler,
    .addressing = desc.addressing,
    .extent = desc.extent,
    .layers = desc.layers,
    .levels = desc.levels,
    .images = desc.data ? desc.data->images : span<const image_data>{},
    .gen_mipmaps = desc.data ? desc.data->generate_mipmaps : false,
  };
}

static expect<void> validate_images(ctx_alloc& alloc,
                                    span<const image_data> images, bool gen_mipmaps,
                                    const extent3d& max_extent,
                                    uint32 max_layers, uint32 max_levels, texture_type type)
{
  if (images.empty()) {
    RET_ERROR("No images");
  }

  if (gen_mipmaps && max_levels == 1) {
    SHOGLE_LOG(warning, "Ignoring mipmap generation for texture with level 1");
  }

  for (size_t i = 0u; const auto& img : images) {
    RET_ERROR_FMT_IF(!img.bitmap, alloc, "No image data at index {}", i);
    RET_ERROR_FMT_IF(img.layer > max_layers, alloc, "Invalid image layer at index {}", i);
    RET_ERROR_FMT_IF(img.level > max_levels, alloc, "Invalid image level index {}", i);

    const uint32 texels = img.extent.x*img.extent.y*img.extent.z;
    RET_ERROR_FMT_IF(texels == 0, alloc, "Invalid texture extent at {}", i);

    const extent3d upload_extent = img.offset+img.extent;
    switch (type) {
      case texture_type::texture1d: {
        RET_ERROR_FMT_IF(upload_extent.x > max_extent.x,
                         alloc, "Invalid image extent at index {}", i);
        break;
      }
      case texture_type::cubemap: [[fallthrough]];
      case texture_type::texture2d: {
        RET_ERROR_FMT_IF(upload_extent.x > max_extent.x ||
                         upload_extent.y > max_extent.y,
                         alloc, "Invalid image extent at index {}", i);
        break;
      }
      case texture_type::texture3d: {
        RET_ERROR_FMT_IF(upload_extent.x > max_extent.x ||
                         upload_extent.y > max_extent.y ||
                         upload_extent.z > max_extent.z,
                         alloc, "Invalid image extent at index {}", i);
        break;
      }
    }
    ++i;
  }
  return {};
}

static expect<void> validate_desc(context_t ctx, const texture_desc& desc) {
  NTF_ASSERT(ctx);
  auto& alloc = ctx->alloc();
  ctx_limits ctx_meta;
  ctx->renderer().get_limits(ctx_meta);

  RET_ERROR_FMT_IF(desc.layers > ctx_meta.tex_max_layers,
                   alloc, "Texture layers to high ({} > {})",desc.layers, ctx_meta.tex_max_layers);

  RET_ERROR_IF(desc.type == texture_type::cubemap && desc.extent.x != desc.extent.y,
               "Invalid cubemap extent");

  RET_ERROR_IF(desc.type == texture_type::texture3d && desc.layers > 1,
               "Invalid layers for texture3d");

  RET_ERROR_IF(desc.type == texture_type::cubemap && desc.layers != 6,
               "Invalid layers for cubemap");

  RET_ERROR_FMT_IF(desc.levels > 7 || desc.levels == 0,
                   alloc, "Invalid texture level \"{}\"", desc.levels);

  switch (desc.type) {
    case texture_type::texture1d: {
      const auto max_ext = ctx_meta.tex_max_extent;
      RET_ERROR_FMT_IF(desc.extent.x > max_ext,
                       alloc, "Requested texture is too big ({} > {})",
                       desc.extent.x, ctx_meta.tex_max_extent);
      break;
    }
    case texture_type::cubemap: [[fallthrough]];
    case texture_type::texture2d: {
      const auto max_ext = ctx_meta.tex_max_extent;
      RET_ERROR_FMT_IF(desc.extent.x > max_ext ||
                       desc.extent.y > max_ext,
                       alloc, "Requested texture is too big ({}x{} > {}x{})",
                       desc.extent.x, desc.extent.y, max_ext, max_ext);
      break;
    }
    case texture_type::texture3d: {
      const auto max_ext = ctx_meta.tex_max_extent3d;
      RET_ERROR_FMT_IF(desc.extent.x > max_ext ||
                       desc.extent.y > max_ext ||
                       desc.extent.z > max_ext,
                       alloc, "Requested texture is too big ({}x{}x{} > {}x{}x{})",
                       desc.extent.x, desc.extent.y, desc.extent.z,
                       max_ext, max_ext, max_ext);
      break;
    }
  }

  if (desc.data) {
    return validate_images(alloc, desc.data->images, desc.data->generate_mipmaps,
                           desc.extent, desc.layers, desc.levels, desc.type);
  }
  return {};
}

static const char* tex_type_str(texture_type type) {
  switch (type) {
    case texture_type::cubemap: return "CUBEMAP";
    case texture_type::texture1d: return "TEX1D";
    case texture_type::texture2d: return "TEX2D";
    case texture_type::texture3d: return "TEX3D";
  }
  NTF_UNREACHABLE();
}

static const char* tex_add_str(texture_addressing addressing) {
  switch (addressing) {
    case texture_addressing::repeat: return "REPEAT";
    case texture_addressing::repeat_mirrored: return "REPEAT_MIRROR";
    case texture_addressing::clamp_edge: return "CLAMP_EDGE";
    case texture_addressing::clamp_border: return "CLAMP_BORDER";
    case texture_addressing::clamp_edge_mirrored: return "CLAMP_EDGE_MIRROR";
  }
  NTF_UNREACHABLE();
}

static const char* tex_smpl_str(texture_sampler sampler) {
  switch (sampler) {
    case texture_sampler::linear: return "LINEAR";
    case texture_sampler::nearest: return "NEAREST";
  }
  NTF_UNREACHABLE();
}

static ntf::unexpected<render_error> handle_error(ctx_tex_status status) {
  switch (status) {
    case CTX_TEX_STATUS_INVALID_LEVELS: {
      RET_ERROR("Invalid texture level value");
    }
    case CTX_TEX_STATUS_INVALID_SAMPLER: {
      RET_ERROR("Invalid texture sampler value");
    }
    case CTX_TEX_STATUS_INVALID_ADDRESING: {
      RET_ERROR("Invalid texture addressing value");
    }
    case CTX_TEX_STATUS_INVALID_HANDLE: {
      RET_ERROR("Invalid texture handle");
    }
    case CTX_TEX_STATUS_OK: NTF_UNREACHABLE();
  }
  NTF_UNREACHABLE();
}

expect<texture_t> create_texture(context_t ctx, const texture_desc& desc) {
  RET_ERROR_IF(!ctx, "Invalid context handle");
  try {
    auto& alloc = ctx->alloc();
    return validate_desc(ctx, desc)
    .transform([&]() -> ctx_tex_desc { return transform_desc(desc); })
    .and_then([&](ctx_tex_desc&& tex_desc)-> expect<texture_t> {
      auto* tex = alloc.allocate_uninited<texture_t_>();
      NTF_ASSERT(tex);

      ctx_tex handle = CTX_HANDLE_TOMB;
      const auto ret = ctx->renderer().create_texture(handle, tex_desc);
      if (ret != CTX_TEX_STATUS_OK) {
        alloc.deallocate(tex, sizeof(texture_t_));
        return handle_error(ret);
      }

      NTF_ASSERT(check_handle(handle));
      std::construct_at(tex,
                        ctx, handle, tex_desc);
      ctx->insert_node(tex);
      NTF_ASSERT(tex->prev == nullptr);
      SHOGLE_LOG(verbose,
        "Texture created ({}) [type: {}, ext: {}x{}x{}, lvl: {}, lyr: {}, add: {}, sam: {}]",
        tex->handle,
        tex_type_str(tex->type), tex->extent.x, tex->extent.y, tex->extent.z,
        tex->levels, tex->layers,
        tex_add_str(tex->addressing), tex_smpl_str(tex->sampler)
      );
      return tex;
    });
  } RET_ERROR_CATCH("Failed to create texture");
}

void destroy_texture(texture_t tex) noexcept {
  if (!tex) {
    return;
  }
  const auto handle = tex->handle;
  auto* ctx = tex->ctx;
  if (--tex->refcount > 0) {
    return;
  }
  SHOGLE_LOG(verbose, 
    "Texture destroyed ({}) [type: {}, ext: {}x{}x{}, lvl: {}, lyr: {}, add: {}, sam: {}]",
    tex->handle,
    tex_type_str(tex->type), tex->extent.x, tex->extent.y, tex->extent.z,
    tex->levels, tex->layers,
    tex_add_str(tex->addressing), tex_smpl_str(tex->sampler)
  );

  ctx->remove_node(tex);
  ctx->renderer().destroy_texture(handle);
  ctx->alloc().destroy(tex);
}

expect<void> texture_upload(texture_t tex, const texture_data& data) {
  RET_ERROR_IF(!tex, "Invalid handle");

  try {
    auto& alloc = tex->ctx->alloc();
    return validate_images(alloc, data.images, data.generate_mipmaps, tex->extent,
                           tex->layers, tex->levels, tex->type)
    .and_then([&]() -> expect<void> {
      const auto ret = tex->ctx->renderer().update_texture(tex->handle, ctx_tex_data {
        .images = data.images,
        .generate_mipmaps = data.generate_mipmaps,
      });
      if (ret != CTX_TEX_STATUS_OK) {
        return handle_error(ret);
      }
      return {};
    });
  }
  RET_ERROR_CATCH("Failed to update texture");
  return {};
}

expect<void> texture_set_sampler(texture_t tex, texture_sampler sampler) {
  RET_ERROR_IF(!tex, "Invalid texture handle");
  if (sampler == tex->sampler) {
    return {};
  }

  try {
    const auto ret = tex->ctx->renderer().update_texture(tex->handle, ctx_tex_opts{
      .sampler = sampler,
      .addresing = tex->addressing,
    });
    RET_ERROR_IF(ret == CTX_TEX_STATUS_INVALID_SAMPLER, "Failed to update sampler");
    tex->sampler = sampler;
  }
  RENDER_ERROR_LOG_CATCH("Failed to update sampler");
  return {};
}

expect<void> texture_set_addressing(texture_t tex, texture_addressing addressing) {
  RET_ERROR_IF(!tex, "Invalid texture handle");
  if (addressing == tex->addressing) {
    return {};
  }

  try {
    const auto ret = tex->ctx->renderer().update_texture(tex->handle, ctx_tex_opts{
      .sampler = tex->sampler,
      .addresing = addressing,
    });
    RET_ERROR_IF(ret == CTX_TEX_STATUS_INVALID_ADDRESING, "Failed to update addressing");
    tex->addressing = addressing;
  }
  RENDER_ERROR_LOG_CATCH("Failed to update addressing");
  return {};
}

texture_type texture_get_type(texture_t tex) {
  NTF_ASSERT(tex);
  return tex->type;
}

image_format texture_get_format(texture_t tex) {
  NTF_ASSERT(tex);
  return tex->format;
}

texture_sampler texture_get_sampler(texture_t tex) {
  NTF_ASSERT(tex);
  return tex->sampler;
}

texture_addressing texture_get_addressing(texture_t tex) {
  NTF_ASSERT(tex);
  return tex->addressing;
}

extent3d texture_get_extent(texture_t tex) {
  NTF_ASSERT(tex);
  return tex->extent;
}

uint32 texture_get_layers(texture_t tex) {
  NTF_ASSERT(tex);
  return tex->layers;
}

uint32 texture_get_levels(texture_t tex) {
  NTF_ASSERT(tex);
  return tex->levels;
}

context_t texture_get_ctx(texture_t tex) {
  NTF_ASSERT(tex);
  return tex->ctx;
}

ctx_handle texture_get_id(texture_t tex){
  NTF_ASSERT(tex);
  return tex->handle;
}

} // namespace shogle
