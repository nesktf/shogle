#include "./context.hpp"
#include "./internal/interface.hpp"
#include "./internal/opengl.hpp"
#include "./pipeline.hpp"

#include "../stl/arena.hpp"

#define RET_ERROR(_log_pfx, _fmt, ...) \
  SHOGLE_LOG(error, _log_pfx " " _fmt __VA_OPT__(,) __VA_ARGS__); \
  return unexpected{r_error::format({_fmt} __VA_OPT__(,) __VA_ARGS__)}

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

struct r_texture_ {
  r_texture_(r_context ctx_, r_platform_texture handle_, r_tex_desc&& desc) :
    ctx{ctx_}, handle{handle_},
    refcount{1},
    type{desc.type}, format{desc.format},
    extent{desc.extent},
    levels{desc.levels}, layers{desc.layers},
    addressing{desc.addressing}, sampler{desc.sampler} {}

  r_context ctx;
  r_platform_texture handle;

  std::atomic<uint32> refcount;
  r_texture_type type;
  r_texture_format format;
  extent3d extent;
  uint32 levels;
  uint32 layers;
  r_texture_address addressing;
  r_texture_sampler sampler;
};

struct r_buffer_ {
  r_buffer_(r_context ctx_, r_platform_buffer handle_, r_buff_desc&& desc) :
    ctx{ctx_}, handle{handle_},
    type{desc.type}, flags{desc.flags}, size{desc.size} {}

  r_context ctx;
  r_platform_buffer handle;

  r_buffer_type type;
  r_buffer_flag flags;
  size_t size;
};

struct r_shader_ {
  r_shader_(r_context ctx_, r_platform_shader handle_, const r_shader_descriptor& desc) :
    ctx{ctx_}, handle{handle_},
    type{desc.type} {}

  r_context ctx;
  r_platform_shader handle;

  r_shader_type type;
};

struct r_uniform_ {
  r_uniform_(r_pipeline pip, r_platform_uniform location_,
             std::string name_, r_attrib_type type_, size_t size_) :
    pipeline{pip}, location{location_},
    name{std::move(name_)}, type{type_}, size{size_} {}

  r_pipeline pipeline;
  r_platform_uniform location;
  std::string name;
  r_attrib_type type;
  size_t size;
};

struct r_pipeline_ {
  r_pipeline_(r_context ctx_, r_platform_pipeline handle_, const r_pipeline_descriptor& desc,
              std::unique_ptr<vertex_layout>&& layout_, uniform_map&& uniforms_,
              r_stages_flag stages_) :
    ctx{ctx_}, handle{handle_},
    stages{stages_},
    primitive{desc.primitive}, poly_mode{desc.poly_mode},
    layout{std::move(layout_)}, uniforms{std::move(uniforms_)} {}

  r_context ctx;
  r_platform_pipeline handle;

  r_stages_flag stages;
  r_primitive primitive;
  r_polygon_mode poly_mode;

  std::unique_ptr<vertex_layout> layout;

  uniform_map uniforms;
};

struct r_framebuffer_ {
  r_framebuffer_(r_context ctx_, r_platform_fbo handle_,
                 extent2d extent_, r_test_buffer test_buffer_,
                 std::vector<r_framebuffer_attachment>&& attachments_) :
    ctx{ctx_}, handle{handle_},
    extent{extent_},
    test_buffer{test_buffer_},
    attachments{std::move(attachments_)} {}

  r_framebuffer_(r_context ctx_, r_platform_fbo handle_,
                 extent2d extent_, r_test_buffer test_buffer_,
                 r_texture_format color_buffer_) :
    ctx{ctx_}, handle{handle_},
    extent{extent_},
    test_buffer{test_buffer_},
    attachments{color_buffer_} {}

  r_framebuffer_(r_context ctx_) :
    ctx{ctx_}, handle{DEFAULT_FBO_HANDLE},
    attachments{std::vector<r_framebuffer_attachment>{}} {}

  r_context ctx;
  r_platform_fbo handle;

  extent2d extent;
  r_test_buffer test_buffer;
  std::variant<std::vector<r_framebuffer_attachment>, r_texture_format> attachments;
};

struct r_context_ {
  r_context_(std::unique_ptr<r_platform_context>&& platform_, command_map&& map,
             win_handle win_, renderer_api api_, linked_arena&& arena,
             const r_allocator& alloc) noexcept :
    api{api_}, win{win_}, platform{std::move(platform_)},
    draw_lists{std::move(map)}, d_cmd{}, alloc{alloc},
    default_fbo{this}, frame_arena{std::move(arena)} {}

  renderer_api api;
  win_handle win;
  std::unique_ptr<r_platform_context> platform;

