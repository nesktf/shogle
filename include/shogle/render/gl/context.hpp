#pragma once

#include <shogle/render/gl/buffer.hpp>
#include <shogle/render/gl/texture.hpp>
#include <shogle/render/gl/vertex.hpp>

#include <shogle/render/gl/framebuffer.hpp>
#include <shogle/render/gl/pipeline.hpp>

#include <memory>

namespace shogle {

struct gl_texture_binding {
  ref_view<const gl_texture> texture;
  u32 index;
};

struct gl_push_uniform {
  attribute_union attrib;
  u32 location;
};

struct gl_buffer_binding {
  ref_view<const gl_buffer> buffer;
  u32 location;
};

struct gl_shader_binding {
  ref_view<const gl_buffer> buffer;
  u32 location;
  size_t offset;
  size_t size;
};

struct gl_clear_opt {
  enum clear_flag : gldefs::GLenum {
    CLEAR_NONE = 0x00000000,    // GL_NONE
    CLEAR_COLOR = 0x00004000,   // GL_COLOR_BUFFER_BIT
    CLEAR_DEPTH = 0x00000100,   // GL_DEPTH_BUFFER_BIT
    CLEAR_STENCIL = 0x00000400, // GL_STENCIL_BUFFER_BIT
  };

  color4 color;
  clear_flag flags;
};

struct gl_frame_init {
  gl_clear_opt clear_opt;
  span<const gl_framebuffer> fbos;
  span<const gl_clear_opt> fbo_clear_opts;
};

struct gl_indexed_draw_cmd {
  enum index_format : gldefs::GLenum {
    FORMAT_I8 = 0x1400,  // GL_BYTE
    FORMAT_U8 = 0x1401,  // GL_UNSIGNED_BYTE
    FORMAT_I16 = 0x402,  // GL_SHORT
    FORMAT_U16 = 0x1403, // GL_UNSIGNED_SHORT
    FORMAT_I32 = 0x1404, // GL_INT
    FORMAT_U32 = 0x1405, // GL_UNSIGNED_INT
  };

  ref_view<const gl_vertex_layout> vertex_layout;
  ref_view<const gl_graphics_pipeline> pipeline;
  span<const gl_buffer_binding> vertex_buffers;
  span<const gl_shader_binding> shader_buffers;
  ref_view<const gl_buffer> index_buffer;
  span<const gl_texture_binding> textures;
  span<const gl_push_uniform> uniforms;
  square_pos<u32> viewport;
  square_pos<u32> scissor;
  u32 vertex_offset;
  u32 vertex_count;
  u32 index_count;
  index_format format;
  u32 instances;
};

struct gl_array_draw_cmd {
  ref_view<const gl_vertex_layout> vertex_layout;
  ref_view<const gl_graphics_pipeline> pipeline;
  span<const gl_buffer_binding> vertex_buffers;
  span<const gl_shader_binding> shader_buffers;
  span<const gl_texture_binding> textures;
  span<const gl_push_uniform> uniforms;
  square_pos<u32> viewport;
  square_pos<u32> scissor;
  u32 vertex_offset;
  u32 vertex_count;
  u32 instances;
};

struct gl_external_cmd {
  ntf::function_view<void(gl_context& gl, gldefs::GLhandle fbo)> callback;
  gl_depth_test_props depth_test;
  gl_stencil_test_props stencil_test;
  gl_blending_props blending;
  gl_culling_props culling;
  gl_graphics_pipeline::primitive_mode primitive;
  gl_graphics_pipeline::polygon_mode poly_mode;
  f32 poly_width;
  square_pos<u32> viewport;
  square_pos<u32> scissor;
};

using PFN_glGetProcAddress = void* (*)(const char* name);

struct gl_surface_provider {
  virtual ~gl_surface_provider() = default;
  virtual PFN_glGetProcAddress proc_loader() noexcept = 0;
  virtual extent2d surface_extent() const noexcept = 0;
  virtual void swap_buffers() noexcept = 0;
};

struct gl_version {
  u32 major;
  u32 minor;
};

class gl_context {
private:
  struct create_t {};

  template<typename F>
  static constexpr bool _scope_frame_invocable =
    std::is_invocable_v<F, gl_context&> || std::is_invocable_v<F>;

  using context_data = std::unique_ptr<gl_private>;

public:
  explicit gl_context(create_t, context_data&& ctx, gl_surface_provider& surf_prov) noexcept;

  gl_context(gl_surface_provider& surf_prov);

public:
  static sv_expect<gl_context> create(gl_surface_provider& surf_prov) noexcept;

public:
  void start_frame(const gl_frame_init& init);
  gl_sv_expect<void> submit_indexed_draw_command(const gl_indexed_draw_cmd& cmd,
                                                 ptr_view<const gl_framebuffer> target = {});
  gl_sv_expect<void> submit_draw_command(const gl_array_draw_cmd& cmd,
                                         ptr_view<const gl_framebuffer> target = {});
  void submit_external_command(const gl_external_cmd& cmd,
                               ptr_view<const gl_framebuffer> target = {});
  void end_frame();

  template<typename F>
  void scope_frame(const gl_frame_init& init, F&& scope)
  requires(_scope_frame_invocable<F>)
  {
    start_frame(init);
    if constexpr (std::is_invocable_v<F, gl_context&>) {
      std::invoke(scope, *this);
    } else {
      std::invoke(scope);
    }
    end_frame();
  }

public:
  gl_surface_provider& provider() const;
  gldefs::GLenum get_error() const;
  gl_version version() const;
  std::string_view renderer_string() const;
  std::string_view vendor_string() const;
  std::string_view version_string() const;

private:
  context_data _ctx;
  ref_view<gl_surface_provider> _surf_prov;
};

} // namespace shogle
