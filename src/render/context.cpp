#include "./opengl.hpp"

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
    return unexpected{r_error{"Caught (...)"}}; \
  }

namespace ntf {

namespace {

template<bool checked>
auto r_context_create_impl(const r_context_params& params)
  -> std::conditional_t<checked, r_expected<r_context>, r_context>
{
  r_api api = params.use_api.value_or(r_api::opengl);

  r_context_data::command_map map;
  auto [it, emplaced] = map.try_emplace(r_context::DEFAULT_FRAMEBUFFER);
  NTF_ASSERT(emplaced);

  std::unique_ptr<r_platform_context> ctx;
  win_handle_t win = params.window->handle();
  auto init_ctx = [&]() {
    switch (api) {
      case r_api::opengl: {
        r_window::gl_set_current(win);
        r_window::gl_set_swap_interval(params.swap_interval);
        auto proc = r_window::gl_load_proc();
        if (!gladLoadGLLoader(proc)) {
          throw ntf::error<>{"Failed to load GLAD"};
        }
        ctx = std::make_unique<gl_context>();
        break;
      }
      default: {
        return false;
        break;
      }
    }
    return true;
  };

  if constexpr (checked) {
    try {
      init_ctx();
    } catch (r_error& err) {
      return unexpected{std::move(err)};
    } catch (const std::exception& ex) {
      return unexpected{r_error::format({"Failed To create context: {}"}, ex.what())};
    } catch (...) {
      return unexpected{r_error{"Failed to create context: (...) caught"}};
    }
  } else {
    auto ret = init_ctx();
    NTF_ASSERT(ret);
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
      ImGui_ImplGlfw_InitForOpenGL(win, true);
      ImGui_ImplOpenGL3_Init("#version 130");
      break;
    }
    case r_api::vulkan: {
      ImGui_ImplGlfw_InitForVulkan(win, true);
      break;
    }
    default: {
      ImGui_ImplGlfw_InitForOther(win, true);
      break;
    }
  }
#endif
#endif

  return r_context{std::make_unique<r_context_data>(win, std::move(ctx), std::move(map))};
}

} // namespace

r_expected<r_context> r_context::create(const r_context_params& params) noexcept {
  return r_context_create_impl<true>(params);
}

r_context r_context::create(unchecked_t, const r_context_params& params) {
  return r_context_create_impl<false>(params);
}

r_context::r_context(uptr_base data) noexcept :
  uptr_base{std::move(data)}, r_context_view{uptr_base::get()}
{ 
  auto meta = _data->platform->query_meta();
  SHOGLE_LOG(debug, "[ntf::r_context][CONSTRUCT] {} ver {} [{} - {}]",
             meta.api == r_api::opengl ? "OpenGL" : "Vulkan",
             meta.version_str, meta.vendor_str, meta.name_str);
}

r_context::~r_context() noexcept {
  SHOGLE_LOG(debug, "[ntf::r_context][DESTROY]");
}

r_context_data::buffer_store::buffer_store(const r_buffer_descriptor& desc) noexcept :
  type{desc.type}, flags{desc.flags}, size{desc.size} {}

r_context_data::texture_store::texture_store(const r_texture_descriptor& desc) noexcept :
  refcount{1}, type{desc.type}, format{desc.format}, extent{desc.extent},
  levels{desc.levels}, layers{desc.layers}, addressing{desc.addressing}, sampler{desc.sampler} {}

r_context_data::shader_store::shader_store(const r_shader_descriptor& desc) noexcept :
  type{desc.type} {}

r_context_data::framebuffer_store::framebuffer_store(
  const r_framebuffer_descriptor& desc
) noexcept :
  extent{desc.extent}, buffers{desc.test_buffers},
  buffer_format{desc.test_buffer_format}, color_buffer_format{desc.color_buffer_format}
{ attachments.reserve(desc.attachments.size()); }

