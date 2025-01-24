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
  _ctx_meta = _ctx->query_meta();
  _d_list = def_list->second;
  def_list->second.viewport = uvec4{0, 0, _win->fb_size()};
}

r_context::~r_context() noexcept {
  if (!_ctx) {
    return;
  }
  _ctx.reset();

#if SHOGLE_ENABLE_IMGUI
  switch (render_api()) {
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
  switch (render_api()) {
    case r_api::opengl: ImGui_ImplOpenGL3_NewFrame(); break;
    // case r_api::vulkan: ImGui_ImplVulkan_NewFrame(); break;
    default: break;
  }
  ImGui::NewFrame();
#endif
}

void r_context::end_frame() noexcept {
  _ctx->submit(_draw_lists);
  ImGui::Render();
  if (render_api() == r_api::opengl) {
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    _win->swap_buffers();
  }
}

void r_context::device_wait() noexcept {
  _ctx->device_wait();
}

r_buffer_handle r_context::create_buffer(const r_buffer_descriptor& desc) {
  auto validate_descriptor = [&]() -> bool {
    if (desc.data) {
      if (!desc.data->data) {
        SHOGLE_LOG(error, "[ntf::r_context::create_buffer] Invalid buffer data");
        return false;
      }

      if (desc.data->size+desc.data->offset > desc.size) {
        SHOGLE_LOG(error, "[ntf::r_context::create_buffer] Invalid buffer data offset");
        return false;
      }
    } else if (!+(desc.flags & r_buffer_flag::dynamic_storage)) {
      SHOGLE_LOG(error,
                 "[ntf::r_context::create_buffer] Attempted to create non dynamic "
                 "buffer with no data!");
      return false;
    }

    return true;
  };

  r_buffer_handle handle{};
  if (!validate_descriptor()) {
    SHOGLE_LOG(warning, "[ntf::r_context::create_buffer] Ignoring invalid descriptor");
    return handle;
  }

  try {
    handle = _ctx->create_buffer(desc);
  } catch (const std::exception& ex) {
    SHOGLE_LOG(error, "[ntf::r_context::create_buffer] Failed to create buffer: \"{}\"",
               ex.what());
    throw;
  }
  NTF_ASSERT(handle);
  NTF_ASSERT(_buffers.find(handle) == _buffers.end());

  auto [it, emplaced] = _buffers.try_emplace(handle);
  NTF_ASSERT(emplaced);

  auto& stored = it->second;
  stored.size = desc.size;
  stored.type = desc.type;
  stored.flags = desc.flags;
  return handle;
}

void r_context::destroy(r_buffer_handle buff) {
  NTF_ASSERT(buff);
  auto it = _buffers.find(buff);
  NTF_ASSERT(it != _buffers.end());

  _ctx->destroy_buffer(buff);
  _buffers.erase(it);
}

void r_context::update(r_buffer_handle buff, const r_buffer_data& desc) {
  NTF_ASSERT(buff);
  NTF_ASSERT(_buffers.find(buff) != _buffers.end());
  auto& buffer = _buffers.at(buff);

  auto validate_descriptor = [&]() -> bool {
    if (!+(buffer.flags & r_buffer_flag::dynamic_storage)) {
      SHOGLE_LOG(error, "[ntf::r_context::update] Can't update non dynamic buffer");
      return false;
    }

    if (!desc.data) {
      SHOGLE_LOG(error, "[ntf::r_context::update] Invalid buffer data");
      return false;
    }

    if (desc.size+desc.offset > buffer.size) {
      SHOGLE_LOG(error, "[ntf::r_context::update] Invalid buffer data offset");
      return false;
    }

    return true;
  };

  if (!validate_descriptor()) {
    SHOGLE_LOG(warning, "[ntf::r_context::update] Ignoring invalid buffer descriptor");
    return;
  }

  try {
    _ctx->update_buffer(buff, desc);
  } catch (const std::exception& ex) {
    SHOGLE_LOG(error, "[ntf::r_context::update] Failed to update buffer with id {}: \"{}\"",
               static_cast<r_handle_value>(buff),
               ex.what());
    throw;
  }
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
  auto validate_descriptor = [&]() -> bool {
    // TODO: Check max texture size
    // data.extent = glm::clamp(desc.extent, glm::uvec3{1, 1, 1}, glm::uvec3{4096, 4096, 4096});

    if (desc.layers > _ctx_meta.tex_max_layers) {
      SHOGLE_LOG(error, "[ntf::r_context::create_texture] Texture layers too high ({} > {})",
                 desc.layers, _ctx_meta.tex_max_layers);
      return false;
    }

    if (desc.type == r_texture_type::cubemap && desc.extent.x != desc.extent.y) {
      SHOGLE_LOG(error, "[ntf::r_context::create_texture] Invalid cubemap size");
      return false;
    }

    if (desc.type == r_texture_type::texture3d && desc.layers > 1) {
      SHOGLE_LOG(error, "[ntf::r_context::create_texture] Invalid layers for texture3d");   
      return false;
    }
    if (desc.type == r_texture_type::cubemap && desc.layers != 6) {
      SHOGLE_LOG(error, "[ntf::r_context::create_texture] Invalid layers for cubemap");
      return false;
    }

    if (desc.levels > 7 || desc.levels == 0) {
      SHOGLE_LOG(error, "[ntf::r_context::create_texture] Invalid texture level \"{}\"",
                 desc.levels);
      return false;
    }

    if (!desc.images) {
      if (desc.gen_mipmaps) {
        SHOGLE_LOG(warning, "[ntf::r_context::create_texture] "
                   "Ignoring mipmap generation for texture with no image data");
      }
      return true;
    }

    for (uint32 i = 0; i < desc.images.size(); ++i) {
      const auto& img = desc.images[i];
      const uvec3 upload_extent = img.offset+img.extent;

      if (!img.texels ||
          img.layer > desc.layers ||
          img.level > desc.levels) {
        SHOGLE_LOG(error, "[ntf::r_context::create_texture] Invalid image at idx {}", i);
        return false;
      }

      if (desc.type == r_texture_type::texture1d) {
        if (upload_extent.x > desc.extent.x) {
          SHOGLE_LOG(error,
                     "[ntf::r_context::create_texture] Invalid image extent at idx {}", i);
          return false;
        }
      } else if (desc.type == r_texture_type::texture2d || 
                 desc.type == r_texture_type::cubemap) {
        if (upload_extent.x > desc.extent.x ||
            upload_extent.y > desc.extent.y) {
          SHOGLE_LOG(error,
                     "[ntf::r_context::create_texture] Invalid image extent at idx {}", i);
          return false;
        }
      } else {
        if (upload_extent.x > desc.extent.x ||
            upload_extent.y > desc.extent.y ||
            upload_extent.z > desc.extent.z) {
          SHOGLE_LOG(error,
                     "[ntf::r_context::create_texture] Invalid image extent at idx {}", i);
          return false;
        }
      }
    }

    if (desc.gen_mipmaps && desc.levels == 1) {
      SHOGLE_LOG(warning, "[ntf::r_context::create_texture] "
                 "Ignoring mipmap generation for texture with level 1");
    }

    return true;
  };

  r_texture_handle handle{};
  if (!validate_descriptor()) {
    SHOGLE_LOG(warning, "[ntf::r_context::create_texture] Ignoring invalid descriptor");
    return handle;
  }

  try {
    handle = _ctx->create_texture(desc);
  } catch (const std::exception& ex) {
    SHOGLE_LOG(error, "[ntf::r_context::create_texture] Failed to create texture: \"{}\"",
               ex.what());
    throw;
  }
  NTF_ASSERT(handle);
  NTF_ASSERT(_textures.find(handle) == _textures.end());
  auto [it, emplaced] = _textures.try_emplace(handle);
  NTF_ASSERT(emplaced);

  auto& stored = it->second;
  stored.refcount.store(1);
  stored.type = desc.type;
  stored.format = desc.format;
  stored.extent = desc.extent;
  stored.levels = desc.levels;
  stored.layers = desc.layers;
  stored.addressing = desc.addressing;
  stored.sampler = desc.sampler;

  SHOGLE_LOG(verbose,
             "[ntf::r_context] TEXTURE CREATE - ID {} - EXT {}x{}x{} - TYPE {}",
             static_cast<r_handle_value>(handle),
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
             static_cast<r_handle_value>(tex),
             data.extent.x, data.extent.y, data.extent.z,
             textypetostr(data.type));
  _textures.erase(it);
}

void r_context::update(r_texture_handle tex, const r_texture_data& desc) {
  NTF_ASSERT(tex);
  NTF_ASSERT(tex);
  NTF_ASSERT(_textures.find(tex) != _textures.end());
  auto& texture = _textures.at(tex);

  auto validate_descriptor = [&]() -> bool {  
    const bool do_address = desc.addressing && *desc.addressing != texture.addressing;
    const bool do_sampler = desc.sampler && *desc.sampler != texture.sampler;

    if (!desc.images) {
      return do_sampler || do_address || desc.gen_mipmaps;
    }

    for (uint32 i = 0; i < desc.images.size(); ++i) {
      const auto& img = desc.images[i];
      const uvec3 upload_extent = img.offset+img.extent;

      if (!img.texels ||
          img.layer > texture.layers ||
          img.level > texture.levels) {
        SHOGLE_LOG(error, "[ntf::r_context::update] Invalid image at idx {}", i);
        return false;
      }

      if (texture.type == r_texture_type::texture1d) {
        if (upload_extent.x > texture.extent.x) {
          SHOGLE_LOG(error, "[ntf::r_context::update] Invalid image extent at idx {}", i);
          return false;
        }
      } else if (texture.type == r_texture_type::texture2d || 
                 texture.type == r_texture_type::cubemap) {
        if (upload_extent.x > texture.extent.x ||
            upload_extent.y > texture.extent.y) {
          SHOGLE_LOG(error, "[ntf::r_context::update] Invalid image extent at idx {}", i);
          return false;
        }
      } else {
        if (upload_extent.x > texture.extent.x ||
            upload_extent.y > texture.extent.y ||
            upload_extent.z > texture.extent.z) {
          SHOGLE_LOG(error, "[ntf::r_context::update] Invalid image extent at idx {}", i);
          return false;
        }
      }
    }

    return true;
  };

  if (!validate_descriptor()) {
    SHOGLE_LOG(warning, "[ntf::r_context::update] Ignoring invalid texture descriptor");
    return;
  }

  try {
    _ctx->update_texture(tex, desc);
  } catch (const std::exception& ex) {
    SHOGLE_LOG(error, "[ntf::r_context::update] Failed to update texture with id {}: \"{}\"",
               static_cast<r_handle_value>(tex),
               ex.what());
    return;
  }

  if (desc.addressing) {
    texture.addressing = *desc.addressing;
  }
  if (desc.sampler) {
    texture.sampler = *desc.sampler;
  }
  SHOGLE_LOG(verbose,
             "[ntf::r_context] TEXTURE UPLOAD - ID {} - EXT {}x{}x{} - TYPE {}",
             static_cast<r_handle_value>(tex),
             texture.extent.x, texture.extent.y, texture.extent.z,
             textypetostr(texture.type));
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
  auto validate_descriptor = [&]() -> bool {
    if (+(desc.test_buffers & r_test_buffer_flag::none) && !desc.test_buffer_format) {
      SHOGLE_LOG(error, "[ntf::r_context::create_framebuffer] Invalid test buffer format");
      return false;
    }
    if (!desc.attachments && !desc.color_buffer_format) {
      SHOGLE_LOG(error, "[ntf::r_context::create_framebuffer] Invalid color buffer format");
      return false;
    }

    for (uint32 i = 0; i < desc.attachments.size(); ++i) {
      const auto& att = desc.attachments[i];
      if (!att.handle || (_textures.find(att.handle) == _textures.end())) {
        SHOGLE_LOG(error, "[ntf::r_context::create_framebuffer] Invalid texture handle at idx {}",
                   i);
        return false;
      }
      const auto& tex = _textures.at(att.handle);
      if (att.layer > tex.layers) {
        SHOGLE_LOG(error, "[ntf::r_context::create_framebuffer] Invalid texture layer at idx {}",
                   i);
        return false;
      }
      if (att.level > tex.levels) {
        SHOGLE_LOG(error, "[ntf::r_context::create_framebuffer] Invalid texture level at idx {}",
                   i);
        return false;
      }
      if (tex.extent.x != desc.extent.x || tex.extent.y != desc.extent.y) {
        SHOGLE_LOG(error, "[ntf::r_context::create_framebuffer] Invalid texture extent at idx {}",
                   i);
        return false;
      }
    }

    if (desc.viewport.x+desc.viewport.z != desc.extent.x ||
        desc.viewport.y+desc.viewport.w != desc.extent.y) {
      SHOGLE_LOG(warning, "[ntf::r_context::create_framebuffer] Mismatching viewport size");
    }

    return true;
  };

  r_framebuffer_handle handle{};
  if (!validate_descriptor()) {
    SHOGLE_LOG(warning, "[ntf::r_context::create_framebuffer] Ignoring invalid descriptor");
    return handle;
  }

  try {
    handle = _ctx->create_framebuffer(desc);
  } catch (const std::exception& ex) {
    SHOGLE_LOG(error, "[ntf::r_context::create_framebuffer] Failed to create framebuffer: \"{}\"",
               ex.what());
    throw;
  }
  NTF_ASSERT(handle);
  NTF_ASSERT(_framebuffers.find(handle) == _framebuffers.end());

  auto [it, emplaced] = _framebuffers.try_emplace(handle);
  NTF_ASSERT(emplaced);

  auto [list_it, cmd_emplaced] = _draw_lists.try_emplace(handle);
  NTF_ASSERT(cmd_emplaced);

  auto& stored = it->second;
  stored.attachments.reserve(desc.attachments.size());
  for (uint32 i = 0; i < desc.attachments.size(); ++i) {
    stored.attachments.push_back(desc.attachments[i]);
    auto& tex = _textures.at(desc.attachments[i].handle);
    tex.refcount++;
  }
  list_it->second.viewport = desc.viewport;
  list_it->second.color = desc.clear_color;
  stored.extent = desc.extent;
  stored.buffers = desc.test_buffers;
  stored.buffer_format = desc.test_buffer_format;
  stored.color_buffer_format = desc.color_buffer_format;

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
  auto handle = _ctx->create_shader(desc);
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
  auto validate_descriptor = [&]() -> bool {
    return true;
  };

  r_pipeline_handle handle{};
  if (!validate_descriptor()) {
    return handle;
  }

  auto layout = std::make_unique<vertex_attrib_t>();
  layout->binding = desc.attrib_binding->binding;
  layout->stride = desc.attrib_binding->stride;
  layout->descriptors.resize(desc.attrib_desc.size());
  std::memcpy(
    layout->descriptors.data(), desc.attrib_desc.data(),
    desc.attrib_desc.size()*sizeof(r_attrib_descriptor)
  );
  uniform_map unif;

  try {
    handle = _ctx->create_pipeline(desc, layout.get(), unif);
  } catch (const std::exception& ex) {
    SHOGLE_LOG(error, "[ntf::r_context::create_pipeline] Failed to create pipeline: \"{}\"",
               ex.what());
    throw;
  }
  NTF_ASSERT(handle);
  NTF_ASSERT(_pipelines.find(handle) == _pipelines.end());

  auto [it, emplaced] = _pipelines.try_emplace(handle);
  NTF_ASSERT(emplaced);

  auto& stored = it->second;
  // stored.stages = create_data.
  stored.layout = std::move(layout);
  stored.uniforms = std::move(unif);
  stored.primitive = desc.primitive;
  stored.poly_mode = desc.poly_mode;
  stored.front_face = desc.front_face;
  stored.cull_mode = desc.cull_mode;
  stored.tests = desc.tests;
  stored.depth_ops = desc.depth_compare_op;
  stored.stencil_ops = desc.stencil_compare_op;

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
  const auto& pipe = _pipelines.at(pipeline);
  if (pipe.uniforms.find(name.data()) == pipe.uniforms.end()) {
    return r_uniform{};
  }
  return pipe.uniforms.at(name.data());
}

} // namespace ntf
