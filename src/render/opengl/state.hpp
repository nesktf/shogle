#pragma once

#include "../render.hpp"
#include "../../stl/optional.hpp"

#include <glad/glad.h>

#define glCheckError() ::ntf::gl_state::check_error(__FILE__, __LINE__)

namespace ntf {

class gl_state {
public:
  enum fbo_bind_flag {
    FBO_BIND_NONE   = 0,
    FBO_BIND_READ   = 1 << 0,
    FBO_BIND_WRITE  = 1 << 1,
    FBO_BIND_BOTH   = (gl_state::FBO_BIND_READ & gl_state::FBO_BIND_WRITE),
  };
  static constexpr size_t fbo_bind_count = 2;

  enum class fbo_status {
    complete = GL_FRAMEBUFFER_COMPLETE,
    undefined = GL_FRAMEBUFFER_UNDEFINED,
    inc_attach = GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT,
    inc_missing_attach = GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT,
    inc_draw_buff = GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER,
    inc_read_buff = GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER,
    unsupported = GL_FRAMEBUFFER_UNSUPPORTED,
    inc_msa = GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE,
    inc_layer_targets = GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS,
  };

  enum class rbo_attachment {
    stencil = GL_STENCIL_ATTACHMENT,
    depth = GL_DEPTH_ATTACHMENT,
    depth_stencil = GL_DEPTH_STENCIL_ATTACHMENT,
  };

  class viewport_data {
  public:
    viewport_data() = default;

  public:
    viewport_data& coords(uint32 x, uint32 y, uint32 w, uint32 h) noexcept;
    viewport_data& coords(uvec2 pos, uvec2 size) noexcept;

    viewport_data& pos(uint32 x, uint32 y) noexcept;
    viewport_data& size(uint32 w, uint32 h) noexcept;

    viewport_data& pos(uvec2 pos) noexcept;
    viewport_data& size(uvec2 size) noexcept;

    viewport_data& color(float32 r, float32 g, float32 b, float32 a) noexcept;
    viewport_data& color(float32 r, float32 g, float32 b) noexcept;
    viewport_data& color(color4 color) noexcept;
    viewport_data& color(color3 color) noexcept;

    viewport_data& flags(r_clear_flag flags) noexcept;

  public:
    [[nodiscard]] uvec4 coords() const noexcept { return _coords; }
    [[nodiscard]] uvec2 size() const noexcept { return uvec2{_coords.z, _coords.w}; }
    [[nodiscard]] uvec2 pos() const noexcept { return uvec2{_coords.x, _coords.y}; }
    [[nodiscard]] color4 color() const noexcept { return _color; }
    [[nodiscard]] r_clear_flag flags() const noexcept;

    [[nodiscard]] GLbitfield glflags() const noexcept { return _flags; }

  private:
    GLbitfield _flags{0};
    color4 _color{.3f, .3f, .3f, 1.f};
    uvec4 _coords{0, 0, 0, 0};
  };

public:
  gl_state() noexcept;

public:
  void init() noexcept;

public:
  [[nodiscard]] GLuint make_buffer(r_buffer_type type, size_t size) noexcept;
  void destroy_buffer(r_buffer_type type, GLuint id) noexcept;
  bool bind_buffer(r_buffer_type type, GLuint id) noexcept;
  [[nodiscard]] GLuint buffer(r_buffer_type type) const noexcept;
  void buffer_data(GLuint id, r_buffer_type type, 
                   const void* data, size_t size, size_t offset) noexcept;


  [[nodiscard]] GLuint make_vao() noexcept;
  void destroy_vao(GLuint id) noexcept;
  bool bind_vao(GLuint vao) noexcept;
  [[nodiscard]] GLuint vao() const noexcept;


  [[nodiscard]] GLuint make_framebuffer() noexcept;
  void destroy_framebuffer(GLuint id) noexcept;
  bool bind_framebuffer(GLuint id, fbo_bind_flag binding) noexcept;
  [[nodiscard]] GLuint framebuffer(fbo_bind_flag binding) const noexcept;
  void framebuffer_tex_attachment(GLuint fbo, GLuint texture, r_texture_type tex_type,
                                  uint32 index, uint32 level) noexcept; 
  void framebuffer_rbo_attachment(GLuint fbo, GLuint rbo, rbo_attachment attachment) noexcept;