r_context_data::pipeline_store::pipeline_store(
  const r_pipeline_descriptor& desc,
  std::unique_ptr<vertex_layout> layout,
  uniform_map uniforms,
  r_stages_flag stages_
) noexcept :
  stages{stages_},
  primitive{desc.primitive}, poly_mode{desc.poly_mode},
  front_face{desc.front_face}, cull_mode{desc.cull_mode},
  tests{desc.tests}, depth_ops{desc.depth_compare_op}, stencil_ops{desc.stencil_compare_op},
  layout{std::move(layout)}, uniforms(std::move(uniforms)) {}

r_context_data::r_context_data(win_handle_t win_,
                               std::unique_ptr<r_platform_context> pctx_,
                               command_map map_) noexcept :
  win{win_}, platform{std::move(pctx_)}, draw_lists(std::move(map_)), d_cmd{}
{
  frame_arena.init(1ull<<29); // 512 MiB
  d_list = draw_lists.at(r_context::DEFAULT_FRAMEBUFFER);
}

r_context_data::~r_context_data() noexcept {
#if SHOGLE_ENABLE_IMGUI
  switch (platform->query_meta().api) {
    case r_api::opengl: {
      ImGui_ImplOpenGL3_Shutdown();
      break;
    }
    default: break;
  }
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();
#endif
}

void r_context_view::start_frame() noexcept {
  for (auto& [_, list] : _data->draw_lists) {
    for (auto& cmd : list.cmds) {
      cmd.get().~draw_command();
    }
    list.cmds.clear();
  }
  _data->frame_arena.reset();

#if SHOGLE_ENABLE_IMGUI
#if SHOGLE_USE_GLFW
  ImGui_ImplGlfw_NewFrame();
#endif
  switch (_data->platform->query_meta().api) {
    case r_api::opengl: ImGui_ImplOpenGL3_NewFrame(); break;
    // case r_api::vulkan: ImGui_ImplVulkan_NewFrame(); break;
    default: break;
  }
  ImGui::NewFrame();
#endif
}

void r_context_view::end_frame() noexcept {
  _data->platform->submit(_data->win, _data->draw_lists);
  ImGui::Render();
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
  _data->platform->swap_buffers(_data->win);
}

void r_context_view::device_wait() noexcept {
  _data->platform->device_wait();
}

void r_context_view::bind_texture(
  r_texture_handle texture,
  uint32 index
) noexcept {
  auto* ptr = _data->frame_arena.allocate<r_context_data::texture_binding>(1);
  ptr->handle = texture;
  ptr->index = index;
  _data->d_cmd.textures.emplace_back(ptr);
}

void r_context_view::bind_framebuffer(r_framebuffer_handle fbo) noexcept {
  NTF_ASSERT(_data->draw_lists.find(fbo) != _data->draw_lists.end());
  _data->d_list = _data->draw_lists.at(fbo);
}

void r_context_view::bind_vertex_buffer(r_buffer_handle buffer) noexcept {
  _data->d_cmd.vertex_buffer = buffer;
}

void r_context_view::bind_index_buffer(r_buffer_handle buffer) noexcept {
  _data->d_cmd.index_buffer = buffer;
}

void r_context_view::bind_pipeline(r_pipeline_handle pipeline) noexcept {
  _data->d_cmd.pipeline = pipeline;
}

void r_context_view::draw_opts(r_draw_opts opts) noexcept {
  _data->d_cmd.count = opts.count;
  _data->d_cmd.offset = opts.offset;
  _data->d_cmd.instances = opts.instances;
}

void r_context_view::submit() noexcept {
  auto* cmd = _data->frame_arena.allocate<r_context_data::draw_command>(1);
  weak_ref<r_context_data::draw_command> ref = std::construct_at(cmd, std::move(_data->d_cmd));
  _data->d_list->cmds.emplace_back(ref);
  _data->d_cmd = {};
}