  handle_map<r_platform_buffer, r_buffer_> buffers;
  handle_map<r_platform_texture, r_texture_> textures;
  handle_map<r_platform_fbo, r_framebuffer_> framebuffers;
  handle_map<r_platform_shader, r_shader_> shaders;
  handle_map<r_platform_pipeline, r_pipeline_> pipelines;

  command_map draw_lists;
  weak_ref<draw_list> d_list;
  draw_command d_cmd;

  r_allocator alloc;
  r_framebuffer_ default_fbo;
  linked_arena frame_arena;
};

static auto load_platform_ctx(
  renderer_api api, win_handle win, uint32 swap_interval
) -> r_expected<std::unique_ptr<r_platform_context>> {
  std::unique_ptr<r_platform_context> ctx;
  try {
    switch (api) {
      case renderer_api::opengl: {
        SHOGLE_GL_MAKE_CTX_CURRENT(win);
        SHOGLE_GL_SET_SWAP_INTERVAL(win, static_cast<int>(swap_interval));
        RET_ERROR_IF(!gladLoadGLLoader(SHOGLE_GL_LOAD_PROC),
                     "[ntf::load_platform_ctx]",
                     "Failed to load GLAD");
        ctx = std::make_unique<gl_context>(win, swap_interval);
        break;
      }
      default: {
        return unexpected{r_error{"Not implemented"}};
        break;
      }
    }
  }
  RET_ERROR_CATCH("[ntf::load_platform_ctx]",
                  "Failed to load platform context");

  // TODO: Use a local imgui context instead of the global one
#if defined(SHOGLE_ENABLE_IMGUI) && SHOGLE_ENABLE_IMGUI
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO();
  // io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
  ImGui::StyleColorsDark();

  switch (api) {
    case renderer_api::opengl: {
      SHOGLE_INIT_IMGUI_OPENGL(win, true, "#version 130");
      break;
    }
    case renderer_api::vulkan: {
      SHOGLE_INIT_IMGUI_VULKAN(win, true);
      break;
    }
    default: {
      SHOGLE_INIT_IMGUI_OTHER(win, true);
      break;
    }
  }
#endif

  return ctx;
}

static r_allocator base_alloc {
  .user_ptr = nullptr,
  .mem_alloc = +[](void*, size_t sz, size_t) -> void* { return std::malloc(sz); },
  .mem_free = +[](void*, void* mem) -> void { std::free(mem); },
  .mem_scratch_alloc = +[](void*, size_t sz, size_t) -> void* { return std::malloc(sz); },
  .mem_scratch_free = +[](void*, void* mem) -> void { std::free(mem); },
};

r_expected<r_context> r_create_context(const r_context_params& params) {
  return load_platform_ctx(params.api, params.window, params.swap_interval)
    .and_then([&](auto&& pctx) -> r_expected<r_context> {
      r_allocator alloc = params.alloc ? *params.alloc : base_alloc;
      r_context ctx = static_cast<r_context>(
        (*alloc.mem_alloc)(alloc.user_ptr, sizeof(r_context_), alignof(r_context_))
      );
      if (!ctx) {
        return unexpected{r_error{"Failed to allocate context"}};
      }

      command_map map; // TODO: use the alloator for this thing
      auto [it, emplaced] = map.try_emplace(DEFAULT_FBO_HANDLE);
      NTF_ASSERT(emplaced);

      auto arena = linked_arena::from_size(mibs(4u));
      if (!arena) {
        return unexpected{std::move(arena.error())};
      }
      std::construct_at(ctx,
                        std::move(pctx), std::move(map), params.window, params.api,
                        std::move(*arena),
                        alloc);
      ctx->d_list = ctx->draw_lists.at(DEFAULT_FBO_HANDLE);
      ctx->d_list->color = params.fb_color;
      ctx->d_list->viewport = params.fb_viewport;
      ctx->d_list->clear = params.fb_clear;
      auto meta = ctx->platform->get_meta();
      SHOGLE_LOG(debug, "[ntf::r_context][CONSTRUCT] {} ver {} [{} - {}]",
                 meta.api == renderer_api::opengl ? "OpenGL" : "Vulkan",
                 meta.version_str, meta.vendor_str, meta.name_str);
      return ctx;
    });
}

void r_destroy_context(r_context ctx) {
  if (!ctx) {
    return;
  }
  r_allocator alloc = ctx->alloc;

#if defined(SHOGLE_ENABLE_IMGUI) && SHOGLE_ENABLE_IMGUI
  switch (ctx->api) {
    case renderer_api::opengl: {
      SHOGLE_DESTROY_IMGUI_OPENGL();
      break;
    }
    case renderer_api::vulkan: {
      SHOGLE_DESTROY_IMGUI_VULKAN();
      break;
    }
    default: {
      SHOGLE_DESTROY_IMGUI_OTHER();
      break;
    }
  }
  ImGui::DestroyContext();
#endif
  ctx->~r_context_();
  (*alloc.mem_free)(alloc.user_ptr, static_cast<void*>(ctx));
  SHOGLE_LOG(debug, "[ntf::r_context][DESTROY]");
}