  [[nodiscard]] GLuint make_renderbuffer(uint32 w, uint32 h) noexcept;
  void destroy_renderbuffer(GLuint id) noexcept;
  bool bind_renderbuffer(GLuint id) noexcept;
  [[nodiscard]] GLuint renderbuffer() const noexcept;


  [[nodiscard]] GLuint make_program() const noexcept;
  void destroy_program(GLuint id) noexcept;
  bool bind_program(GLuint id) noexcept;
  [[nodiscard]] GLuint program() const noexcept;
  optional<std::string_view> link_program(GLuint program, const GLuint* shaders,
                                          uint32 count) noexcept;


  [[nodiscard]] GLuint make_shader(r_shader_type type) noexcept;
  void destroy_shader(GLuint id) noexcept;
  optional<std::string_view> compile_shader(GLuint shader, std::string_view src) noexcept;


  GLenum bind_texture(GLuint id, uint32 index, r_texture_type type) noexcept;
  [[nodiscard]] GLuint make_texture(r_texture_type type, r_texture_format format,
                                    uint32 level, uint32 count, uvec3 dim) noexcept;
  void destroy_texture(GLuint id) noexcept;
  void texture_data(GLuint id, r_texture_type type, r_texture_format format,
                    const uint8* texels, uint32 index, uint32 count, uint32 mipmap,
                    uvec3 dim, uvec3 offset, bool gen_mipmaps) noexcept;
  void texture_sampler(GLuint id, r_texture_type type,
                       r_texture_sampler sampler, uint32 level) noexcept;
  void texture_addressing(GLuint id, r_texture_type, r_texture_address addressing) noexcept;
  fbo_status check_framebuffer(GLuint id) noexcept;

public:
  [[nodiscard]] bool validate_descriptor(const r_attrib_descriptor& desc) const noexcept;
  [[nodiscard]] bool validate_descriptor(const r_texture_descriptor& desc) const noexcept;
  [[nodiscard]] bool validate_descriptor(const r_buffer_descriptor& desc) const noexcept;
  [[nodiscard]] bool validate_descriptor(const r_pipeline_descriptor& desc) const noexcept;
  [[nodiscard]] bool validate_descriptor(const r_shader_descriptor& desc) const noexcept;
  [[nodiscard]] bool validate_descriptor(const r_framebuffer_descriptor& desc) const noexcept;

public:
  [[nodiscard]] std::string_view shader_error() const noexcept { return _shader_err; }
  const viewport_data& viewport() const noexcept { return _default_viewport; }
  viewport_data& viewport() noexcept { return _default_viewport; }

public:
  static GLenum check_error(const char* file, int line) noexcept;

public:
  [[nodiscard]] static GLenum buffer_type_cast(r_buffer_type type) noexcept;
  [[nodiscard]] static GLenum attrib_underlying_type_cast(r_attrib_type type) noexcept;
  [[nodiscard]] static GLenum primitive_cast(r_primitive type) noexcept;
  [[nodiscard]] static GLenum shader_type_cast(r_shader_type type) noexcept;
  [[nodiscard]] static GLenum fbo_bind_cast(fbo_bind_flag flag) noexcept;
  [[nodiscard]] static GLenum texture_type_cast(r_texture_type type) noexcept;
  [[nodiscard]] static GLenum texture_format_cast(r_texture_format format) noexcept;
  [[nodiscard]] static GLenum texture_sampler_cast(r_texture_sampler samp, bool mipmaps) noexcept;
  [[nodiscard]] static GLenum texture_addressing_cast(r_texture_address address) noexcept;

private:
  viewport_data _default_viewport;
  uint32 _fbo_max_attachments{0};
  GLuint _fbo[fbo_bind_count];
  GLuint _rbo{0};

  uint32 _tex_max_dim{0}, _tex_max_dim3d{0}, _tex_max_layers{0};
  size_t _tex_active_index{0};
  std::vector<std::pair<GLuint, GLenum>> _tex_active; // id, type

  GLuint _vao{0};
  GLuint _buff_active[r_buffer_type_count];
  size_t _vertex_attrib_stride{0};
  uint32 _vertex_attrib_enabled_bits{0};

  GLuint _shad_prog{0};
  std::string _shader_err;
};

} // namespace ntf
