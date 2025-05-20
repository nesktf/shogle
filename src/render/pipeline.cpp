#include "./internal/common.hpp"

namespace ntf {

r_uniform_::r_uniform_(r_pipeline pip, r_platform_uniform location_,
                       std::string name_, r_attrib_type type_, size_t size_) noexcept :
  pipeline{pip}, location{location_},
  name{std::move(name_)}, type{type_}, size{size_} {}

r_pipeline_::r_pipeline_(r_context ctx_, r_platform_pipeline handle_,
                         r_stages_flag stages_, r_primitive primitive_, r_polygon_mode poly_mode_,
                         rp_alloc::uptr_t<vertex_layout>&& layout_,
                         uniform_map&& uniforms_) noexcept :
  rp_res_node<r_pipeline_>{ctx_},
  handle{handle_},
  stages{stages_}, primitive{primitive_}, poly_mode{poly_mode_},
  layout{std::move(layout_)}, uniforms{std::move(uniforms_)} {}

r_pipeline_::~r_pipeline_() noexcept {}

static r_stages_flag parse_stages(cspan<r_shader> shaders) {
  r_stages_flag out = r_stages_flag::none;
  for (r_shader shader : shaders) {
    switch (shader->type) {
      case r_shader_type::vertex: {
        out |= r_stages_flag::vertex;
        break;
      }
      case r_shader_type::fragment: {
        out |= r_stages_flag::fragment;
        break;
      }
      case r_shader_type::compute: {
        out |= r_stages_flag::compute;
        break;
      }
      case r_shader_type::geometry: {
        out |= r_stages_flag::geometry;
        break;
      }
      case r_shader_type::tesselation_eval: {
        out |= r_stages_flag::tesselation_eval;
        break;
      }
      case r_shader_type::tesselation_control: {
        out |= r_stages_flag::tesselation_control;
        break;
      }
    }
  }
  return out;
}

static r_expected<rp_pip_desc> transform_desc(rp_alloc& alloc,
                                              weak_ref<rp_uniform_query_vec> unifs,
                                              const r_pipeline_descriptor& desc) {
  auto stages = alloc.arena_span<r_platform_shader>(desc.stages.size());
  RET_ERROR_IF(!stages, "Failed to allocate stage handles");
  for (size_t i = 0u; const auto& stage : desc.stages) {
    stages[i] = stage->handle;
    ++i;
  }

  auto descs = alloc.make_uninited_array<r_attrib_descriptor>(desc.attribs.size());
  RET_ERROR_IF(!descs, "Failed to allocate layout descriptors");
  for (size_t i = 0u; const auto& attrib : desc.attribs) {
    std::construct_at(descs.get()+i, attrib);
    ++i;
  }

  auto layout = alloc.make_unique<vertex_layout>(desc.attrib_binding, desc.attrib_stride,
                                                 std::move(descs));
  RET_ERROR_IF(!layout, "Failed to allocate vertex layout");

  return rp_pip_desc{
    .layout = std::move(layout),
    .uniforms = unifs,
    .stages = stages,
    .stages_flags = parse_stages(desc.stages),
    .primitive = desc.primitive,
    .poly_mode = desc.poly_mode,
    .poly_width = desc.poly_width,
    .stencil_test = desc.stencil_test,
    .depth_test = desc.depth_test,
    .scissor_test = desc.scissor_test,
    .face_culling = desc.face_culling,
    .blending = desc.blending,
  };
}

static r_expected<r_pipeline_::uniform_map> copy_uniforms(rp_alloc& alloc, r_pipeline pip,
                                                          const rp_uniform_query_vec& unifs)
{
  return r_pipeline_::uniform_map::from_size(unifs.size(), alloc.make_adaptor<uint8>())
  .transform([&](r_pipeline_::uniform_map&& map) -> r_pipeline_::uniform_map {
    for (const auto& unif : unifs) {
      map.try_emplace(unif.name, pip, unif.location, unif.name, unif.type, unif.size);
    }
    return map;
  });
}

r_expected<void> validate_desc(const r_pipeline_descriptor&) {
  return {}; // TODO: validation
}

r_expected<r_pipeline> r_create_pipeline(r_context ctx, const r_pipeline_descriptor& desc) {
  RET_ERROR_IF(!ctx, "Invalid context handle");

  auto& alloc = ctx->alloc();
  auto unif_query = alloc.make_vector<rp_uniform_query>();

  try {
    return validate_desc(desc)
    .and_then([&]() -> r_expected<rp_pip_desc> { return transform_desc(alloc, unif_query, desc); })
    .and_then([&](rp_pip_desc&& pip_desc) -> r_expected<r_pipeline> { 
      r_platform_pipeline handle = ctx->renderer().create_pipeline(pip_desc);
      RET_ERROR_IF(!handle, "Failed to create pipeline");

      auto* pip = alloc.allocate_uninited<r_pipeline_>(1u);
      if (!pip) {
        ctx->renderer().destroy_pipeline(handle);
        RET_ERROR("Failed to allocate pipeline");
      }

      auto unifs = copy_uniforms(alloc, pip, unif_query);
      if (!unifs) {
        alloc.deallocate(pip, sizeof(r_pipeline_));
        ctx->renderer().destroy_pipeline(handle);
        RET_ERROR("Failed to allocate uniforms, {}", unifs.error().what());
      }
      std::construct_at(pip,
                        ctx, handle, pip_desc.stages_flags, pip_desc.primitive, pip_desc.poly_mode,
                        std::move(pip_desc.layout), std::move(*unifs));
      ctx->insert_node(pip);
      NTF_ASSERT(pip->prev == nullptr);
      SHOGLE_LOG(debug, "[ntf::r_create_pipeline] Pipeline created");

      return pip;
    });
  } RET_ERROR_CATCH("Failed to create pipeline");
}

void r_destroy_pipeline(r_pipeline pip) noexcept {
  if (!pip) {
    return;
  }

  const auto handle = pip->handle;
  auto* ctx = pip->ctx;

  ctx->remove_node(pip);
  ctx->renderer().destroy_pipeline(handle);
  ctx->alloc().destroy(pip);
  SHOGLE_LOG(debug, "[ntf::r_destroy_pipeline] Pipeline destroyed");
}

r_stages_flag r_pipeline_get_stages(r_pipeline pip) {
  NTF_ASSERT(pip);
  return pip->stages;
}

optional<r_uniform> r_pipeline_get_uniform(r_pipeline pip, std::string_view name) {
  if (!pip) {
    return nullopt;
  }

  auto unif_it = pip->uniforms.find(name.data());
  if (unif_it == pip->uniforms.end()) {
    return nullopt;
  }

  return &unif_it->second;
}

size_t r_pipeline_get_uniform_count(r_pipeline pip) {
  NTF_ASSERT(pip);
  return pip->uniforms.size();
}

span<r_uniform> r_pipeline_get_all_uniforms(r_pipeline pip) {
  auto& alloc = pip->ctx->alloc();
  auto unifs = alloc.arena_span<r_uniform>(pip->uniforms.size());
  if (!unifs) {
    return {};
  }
  for (size_t i = 0u; auto& [_, unif] : pip->uniforms) {
    unifs[i] = &unif;
    ++i;
  }
  return unifs;
}

r_attrib_type r_uniform_get_type(r_uniform unif) {
  NTF_ASSERT(unif);
  return unif->type;
}

std::string_view r_uniform_get_name(r_uniform unif) {
  NTF_ASSERT(unif);
  return unif->name;
}

r_context r_pipeline_get_ctx(r_pipeline pip) {
  NTF_ASSERT(pip);
  return pip->ctx;
}

r_platform_pipeline r_pipeline_get_handle(r_pipeline pip) {
  NTF_ASSERT(pip);
  return pip->handle;
}

} // namespace ntf
