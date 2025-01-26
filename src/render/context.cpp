#include "./opengl.hpp"

#if SHOGLE_USE_GLFW
#include <imgui_impl_glfw.h>
#endif

#include <imgui_impl_opengl3.h> // Should work fine with OGL 4.x

#define RET_ERROR(_log_pfx, _fmt, ...) \
  SHOGLE_LOG(error, _log_pfx " " _fmt __VA_OPT__(,) __VA_ARGS__); \
  return unexpected<r_error>{r_error::format({_fmt} __VA_OPT__(,) __VA_ARGS__)}

#define RET_ERROR_IF(_cond, _log_pfx, _fmt, ...) \
  if (_cond) { \
    RET_ERROR(_log_pfx, _fmt, __VA_ARGS__); \
  }

#define RET_ERROR_CATCH(_log_pfx, _msg) \
  catch (r_error& err) { \
    SHOGLE_LOG(error, _log_pfx " " _msg ": {}", err.what()); \
    return unexpected{std::move(err)}; \
  } catch (const std::exception& ex) { \
    SHOGLE_LOG(error, _log_pfx " " _msg ": {}", ex.what()); \
    return unexpected{r_error::format({"{}"}, ex.what())}; \
  } catch (...) { \
    SHOGLE_LOG(error, _log_pfx " " _msg ": Caught (...)"); \
    return unexpected<r_error>{"Caught (...)"}; \
  }