void r_context_view::submit(const r_draw_command& cmd) noexcept {
  _data->d_list = _data->draw_lists.at(cmd.target);
  _data->d_cmd.pipeline = cmd.pipeline;

  _data->d_cmd.count = cmd.draw_opts->count;
  _data->d_cmd.offset = cmd.draw_opts->offset;
  _data->d_cmd.instances = cmd.draw_opts->instances;

  if (cmd.on_render) {
    _data->d_cmd.on_render = [this, on_render=cmd.on_render]() { on_render(*this); };
  } else {
    _data->d_cmd.on_render = {};
  }

  for (const auto& buff : cmd.buffers) {
    if (buff.type == r_buffer_type::index) {
      _data->d_cmd.index_buffer = buff.buffer;
    } else {
      _data->d_cmd.vertex_buffer = buff.buffer;
    }
  }

  for (const auto& tex : cmd.textures) {
    auto* ptr = _data->frame_arena.allocate<r_context_data::texture_binding>(1);
    ptr->handle = tex.texture;
    ptr->index = tex.location;
    _data->d_cmd.textures.emplace_back(ptr);
  }

  for (const auto& unif : cmd.uniforms) {
    auto* desc = _data->frame_arena.allocate<r_context_data::uniform_descriptor>(1);

    size_t data_sz = r_attrib_type_size(unif.type);
    auto* data = _data->frame_arena.allocate(data_sz, unif.alignment);
    std::memcpy(data, unif.data, data_sz);

    desc->location = unif.location;
    desc->type = unif.type;
    desc->data = data;
    _data->d_cmd.uniforms.emplace_back(desc);
  }

  auto* lcmd = _data->frame_arena.allocate<r_context_data::draw_command>(1);
  weak_ref<r_context_data::draw_command> ref = std::construct_at(lcmd, std::move(_data->d_cmd));
  _data->d_list->cmds.emplace_back(ref);
  _data->d_cmd = {};
}

r_expected<r_buffer_handle> r_context_view::buffer_create(
  const r_buffer_descriptor& desc
) noexcept {
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
    handle = _data->platform->create_buffer(desc);
    NTF_ASSERT(handle);
  }
  RET_ERROR_CATCH("[ntf::r_context::buffer_create]",
                  "Failed to create buffer");

  [[maybe_unused]] auto [it, emplaced] = _data->buffers.try_emplace(handle, desc);
  NTF_ASSERT(emplaced);

  return handle;
}

r_buffer_handle r_context_view::buffer_create(
  unchecked_t,
  const r_buffer_descriptor& desc
) {
  auto handle = _data->platform->create_buffer(desc);
  NTF_ASSERT(handle);
  NTF_ASSERT(_data->buffers.find(handle) == _data->buffers.end());

  [[maybe_unused]] auto [it, emplaced] = _data->buffers.try_emplace(handle, desc);
  NTF_ASSERT(emplaced);

  return handle;
}

void r_context_view::destroy(r_buffer_handle buff) noexcept {
  if (!buff) {
    return;
  }
  auto it = _data->buffers.find(buff);
  if (it == _data->buffers.end()) {
    return;
  }

  _data->platform->destroy_buffer(buff);
  _data->buffers.erase(it);
}

r_expected<void> r_context_view::buffer_update(
  r_buffer_handle buf,
  const r_buffer_data& desc
) noexcept {
  RET_ERROR_IF(!desc.data,
               "[ntf::r_context::buffer_update]",
               "Invalid buffer data");

  RET_ERROR_IF(!buf,
               "[ntf::r_context::buffer_update]",
               "Invalid handle");

  auto it = _data->buffers.find(buf);
  RET_ERROR_IF(it == _data->buffers.end(),
               "[ntf::r_context::buffer_update]",
               "Invalid handle");
  
  auto& buffer = it->second;
  RET_ERROR_IF(!+(buffer.flags & r_buffer_flag::dynamic_storage),
               "[ntf::r_context::buffer_update]",
               "Can't update non dynamic buffer");

  RET_ERROR_IF(desc.size+desc.offset > buffer.size,
               "[ntf::r_context::buffer_update]",
               "Invalid buffer data offset");

  try {
    _data->platform->update_buffer(buf, desc);
  } 
  RET_ERROR_CATCH("[ntf::r_context::buffer_update]",
                  "Failed to update buffer");

  return {};
}

