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
  _fb_cmds.clear();
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
  for (auto& [_, cmds] : _fb_cmds) {
    _ctx->submit(cmds);
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

  buff_handle_data_t data;
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

void r_context::update_data(r_buffer_handle buff,
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
  tex_create_t create;
  create.data = desc.texels;
  create.type = desc.type;
  create.format = desc.format;
  create.sampler = desc.sampler;
  create.addressing = desc.addressing;
  create.mipmaps = desc.mipmaps;
  create.dim = desc.extent;
  create.array_size = desc.count;

  auto handle = _ctx->create_texture(create);
  NTF_ASSERT(handle);
  NTF_ASSERT(_textures.find(handle) == _textures.end());

  tex_handle_data_t data;
  data.type = desc.type;
  data.format = desc.format;
  data.dim = desc.extent;
  data.mipmaps = desc.mipmaps;
  data.layers = desc.count;
  data.addressing = desc.addressing;
  data.sampler = desc.sampler;

  _textures.insert(std::make_pair(handle, data));
  return handle;
}

void r_context::destroy(r_texture_handle tex) {
  NTF_ASSERT(tex);
  auto it = _textures.find(tex);
  NTF_ASSERT(it != _textures.end());
  _ctx->destroy_texture(tex);
  _textures.erase(it);
}

void r_context::update(r_texture_handle tex, r_texture_sampler sampler) {
}

void r_context::update(r_texture_handle tex, r_texture_address addressing) {
}

void r_context::update_data(r_texture_handle tex,
                   const void* data, size_t size, size_t offset, r_texture_format format,
                   uint32 layer, uint32 mipmap) {
}

r_texture_type r_context::query(r_texture_handle tex, r_query_type_t) const {
}

r_texture_format r_context::query(r_texture_handle tex, r_query_format_t) const {
}

r_texture_sampler r_context::query(r_texture_handle tex, r_query_sampler_t) const {
}

r_texture_address r_context::query(r_texture_handle tex, r_query_addressing_t) const {
}

uvec3 r_context::query(r_texture_handle tex, r_query_dim_t) const {
}

uint32 r_context::query(r_texture_handle tex, r_query_layer_t) const {
}

uint32 r_context::query(r_texture_handle tex, r_query_mipmaps_t) const {
}

r_framebuffer_handle r_context::create_framebuffer(const r_framebuffer_descriptor& desc) {
}

void r_context::destroy(r_framebuffer_handle fbo) {
}

void r_context::attach(r_framebuffer_handle fbo, r_texture_handle tex) {
}

r_texture_handle r_context::dettach(r_framebuffer_handle fbo, uint32 index) {
}

r_shader_handle r_context::create_shader(const r_shader_descriptor& desc) {
}

void r_context::destroy(r_shader_handle shader) {
}

r_shader_type r_context::query(r_shader_handle shader, r_query_type_t) const {
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