void r_start_frame(r_context ctx) {
  if (!ctx) {
    return;
  }

  for (auto& [_, list] : ctx->draw_lists) {
    for (auto& cmd : list.cmds) {
      cmd.get().~draw_command();
    }
    list.cmds.clear();
  }
  ctx->frame_arena.clear();

#if defined(SHOGLE_ENABLE_IMGUI) && SHOGLE_ENABLE_IMGUI
  switch (ctx->api) {
    case renderer_api::opengl: {
      SHOGLE_IMGUI_OPENGL_NEW_FRAME();
      break;
    }
    case renderer_api::vulkan: {
      SHOGLE_IMGUI_VULKAN_NEW_FRAME();
      break;
    }
    default: {
      SHOGLE_IMGUI_OTHER_NEW_FRAME();
      break;
    }
  }
  ImGui::NewFrame();
#endif
}

void r_end_frame(r_context ctx) {
  if (!ctx) {
    return;
  }
  ctx->platform->submit(ctx->draw_lists);
#if defined(SHOGLE_ENABLE_IMGUI) && SHOGLE_ENABLE_IMGUI
  ImGui::Render();
  switch (ctx->api) {
    case renderer_api::opengl: {
      SHOGLE_IMGUI_OPENGL_END_FRAME(ImGui::GetDrawData());
      break;
    }
    case renderer_api::vulkan: {
      SHOGLE_IMGUI_VULKAN_END_FRAME(ImGui::GetDrawData());
      break;
    }
    default: {
      SHOGLE_IMGUI_OTHER_END_FRAME(ImGui::GetDrawData());
    }
  }
#endif
  ctx->platform->swap_buffers();
}

void r_device_wait(r_context ctx) {
  if (!ctx) {
    return;
  }
  ctx->platform->device_wait();
}

void r_submit_command(r_context ctx, const r_draw_command& cmd) {
  if (!ctx) {
    return;
  }

  ctx->d_list = ctx->draw_lists.at(cmd.target->handle);
  ctx->d_cmd.pipeline = cmd.pipeline;

  ctx->d_cmd.count = cmd.draw_opts.count;
  ctx->d_cmd.offset = cmd.draw_opts.offset;
  ctx->d_cmd.instances = cmd.draw_opts.instances;

  if (cmd.on_render) {
    ctx->d_cmd.on_render = [ctx, on_render=cmd.on_render]() { on_render(ctx); };
  } else {
    ctx->d_cmd.on_render = {};
  }

  for (const auto& buff : cmd.buffers) {
    auto* ptr = ctx->frame_arena.construct<r_buffer_binding>(buff);
    ctx->d_cmd.buffers.emplace_back(ptr);
  }

  for (const auto& tex : cmd.textures) {
    auto* ptr = ctx->frame_arena.construct<texture_binding>(tex.texture, tex.location);
    ctx->d_cmd.textures.emplace_back(ptr);
  }

  for (const auto& unif : cmd.uniforms) {
    auto* data = ctx->frame_arena.allocate(unif.data.size, unif.data.alignment);
    std::memcpy(data, unif.data.data, unif.data.size);

    auto* desc = ctx->frame_arena.construct<uniform_descriptor>(unif.data.type,
                                                                unif.uniform->location,
                                                                data, unif.data.size);
    ctx->d_cmd.uniforms.emplace_back(desc);
  }

  auto* lcmd = ctx->frame_arena.construct<draw_command>(std::move(ctx->d_cmd));
  ctx->d_list->cmds.emplace_back(lcmd);
  ctx->d_cmd = {};
}

r_framebuffer r_get_default_framebuffer(r_context ctx) {
  return &ctx->default_fbo;
}

static auto transform_descriptor(
  r_context, const r_buffer_descriptor& desc
) -> r_buff_desc {
  return {
    .type = desc.type,
    .flags = desc.flags,
    .size = desc.size,
    .initial_data = desc.data,
  };
}

static auto check_and_transform_descriptor(
  r_context ctx, const r_buffer_descriptor& desc
) -> r_expected<r_buff_desc> {
  RET_ERROR_IF(!ctx,
               "[ntf::r_create_buffer]",
               "Invalid context handle");

  if (desc.data) {
    RET_ERROR_IF(!desc.data->data,
                 "[ntf::r_create_buffer]",
                 "Invalid buffer data");

    RET_ERROR_IF(desc.data->size+desc.data->offset > desc.size,
                 "[ntf::r_create_buffer]",
                 "Invalid buffer data offset");
  } else {
    RET_ERROR_IF(!+(desc.flags & r_buffer_flag::dynamic_storage),
                 "[ntf::r_create_buffer]",
                 "Attempted to create non dynamic buffer with no data");
  }

  return transform_descriptor(ctx, desc);
}

