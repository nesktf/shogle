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
  _win = &win;
  _ctx_api = api;
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
  _uniforms.clear();
  for (auto& [_, vec] : _cmds) {
    vec.clear();
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
  for (auto& [_, vec] : _cmds) {
    _ctx->submit(vec);
  }
}

void r_context::device_wait() noexcept {
  _ctx->device_wait();
}

r_buffer_handle r_context::create_buffer(const r_buffer_descriptor& desc) {
  buffer_create_t create;
  create.size = desc.size;
  create.type = desc.type;
  create.data = desc.data;

  auto handle = _ctx->create_buffer(create);
  NTF_ASSERT(handle);
  NTF_ASSERT(_buffers.find(handle) == _buffers.end());

  buff_store_t data;
  data.size = desc.size;
  data.type = desc.type;
  _buffers.insert(std::make_pair(handle, data));
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

  buffer_update_t upd;
  upd.size = size;
  upd.data = data;
  upd.offset = offset;
  _ctx->update_buffer(buff, upd);
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

r_texture_handle r_context::create_texture(const r_texture_descriptor& desc) {
  auto validate_descriptor = [&](tex_create_t& data) -> bool {
    data.type = desc.type;
    data.format = desc.format;
    data.texels = desc.texels;

    // TODO: Get max texture size from the platform context
    data.extent = glm::clamp(desc.extent, glm::uvec3{1, 1, 1}, glm::uvec3{4096, 4096, 4096});
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

    data.gen_mipmaps = desc.gen_mipmaps;
    if (desc.gen_mipmaps && desc.levels == 1) {
      SHOGLE_LOG(warning, "[r_context::create_texture] "
                 "Ignoring mipmaps generation for texture with level 1");
      data.gen_mipmaps = false;
    }
    if (desc.gen_mipmaps && !data.texels) {
      SHOGLE_LOG(warning, "[r_context::create_texture] "
                 "Ignoring mipmaps generation for texture with no texel data");
      data.gen_mipmaps = false;
    }

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
  stored.addressing = create_data.addressing;
  stored.sampler = create_data.sampler;

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
    .genmips = texture.levels == 1 ? genmips : false,
  });
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
  fb_create_t create;
  create.w = desc.size.x;
  create.h = desc.size.y;
  create.buffers = desc.buffers;
  create.attachments = desc.attachments;
  create.attachment_levels = desc.attachment_levels;
  create.attachment_count = desc.attachment_count;
  create.format = r_texture_format::rgb;

  auto handle = _ctx->create_framebuffer(create);
  NTF_ASSERT(handle);
  NTF_ASSERT(_framebuffers.find(handle) == _framebuffers.end());

  auto [it, emplaced] = _framebuffers.try_emplace(handle);
  NTF_ASSERT(emplaced);

  auto& data = it->second;
  data.attachments.reserve(desc.attachment_count);
  for (uint32 i = 0; i < desc.attachment_count; ++i) {
    data.attachments.push_back(desc.attachments[i]);
  }

  return handle;
}

void r_context::destroy(r_framebuffer_handle fbo) {
  NTF_ASSERT(fbo);
  auto it = _framebuffers.find(fbo);
  NTF_ASSERT(it != _framebuffers.end());
  _ctx->destroy_framebuffer(fbo);
  for (auto att : it->second.attachments) {
    destroy(att);
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
}

void r_context::destroy(r_pipeline_handle pipeline) {
}

r_shader_type r_context::query(r_pipeline_handle pipeline, r_query_stages_t) const {
}

r_uniform r_context::query(r_pipeline_handle pipeline, std::string_view name,
                           r_query_uniform_t) const {
}

} // namespace ntf