void r_context_view::buffer_update(
  unchecked_t,
  r_buffer_handle buf,
  const r_buffer_data& des
) {
  NTF_ASSERT(buf);
  NTF_ASSERT(_data->buffers.find(buf) != _data->buffers.end());
  _data->platform->update_buffer(buf, des);
}

r_expected<void*> r_context_view::buffer_map(r_buffer_handle buf, size_t offset, size_t len) {
  RET_ERROR_IF(!buf,
               "[ntf::r_context::buffer_map]",
               "Invalid handle");

  auto it = _data->buffers.find(buf);
  RET_ERROR_IF(it == _data->buffers.end(),
               "[ntf::r_context::buffer_map]",
               "Invalid handle");
  auto& buffer = it->second;
  RET_ERROR_IF(
    !+(buffer.flags & r_buffer_flag::read_mappable) ||
    !+(buffer.flags & r_buffer_flag::write_mappable),
   "[ntf::r_context::buffer_map]",
    "Non mappable buffer"
  );
  RET_ERROR_IF(offset+len <= buffer.size,
               "[ntf::r_context::buffer_map]",
               "Invalid mapping size");
  void* ptr = nullptr;
  try {
    ptr = _data->platform->map_buffer(buf, offset, len);
  }
  RET_ERROR_CATCH("[ntf::r_context::buffer_map]",
                  "Failed to map buffer");
  RET_ERROR_IF(!ptr,
               "[ntf::r_context::buffer_map]",
               "Failed to map buffer");
  return ptr;
}

void* r_context_view::buffer_map(unchecked_t, r_buffer_handle buf, size_t offset, size_t len) {
  NTF_ASSERT(buf);
  NTF_ASSERT(_data->buffers.find(buf) != _data->buffers.end());
  return _data->platform->map_buffer(buf, offset, len);
}

void r_context_view::buffer_unmap(r_buffer_handle buf, void* ptr) {
  if (!buf || !ptr) {
    return;
  }
  auto it = _data->buffers.find(buf);
  if (it == _data->buffers.end()) {
    return;
  }
  _data->platform->unmap_buffer(buf, ptr);
}

r_buffer_type r_context_view::buffer_type(
  r_buffer_handle buff
) const {
  NTF_ASSERT(buff);
  NTF_ASSERT(_data->buffers.find(buff) != _data->buffers.end());
  return _data->buffers.at(buff).type;
}

size_t r_context_view::buffer_size(
  r_buffer_handle buff
) const {
  NTF_ASSERT(buff);
  NTF_ASSERT(_data->buffers.find(buff) != _data->buffers.end());
  return _data->buffers.at(buff).size;
}

r_expected<r_texture_handle> r_context_view::texture_create(
  const r_texture_descriptor& desc
) noexcept {
  auto ctx_meta = _data->platform->query_meta();
  RET_ERROR_IF(desc.layers > ctx_meta.tex_max_layers,
               "[ntf::r_context::texture_create]",
               "Texture layers to high ({} > {})",
               desc.layers, ctx_meta.tex_max_layers);

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
      const auto max_ext = ctx_meta.tex_max_extent;
      RET_ERROR_IF(desc.extent.x > max_ext,
                   "[ntf::r_context::texture_create]",
                   "Requested texture is too big ({} > {})",
                   desc.extent.x, ctx_meta.tex_max_extent);
      break;
    }
    case r_texture_type::cubemap: [[fallthrough]];
    case r_texture_type::texture2d: {
      const auto max_ext = ctx_meta.tex_max_extent;
      RET_ERROR_IF(desc.extent.x > max_ext ||
                   desc.extent.y > max_ext,
                   "[ntf::r_context::texture_create]",
                   "Requested texture is too big ({}x{} > {}x{})",
                   desc.extent.x, desc.extent.y, max_ext, max_ext);
      break;
    }
    case r_texture_type::texture3d: {
      const auto max_ext = ctx_meta.tex_max_extent3d;
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
    handle = _data->platform->create_texture(desc);
    NTF_ASSERT(handle);
  } 
  RET_ERROR_CATCH("[ntf::r_context::texture_create]",
                  "Failed to create texture handle");

  NTF_ASSERT(_data->textures.find(handle) == _data->textures.end());

  auto [it, emplaced] = _data->textures.try_emplace(handle, desc);
  NTF_ASSERT(emplaced);

  return handle;
}