namespace ntf {

r_context::r_context(r_error err) noexcept :
  _err{std::move(err)}, _win{}, _ctx{} {}

r_context::r_context(r_window& win, std::unique_ptr<r_platform_context> ctx,
                     command_map map) noexcept :
  _err{}, _win{win}, _ctx{std::move(ctx)}, _draw_lists(std::move(map)) {
  _ctx_meta = _ctx->query_meta();
  _frame_arena.init(1ull<<29); // 512 MiB
  _d_list = _draw_lists.at(DEFAULT_FRAMEBUFFER);
}

r_context r_context::create(r_window& win, const r_context_params& params) noexcept {
  r_api api = params.use_api.value_or(r_api::opengl);
  r_window::ctx_params_t win_params {
    .api = api,
    .gl_maj = 4,
    .gl_min = 6,
  };

  if (!win.init_context(win_params)) {
    return r_context{{"Failed to initialize window context"}};
  }

  std::unique_ptr<r_platform_context> ctx;
  try {
    switch (api) {
      case r_api::opengl: {
        ctx = std::make_unique<gl_context>(win, 4, 6);
        break;
      }
      default: {
        NTF_ASSERT(false, "Not implemented");
        break;
      }
    }
  } catch (r_error& err) {
    win.reset();
    return r_context{std::move(err)};
  } catch (const std::exception& ex) {
    win.reset();
    return r_context{r_error::format({"Failed To create context: {}"}, ex.what())};
  } catch (...) {
    win.reset();
    return r_context{{"Failed to create context: (...) caught"}};
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

  command_map map;
  auto [it, emplaced] = map.try_emplace(DEFAULT_FRAMEBUFFER);
  if (!emplaced) {
    return r_context{{"Failed to init default framebuffer"}};
  }
  it->second.viewport = uvec4{0, 0, win.fb_size()};

  return r_context{win, std::move(ctx), std::move(map)};
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

r_expected<r_buffer_handle> r_context::buffer_create(const r_buffer_descriptor& desc) noexcept {
  if (desc.data) {
    RET_ERROR_IF(!desc.data->data,
                 "[ntf::r_context::buffer_create]",
                 "Invalid buffer data");

    RET_ERROR_IF(desc.data->size+desc.data->offset > desc.size,
                 "[ntf::r_context::buffer_create]",
                 "Invalid buffer data offset");
  } else {
    RET_ERROR_IF(!+(desc.flags & r_buffer_flag::dynamic_storage),
                 "[ntf::r_context::buffer_create]",
                 "Attempted to create non dynamic buffer with no data");
  }

  r_buffer_handle handle{};
  try {
    handle = _ctx->create_buffer(desc);
    NTF_ASSERT(handle);
  }
  RET_ERROR_CATCH("[ntf::r_context::buffer_create]",
                  "Failed to create buffer");

  [[maybe_unused]] auto [it, emplaced] = _buffers.try_emplace(handle);
  NTF_ASSERT(emplaced);

  _init_buffer(it->second, desc);

  return handle;
}

r_buffer_handle r_context::buffer_create(unchecked_t, const r_buffer_descriptor& desc) {
  auto handle = _ctx->create_buffer(desc);
  NTF_ASSERT(handle);
  NTF_ASSERT(_buffers.find(handle) == _buffers.end());

  [[maybe_unused]] auto [it, emplaced] = _buffers.try_emplace(handle);
  NTF_ASSERT(emplaced);

  _init_buffer(it->second, desc);

  return handle;
}

void r_context::_init_buffer(buff_store_t& buff, const r_buffer_descriptor& desc) {
  buff.size = desc.size;
  buff.type = desc.type;
  buff.flags = desc.flags;
}

void r_context::destroy(r_buffer_handle buff) noexcept {
  if (!buff) {
    return;
  }
  auto it = _buffers.find(buff);
  if (it == _buffers.end()) {
    return;
  }

  _ctx->destroy_buffer(buff);
  _buffers.erase(it);
}

r_expected<void> r_context::buffer_update(r_buffer_handle buf, const r_buffer_data& des) noexcept {
  RET_ERROR_IF(!des.data,
               "[ntf::r_context::buffer_update]",
               "Invalid buffer data");

  RET_ERROR_IF(!buf,
               "[ntf::r_context::buffer_update]",
               "Invalid handle");

  auto it = _buffers.find(buf);
  RET_ERROR_IF(it == _buffers.end(),
               "[ntf::r_context::buffer_update]",
               "Invalid handle");
  
  auto& buffer = it->second;
  RET_ERROR_IF(!+(buffer.flags & r_buffer_flag::dynamic_storage),
               "[ntf::r_context::buffer_update]",
               "Can't update non dynamic buffer");

  RET_ERROR_IF(des.size+des.offset > buffer.size,
               "[ntf::r_context::buffer_update]",
               "Invalid buffer data offset");

  try {
    _ctx->update_buffer(buf, des);
  } 
  RET_ERROR_CATCH("[ntf::r_context::buffer_update]",
                  "Failed to update buffer");

  return {};
}

void r_context::buffer_update(unchecked_t, r_buffer_handle buf, const r_buffer_data& des) {
  NTF_ASSERT(buf);
  NTF_ASSERT(_buffers.find(buf) != _buffers.end());
  _ctx->update_buffer(buf, des);
}

r_buffer_type r_context::buffer_type(r_buffer_handle buff) const {
  NTF_ASSERT(buff);
  NTF_ASSERT(_buffers.find(buff) != _buffers.end());
  return _buffers.at(buff).type;
}

size_t r_context::buffer_size(r_buffer_handle buff) const {
  NTF_ASSERT(buff);
  NTF_ASSERT(_buffers.find(buff) != _buffers.end());
  return _buffers.at(buff).size;
}

// static const char* textypetostr(r_texture_type type) {
//   switch (type) {
//     case r_texture_type::texture1d: return "TEX1D";
//     case r_texture_type::texture2d: return "TEX2D";
//     case r_texture_type::texture3d: return "TEX3D";
//     case r_texture_type::cubemap:   return "CUBEMAP";
//   }
//
//   NTF_UNREACHABLE();
// }

r_expected<r_texture_handle> r_context::texture_create(const r_texture_descriptor& desc) noexcept {
  RET_ERROR_IF(desc.layers > _ctx_meta.tex_max_layers,
               "[ntf::r_context::texture_create]",
               "Texture layers to high ({} > {})",
               desc.layers, _ctx_meta.tex_max_layers);

  RET_ERROR_IF(desc.type == r_texture_type::cubemap && desc.extent.x != desc.extent.y,
               "[ntf::r_context::texture_create]",
               "Invalid cubemap extent");

  RET_ERROR_IF(desc.type == r_texture_type::texture3d && desc.layers > 1,
               "[ntf::r_context::texture_create]",
               "Invalid layers for texture3d");

  RET_ERROR_IF(desc.type == r_texture_type::cubemap && desc.layers != 6,
               "[ntf::r_context::texture_create]",
               "Invalid layers for cubemap");

  RET_ERROR_IF(desc.levels > 7 || desc.levels == 0,
               "[ntf::r_context::texture_create]",
               "Invalid texture level \"{}\"",
               desc.levels);

  if (desc.gen_mipmaps) {
    if (!desc.images) {
      SHOGLE_LOG(warning, "[ntf::r_context::texture_create] "
                 "Ignoring mipmap generation for texture with no image data");
    }
    if (desc.levels == 1) {
      SHOGLE_LOG(warning, "[ntf::r_context::texture_create] "
                 "Ignoring mipmap generation for texture with level 1");
    }
  }

  switch (desc.type) {
    case r_texture_type::texture1d: {
      const auto max_ext = _ctx_meta.tex_max_extent;
      RET_ERROR_IF(desc.extent.x > max_ext,
                   "[ntf::r_context::texture_create]",
                   "Requested texture is too big ({} > {})",
                   desc.extent.x, _ctx_meta.tex_max_extent);
      break;
    }
    case r_texture_type::cubemap: [[fallthrough]];
    case r_texture_type::texture2d: {
      const auto max_ext = _ctx_meta.tex_max_extent;
      RET_ERROR_IF(desc.extent.x > max_ext ||
                   desc.extent.y > max_ext,
                   "[ntf::r_context::texture_create]",
                   "Requested texture is too big ({}x{} > {}x{})",
                   desc.extent.x, desc.extent.y, max_ext, max_ext);
      break;
    }
    case r_texture_type::texture3d: {
      const auto max_ext = _ctx_meta.tex_max_extent3d;
      RET_ERROR_IF(desc.extent.x > max_ext ||
                   desc.extent.y > max_ext ||
                   desc.extent.z > max_ext,
                   "[ntf::r_context::texture_create]",
                   "Requested texture is too big ({}x{}x{} > {}x{}x{})",
                   desc.extent.x, desc.extent.y, desc.extent.z,
                   max_ext, max_ext, max_ext);
      break;
    }
  }

  for (uint32 i = 0; i < desc.images.size(); ++i) {
    const auto& img = desc.images[i];
    const uvec3 upload_extent = img.offset+img.extent;

    RET_ERROR_IF(!img.texels ||
                 img.layer > desc.layers ||
                 img.level > desc.levels,
                 "[ntf::r_context::texture_create]",
                 "Invalid image at index {}",
                 i);
    switch (desc.type) {
      case r_texture_type::texture1d: {
        RET_ERROR_IF(upload_extent.x > desc.extent.x,
                     "[ntf::r_context::texture_create]",
                     "Invalid image extent at index {}",
                     i);
        break;
      }
      case r_texture_type::cubemap: [[fallthrough]];
      case r_texture_type::texture2d: {
        RET_ERROR_IF(upload_extent.x > desc.extent.x ||
                      upload_extent.y > desc.extent.y,
                     "[ntf::r_context::texture_create]",
                     "Invalid image extent at index {}",
                     i);
        break;
      }
      case r_texture_type::texture3d: {
        RET_ERROR_IF(upload_extent.x > desc.extent.x ||
                      upload_extent.y > desc.extent.y ||
                      upload_extent.z > desc.extent.z,
                     "[ntf::r_context::texture_create]",
                     "Invalid image extent at index {}",
                     i);
        break;
      }
    }
  }

  r_texture_handle handle{};
  try {
    handle = _ctx->create_texture(desc);
    NTF_ASSERT(handle);
  } 
  RET_ERROR_CATCH("[ntf::r_context::texture_create]",
                  "Failed to create texture handle");

  NTF_ASSERT(_textures.find(handle) == _textures.end());

  auto [it, emplaced] = _textures.try_emplace(handle);
  NTF_ASSERT(emplaced);

  _init_texture(it->second, desc);

  return handle;
}

r_texture_handle r_context::texture_create(unchecked_t, const r_texture_descriptor& desc) {
  auto handle = _ctx->create_texture(desc);
  NTF_ASSERT(handle);
  NTF_ASSERT(_textures.find(handle) == _textures.end());

  [[maybe_unused]] auto [it, emplaced] = _textures.try_emplace(handle);
  NTF_ASSERT(emplaced);

  _init_texture(it->second, desc);

  return handle;
}

void r_context::_init_texture(tex_store_t& stored, const r_texture_descriptor& desc) {
  stored.refcount.store(1);
  stored.type = desc.type;
  stored.format = desc.format;
  stored.extent = desc.extent;
  stored.levels = desc.levels;
  stored.layers = desc.layers;
  stored.addressing = desc.addressing;
  stored.sampler = desc.sampler;

  // SHOGLE_LOG(verbose,
  //            "[ntf::r_context] TEXTURE CREATE - ID {} - EXT {}x{}x{} - TYPE {}",
  //            static_cast<r_handle_value>(handle),
  //            stored.extent.x, stored.extent.y, stored.extent.z,
  //            textypetostr(stored.type));
}

void r_context::destroy(r_texture_handle tex) noexcept {
  if (!tex) {
    return;
  }

  auto it = _textures.find(tex);
  if (it == _textures.end()) {
    return;
  }

  if (--it->second.refcount > 0) {
    return;
  }

  _ctx->destroy_texture(tex);
  _textures.erase(it);
  // SHOGLE_LOG(verbose,
  //            "[ntf::r_context] TEXTURE DESTROY - ID {} - EXT {}x{}x{} - TYPE {}",
  //            static_cast<r_handle_value>(tex),
  //            data.extent.x, data.extent.y, data.extent.z,
  //            textypetostr(data.type));
}

r_expected<void> r_context::texture_update(r_texture_handle tex,
                                           const r_texture_data& data) noexcept {
  RET_ERROR_IF(!tex,
               "[ntf::r_context::texture_update]",
               "Invalid handle");

  auto it = _textures.find(tex);
  RET_ERROR_IF(it == _textures.end(),
               "[ntf::r_context::texture_update]",
               "Invalid handle");

  auto& texture = it->second;

  const bool do_address = data.addressing && *data.addressing != texture.addressing;
  const bool do_sampler = data.sampler && *data.sampler != texture.sampler;

  if (!data.images) {
    RET_ERROR_IF(!(do_sampler || do_address),
                 "[ntf::r_context::texture_update]",
                 "Invalid update descriptor");
  } else {
    // TODO: Unify the update & create image tests?
    for (uint32 i = 0; i < data.images.size(); ++i) {
      const auto& img = data.images[i];
      const uvec3 upload_extent = img.offset+img.extent;

      RET_ERROR_IF(!img.texels ||
                   img.layer > texture.layers ||
                   img.level > texture.levels,
                   "[ntf::r_context::texture_update]",
                   "Invalid image at index {}",
                   i);
      switch (texture.type) {
        case r_texture_type::texture1d: {
          RET_ERROR_IF(upload_extent.x > texture.extent.x,
                       "[ntf::r_context::texture_update]",
                       "Invalid image extent at index {}",
                       i);
          break;
        }
        case r_texture_type::cubemap: [[fallthrough]];
        case r_texture_type::texture2d: {
          RET_ERROR_IF(upload_extent.x > texture.extent.x ||
                        upload_extent.y > texture.extent.y,
                       "[ntf::r_context::texture_update]",
                       "Invalid image extent at index {}",
                       i);
          break;
        }
        case r_texture_type::texture3d: {
          RET_ERROR_IF(upload_extent.x > texture.extent.x ||
                        upload_extent.y > texture.extent.y ||
                        upload_extent.z > texture.extent.z,
                       "[ntf::r_context::texture_update]",
                       "Invalid image extent at index {}",
                       i);
          break;
        }
      }
    }
  }

  try {
    _ctx->update_texture(tex, data);
  } 
  RET_ERROR_CATCH("[ntf::r_context::texture_update]",
                  "Failed to update texture");

  if (data.addressing) {
    texture.addressing = *data.addressing;
  }
  if (data.sampler) {
    texture.sampler = *data.sampler;
  }

  // SHOGLE_LOG(verbose,
  //            "[ntf::r_context] TEXTURE UPLOAD - ID {} - EXT {}x{}x{} - TYPE {}",
  //            static_cast<r_handle_value>(tex),
  //            texture.extent.x, texture.extent.y, texture.extent.z,
  //            textypetostr(texture.type));

  return {};
}

void r_context::texture_update(unchecked_t, r_texture_handle tex, const r_texture_data& data) {
  NTF_ASSERT(tex);

  auto it = _textures.find(tex);
  NTF_ASSERT(it != _textures.end());

  auto& texture = it->second;
  _ctx->update_texture(tex, data);

  if (data.addressing) {
    texture.addressing = *data.addressing;
  }
  if (data.sampler) {
    texture.sampler = *data.sampler;
  }
}

r_expected<void> r_context::texture_update(r_texture_handle tex,
                                           span_view<r_image_data> images, bool mips) noexcept {
  return texture_update(tex, r_texture_data{
    .images = images,
    .gen_mipmaps = mips,
  });
}

void r_context::texture_update(unchecked_t, r_texture_handle tex,
                               span_view<r_image_data> images, bool mips) {
  texture_update(::ntf::unchecked, tex, r_texture_data{
    .images = images,
    .gen_mipmaps = mips,
  });
}

r_expected<void> r_context::texture_sampler(r_texture_handle tex,
                                            r_texture_sampler sampler) noexcept {
  return texture_update(tex, r_texture_data{
    .sampler = sampler,
  });
}

void r_context::texture_sampler(unchecked_t, r_texture_handle tex,
                                r_texture_sampler sampler) {
  texture_update(::ntf::unchecked, tex, r_texture_data{
    .sampler = sampler,
  });
}

r_expected<void> r_context::texture_addressing(r_texture_handle tex,
                                               r_texture_address addressing) noexcept {
  return texture_update(tex, r_texture_data{
    .addressing = addressing,
  });
}

void r_context::texture_addressing(unchecked_t, r_texture_handle tex,
                                   r_texture_address addressing) {
  texture_update(::ntf::unchecked, tex, r_texture_data{
    .addressing = addressing,
  });
}

r_texture_type r_context::texture_type(r_texture_handle tex) const {
  NTF_ASSERT(tex);
  NTF_ASSERT(_textures.find(tex) != _textures.end());
  return _textures.at(tex).type;
}

r_texture_format r_context::texture_format(r_texture_handle tex) const {
  NTF_ASSERT(tex);
  NTF_ASSERT(_textures.find(tex) != _textures.end());
  return _textures.at(tex).format;
}

r_texture_sampler r_context::texture_sampler(r_texture_handle tex) const {
  NTF_ASSERT(tex);
  NTF_ASSERT(_textures.find(tex) != _textures.end());
  return _textures.at(tex).sampler;
}

r_texture_address r_context::texture_addressing(r_texture_handle tex) const {
  NTF_ASSERT(tex);
  NTF_ASSERT(_textures.find(tex) != _textures.end());
  return _textures.at(tex).addressing;
}

uvec3 r_context::texture_extent(r_texture_handle tex) const {
  NTF_ASSERT(tex);
  NTF_ASSERT(_textures.find(tex) != _textures.end());
  return _textures.at(tex).extent;
}

uint32 r_context::texture_layers(r_texture_handle tex) const {
  NTF_ASSERT(tex);
  NTF_ASSERT(_textures.find(tex) != _textures.end());
  return _textures.at(tex).layers;
}

uint32 r_context::texture_levels(r_texture_handle tex) const {
  NTF_ASSERT(tex);
  NTF_ASSERT(_textures.find(tex) != _textures.end());
  return _textures.at(tex).levels;
}

r_expected<r_framebuffer_handle> r_context::framebuffer_create(
                                                   const r_framebuffer_descriptor& desc) noexcept {
  RET_ERROR_IF(+(desc.test_buffers & r_test_buffer_flag::none) && !desc.test_buffer_format,
               "[ntf::r_context::framebuffer_create]",
               "Invalid test buffer format");

  RET_ERROR_IF(!desc.attachments && !desc.color_buffer_format,
               "[ntf::r_context::framebuffer_create]",
               "Invalid color buffer format");

  for (uint32 i = 0; i < desc.attachments.size(); ++i) {
    const auto& att = desc.attachments[i];
    RET_ERROR_IF(!att.handle || (_textures.find(att.handle) == _textures.end()),
                 "[ntf::r_context::framebuffer_create]",
                 "Invalid texture handle at index {}",
                 i);

    const auto& tex = _textures.at(att.handle);
    RET_ERROR_IF(att.layer > tex.layers,
                 "[ntf::r_context::framebuffer_create]",
                 "Invalid texture layer at index {}",
                 i);

    RET_ERROR_IF(att.level > tex.levels,
                 "[ntf::r_context::framebuffer_create]",
                 "Invalid texture level at index {}",
                 i);

    RET_ERROR_IF(tex.extent.x != desc.extent.x || tex.extent.y != desc.extent.y,
                 "[ntf::r_context::framebuffer_create]",
                 "Invalid texture extent at index {}",
                 i);
  }

  if (desc.viewport.x+desc.viewport.z != desc.extent.x ||
      desc.viewport.y+desc.viewport.w != desc.extent.y) {
    SHOGLE_LOG(warning, "[ntf::r_context::framebuffer_create] Mismatching viewport size");
  }

  r_framebuffer_handle handle{};
  try {
    handle = _ctx->create_framebuffer(desc);
    NTF_ASSERT(handle);
  }
  RET_ERROR_CATCH("[ntf::r_context::framebuffer_create]",
                  "Failed to create framebuffer handle");

  NTF_ASSERT(_framebuffers.find(handle) == _framebuffers.end());

  [[maybe_unused]] auto [it, emplaced] = _framebuffers.try_emplace(handle);
  NTF_ASSERT(emplaced);

  _init_framebuffer(handle, it->second, desc);

  return handle;
}

r_framebuffer_handle r_context::framebuffer_create(unchecked_t,
                                                   const r_framebuffer_descriptor& desc) {
  auto handle = _ctx->create_framebuffer(desc);
  NTF_ASSERT(handle);
  NTF_ASSERT(_framebuffers.find(handle) == _framebuffers.end());

  [[maybe_unused]] auto [it, emplaced] = _framebuffers.try_emplace(handle);
  NTF_ASSERT(emplaced);

  _init_framebuffer(handle, it->second, desc);

  return handle;
}

void r_context::_init_framebuffer(r_framebuffer_handle handle, fb_store_t& fbo,
                                  const r_framebuffer_descriptor& desc) {
  fbo.attachments.reserve(desc.attachments.size());
  for (uint32 i = 0; i < desc.attachments.size(); ++i) {
    fbo.attachments.push_back(desc.attachments[i]);
    auto& tex = _textures.at(desc.attachments[i].handle);
    tex.refcount++;
  }
  fbo.extent = desc.extent;
  fbo.buffers = desc.test_buffers;
  fbo.buffer_format = desc.test_buffer_format;
  fbo.color_buffer_format = desc.color_buffer_format;

  _draw_lists[handle] = {};
  auto& list = _draw_lists.at(handle);
  list.viewport = desc.viewport;
  list.color = desc.clear_color;
  list.clear = desc.clear_flags;
}

void r_context::destroy(r_framebuffer_handle fbo) noexcept {
  if (!fbo) {
    return;
  }
  auto it = _framebuffers.find(fbo);
  if (it == _framebuffers.end()) {
    return;
  }

  _ctx->destroy_framebuffer(fbo);
  for (auto att : it->second.attachments) {
    destroy(att.handle); // decreases refcount and maybe destroys
  }
  _framebuffers.erase(it);
}

void r_context::framebuffer_clear(r_framebuffer_handle fbo, r_clear_flag flags) {
  NTF_ASSERT(_draw_lists.find(fbo) != _draw_lists.end());
  _draw_lists.at(fbo).clear = flags;
}

r_clear_flag r_context::framebuffer_clear(r_framebuffer_handle fbo) const {
  NTF_ASSERT(_draw_lists.find(fbo) != _draw_lists.end());
  return _draw_lists.at(fbo).clear;
}

void r_context::framebuffer_viewport(r_framebuffer_handle fbo, uvec4 vp) {
  NTF_ASSERT(_draw_lists.find(fbo) != _draw_lists.end());
  _draw_lists.at(fbo).viewport = vp;
}

uvec4 r_context::framebuffer_viewport(r_framebuffer_handle fbo) const {
  NTF_ASSERT(_draw_lists.find(fbo) != _draw_lists.end());
  return _draw_lists.at(fbo).viewport;
}

void r_context::framebuffer_color(r_framebuffer_handle fbo, color4 color) {
  NTF_ASSERT(_draw_lists.find(fbo) != _draw_lists.end());
  _draw_lists.at(fbo).color = color;
}

color4 r_context::framebuffer_color(r_framebuffer_handle fbo) const {
  NTF_ASSERT(_draw_lists.find(fbo) != _draw_lists.end());
  return _draw_lists.at(fbo).color;
}

r_expected<r_shader_handle> r_context::shader_create(const r_shader_descriptor& desc) noexcept {
  r_shader_handle handle{};
  try {
    handle = _ctx->create_shader(desc);
    NTF_ASSERT(handle);
  }
  RET_ERROR_CATCH("[ntf::r_context::shader_create]",
                  "Failed to create shader handle");

  [[maybe_unused]] auto [it, emplaced] = _shaders.try_emplace(handle);
  NTF_ASSERT(emplaced);

  _init_shader(it->second, desc);

  return handle;
}

r_shader_handle r_context::shader_create(unchecked_t, const r_shader_descriptor& desc) {
  auto handle = _ctx->create_shader(desc);
  NTF_ASSERT(handle);
  NTF_ASSERT(_shaders.find(handle) == _shaders.end());

  [[maybe_unused]] auto [it, emplaced] = _shaders.try_emplace(handle);
  NTF_ASSERT(emplaced);

  _init_shader(it->second, desc);

  return handle;
}

void r_context::_init_shader(shader_store_t& shad, const r_shader_descriptor& desc) {
  shad.type = desc.type;
}

void r_context::destroy(r_shader_handle shader) noexcept {
  if (!shader) {
    return;
  }

  auto it = _shaders.find(shader);
  if (it == _shaders.end()) {
    return;
  }

  _ctx->destroy_shader(shader);
  _shaders.erase(it);
}

r_shader_type r_context::shader_type(r_shader_handle shader) const {
  NTF_ASSERT(shader);
  NTF_ASSERT(_shaders.find(shader) != _shaders.end());
  return _shaders.at(shader).type;
}

r_expected<r_pipeline_handle> r_context::pipeline_create(
                                                      const r_pipeline_descriptor& desc) noexcept {
  // TODO: validation
  auto layout = _copy_pipeline_layout(desc);
  uniform_map uniforms;
  r_pipeline_handle handle{};
  try {
    _ctx->create_pipeline(desc, layout.get(), uniforms);
    NTF_ASSERT(handle);
  }
  RET_ERROR_CATCH("[ntf::r_context::pipeline_create]",
                  "Failed to create pipeline");

  [[maybe_unused]] auto [it, emplaced] = _pipelines.try_emplace(handle);
  NTF_ASSERT(emplaced);

  _init_pipeline(it->second, desc, std::move(layout), std::move(uniforms));

  return handle;
}

r_pipeline_handle r_context::pipeline_create(unchecked_t, const r_pipeline_descriptor& desc) {
  auto layout = _copy_pipeline_layout(desc);
  uniform_map uniforms;

  auto handle = _ctx->create_pipeline(desc, layout.get(), uniforms);
  NTF_ASSERT(handle);
  NTF_ASSERT(_pipelines.find(handle) == _pipelines.end());

  [[maybe_unused]] auto [it, emplaced] = _pipelines.try_emplace(handle);
  NTF_ASSERT(emplaced);

  _init_pipeline(it->second, desc, std::move(layout), std::move(uniforms));

  return handle;
}

auto r_context::_copy_pipeline_layout(const r_pipeline_descriptor& desc)
                                                              -> std::unique_ptr<vertex_attrib_t> {
  auto layout = std::make_unique<vertex_attrib_t>();
  layout->binding = desc.attrib_binding->binding;
  layout->stride = desc.attrib_binding->stride;
  layout->descriptors.resize(desc.attrib_desc.size());
  std::memcpy(
    layout->descriptors.data(), desc.attrib_desc.data(),
    desc.attrib_desc.size()*sizeof(r_attrib_descriptor)
  );
  return layout;
}

void r_context::_init_pipeline(pipeline_store_t& pip, const r_pipeline_descriptor& desc,
                               std::unique_ptr<vertex_attrib_t> layout, uniform_map uniforms) {
  // stored.stages = create_data.
  pip.layout = std::move(layout);
  pip.uniforms = std::move(uniforms);
  pip.primitive = desc.primitive;
  pip.poly_mode = desc.poly_mode;
  pip.front_face = desc.front_face;
  pip.cull_mode = desc.cull_mode;
  pip.tests = desc.tests;
  pip.depth_ops = desc.depth_compare_op;
  pip.stencil_ops = desc.stencil_compare_op;
}

void r_context::destroy(r_pipeline_handle pipeline) noexcept {
  if (!pipeline) {
    return;
  }

  auto it = _pipelines.find(pipeline);
  if (it == _pipelines.end()) {
    return;
  }

  _ctx->destroy_pipeline(pipeline);
  _pipelines.erase(it);
}

r_stages_flag r_context::pipeline_stages(r_pipeline_handle pipeline) const {
  NTF_ASSERT(pipeline);
  NTF_ASSERT(_pipelines.find(pipeline) != _pipelines.end());
  return _pipelines.at(pipeline).stages;
}

optional<r_uniform> r_context::pipeline_uniform(r_pipeline_handle pipeline,
                                                std::string_view name) const noexcept {
  if (!pipeline) {
    return nullopt;
  }
  auto it = _pipelines.find(pipeline);
  if (it == _pipelines.end()) {
    return nullopt;
  }

  const auto& pip = it->second;
  auto unif_it = pip.uniforms.find(name.data());
  if (unif_it == pip.uniforms.end()) {
    return nullopt;
  }

  return unif_it->second;
}

r_uniform r_context::pipeline_uniform(unchecked_t, r_pipeline_handle pipeline,
                                      std::string_view name) const {
  NTF_ASSERT(pipeline);

  auto it = _pipelines.find(pipeline);
  NTF_ASSERT(it != _pipelines.end());

  const auto& pip = it->second;
  auto unif_it = pip.uniforms.find(name.data());
  NTF_ASSERT(unif_it != pip.uniforms.end());

  return unif_it->second;
}

} // namespace ntf
