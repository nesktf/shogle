#include "./pipeline.hpp"
#include "./context.hpp"

namespace shogle {

shader_t_::shader_t_(context_t ctx_, ctx_shad handle_, const ctx_shad_desc& desc) noexcept:
  ctx_res_node<shader_t_>{ctx_},
  handle{handle_},
  type{desc.type} {}

shader_t_::~shader_t_() noexcept {}

static cstring_view<char> concatenate_sources(ctx_alloc& alloc, span<const std::string_view> srcs) {
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

static ntf::unexpected<render_error> handle_error(ctx_shad_status status, shad_err_str err) {
  switch (status) {
    case CTX_SHAD_STATUS_COMPILATION_FAILED: {
      SHOGLE_LOG(error, "Shader compilation failed: {}", err);
      return ntf::unexpected{render_error{err}};
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
      return expect<ctx_shad_desc>{ntf::in_place, desc.type, src};
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
      SHOGLE_LOG(verbose, "Shader created ({}) [type: {}]",
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
  SHOGLE_LOG(verbose, "Shader destroyed ({}) [type: {}]",
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


uniform_t_::uniform_t_(pipeline_t pip_, ctx_unif handle_,
                       ctx_alloc::string_t<char> name_,
                       attribute_type type_, size_t size_) noexcept :
  pip{pip_}, handle{handle_},
  name{std::move(name_)}, type{type_}, size{size_} {}

pipeline_t_::pipeline_t_(context_t ctx_, ctx_pip handle_,
                         stages_flag stages_, primitive_mode primitive_, polygon_mode poly_mode_,
                         ctx_alloc::uarray_t<attribute_binding>&& layout_,
                         unif_map&& unifs_) noexcept :
  ctx_res_node<pipeline_t_>{ctx_},
  handle{handle_},
  stages{stages_}, primitive{primitive_}, poly_mode{poly_mode_},
  layout{std::move(layout_)}, unifs{std::move(unifs_)} {}

pipeline_t_::~pipeline_t_() noexcept {}

static ctx_pip_desc transform_desc(ctx_alloc& alloc,
                                   weak_ptr<unif_meta_vec> unifs,
                                   const pipeline_desc& desc)
{
  auto parse_stages = [](span<const shader_t> shaders) -> stages_flag {
    stages_flag out = stages_flag::none;
    for (const shader_t shad : shaders) {
      switch (shad->type) {
        case shader_type::vertex: {
          out |= stages_flag::vertex;
          break;
        }
        case shader_type::fragment: {
          out |= stages_flag::fragment;
          break;
        }
        case shader_type::compute: {
          out |= stages_flag::compute;
          break;
        }
        case shader_type::geometry: {
          out |= stages_flag::geometry;
          break;
        }
        case shader_type::tesselation_eval: {
          out |= stages_flag::tesselation_eval;
          break;
        }
        case shader_type::tesselation_control: {
          out |= stages_flag::tesselation_control;
          break;
        }
      }
    }
    return out;
  };
  auto stages = alloc.arena_span<ctx_shad>(desc.stages.size());
  NTF_ASSERT(!stages.empty());
  for (size_t i = 0u; const auto& stage : desc.stages) {
    stages[i] = stage->handle;
    ++i;
  }

  auto layout = alloc.make_uninited_array<attribute_binding>(desc.attributes.size());
  NTF_ASSERT(!layout.empty());
  for (size_t i = 0u; const auto& attrib : desc.attributes) {
    std::construct_at(layout.get()+i, attrib);
    ++i;
  }

  return {
    .layout = std::move(layout),
    .uniforms = unifs,
    .stages = stages,
    .stages_flags = parse_stages(desc.stages),
    .primitive = desc.primitive,
    .poly_mode = desc.poly_mode,
    .poly_width = desc.poly_width,
    .tests = desc.tests,
  };
}

static expect<void> validate_desc(const pipeline_desc&) {
  return {}; // TODO: validation
}

static expect<pipeline_t_::unif_map> make_uniform_map(ctx_alloc& alloc, pipeline_t pip,
                                                      const unif_meta_vec& unifs)
{
  try {
    auto map = alloc.make_string_map<uniform_t_>(unifs.size());
    for (const auto& unif : unifs) {
      auto [_, emp] =
        map.try_emplace(unif.name, pip, unif.handle, unif.name, unif.type, unif.size);
      NTF_ASSERT(emp);
    }
    return map;
  } catch (const std::bad_alloc&) {
    RET_ERROR("Failed to allocate uniform map");
  }
}

static ntf::unexpected<render_error> handle_error(ctx_pip_status status, pip_err_str err) {
  switch (status) {
    case CTX_PIP_STATUS_LINKING_FAILED: {
      SHOGLE_LOG(error, "Pipeline linking failed: {}", err);
      return ntf::unexpected{render_error{err}};
    }
    case CTX_PIP_STATUS_INVALID_HANDLE: {
      RET_ERROR("Invalid texture handle");
    }
    case CTX_PIP_STATUS_OK: NTF_UNREACHABLE();
  }
  NTF_UNREACHABLE();
}

expect<pipeline_t> create_pipeline(context_t ctx, const pipeline_desc& desc) {
  RET_ERROR_IF(!ctx, "Invalid context handle");

  try {
    auto& alloc = ctx->alloc();
    auto unif_query = alloc.make_vector<ctx_unif_meta>();
    return validate_desc(desc)
    .and_then([&]() -> expect<ctx_pip_desc> { return transform_desc(alloc, unif_query, desc); })
    .and_then([&](ctx_pip_desc&& pip_desc) -> expect<pipeline_t> { 
      auto* pip = alloc.allocate_uninited<pipeline_t_>();
      NTF_ASSERT(pip);

      ctx_pip handle = CTX_HANDLE_TOMB;
      pip_err_str err;
      const auto ret = ctx->renderer().create_pipeline(handle, err, pip_desc);
      if (ret != CTX_PIP_STATUS_OK) {
        alloc.deallocate(pip, sizeof(pipeline_t_));
        return handle_error(ret, err);
      }

      auto unifs = make_uniform_map(alloc, pip, unif_query);
      if (!unifs) {
        ctx->renderer().destroy_pipeline(handle);
        alloc.deallocate(pip, sizeof(pipeline_t_));
        return ntf::unexpected{std::move(unifs.error())};
      }

      std::construct_at(pip,
                        ctx, handle, pip_desc.stages_flags, pip_desc.primitive, pip_desc.poly_mode,
                        std::move(pip_desc.layout), std::move(*unifs));
      ctx->insert_node(pip);
      NTF_ASSERT(pip->prev == nullptr);
      SHOGLE_LOG(verbose, "Pipeline created ({})", pip->handle);

      return pip;
    });
  } RET_ERROR_CATCH("Failed to create pipeline");
}

void destroy_pipeline(pipeline_t pip) noexcept {
  if (!pip) {
    return;
  }

  const auto handle = pip->handle;
  auto* ctx = pip->ctx;
  SHOGLE_LOG(verbose, "Pipeline destroyed ({})", pip->handle);

  ctx->remove_node(pip);
  ctx->renderer().destroy_pipeline(handle);
  ctx->alloc().destroy(pip);
}

span<uniform_t> pipeline_get_uniforms(pipeline_t pip) {
  if (!pip) {
    return {};
  }
  try {
    auto& alloc = pip->ctx->alloc();
    auto unifs = alloc.arena_span<uniform_t>(pip->unifs.size());
    for (size_t i = 0u; auto& [_, unif] : pip->unifs) {
      unifs[i] = &unif;
      ++i;
    }
    return unifs;
  } catch(...) {}

  return {};
}

uniform_t pipeline_get_uniform(pipeline_t pip, cstring_view<char> name) {
  if (!pip) {
    return nullptr;
  }
  auto& alloc = pip->ctx->alloc();
  auto str = alloc.make_string(name.size());
  str.append(name);

  auto unif_it = pip->unifs.find(str);
  if (unif_it == pip->unifs.end()) {
    SHOGLE_LOG(warning, "Uniform not found: {}", name);
    return nullptr;
  }

  return &unif_it->second;
}

size_t pipeline_get_uniform_count(pipeline_t pip) {
  if (!pip){
    return 0u;
  }
  return pip->unifs.size();
}

stages_flag pipeline_get_stages(pipeline_t pip) {
  NTF_ASSERT(pip);
  return pip->stages;
}

attribute_type uniform_get_type(uniform_t unif) {
  NTF_ASSERT(unif);
  return unif->type;
}

cstring_view<char> uniform_get_name(uniform_t unif) {
  NTF_ASSERT(unif);
  return unif->name;
}

u32 uniform_get_location(uniform_t unif) {
  NTF_ASSERT(unif);
  return static_cast<u32>(unif->handle);
}

context_t r_pipeline_get_ctx(pipeline_t pip) {
  NTF_ASSERT(pip);
  return pip->ctx;
}

ctx_handle pipeline_get_id(pipeline_t pip) {
  NTF_ASSERT(pip);
  return pip->handle;
}

} // namespace shogle