r_texture_handle r_context_view::texture_create(
  unchecked_t,
  const r_texture_descriptor& desc
) {
  auto handle = _data->platform->create_texture(desc);
  NTF_ASSERT(handle);
  NTF_ASSERT(_data->textures.find(handle) == _data->textures.end());

  [[maybe_unused]] auto [it, emplaced] = _data->textures.try_emplace(handle, desc);
  NTF_ASSERT(emplaced);

  return handle;
}

void r_context_view::destroy(
  r_texture_handle tex
) noexcept {
  if (!tex) {
    return;
  }

  auto it = _data->textures.find(tex);
  if (it == _data->textures.end()) {
    return;
  }

  if (--it->second.refcount > 0) {
    return;
  }

  _data->platform->destroy_texture(tex);
  _data->textures.erase(it);
}

r_expected<void> r_context_view::texture_update(
  r_texture_handle tex,
  const r_texture_data& data
) noexcept {
  RET_ERROR_IF(!tex,
               "[ntf::r_context::texture_update]",
               "Invalid handle");

  auto it = _data->textures.find(tex);
  RET_ERROR_IF(it == _data->textures.end(),
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
    _data->platform->update_texture(tex, data);
  } 
  RET_ERROR_CATCH("[ntf::r_context::texture_update]",
                  "Failed to update texture");

  if (data.addressing) {
    texture.addressing = *data.addressing;
  }
  if (data.sampler) {
    texture.sampler = *data.sampler;
  }

  return {};
}

void r_context_view::texture_update(
  unchecked_t,
  r_texture_handle tex,
  const r_texture_data& data
) {
  NTF_ASSERT(tex);

  auto it = _data->textures.find(tex);
  NTF_ASSERT(it != _data->textures.end());

  auto& texture = it->second;
  _data->platform->update_texture(tex, data);

  if (data.addressing) {
    texture.addressing = *data.addressing;
  }
  if (data.sampler) {
    texture.sampler = *data.sampler;
  }
}

r_expected<void> r_context_view::texture_update(
  r_texture_handle tex,
  span_view<r_image_data> images,
  bool mips
) noexcept {
  return texture_update(tex, r_texture_data{
    .images = images,
    .gen_mipmaps = mips,
  });
}

void r_context_view::texture_update(
  unchecked_t,
  r_texture_handle tex,
  span_view<r_image_data> images,
  bool mips
) {
  texture_update(::ntf::unchecked, tex, r_texture_data{
    .images = images,
    .gen_mipmaps = mips,
  });
}

r_expected<void> r_context_view::texture_sampler(
  r_texture_handle tex,
  r_texture_sampler sampler
) noexcept {
  return texture_update(tex, r_texture_data{
    .sampler = sampler,
  });
}

void r_context_view::texture_sampler(
  unchecked_t,
  r_texture_handle tex,
  r_texture_sampler sampler
) {
  texture_update(::ntf::unchecked, tex, r_texture_data{
    .sampler = sampler,
  });
}

r_expected<void> r_context_view::texture_addressing(
  r_texture_handle tex,
  r_texture_address addressing
) noexcept {
  return texture_update(tex, r_texture_data{
    .addressing = addressing,
  });
}

void r_context_view::texture_addressing(
  unchecked_t,
  r_texture_handle tex,
  r_texture_address addressing
) {
  texture_update(::ntf::unchecked, tex, r_texture_data{
    .addressing = addressing,
  });
}

