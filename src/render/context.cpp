#include "./opengl.hpp"

#if SHOGLE_USE_GLFW
#include <imgui_impl_glfw.h>
#endif

#include <imgui_impl_opengl3.h> // Should work fine with OGL 4.x

namespace ntf {

void r_context::init(r_window& win, const r_context_params& params) noexcept {
  r_api api = params.use_api.value_or(r_api::opengl);
  r_window::ctx_params_t win_params {
    .api = api,
    .gl_maj = 4,
    .gl_min = 6,
  };
  if (!win.init_context(win_params)) {
    SHOGLE_LOG(error, "[ntf::r_context] Failed to initialize window context");
    return;
  }

  try {
    switch (api) {
      case r_api::opengl: {
        _ctx = std::make_unique<gl_context>(win, 4, 6);
        break;
      }
      default: {
        NTF_ASSERT(false, "Not implemented");
        break;
      }
    }
  } catch (const std::exception& err) {
    win.reset();
    SHOGLE_LOG(error, "[ntf::r_context] Failed to create context: {}", err.what());
    return;
  }

#if SHOGLE_ENABLE_IMGUI
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO();
  // io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
  ImGui::StyleColorsDark();

#if SHOGLE_USE_GLFW
  switch (api) {
    case r_api::opengl: {
      ImGui_ImplGlfw_InitForOpenGL(win._handle, true);
      ImGui_ImplOpenGL3_Init("#version 130");
      break;
    }
    case r_api::vulkan: {
      ImGui_ImplGlfw_InitForVulkan(win._handle, true);
      break;
    }
    default: {
      ImGui_ImplGlfw_InitForOther(win._handle, true);
      break;
    }
  }
#endif
#endif
  auto [def_list, cmd_emplaced] = _draw_lists.try_emplace(DEFAULT_FRAMEBUFFER);
  NTF_ASSERT(cmd_emplaced);

  _frame_arena.init(1ull<<29); // 512MiB
  _win = &win;
  _ctx_api = api;
  _d_list = def_list->second;
  def_list->second.viewport = uvec4{0, 0, _win->fb_size()};
}

r_context::~r_context() noexcept {
  if (!_ctx) {
    return;
  }
  _ctx.reset();

#if SHOGLE_ENABLE_IMGUI
  switch (_ctx_api) {
    case r_api::opengl: {
      ImGui_ImplOpenGL3_Shutdown();
      break;
    }
    default: break;
  }
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();
#endif

  _win->reset();
}

void r_context::start_frame() noexcept {
  for (auto& [_, list] : _draw_lists) {
    list.clear = r_clear_flag::none;
    for (auto& cmd : list.cmds) {
      cmd.get().~draw_command_t();
    }
    list.cmds.clear();
  }
  _frame_arena.reset();

#if SHOGLE_ENABLE_IMGUI
#if SHOGLE_USE_GLFW
  ImGui_ImplGlfw_NewFrame();
#endif
  switch (_ctx_api) {
    case r_api::opengl: ImGui_ImplOpenGL3_NewFrame(); break;
    // case r_api::vulkan: ImGui_ImplVulkan_NewFrame(); break;
    default: break;
  }
  ImGui::NewFrame();
#endif
}

void r_context::end_frame() noexcept {
  for (auto& [fbo, list] : _draw_lists) {
    _ctx->submit(fbo, list);
  }
  ImGui::Render();
  if (_ctx_api == r_api::opengl) {
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    _win->swap_buffers();
  }
}

void r_context::device_wait() noexcept {
  _ctx->device_wait();
}

r_buffer_handle r_context::create_buffer(const r_buffer_descriptor& desc) {
  auto validate_descriptor = [&](buffer_create_t& data) -> bool {
    data.size = desc.size;
    data.type = desc.type;
    data.data = desc.data;
    return true;
  };

  buffer_create_t create_data;
  if (!validate_descriptor(create_data)) {
    return r_buffer_handle{};
  }

  auto handle = _ctx->create_buffer(create_data);
  NTF_ASSERT(handle);
  NTF_ASSERT(_buffers.find(handle) == _buffers.end());

  auto [it, emplaced] = _buffers.try_emplace(handle);
  NTF_ASSERT(emplaced);

  auto& stored = it->second;
  stored.type = create_data.type;
  stored.size = create_data.size;
  return handle;
}

void r_context::destroy(r_buffer_handle buff) {
  NTF_ASSERT(buff);
  auto it = _buffers.find(buff);
  NTF_ASSERT(it != _buffers.end());

  _ctx->destroy_buffer(buff);
  _buffers.erase(it);
}

void r_context::update(r_buffer_handle buff,
                       const void* data, size_t size, size_t offset) {
  NTF_ASSERT(buff);
  NTF_ASSERT(_buffers.find(buff) != _buffers.end());

  NTF_ASSERT(size+offset <= _buffers.at(buff).size);
  NTF_ASSERT(data, "Invalid buffer data");
  _ctx->update_buffer(buff, {
    .data = data,
    .size = size,
    .offset = offset,
  });
}

r_buffer_type r_context::query(r_buffer_handle buff, r_query_type_t) const {
  NTF_ASSERT(buff);
  NTF_ASSERT(_buffers.find(buff) != _buffers.end());
  return _buffers.at(buff).type;
}

size_t r_context::query(r_buffer_handle buff, r_query_size_t) const {
  NTF_ASSERT(buff);
  NTF_ASSERT(_buffers.find(buff) != _buffers.end());
  return _buffers.at(buff).size;
}

static const char* textypetostr(r_texture_type type) {
  switch (type) {
    case r_texture_type::texture1d: return "TEX1D";
    case r_texture_type::texture2d: return "TEX2D";
    case r_texture_type::texture3d: return "TEX3D";
    case r_texture_type::cubemap:   return "CUBEMAP";
  }

  NTF_UNREACHABLE();
}

r_texture_handle r_context::create_texture(const r_texture_descriptor& desc) {
  auto validate_descriptor = [&](tex_create_t& data) -> bool {
    data.type = desc.type;
    data.format = desc.format;
    // data.texels = desc.texels;

    // TODO: Get max texture size from the platform context
    // data.extent = glm::clamp(desc.extent, glm::uvec3{1, 1, 1}, glm::uvec3{4096, 4096, 4096});
    data.extent = desc.extent;
    if (desc.type == r_texture_type::cubemap && data.extent.x != data.extent.y) {
      SHOGLE_LOG(warning, "[r_context::create_texture] "
                 "Ignoring non square cubemap size, clamping to greater dim");
      data.extent.x = glm::max(data.extent.x, data.extent.y);
      data.extent.y = data.extent.x;
    }

    // TODO: Check max layer count
    data.layers = desc.layers;
    if (desc.type == r_texture_type::texture3d && desc.layers > 1) {
      SHOGLE_LOG(warning, "[r_context::create_texture] "
                 "Ignoring multiple layers for texture3d");   
      data.layers = 1;
    }
    if (desc.type == r_texture_type::cubemap && desc.layers) {
      SHOGLE_LOG(warning, "[r_context::create_texture] "
                 "Ignoring invalid layers for cubemap");
      data.layers = 6;
    }

    // data.gen_mipmaps = desc.gen_mipmaps;
    // if (desc.gen_mipmaps && desc.levels == 1) {
    //   SHOGLE_LOG(warning, "[r_context::create_texture] "
    //              "Ignoring mipmaps generation for texture with level 1");
    //   data.gen_mipmaps = false;
    // }
    // if (desc.gen_mipmaps && !data.texels) {
    //   SHOGLE_LOG(warning, "[r_context::create_texture] "
    //              "Ignoring mipmaps generation for texture with no texel data");
    //   data.gen_mipmaps = false;
    // }

    data.levels = glm::clamp(desc.levels, uint32{0}, uint32{7});
    if (desc.levels > 7) {
      SHOGLE_LOG(warning, "[r_context::create_texture] "
                 "Ignoring texture level greater than 7, clamping to 7");
    }
    if (desc.levels == 0) {
      SHOGLE_LOG(warning, "[r_context::create_texture] "
                 "Ignoring empty texture level, clamping to 1");
    }

    data.sampler = desc.sampler;
    data.addressing = desc.addressing;

    return true;
  };

  tex_create_t create_data;
  if (!validate_descriptor(create_data)) {
    return r_texture_handle{};
  }

  auto handle = _ctx->create_texture(create_data);
  NTF_ASSERT(handle);
  NTF_ASSERT(_textures.find(handle) == _textures.end());

  auto [it, emplaced] = _textures.try_emplace(handle);
  NTF_ASSERT(emplaced);

  auto& stored = it->second;
  stored.refcount.store(1);
  stored.type = create_data.type;
  stored.format = create_data.format;
  stored.extent = create_data.extent;
  stored.layers = create_data.layers;
  stored.levels = create_data.levels;
  stored.addressing = create_data.addressing;
  stored.sampler = create_data.sampler;

  SHOGLE_LOG(verbose,
             "[ntf::r_context] TEXTURE CREATE - ID {} - EXT {}x{}x{} - TYPE {}",
             handle.value(),
             stored.extent.x, stored.extent.y, stored.extent.z,
             textypetostr(stored.type));

  return handle;
}

void r_context::destroy(r_texture_handle tex) {
  NTF_ASSERT(tex);
  auto it = _textures.find(tex);
  NTF_ASSERT(it != _textures.end());
  if (--it->second.refcount > 0) {
    return;
  }
  _ctx->destroy_texture(tex);
  auto& data = it->second;
  SHOGLE_LOG(verbose,
             "[ntf::r_context] TEXTURE DESTROY - ID {} - EXT {}x{}x{} - TYPE {}",
             tex.value(),
             data.extent.x, data.extent.y, data.extent.z,
             textypetostr(data.type));
  _textures.erase(it);
}

void r_context::update(r_texture_handle tex, r_texture_sampler sampler) {
  NTF_ASSERT(tex);
  NTF_ASSERT(_textures.find(tex) != _textures.end());
  auto& texture = _textures.at(tex);
  if (texture.sampler == sampler) {
    return;
  } 
  _ctx->update_texture(tex, {
    .sampler = sampler,
  });
  texture.sampler = sampler;
}

void r_context::update(r_texture_handle tex, r_texture_address addressing) {
  NTF_ASSERT(tex);
  NTF_ASSERT(_textures.find(tex) != _textures.end());
  auto& texture = _textures.at(tex);
  if (texture.addressing == addressing) {
    return;
  }
  _ctx->update_texture(tex, {
    .addressing = addressing,
  });
  texture.addressing = addressing;
}

void r_context::update(r_texture_handle tex,
                       const void* texels, r_texture_format format, uvec3 offset,
                       uint32 layer, uint32 level, bool genmips) {
  NTF_ASSERT(tex);
  NTF_ASSERT(_textures.find(tex) != _textures.end());

  auto& texture = _textures.at(tex);
  NTF_ASSERT(texels, "Invalid texture data");
  NTF_ASSERT(level <= texture.levels, "Invalid texture level");
  NTF_ASSERT(layer <= texture.layers, "Invalid texture layer");
  NTF_ASSERT([&]() {
    switch (texture.type) {
      case r_texture_type::texture1d: {
        return offset.x < texture.extent.x;
        break;
      }
      case r_texture_type::cubemap: [[fallthrough]];
      case r_texture_type::texture2d: {
        return
          offset.x < texture.extent.x &&
          offset.y < texture.extent.y;
        break;
      }
      case r_texture_type::texture3d: {
        return 
          offset.x < texture.extent.x &&
          offset.y < texture.extent.y &&
          offset.z < texture.extent.z;
        break;
      }
    }
    return true;
  }(), "Invalid texture offset");

  _ctx->update_texture(tex, {
    .format = format,
    .texels = texels,
    .offset = offset,
    .layer = layer,
    .level = level,
    .genmips = texture.levels > 1 ? genmips : false,
  });
  auto& data = _textures.find(tex)->second;
  SHOGLE_LOG(verbose,
             "[ntf::r_context] TEXTURE UPLOAD - ID {} - EXT {}x{}x{} - TYPE {}",
             tex.value(),
             data.extent.x, data.extent.y, data.extent.z,
             textypetostr(data.type));
}

r_texture_type r_context::query(r_texture_handle tex, r_query_type_t) const {
  NTF_ASSERT(tex);
  NTF_ASSERT(_textures.find(tex) != _textures.end());
  return _textures.at(tex).type;
}

r_texture_format r_context::query(r_texture_handle tex, r_query_format_t) const {
  NTF_ASSERT(tex);
  NTF_ASSERT(_textures.find(tex) != _textures.end());
  return _textures.at(tex).format;
}

r_texture_sampler r_context::query(r_texture_handle tex, r_query_sampler_t) const {
  NTF_ASSERT(tex);
  NTF_ASSERT(_textures.find(tex) != _textures.end());
  return _textures.at(tex).sampler;
}

r_texture_address r_context::query(r_texture_handle tex, r_query_addressing_t) const {
  NTF_ASSERT(tex);
  NTF_ASSERT(_textures.find(tex) != _textures.end());
  return _textures.at(tex).addressing;
}

uvec3 r_context::query(r_texture_handle tex, r_query_extent_t) const {
  NTF_ASSERT(tex);
  NTF_ASSERT(_textures.find(tex) != _textures.end());
  return _textures.at(tex).extent;
}

uint32 r_context::query(r_texture_handle tex, r_query_layers_t) const {
  NTF_ASSERT(tex);
  NTF_ASSERT(_textures.find(tex) != _textures.end());
  return _textures.at(tex).layers;
}

uint32 r_context::query(r_texture_handle tex, r_query_levels_t) const {
  NTF_ASSERT(tex);
  NTF_ASSERT(_textures.find(tex) != _textures.end());
  return _textures.at(tex).levels;
}

r_framebuffer_handle r_context::create_framebuffer(const r_framebuffer_descriptor& desc) {
  auto validate_descriptor = [&](fb_create_t& data) -> bool {
    data.extent.x = desc.extent.x;
    data.extent.y = desc.extent.y;

    data.attachments = desc.attachments;
    data.attachment_count = desc.attachment_count;
    data.buffer_format = desc.test_buffer_format;
    data.buffers = desc.test_buffers;
    data.color_buffer_format = desc.color_buffer_format;
    data.clear_color = desc.clear_color;

    return true;
  };

  fb_create_t create_data;
  if (!validate_descriptor(create_data)) {
    return r_framebuffer_handle{};
  }

  auto handle = _ctx->create_framebuffer(create_data);
  NTF_ASSERT(handle);
  NTF_ASSERT(_framebuffers.find(handle) == _framebuffers.end());

  auto [it, emplaced] = _framebuffers.try_emplace(handle);
  NTF_ASSERT(emplaced);

  auto [_, cmd_emplaced] = _draw_lists.try_emplace(handle);
  NTF_ASSERT(cmd_emplaced);

  auto& stored = it->second;
  stored.attachments.reserve(create_data.attachment_count);
  for (uint32 i = 0; i < create_data.attachment_count; ++i) {
    stored.attachments.push_back(create_data.attachments[i]);
    auto& tex = _textures.at(create_data.attachments[i].handle);
    tex.refcount++;
  }
  stored.viewport.x = 0;
  stored.viewport.y = 0;
  stored.viewport.z = create_data.extent.x;
  stored.viewport.w = create_data.extent.y;
  stored.color = create_data.clear_color;

  return handle;
}

void r_context::destroy(r_framebuffer_handle fbo) {
  NTF_ASSERT(fbo);
  auto it = _framebuffers.find(fbo);
  NTF_ASSERT(it != _framebuffers.end());
  _ctx->destroy_framebuffer(fbo);
  for (auto att : it->second.attachments) {
    destroy(att.handle);
  }
  _framebuffers.erase(it);
}

r_shader_handle r_context::create_shader(const r_shader_descriptor& desc) {
  shader_create_t create;
  create.type = desc.type;
  create.src = desc.source;

  auto handle = _ctx->create_shader(create);
  NTF_ASSERT(handle);
  NTF_ASSERT(_shaders.find(handle) == _shaders.end());

  auto [it, emplaced] = _shaders.try_emplace(handle);
  NTF_ASSERT(emplaced);

  auto& data = it->second;
  data.type = desc.type;

  return handle;
}

void r_context::destroy(r_shader_handle shader) {
  NTF_ASSERT(shader);
  auto it = _shaders.find(shader);
  NTF_ASSERT(it != _shaders.end());
  _ctx->destroy_shader(shader);
  _shaders.erase(it);
}

r_shader_type r_context::query(r_shader_handle shader, r_query_type_t) const {
  NTF_ASSERT(shader);
  NTF_ASSERT(_shaders.find(shader) != _shaders.end());
  return _shaders.at(shader).type;
}

r_pipeline_handle r_context::create_pipeline(const r_pipeline_descriptor& desc) {
  auto validate_descriptor = [&](pipeline_create_t& data) -> bool {
    data.shaders = desc.stages;
    data.shader_count = desc.stage_count;

    data.primitive = desc.primitive;
    data.poly_mode = desc.poly_mode;
    data.front_face = desc.front_face;
    data.cull_mode = desc.cull_mode;
    data.tests = desc.tests;
    data.depth_ops = desc.depth_compare_op;
    data.stencil_ops = desc.stencil_compare_op;
    return true;
  };

  pipeline_create_t create_data;
  if (!validate_descriptor(create_data)) {
    return r_pipeline_handle{};
  }

  auto layout = std::make_unique<vertex_attrib_t>();
  layout->binding = desc.attrib_binding.binding;
  layout->stride = desc.attrib_binding.stride;
  layout->descriptors.resize(desc.attrib_desc_count);
  std::memcpy(
    layout->descriptors.data(), desc.attrib_desc,
    desc.attrib_desc_count*sizeof(r_attrib_descriptor)
  );

  create_data.layout = layout.get();

  auto handle = _ctx->create_pipeline(create_data);
  NTF_ASSERT(handle);
  NTF_ASSERT(_pipelines.find(handle) == _pipelines.end());

  auto [it, emplaced] = _pipelines.try_emplace(handle);
  NTF_ASSERT(emplaced);

  auto& stored = it->second;
  // stored.stages = create_data.
  stored.layout = std::move(layout);
  stored.primitive = create_data.primitive;
  stored.poly_mode = create_data.poly_mode;
  stored.front_face = create_data.front_face;
  stored.cull_mode = create_data.cull_mode;
  stored.tests = create_data.tests;
  stored.depth_ops = create_data.depth_ops;
  stored.stencil_ops = create_data.stencil_ops;

  return handle;
}

void r_context::destroy(r_pipeline_handle pipeline) {
  NTF_ASSERT(pipeline);
  auto it = _pipelines.find(pipeline);
  NTF_ASSERT(it != _pipelines.end());
  _ctx->destroy_pipeline(pipeline);
  _pipelines.erase(it);
}

r_stages_flag r_context::query(r_pipeline_handle pipeline, r_query_stages_t) const {
  NTF_ASSERT(pipeline);
  NTF_ASSERT(_pipelines.find(pipeline) != _pipelines.end());
  return _pipelines.at(pipeline).stages;
}

r_uniform r_context::query(r_pipeline_handle pipeline, r_query_uniform_t,
                           std::string_view name) const {
  NTF_ASSERT(pipeline);
  NTF_ASSERT(_pipelines.find(pipeline) != _pipelines.end());
  return _ctx->pipeline_uniform(pipeline, name);
}

} // namespace ntf