r_expected<r_buffer> r_create_buffer(r_context ctx, const r_buffer_descriptor& desc) {
  return check_and_transform_descriptor(ctx, desc)
  .and_then([ctx](r_buff_desc&& buff_desc) -> r_expected<r_buffer> {
    r_platform_buffer handle;
    try {
      handle = ctx->platform->create_buffer(buff_desc);
      RET_ERROR_IF(!handle,
                   "[ntf::r_create_buffer]",
                   "Failed to create buffer");
    }
    RET_ERROR_CATCH("[ntf::r_create_buffer]",
                    "Failed to create buffer");

    [[maybe_unused]] auto [it, emplaced] = ctx->buffers.try_emplace(
      handle, ctx, handle, std::move(buff_desc)
    );
    NTF_ASSERT(emplaced);
    return &it->second;
  });
}

r_buffer r_create_buffer(unchecked_t, r_context ctx, const r_buffer_descriptor& desc) {
  NTF_ASSERT(ctx);
  auto pdesc = transform_descriptor(ctx, desc);
  auto handle = ctx->platform->create_buffer(pdesc);
  NTF_ASSERT(handle);

  [[maybe_unused]] auto [it, emplaced] = ctx->buffers.try_emplace(
    handle, ctx, handle, std::move(pdesc)
  );
  NTF_ASSERT(emplaced);

  return &it->second;
}

void r_destroy_buffer(r_buffer buffer) {
  if (!buffer) {
    return;
  }

  const auto handle = buffer->handle;
  auto* ctx = buffer->ctx;
  auto it = ctx->buffers.find(handle);
  if (it == ctx->buffers.end()) {
    return;
  }

  ctx->platform->destroy_buffer(handle);
  ctx->buffers.erase(it);
}

static void upload_buffer_data(
  r_buffer buffer, size_t offset, size_t len, const void* data
) {
  NTF_ASSERT(buffer);
  NTF_ASSERT(data);
  r_buff_data desc;
  desc.data = data;
  desc.len = len;
  desc.offset = offset;
  buffer->ctx->platform->update_buffer(buffer->handle, desc);
}

r_expected<void> r_buffer_upload(r_buffer buffer, size_t offset, size_t len, const void* data) {
  RET_ERROR_IF(!buffer,
               "[ntf::r_buffer_upload]",
               "Invalid buffer handle");

  RET_ERROR_IF(!data,
               "[ntf::r_buffer_upload]",
               "Invalid buffer data");

  RET_ERROR_IF(!+(buffer->flags & r_buffer_flag::dynamic_storage),
               "[ntf::r_buffer_upload]",
               "Can't update non dynamic buffer");

  RET_ERROR_IF(len+offset > buffer->size,
               "[ntf::r_buffer_upload]",
               "Invalid buffer data offset");

  try {
    upload_buffer_data(buffer, offset, len, data);
  }
  RET_ERROR_CATCH("[ntf::r_buffer_upload]",
                  "Failed to update buffer");

  return {};
}

void r_buffer_upload(unchecked_t, r_buffer buffer, size_t offset, size_t len,
                     const void* data) {
  upload_buffer_data(buffer, offset, len, data);
}

void* map_buffer(r_buffer buffer, size_t offset, size_t len) {
  NTF_ASSERT(buffer);
  r_buff_mapping mapping;
  mapping.len = len;
  mapping.offset = offset;
  return buffer->ctx->platform->map_buffer(buffer->handle, mapping);
}

r_expected<void*> r_buffer_map(r_buffer buffer, size_t offset, size_t len) {
  RET_ERROR_IF(!buffer,
               "[ntf::r_buffer_map]",
               "Invalid buffer handle");

  RET_ERROR_IF(
    !+(buffer->flags & r_buffer_flag::read_mappable) ||
    !+(buffer->flags & r_buffer_flag::write_mappable),
   "[ntf::r_buffer_map]",
    "Non mappable buffer"
  );
  RET_ERROR_IF(offset+len > buffer->size,
               "[ntf::r_buffer_map]",
               "Invalid mapping size");
  void* ptr = nullptr;
  try {
    ptr = map_buffer(buffer, offset, len);
  }
  RET_ERROR_CATCH("[ntf::r_buffer_map]",
                  "Failed to map buffer");
  RET_ERROR_IF(!ptr,
               "[ntf::r_buffer_map]",
               "Failed to map buffer");
  return ptr;
}

void* r_buffer_map(unchecked_t, r_buffer buffer, size_t offset, size_t len) {
  return map_buffer(buffer, offset, len);
}

void r_buffer_unmap(r_buffer buffer, void* mapped) {
  if (!buffer || !mapped) {
    return;
  }
  buffer->ctx->platform->unmap_buffer(buffer->handle, mapped);
}