r_texture_type r_context_view::texture_type(
  r_texture_handle tex
) const {
  NTF_ASSERT(tex);
  NTF_ASSERT(_data->textures.find(tex) != _data->textures.end());
  return _data->textures.at(tex).type;
}

r_texture_format r_context_view::texture_format(
  r_texture_handle tex
) const {
  NTF_ASSERT(tex);
  NTF_ASSERT(_data->textures.find(tex) != _data->textures.end());
  return _data->textures.at(tex).format;
}

r_texture_sampler r_context_view::texture_sampler(
  r_texture_handle tex
) const {
  NTF_ASSERT(tex);
  NTF_ASSERT(_data->textures.find(tex) != _data->textures.end());
  return _data->textures.at(tex).sampler;
}

r_texture_address r_context_view::texture_addressing(
  r_texture_handle tex
) const {
  NTF_ASSERT(tex);
  NTF_ASSERT(_data->textures.find(tex) != _data->textures.end());
  return _data->textures.at(tex).addressing;
}

uvec3 r_context_view::texture_extent(
  r_texture_handle tex
) const {
  NTF_ASSERT(tex);
  NTF_ASSERT(_data->textures.find(tex) != _data->textures.end());
  return _data->textures.at(tex).extent;
}

uint32 r_context_view::texture_layers(
  r_texture_handle tex
) const {
  NTF_ASSERT(tex);
  NTF_ASSERT(_data->textures.find(tex) != _data->textures.end());
  return _data->textures.at(tex).layers;
}

uint32 r_context_view::texture_levels(
  r_texture_handle tex
) const {
  NTF_ASSERT(tex);
  NTF_ASSERT(_data->textures.find(tex) != _data->textures.end());
  return _data->textures.at(tex).levels;
}

void r_context_view::_populate_attachments(
  r_context_data::framebuffer_store& fb,
  const r_framebuffer_descriptor& desc,
  r_framebuffer_handle handle
) {
  for (uint32 i = 0; i < desc.attachments.size(); ++i) {
    fb.attachments.push_back(desc.attachments[i]);
    auto& tex = _data->textures.at(desc.attachments[i].handle);
    tex.refcount++;
  }

  _data->draw_lists[handle] = {};
  auto& list = _data->draw_lists.at(handle);
  list.viewport = desc.viewport;
  list.color = desc.clear_color;
  list.clear = desc.clear_flags;
}

r_expected<r_framebuffer_handle> r_context_view::framebuffer_create(
  const r_framebuffer_descriptor& desc
) noexcept {
  RET_ERROR_IF(+(desc.test_buffers & r_test_buffer_flag::none) && !desc.test_buffer_format,
               "[ntf::r_context::framebuffer_create]",
               "Invalid test buffer format");

  RET_ERROR_IF(!desc.attachments && !desc.color_buffer_format,
               "[ntf::r_context::framebuffer_create]",
               "Invalid color buffer format");

  for (uint32 i = 0; i < desc.attachments.size(); ++i) {
    const auto& att = desc.attachments[i];
    RET_ERROR_IF(!att.handle || (_data->textures.find(att.handle) == _data->textures.end()),
                 "[ntf::r_context::framebuffer_create]",
                 "Invalid texture handle at index {}",
                 i);

    const auto& tex = _data->textures.at(att.handle);
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
    handle = _data->platform->create_framebuffer(desc);
    NTF_ASSERT(handle);
  }
  RET_ERROR_CATCH("[ntf::r_context::framebuffer_create]",
                  "Failed to create framebuffer handle");

  NTF_ASSERT(_data->framebuffers.find(handle) == _data->framebuffers.end());

  [[maybe_unused]] auto [it, emplaced] = _data->framebuffers.try_emplace(handle, desc);
  NTF_ASSERT(emplaced);

  _populate_attachments(it->second, desc, handle);

  return handle;
}

