#include "./internal/common.hpp"

namespace ntf {

static auto transform_descriptor(
  r_context, const r_texture_descriptor& desc
) -> rp_tex_desc {
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

static auto check_and_transform_descriptor(
  r_context ctx, const r_texture_descriptor& desc
) -> r_expected<rp_tex_desc> {
  ::ntf::logger::error(__PRETTY_FUNCTION__);
  RET_ERROR_IF(!ctx,
               "Invalid context handle");

  rp_platform_meta ctx_meta{};
  ctx->platform->get_meta(ctx_meta);

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

  for (uint32 i = 0; i < desc.images.size(); ++i) {
    const auto& img = desc.images[i];
    const uvec3 upload_extent = img.offset+img.extent;

    const uint32 texels = img.extent.x*img.extent.y*img.extent.z;

    RET_ERROR_IF(texels == 0,
                 "Invalid texture extent at {}", i);

    RET_ERROR_IF(!img.texels ||
                 img.layer > desc.layers ||
                 img.level > desc.levels,
                 "Invalid image at index {}",
                 i);
    switch (desc.type) {
      case r_texture_type::texture1d: {
        RET_ERROR_IF(upload_extent.x > desc.extent.x,
                     "Invalid image extent at index {}", i);
        break;
      }
      case r_texture_type::cubemap: [[fallthrough]];
      case r_texture_type::texture2d: {
        RET_ERROR_IF(upload_extent.x > desc.extent.x ||
                     upload_extent.y > desc.extent.y,
                     "Invalid image extent at index {}", i);
        break;
      }
      case r_texture_type::texture3d: {
        RET_ERROR_IF(upload_extent.x > desc.extent.x ||
                     upload_extent.y > desc.extent.y ||
                     upload_extent.z > desc.extent.z,
                     "Invalid image extent at index {}", i);
        break;
      }
    }
  }

  return transform_descriptor(ctx, desc);
}

r_expected<r_texture> r_create_texture(r_context ctx, const r_texture_descriptor& desc) {
  return check_and_transform_descriptor(ctx, desc)
  .and_then([ctx](rp_tex_desc&& tex_desc) -> r_expected<r_texture> {
    r_platform_texture handle;
    try {
      handle = ctx->platform->create_texture(tex_desc);
      RET_ERROR_IF(!handle, "Failed to create texture handle");
    } 
    RET_ERROR_CATCH("Failed to create texture handle");

    [[maybe_unused]] auto [it, emplaced] = ctx->textures.try_emplace(
      handle, ctx, handle, std::move(tex_desc)
    );
    NTF_ASSERT(emplaced);

    return &it->second;
  });
}

r_texture r_create_texture(unchecked_t, r_context ctx, const r_texture_descriptor& desc) {
  NTF_ASSERT(ctx);
  auto tex_desc = transform_descriptor(ctx, desc);
  auto handle = ctx->platform->create_texture(tex_desc);
  NTF_ASSERT(handle);

  [[maybe_unused]] auto [it, emplaced] = ctx->textures.try_emplace(
    handle, ctx, handle, desc
  );
  NTF_ASSERT(emplaced);

  return &it->second;
}

void r_destroy_texture(r_texture tex) {
  if (!tex) {
    return;
  }

  const auto handle = tex->handle;
  auto* ctx = tex->ctx;
  auto it = ctx->textures.find(handle);
  if (it == ctx->textures.end()) {
    return;
  }

  if (--it->second.refcount > 0) {
    return;
  }

  ctx->platform->destroy_texture(handle);
  ctx->textures.erase(it);
}

static void update_texture_images(r_texture tex, cspan<r_image_data> images) {
  for (const auto& image_in : images) {
    rp_tex_image_data image;
    image.texels = image_in.texels;
    image.format = image_in.format;
    image.alignment = image_in.alignment;
    image.extent = image_in.extent;
    image.offset = image_in.offset;
    image.layer = image_in.layer;
    image.level = image_in.level;
    tex->ctx->platform->update_texture_image(tex->handle, image);
  }
}

const void update_texture_opts(r_texture tex, optional<r_texture_sampler> sampler,
                               optional<r_texture_address> addressing, bool regen_mips) {
  rp_tex_opts opts;
  opts.addressing = addressing.value_or(tex->addressing);
  opts.sampler = sampler.value_or(tex->sampler);
  opts.regen_mips = regen_mips;
  tex->ctx->platform->update_texture_options(tex->handle, opts);

  if (addressing) {
    tex->addressing = *addressing;
  }
  if (sampler) {
    tex->sampler = *sampler;
  }
}

static r_expected<void> check_images_to_upload(r_texture tex, cspan<r_image_data> images) {
  // TODO: Unify the update & create image tests?
  for (size_t i = 0; i < images.size(); ++i) {
    const auto& img = images[i];
    const uvec3 upload_extent = img.offset+img.extent;

    const uint32 texels = img.extent.x*img.extent.y*img.extent.z;

    RET_ERROR_IF(texels == 0,
                 "Invalid texture extent at {}", i);

    RET_ERROR_IF(!img.texels ||
                 img.layer > tex->layers ||
                 img.level > tex->levels,
                 "Invalid image at index {}",
                 i);
    switch (tex->type) {
      case r_texture_type::texture1d: {
        RET_ERROR_IF(upload_extent.x > tex->extent.x,
                     "Invalid image extent at index {}", i);
        break;
      }
      case r_texture_type::cubemap: [[fallthrough]];
      case r_texture_type::texture2d: {
        RET_ERROR_IF(upload_extent.x > tex->extent.x ||
                     upload_extent.y > tex->extent.y,
                     "Invalid image extent at index {}", i);
        break;
      }
      case r_texture_type::texture3d: {
        RET_ERROR_IF(upload_extent.x > tex->extent.x ||
                     upload_extent.y > tex->extent.y ||
                     upload_extent.z > tex->extent.z,
                     "Invalid image extent at index {}", i);
        break;
      }
    }
  }

  return {};
}

r_expected<void> r_texture_upload(r_texture tex, const r_texture_data& data) {
  RET_ERROR_IF(!tex, "Invalid handle");

  try {
    const bool do_address = data.addressing && *data.addressing != tex->addressing;
    const bool do_sampler = data.sampler && *data.sampler != tex->sampler;
    RET_ERROR_IF(!(do_sampler || do_address) && data.images.empty(),
                 "Invalid descriptor");
    if (!data.images.empty()) {
      return check_images_to_upload(tex, data.images)
        .transform([&]() {
          update_texture_images(tex, data.images);
          update_texture_opts(tex, data.sampler, data.addressing, data.gen_mipmaps);
        });
    } else {
      update_texture_opts(tex, data.sampler, data.addressing, data.gen_mipmaps);
    }
  }
  RET_ERROR_CATCH("Failed to update texture");

  return {};
}

void r_texture_upload(unchecked_t, r_texture tex, const r_texture_data& data) {
  NTF_ASSERT(tex);
  update_texture_images(tex, data.images);
  update_texture_opts(tex, data.sampler, data.addressing, data.gen_mipmaps);
}

r_expected<void> r_texture_upload(r_texture tex,
                                  cspan<r_image_data> images, bool gen_mips) {
  RET_ERROR_IF(images.empty(), "No images provided");

  return check_images_to_upload(tex, images)
    .transform([&]() {
      update_texture_images(tex, images);
      if (gen_mips) {
        update_texture_opts(tex, nullopt, nullopt, true);
      }
    });
}

void r_texture_upload(unchecked_t, r_texture tex,
                      cspan<r_image_data> images, bool gen_mips) {
  if (images.empty()) {
    return;
  }

  NTF_ASSERT(tex);
  update_texture_images(tex, images);
  if (gen_mips) {
    update_texture_opts(tex,nullopt, nullopt, true);
  }
}

r_expected<void> r_texture_set_sampler(r_texture tex, r_texture_sampler sampler) {
  RET_ERROR_IF(!tex, "Invalid handle");
  update_texture_opts(tex, sampler, nullopt, false);
  return {};
}

void r_texture_set_sampler(unchecked_t, r_texture tex, r_texture_sampler sampler) {
  NTF_ASSERT(tex);
  update_texture_opts(tex, sampler, nullopt, false);
}

r_expected<void> r_texture_set_addressing(r_texture tex, r_texture_address adressing) {
  RET_ERROR_IF(!tex, "Invalid handle");
  update_texture_opts(tex, nullopt, adressing, false);
  return {};
}

void r_texture_set_addressing(unchecked_t, r_texture tex, r_texture_address addressing) {
  NTF_ASSERT(tex);
  update_texture_opts(tex, nullopt, addressing, false);
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