r_buffer_type r_buffer_get_type(r_buffer buffer) {
  NTF_ASSERT(buffer);
  return buffer->type;
}

size_t r_buffer_get_size(r_buffer buffer) {
  NTF_ASSERT(buffer);
  return buffer->size;
}

r_context r_buffer_get_ctx(r_buffer buffer) {
  NTF_ASSERT(buffer);
  return buffer->ctx;
}

r_platform_buffer r_buffer_get_handle(r_buffer buffer) {
  NTF_ASSERT(buffer);
  return buffer->handle;
}

static auto transform_descriptor(
  r_context, const r_texture_descriptor& desc
) -> r_tex_desc {
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
) -> r_expected<r_tex_desc> {
  RET_ERROR_IF(!ctx,
               "[ntf::r_create_texture]",
               "Invalid context handle");

  auto ctx_meta = ctx->platform->get_meta();
  RET_ERROR_IF(desc.layers > ctx_meta.tex_max_layers,
               "[ntf::r_create_texture]",
               "Texture layers to high ({} > {})",
               desc.layers, ctx_meta.tex_max_layers);

  RET_ERROR_IF(desc.type == r_texture_type::cubemap && desc.extent.x != desc.extent.y,
               "[ntf::r_create_texture]",
               "Invalid cubemap extent");

  RET_ERROR_IF(desc.type == r_texture_type::texture3d && desc.layers > 1,
               "[ntf::r_create_texture]",
               "Invalid layers for texture3d");

  RET_ERROR_IF(desc.type == r_texture_type::cubemap && desc.layers != 6,
               "[ntf::r_create_texture]",
               "Invalid layers for cubemap");

  RET_ERROR_IF(desc.levels > 7 || desc.levels == 0,
               "[ntf::r_create_texture]",
               "Invalid texture level \"{}\"",
               desc.levels);

  if (desc.gen_mipmaps) {
    if (!desc.images) {
      SHOGLE_LOG(warning, "[ntf::r_create_texture] "
                 "Ignoring mipmap generation for texture with no image data");
    }
    if (desc.levels == 1) {
      SHOGLE_LOG(warning, "[ntf::r_create_texture] "
                 "Ignoring mipmap generation for texture with level 1");
    }
  }

  switch (desc.type) {
    case r_texture_type::texture1d: {
      const auto max_ext = ctx_meta.tex_max_extent;
      RET_ERROR_IF(desc.extent.x > max_ext,
                   "[ntf::r_create_texture]",
                   "Requested texture is too big ({} > {})",
                   desc.extent.x, ctx_meta.tex_max_extent);
      break;
    }
    case r_texture_type::cubemap: [[fallthrough]];
    case r_texture_type::texture2d: {
      const auto max_ext = ctx_meta.tex_max_extent;
      RET_ERROR_IF(desc.extent.x > max_ext ||
                   desc.extent.y > max_ext,
                   "[ntf::r_create_texture]",
                   "Requested texture is too big ({}x{} > {}x{})",
                   desc.extent.x, desc.extent.y, max_ext, max_ext);
      break;
    }
    case r_texture_type::texture3d: {
      const auto max_ext = ctx_meta.tex_max_extent3d;
      RET_ERROR_IF(desc.extent.x > max_ext ||
                   desc.extent.y > max_ext ||
                   desc.extent.z > max_ext,
                   "[ntf::r_create_texture]",
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
                 "[ntf::r_create_texture]",
                 "Invalid image at index {}",
                 i);
    switch (desc.type) {
      case r_texture_type::texture1d: {
        RET_ERROR_IF(upload_extent.x > desc.extent.x,
                     "[ntf::r_create_texture]",
                     "Invalid image extent at index {}",
                     i);
        break;
      }
      case r_texture_type::cubemap: [[fallthrough]];
      case r_texture_type::texture2d: {
        RET_ERROR_IF(upload_extent.x > desc.extent.x ||
                      upload_extent.y > desc.extent.y,
                     "[ntf::r_create_texture]",
                     "Invalid image extent at index {}",
                     i);
        break;
      }
      case r_texture_type::texture3d: {
        RET_ERROR_IF(upload_extent.x > desc.extent.x ||
                      upload_extent.y > desc.extent.y ||
                      upload_extent.z > desc.extent.z,
                     "[ntf::r_create_texture]",
                     "Invalid image extent at index {}",
                     i);
        break;
      }
    }
  }

  return transform_descriptor(ctx, desc);
}

r_expected<r_texture> r_create_texture(r_context ctx, const r_texture_descriptor& desc) {
  return check_and_transform_descriptor(ctx, desc)
  .and_then([ctx](r_tex_desc&& tex_desc) -> r_expected<r_texture> {
    r_platform_texture handle;
    try {
      handle = ctx->platform->create_texture(tex_desc);
      RET_ERROR_IF(!handle,
                   "[ntf::r_create_texture]",
                   "Failed to create texture handle");
    } 
    RET_ERROR_CATCH("[ntf::r_context::texture_create]",
                    "Failed to create texture handle");

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
    r_tex_image_data image;
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
  r_tex_opts opts;
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
  for (uint32 i = 0; i < images.size(); ++i) {
    const auto& img = images[i];
    const uvec3 upload_extent = img.offset+img.extent;

    RET_ERROR_IF(!img.texels ||
                 img.layer > tex->layers ||
                 img.level > tex->levels,
                 "[ntf::r_texture_upload]",
                 "Invalid image at index {}",
                 i);
    switch (tex->type) {
      case r_texture_type::texture1d: {
        RET_ERROR_IF(upload_extent.x > tex->extent.x,
                     "[ntf::r_texture_upload]",
                     "Invalid image extent at index {}",
                     i);
        break;
      }
      case r_texture_type::cubemap: [[fallthrough]];
      case r_texture_type::texture2d: {
        RET_ERROR_IF(upload_extent.x > tex->extent.x ||
                      upload_extent.y > tex->extent.y,
                     "[ntf::r_texture_upload]",
                     "Invalid image extent at index {}",
                     i);
        break;
      }
      case r_texture_type::texture3d: {
        RET_ERROR_IF(upload_extent.x > tex->extent.x ||
                      upload_extent.y > tex->extent.y ||
                      upload_extent.z > tex->extent.z,
                     "[ntf::r_texture_upload]",
                     "Invalid image extent at index {}",
                     i);
        break;
      }
    }
  }
}

r_expected<void> r_texture_upload(r_texture tex, const r_texture_data& data) {
  RET_ERROR_IF(!tex,
               "[ntf::r_texture_upload]",
               "Invalid handle");

  try {
    const bool do_address = data.addressing && *data.addressing != tex->addressing;
    const bool do_sampler = data.sampler && *data.sampler != tex->sampler;
    RET_ERROR_IF(!(do_sampler || do_address) && data.images.empty(),
                 "[ntf::r_texture_upload]",
                 "Invalid update descriptor");
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
  RET_ERROR_CATCH("[ntf::r_texture_upload]",
                  "Failed to update texture");

  return {};
}

void r_texture_upload(unchecked_t, r_texture tex, const r_texture_data& data) {
  NTF_ASSERT(tex);
  update_texture_images(tex, data.images);
  update_texture_opts(tex, data.sampler, data.addressing, data.gen_mipmaps);
}

r_expected<void> r_texture_upload(r_texture tex,
                                  cspan<r_image_data> images, bool gen_mips) {
  if (images.empty()) {
    return {};
  }

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
  RET_ERROR_IF(!tex,
             "[ntf::r_texture_set_sampler]",
             "Invalid handle");
  update_texture_opts(tex, sampler, nullopt, false);
  return {};
}

void r_texture_set_sampler(unchecked_t, r_texture tex, r_texture_sampler sampler) {
  NTF_ASSERT(tex);
  update_texture_opts(tex, sampler, nullopt, false);
}

r_expected<void> r_texture_set_addressing(r_texture tex, r_texture_address adressing) {
  RET_ERROR_IF(!tex,
             "[ntf::r_texture_set_addressing]",
             "Invalid handle");
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

static auto transform_descriptor(
  r_context ctx, const r_framebuffer_descriptor& desc
) -> r_fbo_desc {

  if (std::holds_alternative<cspan<r_framebuffer_attachment>>(desc.attachments)) {
    auto attachments_in = std::get<cspan<r_framebuffer_attachment>>(desc.attachments);
    std::vector<r_framebuffer_attachment> attachments;
    attachments.reserve(attachments_in.size());
  }

};

static auto check_and_transform_descriptor(
  r_context ctx, const r_framebuffer_descriptor& desc
) -> r_expected<r_fbo_desc> {
  RET_ERROR_IF(!ctx,
               "[ntf::r_create_framebuffer]",
               "Invalid context handle");

  if (desc.viewport.x+desc.viewport.z != desc.extent.x ||
      desc.viewport.y+desc.viewport.w != desc.extent.y) {
    SHOGLE_LOG(warning, "[ntf::r_create_framebuffer] Mismatching viewport size");
  }

  if (std::holds_alternative<cspan<r_framebuffer_attachment>>(desc.attachments)) {
    auto attachments_in = std::get<cspan<r_framebuffer_attachment>>(desc.attachments);
    RET_ERROR_IF(attachments_in.empty(),
                 "[ntf::r_create_framebuffer]",
                 "Invalid attachment span");
    for (uint32 i = 0; i < attachments_in.size(); ++i) {
      const auto& att = attachments_in[i];
      r_texture tex = att.handle;
      RET_ERROR_IF(!tex || tex->ctx != ctx,
                   "[ntf::r_create_framebuffer]",
                   "Invalid texture handle at index {}",
                   i);

      RET_ERROR_IF(att.layer > tex->layers,
                   "[ntf::r_create_framebuffer]",
                   "Invalid texture layer at index {}",
                   i);

      RET_ERROR_IF(att.level > tex->levels,
                   "[ntf::r_create_framebuffer]",
                   "Invalid texture level at index {}",
                   i);

      RET_ERROR_IF(tex->extent.x != desc.extent.x || tex->extent.y != desc.extent.y,
                   "[ntf::r_create_framebuffer]",
                   "Invalid texture extent at index {}",
                   i);
    }
  } else {
    RET_ERROR("[ntf::r_create_framebuffer]",
              "Color buffer not implemented");
  }

  return transform_descriptor(ctx, desc);
}

r_expected<r_framebuffer> r_create_framebuffer(r_context ctx,
                                               const r_framebuffer_descriptor& desc) {

  return check_and_transform_descriptor(ctx, desc)
  .and_then([&](r_fbo_desc&& fbo) -> r_expected<r_framebuffer> {
    r_platform_fbo handle{};
    try {
      handle = ctx->platform->create_framebuffer(desc);
      RET_ERROR_IF(!handle,
                   "[ntf::r_create_framebuffer]",
                   "Failed to create framebuffer");
    }
    RET_ERROR_CATCH("[ntf::r_create_framebuffer]",
                    "Failed to create framebuffer");
  });
  for (size_t i = 0; i < desc.attachments.size(); ++i) {
    attachments.push_back(desc.attachments[i]);
    desc.attachments[i].handle->refcount++;
  }

  [[maybe_unused]] auto [it, emplaced] = ctx->framebuffers.try_emplace(
    handle, ctx, handle, desc, std::move(attachments)
  );
  NTF_ASSERT(emplaced);

  ctx->draw_lists.try_emplace(handle);
  auto& list = ctx->draw_lists.at(handle);
  list.viewport = desc.viewport;
  list.color = desc.clear_color;
  list.clear = desc.clear_flags;

  return &it->second;
}

r_framebuffer r_create_framebuffer(unchecked_t, r_context ctx,
                                   const r_framebuffer_descriptor& desc) {
  NTF_ASSERT(ctx);
  std::vector<r_framebuffer_attachment> attachments;
  attachments.reserve(desc.attachments.size());

  auto handle = ctx->platform->create_framebuffer(desc);

  for (size_t i = 0; i < desc.attachments.size(); ++i) {
    attachments.push_back(desc.attachments[i]);
    desc.attachments[i].handle->refcount++;
  }

  [[maybe_unused]] auto [it, emplaced] = ctx->framebuffers.try_emplace(
    handle, ctx, handle, desc, std::move(attachments)
  );
  NTF_ASSERT(emplaced);

  ctx->draw_lists.try_emplace(handle);
  auto& list = ctx->draw_lists.at(handle);
  list.viewport = desc.viewport;
  list.color = desc.clear_color;
  list.clear = desc.clear_flags;

  return &it->second;
}

void r_destroy_framebuffer(r_framebuffer fbo) {
  if (!fbo) {
    return;
  }

  auto* ctx = fbo->ctx;
  if (fbo == &ctx->default_fbo) {
    return;
  }

  const auto handle = fbo->handle;
  auto it = ctx->framebuffers.find(handle);
  if (it == ctx->framebuffers.end()) {
    return;
  }

  ctx->platform->destroy_framebuffer(handle);
  for (auto att : it->second.attachments) {
    r_destroy_texture(att.handle); // decreases refcount and maybe destroys
  }
  ctx->framebuffers.erase(it);
}

void r_framebuffer_set_clear(r_framebuffer fbo, r_clear_flag flags) {
  NTF_ASSERT(fbo);
  fbo->ctx->draw_lists.at(fbo->handle).clear = flags;
}

void r_framebuffer_set_viewport(r_framebuffer fbo, const uvec4& vp) {
  NTF_ASSERT(fbo);
  fbo->ctx->draw_lists.at(fbo->handle).viewport = vp;
}

void r_framebuffer_set_color(r_framebuffer fbo, const color4& color) {
  NTF_ASSERT(fbo);
  fbo->ctx->draw_lists.at(fbo->handle).color = color;
}

r_clear_flag r_framebuffer_get_clear(r_framebuffer fbo) {
  NTF_ASSERT(fbo);
  return fbo->ctx->draw_lists.at(fbo->handle).clear;
}

uvec4 r_frambuffer_get_viewport(r_framebuffer fbo) {
  NTF_ASSERT(fbo);
  return fbo->ctx->draw_lists.at(fbo->handle).viewport;
}

color4 r_framebuffer_get_color(r_framebuffer fbo) {
  NTF_ASSERT(fbo);
  return fbo->ctx->draw_lists.at(fbo->handle).color;
}

r_context r_framebuffer_get_ctx(r_framebuffer fbo) {
  NTF_ASSERT(fbo);
  return fbo->ctx;
}

r_platform_fbo r_framebuffer_get_handle(r_framebuffer fbo) {
  NTF_ASSERT(fbo);
  return fbo->handle;
}

r_expected<r_shader> r_create_shader(r_context ctx, const r_shader_descriptor& desc) {
  r_platform_shader handle{};
  try {
    handle = ctx->platform->create_shader(desc);
    RET_ERROR_IF(!handle,
                 "[ntf::r_create_shader]",
                 "Failed to create shader");
  }
  RET_ERROR_CATCH("[ntf::r_create_shader]",
                  "Failed to create shader");

  [[maybe_unused]] auto [it, emplaced] = ctx->shaders.try_emplace(
    handle, ctx, handle, desc
  );
  NTF_ASSERT(emplaced);

  return &it->second;
}

r_shader r_create_shader(unchecked_t, r_context ctx, const r_shader_descriptor& desc) {
  NTF_ASSERT(ctx);
  auto handle = ctx->platform->create_shader(desc);
  NTF_ASSERT(handle);

  [[maybe_unused]] auto [it, emplaced] = ctx->shaders.try_emplace(
    handle, ctx, handle, desc
  );
  NTF_ASSERT(emplaced);

  return &it->second;
}

void r_destroy_shader(r_shader shader) {
  if (!shader) {
    return;
  }

  const auto handle = shader->handle;
  auto* ctx = shader->ctx;
  auto it = ctx->shaders.find(handle);
  if (it == ctx->shaders.end()) {
    return;
  }

  ctx->platform->destroy_shader(handle);
  ctx->shaders.erase(it);
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

static auto _copy_pipeline_layout(
  const r_pipeline_descriptor& desc
) -> std::unique_ptr<vertex_layout> {
  auto layout = std::make_unique<vertex_layout>();
  layout->binding = desc.attrib_binding->binding;
  layout->stride = desc.attrib_binding->stride;
  layout->descriptors.resize(desc.attrib_desc.size());
  std::memcpy(
    layout->descriptors.data(), desc.attrib_desc.data(),
    desc.attrib_desc.size()*sizeof(r_attrib_descriptor)
  );
  return layout;
}

r_expected<r_pipeline> r_create_pipeline(r_context ctx, const r_pipeline_descriptor& desc) {
  // TODO: validation
  RET_ERROR_IF(!ctx,
               "[ntf::r_create_pipeline]",
               "Invalid context handle");

  std::unique_ptr<vertex_layout> layout = _copy_pipeline_layout(desc);
  uniform_map uniforms;

  r_platform_pipeline handle{};
  try {
    handle = ctx->platform->create_pipeline(desc, layout.get(), uniforms);
    RET_ERROR_IF(!handle,
                 "[ntf::r_create_pipeline]",
                 "Failed to create pipeline");
  }
  RET_ERROR_CATCH("[ntf::r_create_pipeline]",
                  "Failed to create pipeline");

  r_stages_flag stages{}; // TODO: parse stages
  [[maybe_unused]] auto [it, emplaced] = ctx->pipelines.try_emplace(
    handle, ctx, handle, desc, std::move(layout), std::move(uniforms), stages
  );
  NTF_ASSERT(emplaced);

  return &it->second;
}

r_pipeline r_create_pipeline(unchecked_t, r_context ctx, const r_pipeline_descriptor& desc) {
  auto layout = _copy_pipeline_layout(desc);
  uniform_map uniforms;

  auto handle = ctx->platform->create_pipeline(desc, layout.get(), uniforms);
  NTF_ASSERT(handle);

  r_stages_flag stages{}; // TODO: parse stages
  [[maybe_unused]] auto [it, emplaced] = ctx->pipelines.try_emplace(
    handle, ctx, handle, desc, std::move(layout), std::move(uniforms), stages
  );
  NTF_ASSERT(emplaced);

  return &it->second;
}

void r_destroy_pipeline(r_pipeline pip) {
  if (!pip) {
    return;
  }

  const auto handle = pip->handle;
  auto* ctx = pip->ctx;
  auto it = ctx->pipelines.find(handle);
  if (it == ctx->pipelines.end()) {
    return;
  }

  ctx->platform->destroy_pipeline(handle);
  ctx->pipelines.erase(it);
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

  return unif_it->second;
}

r_uniform r_pipeline_get_uniform(unchecked_t, r_pipeline pip, std::string_view name) {
  NTF_ASSERT(pip);

  auto unif_it = pip->uniforms.find(name.data());
  NTF_ASSERT(unif_it != pip->uniforms.end());

  return unif_it->second;
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