r_framebuffer_handle r_context_view::framebuffer_create(
  unchecked_t,
  const r_framebuffer_descriptor& desc
) {
  auto handle = _data->platform->create_framebuffer(desc);
  NTF_ASSERT(handle);
  NTF_ASSERT(_data->framebuffers.find(handle) == _data->framebuffers.end());

  [[maybe_unused]] auto [it, emplaced] = _data->framebuffers.try_emplace(handle, desc);
  NTF_ASSERT(emplaced);

  _populate_attachments(it->second, desc, handle);

  return handle;
}

void r_context_view::destroy(
  r_framebuffer_handle fbo
) noexcept {
  if (!fbo) {
    return;
  }
  auto it = _data->framebuffers.find(fbo);
  if (it == _data->framebuffers.end()) {
    return;
  }

  _data->platform->destroy_framebuffer(fbo);
  for (auto att : it->second.attachments) {
    destroy(att.handle); // decreases refcount and maybe destroys
  }
  _data->framebuffers.erase(it);
}

void r_context_view::framebuffer_clear(
  r_framebuffer_handle fbo,
  r_clear_flag flags
) {
  NTF_ASSERT(_data->draw_lists.find(fbo) != _data->draw_lists.end());
  _data->draw_lists.at(fbo).clear = flags;
}

r_clear_flag r_context_view::framebuffer_clear(
  r_framebuffer_handle fbo
) const {
  NTF_ASSERT(_data->draw_lists.find(fbo) != _data->draw_lists.end());
  return _data->draw_lists.at(fbo).clear;
}

void r_context_view::framebuffer_viewport(
  r_framebuffer_handle fbo,
  uvec4 vp
) {
  NTF_ASSERT(_data->draw_lists.find(fbo) != _data->draw_lists.end());
  _data->draw_lists.at(fbo).viewport = vp;
}

uvec4 r_context_view::framebuffer_viewport(
  r_framebuffer_handle fbo
) const {
  NTF_ASSERT(_data->draw_lists.find(fbo) != _data->draw_lists.end());
  return _data->draw_lists.at(fbo).viewport;
}

void r_context_view::framebuffer_color(r_framebuffer_handle fbo, color4 color) {
  NTF_ASSERT(_data->draw_lists.find(fbo) != _data->draw_lists.end());
  _data->draw_lists.at(fbo).color = color;
}

color4 r_context_view::framebuffer_color(
  r_framebuffer_handle fbo
) const {
  NTF_ASSERT(_data->draw_lists.find(fbo) != _data->draw_lists.end());
  return _data->draw_lists.at(fbo).color;
}

r_expected<r_shader_handle> r_context_view::shader_create(
  const r_shader_descriptor& desc
) noexcept {
  r_shader_handle handle{};
  try {
    handle = _data->platform->create_shader(desc);
    NTF_ASSERT(handle);
  }
  RET_ERROR_CATCH("[ntf::r_context::shader_create]",
                  "Failed to create shader handle");

  [[maybe_unused]] auto [it, emplaced] = _data->shaders.try_emplace(handle, desc);
  NTF_ASSERT(emplaced);

  return handle;
}

r_shader_handle r_context_view::shader_create(
  unchecked_t,
  const r_shader_descriptor& desc
) {
  auto handle = _data->platform->create_shader(desc);
  NTF_ASSERT(handle);
  NTF_ASSERT(_data->shaders.find(handle) == _data->shaders.end());

  [[maybe_unused]] auto [it, emplaced] = _data->shaders.try_emplace(handle, desc);
  NTF_ASSERT(emplaced);

  return handle;
}

void r_context_view::destroy(
  r_shader_handle shader
) noexcept {
  if (!shader) {
    return;
  }

  auto it = _data->shaders.find(shader);
  if (it == _data->shaders.end()) {
    return;
  }

  _data->platform->destroy_shader(shader);
  _data->shaders.erase(it);
}

r_shader_type r_context_view::shader_type(
  r_shader_handle shader
) const {
  NTF_ASSERT(shader);
  NTF_ASSERT(_data->shaders.find(shader) != _data->shaders.end());
  return _data->shaders.at(shader).type;
}

