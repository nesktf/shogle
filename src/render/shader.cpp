#include "./internal/platform.hpp"

namespace ntf::render {

shader_t_::shader_t_(context_t ctx_, ctx_shad handle_, const ctx_shad_desc& desc) noexcept:
  ctx_res_node<shader_t_>{ctx_},
  handle{handle_},
  type{desc.type} {}

shader_t_::~shader_t_() noexcept {}

static cstring_view<char> concatenate_sources(ctx_alloc& alloc, cspan<std::string_view> srcs) {
  NTF_ASSERT(!srcs.empty());

  size_t char_count = 1u; // Null terminated
  for (const auto& src : srcs) {
    char_count += src.size();
  }
  if (char_count == 1u) {
    return {};
  }

  char* string_data = alloc.arena_allocate_uninited<char>(char_count);
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

static const char* shader_type_str(shader_type type) {
  switch (type) {
    case shader_type::vertex: return "VERTEX";
    case shader_type::fragment: return "FRAGMENT";
    case shader_type::geometry: return "GEOMETRY";
    case shader_type::tesselation_control: return "TESS_CONT";
    case shader_type::tesselation_eval: return "TESS_EVAL";
    case shader_type::compute: return "COMPUTE";
  }
  NTF_UNREACHABLE();
}

static expect<void> validate_desc(ctx_alloc& alloc, const shader_desc& desc) {
  // TODO: Validate the actual source using reflection or something?
  RET_ERROR_IF(desc.source.empty(), "No shader sources provided");
  for (size_t i = 0u; const auto& src : desc.source) {
    RET_ERROR_FMT_IF(src.empty(), alloc, "Empty shader source at index {}", i);
    ++i;
  }
  return {};
}

static unexpected<render_error> handle_error(ctx_shad_status status, shad_err_str err) {
  switch (status) {
    case CTX_SHAD_STATUS_COMPILATION_FAILED: {
      RENDER_ERROR_LOG("Shader compilation failed: {}", err);
      return unexpected{render_error{err}};
    }
    case CTX_SHAD_STATUS_INVALID_HANDLE: {
      RET_ERROR("Invalid texture handle");
    }
    case CTX_SHAD_STATUS_OK: NTF_UNREACHABLE();
  }
  NTF_UNREACHABLE();
}

expect<shader_t> create_shader(context_t ctx, const shader_desc& desc) {
  RET_ERROR_IF(!ctx, "Invalid context handle");
  try {
    auto& alloc = ctx->alloc();
    return validate_desc(alloc, desc)
    .and_then([&]() -> expect<ctx_shad_desc> {
      auto src = concatenate_sources(alloc, desc.source);
      RET_ERROR_IF(src.empty(), "Failed to concatenate shader sources");
      return expect<ctx_shad_desc>{in_place, desc.type, src};
    })
    .and_then([&](ctx_shad_desc&& shad_desc) -> expect<shader_t> {
      auto* shad = alloc.allocate_uninited<shader_t_>();
      NTF_ASSERT(shad);

      ctx_shad handle = CTX_HANDLE_TOMB;
      shad_err_str err;
      const auto ret = ctx->renderer().create_shader(handle, err, shad_desc);
      if (ret != CTX_SHAD_STATUS_OK) {
        alloc.deallocate(shad, sizeof(shader_t_));
        return handle_error(ret, err);
      }

      NTF_ASSERT(check_handle(handle));
      std::construct_at(shad,
                        ctx, handle, shad_desc);
      ctx->insert_node(shad);
      NTF_ASSERT(shad->prev == nullptr);
      RENDER_DBG_LOG("Shader created ({}) [type: {}]",
                     shad->handle, shader_type_str(shad_desc.type));

      return shad;
    });
  } RET_ERROR_CATCH("Failed to create shader");
}

void destroy_shader(shader_t shader) noexcept {
  if (!shader) {
    return;
  }
  const auto handle = shader->handle;
  auto* ctx = shader->ctx;
  RENDER_DBG_LOG("Shader destroyed ({}) [type: {}]",
                 shader->handle, shader_type_str(shader->type));

  ctx->remove_node(shader);
  ctx->renderer().destroy_shader(handle);
  ctx->alloc().destroy(shader);
}

shader_type shader_get_type(shader_t shader) {
  NTF_ASSERT(shader);
  return shader->type;
}

context_t shader_get_ctx(shader_t shader) {
  NTF_ASSERT(shader);
  return shader->ctx;
}

ctx_handle shader_get_id(shader_t shader) {
  NTF_ASSERT(shader);
  return shader->handle;
}

} // namespace ntf::render
