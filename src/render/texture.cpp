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

static render_expect<void> validate_images(
                                    span<const image_data> images, bool gen_mipmaps,
                                    const extent3d& max_extent,
                                    uint32 max_layers, uint32 max_levels, texture_type type)
{
  if (images.empty()) {
    return {ntf::unexpect, render_error::tex_no_images};
  }

  if (gen_mipmaps && max_levels == 1) {
    SHOGLE_LOG(warning, "Ignoring mipmap generation for texture with level 1");
  }

  for (const auto& img : images) {
    if (!img.bitmap) {
      return {ntf::unexpect, render_error::no_data};
    }

    if (img.layer > max_layers) {
      return {ntf::unexpect, render_error::tex_invalid_layer};
    }

    if (img.level > max_levels) {
      return {ntf::unexpect, render_error::tex_invalid_level};
    }

    const uint32 texels = img.extent.x*img.extent.y*img.extent.z;
    if (texels == 0) {
      return {ntf::unexpect, render_error::tex_invalid_extent};
    }

    const extent3d upload_extent = img.offset+img.extent;
    switch (type) {
      case texture_type::texture1d: {
        if (upload_extent.x > max_extent.x) {
          return {ntf::unexpect, render_error::tex_invalid_extent};
        }
        break;
      }
      case texture_type::cubemap: [[fallthrough]];
      case texture_type::texture2d: {
        if (upload_extent.x > max_extent.x ||
            upload_extent.y > max_extent.y) {
          return {ntf::unexpect, render_error::tex_invalid_extent};
        }
        break;
      }
      case texture_type::texture3d: {
        if (upload_extent.x > max_extent.x ||
            upload_extent.y > max_extent.y ||
            upload_extent.z > max_extent.z) {
          return {ntf::unexpect, render_error::tex_invalid_extent};
        }
        break;
      }
    }
  }
  return {};
}

static render_expect<void> validate_desc(context_t ctx, const texture_desc& desc) {
  NTF_ASSERT(ctx);
  ctx_limits ctx_meta;
  ctx->renderer().get_limits(ctx_meta);

  if (desc.layers > ctx_meta.tex_max_layers) {
    return {ntf::unexpect, render_error::tex_invalid_layer};
  }

  if (desc.type == texture_type::cubemap && desc.extent.x != desc.extent.y) {
    return {ntf::unexpect, render_error::tex_invalid_extent};
  }

  if (desc.type == texture_type::texture3d && desc.layers > 1) {
    return {ntf::unexpect, render_error::tex_invalid_layer};
  }

  if (desc.type == texture_type::cubemap && desc.layers != 0) {
    return {ntf::unexpect, render_error::tex_invalid_layer};
  }

  if (desc.levels > 7 || desc.levels == 0) {
    return {ntf::unexpect, render_error::tex_invalid_level};
  }

  switch (desc.type) {
    case texture_type::texture1d: {
      const auto max_ext = ctx_meta.tex_max_extent;
      if (desc.extent.x > max_ext) {
        return {ntf::unexpect, render_error::tex_out_of_limits_extent};
      }
      break;
    }
    case texture_type::cubemap: [[fallthrough]];
    case texture_type::texture2d: {
      const auto max_ext = ctx_meta.tex_max_extent;
      if (desc.extent.x > max_ext ||
          desc.extent.y > max_ext) {
        return {ntf::unexpect, render_error::tex_out_of_limits_extent};
      }
      break;
    }
    case texture_type::texture3d: {
      const auto max_ext = ctx_meta.tex_max_extent3d;
      if (desc.extent.x > max_ext ||
          desc.extent.y > max_ext ||
          desc.extent.z > max_ext) {
        return {ntf::unexpect, render_error::tex_out_of_limits_extent};
      }
      break;
    }
  }

  if (desc.data) {
    return validate_images(desc.data->images, desc.data->generate_mipmaps,
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

render_expect<texture_t> create_texture(context_t ctx, const texture_desc& desc) {
  if (!ctx) {
    return {ntf::unexpect, render_error::invalid_handle};
  }

  try {
    auto& alloc = ctx->alloc();
    return validate_desc(ctx, desc)
    .transform([&]() -> ctx_tex_desc { return transform_desc(desc); })
    .and_then([&](ctx_tex_desc&& tex_desc)-> render_expect<texture_t> {
      auto* tex = alloc.allocate_uninited<texture_t_>();
      NTF_ASSERT(tex);

      ctx_tex handle = CTX_HANDLE_TOMB;
      const auto ret = ctx->renderer().create_texture(handle, tex_desc);
      if (ret != render_error::no_error) {
        alloc.deallocate(tex, sizeof(texture_t_));
        return {ntf::unexpect, ret};
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

render_expect<void> texture_upload(texture_t tex, const texture_data& data) {
  if (!tex) {
    return {ntf::unexpect, render_error::invalid_handle};
  }

  try {
    return validate_images(data.images, data.generate_mipmaps, tex->extent,
                           tex->layers, tex->levels, tex->type)
    .and_then([&]() -> render_expect<void> {
      const auto ret = tex->ctx->renderer().update_texture(tex->handle, ctx_tex_data {
        .images = data.images,
        .generate_mipmaps = data.generate_mipmaps,
      });
      if (ret != render_error::no_error) {
        return {ntf::unexpect, ret};
      }
      return {};
    });
  }
  RET_ERROR_CATCH("Failed to update texture");
  return {};
}

render_expect<void> texture_set_sampler(texture_t tex, texture_sampler sampler) {
  if (!tex) {
    return {ntf::unexpect, render_error::invalid_handle};
  }
  if (sampler == tex->sampler) {
    return {};
  }

  try {
    const auto ret = tex->ctx->renderer().update_texture(tex->handle, ctx_tex_opts{
      .sampler = sampler,
      .addresing = tex->addressing,
    });
    if (ret == render_error::tex_invalid_sampler) {
      return {ntf::unexpect, ret};
    }
    tex->sampler = sampler;
  }
  RENDER_ERROR_LOG_CATCH("Failed to update sampler");
  return {};
}

render_expect<void> texture_set_addressing(texture_t tex, texture_addressing addressing) {
  if (!tex) {
    return {ntf::unexpect, render_error::invalid_handle};
  }
  if (addressing == tex->addressing) {
    return {};
  }

  try {
    const auto ret = tex->ctx->renderer().update_texture(tex->handle, ctx_tex_opts{
      .sampler = tex->sampler,
      .addresing = addressing,
    });
    if (ret == render_error::tex_invalid_addressing) {
      return {ntf::unexpect, ret};
    }
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