auto r_context_view::_copy_pipeline_layout(
  const r_pipeline_descriptor& desc
) -> std::unique_ptr<r_context_data::vertex_layout> {
  auto layout = std::make_unique<r_context_data::vertex_layout>();
  layout->binding = desc.attrib_binding->binding;
  layout->stride = desc.attrib_binding->stride;
  layout->descriptors.resize(desc.attrib_desc.size());
  std::memcpy(
    layout->descriptors.data(), desc.attrib_desc.data(),
    desc.attrib_desc.size()*sizeof(r_attrib_descriptor)
  );
  return layout;
}

r_expected<r_pipeline_handle> r_context_view::pipeline_create(
  const r_pipeline_descriptor& desc
) noexcept {
  // TODO: validation
  auto layout = _copy_pipeline_layout(desc);
  r_context_data::uniform_map uniforms;
  r_pipeline_handle handle{};
  try {
    _data->platform->create_pipeline(desc, layout.get(), uniforms);
    NTF_ASSERT(handle);
  }
  RET_ERROR_CATCH("[ntf::r_context::pipeline_create]",
                  "Failed to create pipeline");

  r_stages_flag stages{}; // TODO: parse stages
  [[maybe_unused]] auto [it, emplaced] = _data->pipelines.try_emplace(
    handle, desc, std::move(layout), std::move(uniforms), stages
  );
  NTF_ASSERT(emplaced);

  return handle;
}

r_pipeline_handle r_context_view::pipeline_create(
  unchecked_t,
  const r_pipeline_descriptor& desc
) {
  auto layout = _copy_pipeline_layout(desc);
  r_context_data::uniform_map uniforms;

  auto handle = _data->platform->create_pipeline(desc, layout.get(), uniforms);
  NTF_ASSERT(handle);
  NTF_ASSERT(_data->pipelines.find(handle) == _data->pipelines.end());

  r_stages_flag stages{}; // TODO: parse stages
  [[maybe_unused]] auto [it, emplaced] = _data->pipelines.try_emplace(
    handle, desc, std::move(layout), std::move(uniforms), stages
  );
  NTF_ASSERT(emplaced);

  return handle;
}


void r_context_view::destroy(
  r_pipeline_handle pipeline
) noexcept {
  if (!pipeline) {
    return;
  }

  auto it = _data->pipelines.find(pipeline);
  if (it == _data->pipelines.end()) {
    return;
  }

  _data->platform->destroy_pipeline(pipeline);
  _data->pipelines.erase(it);
}

r_stages_flag r_context_view::pipeline_stages(
  r_pipeline_handle pipeline
) const {
  NTF_ASSERT(pipeline);
  NTF_ASSERT(_data->pipelines.find(pipeline) != _data->pipelines.end());
  return _data->pipelines.at(pipeline).stages;
}

optional<r_uniform> r_context_view::pipeline_uniform(
  r_pipeline_handle pipeline,
  std::string_view name
) const noexcept {
  if (!pipeline) {
    return nullopt;
  }
  auto it = _data->pipelines.find(pipeline);
  if (it == _data->pipelines.end()) {
    return nullopt;
  }

  const auto& pip = it->second;
  auto unif_it = pip.uniforms.find(name.data());
  if (unif_it == pip.uniforms.end()) {
    return nullopt;
  }

  return unif_it->second;
}

r_uniform r_context_view::pipeline_uniform(
  unchecked_t,
  r_pipeline_handle pipeline,
  std::string_view name
) const {
  NTF_ASSERT(pipeline);

  auto it = _data->pipelines.find(pipeline);
  NTF_ASSERT(it != _data->pipelines.end());

  const auto& pip = it->second;
  auto unif_it = pip.uniforms.find(name.data());
  NTF_ASSERT(unif_it != pip.uniforms.end());

  return unif_it->second;
}

} // namespace ntf
