#include "./internal/common.hpp"

namespace ntf {

r_shader_::r_shader_(r_context ctx_, r_platform_shader handle_, const rp_shad_desc& desc) noexcept:
  rp_res_node<r_shader_>{ctx_},
  handle{handle_},
  type{desc.type} {}

r_shader_::~r_shader_() noexcept {}

static std::string_view concatenate_sources(rp_alloc& alloc, cspan<std::string_view> srcs) {
  NTF_ASSERT(!srcs.empty());

  size_t char_count = 1u; // Null terminated
  for (const auto& src : srcs) {
    char_count += src.size();
  }
  if (char_count == 1u) {
    return {};
  }

  char* string_data = alloc.arena_allocate_uninited<char>(char_count);
  if (!string_data) {
    return {};
  }

  size_t char_pos = 0u;
  for (const auto& src : srcs) {
    // We dont' want the null terminator if its there
    const size_t copy_n = (src[src.size()-1] == '\0') ? src.size()-1 : src.size();
    std::strncpy(string_data+char_pos, src.data(), copy_n);
    char_pos += copy_n;
  }
  NTF_ASSERT(char_pos <= char_count-1);
  string_data[char_pos] = '\0';

  return {string_data, char_count};
}

static r_expected<void> validate_desc(const r_shader_descriptor& desc) {
  // TODO: Validate the actual source using reflection or something?
  RET_ERROR_IF(desc.source.empty(), "No shader sources provided");
  for (size_t i = 0u; const auto& src : desc.source) {
    RET_ERROR_IF(src.empty(), "Empty shader source at index {}", i);
    ++i;
  }

  return {};
}

r_expected<r_shader> r_create_shader(r_context ctx, const r_shader_descriptor& desc) {
  RET_ERROR_IF(!ctx, "Invalid context handle");
  auto& alloc = ctx->alloc();
  return validate_desc(desc)
    .and_then([&]() -> r_expected<rp_shad_desc> {
      auto src = concatenate_sources(alloc, desc.source);
      RET_ERROR_IF(src.empty(), "Failed to concatenate shader sources");
      return rp_shad_desc{.type = desc.type, .source = src};
    })
    .and_then([&](rp_shad_desc&& shad_desc) -> r_expected<r_shader> {
      r_platform_shader handle;
      try {
        handle = ctx->renderer().create_shader(shad_desc);
        RET_ERROR_IF(!handle, "Failed to create shader");
      }
      RET_ERROR_CATCH("Failed to create shader");

      auto* shad = alloc.allocate_uninited<r_shader_>(1u);
      if (!shad) {
        ctx->renderer().destroy_shader(handle);
        RET_ERROR("Failed to allocate shader");
      }
      std::construct_at(shad,
                        ctx, handle, shad_desc);
      ctx->insert_node(shad);
      NTF_ASSERT(shad->prev == nullptr);
      return shad;
    });
}

r_shader r_create_shader(unchecked_t, r_context ctx, const r_shader_descriptor& desc) {
  if (!ctx) {
    RENDER_ERROR_LOG("Invalid ctx");
    return nullptr;
  }
  auto& alloc = ctx->alloc();

  const rp_shad_desc shad_desc {
    .type = desc.type,
    .source = concatenate_sources(alloc, desc.source)
  };
  auto handle = ctx->renderer().create_shader(shad_desc);
  if (!handle) {
    RENDER_ERROR_LOG("Failed to create shader");
    return nullptr;
  }

  auto* shad = alloc.allocate_uninited<r_shader_>(1u);
  if (!shad) {
    RENDER_ERROR_LOG("Failed to allocate shader");
    ctx->renderer().destroy_shader(handle);
    return nullptr;
  }
  std::construct_at(shad,
                    ctx, handle, shad_desc);
  ctx->insert_node(shad);
  NTF_ASSERT(shad->prev == nullptr);

  return shad;
}

void r_destroy_shader(r_shader shader) {
  if (!shader) {
    return;
  }
  const auto handle = shader->handle;
  auto* ctx = shader->ctx;

  ctx->remove_node(shader);
  ctx->renderer().destroy_shader(handle);
  ctx->alloc().destroy(shader);
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
