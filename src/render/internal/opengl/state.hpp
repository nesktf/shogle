#pragma once

#include "../platform.hpp"

namespace ntf {

class gl_state {
public:
  static constexpr uint32 MAX_FBO_ATTACHMENTS = 8;
  static constexpr uint32 MAX_TEXTURE_LEVEL = 7;
  static constexpr GLuint NULL_BINDING = 0;
  static constexpr GLuint DEFAULT_FBO = 0;

public:
  struct buffer_t {
    GLuint id{0};
    GLenum type;
    GLbitfield flags;
    size_t size;
    void* mapping_ptr;
    GLbitfield mapping;
  };

  enum buffer_bindings {
    BUFFER_TYPE_VERTEX = 0,
    BUFFER_TYPE_INDEX,
    BUFFER_TYPE_TEXEL,
    BUFFER_TYPE_UNIFORM,
    BUFFER_TYPE_SHADER,

    BUFFER_TYPE_COUNT,
  };

  struct vao_t {
    GLuint id{0};
  };

  struct shader_t {
    GLuint id{0};
    GLenum type;
  };

  struct program_t {
    GLuint id{0};
    GLenum primitive;
    weak_ref<vertex_layout> layout;
  };

  struct texture_t {
    GLuint id{0};
    GLenum type;
    GLenum format;
    GLenum sampler[2]; // MIN, MAG
    GLenum addressing;
    uvec3 extent;
    uint32 layers;
    uint32 levels;
  };

  struct framebuffer_t {
    GLuint id{0};
    uvec2 extent;
    GLuint sd_rbo;
    GLuint color_rbo;
  };

  enum fbo_binding {
    FBO_BIND_READ = 0,
    FBO_BIND_WRITE,

    FBO_BIND_COUNT,
    FBO_BIND_BOTH,
  };

  struct init_data_t {
    GLDEBUGPROC dbg;
  };

  struct fbo_attachment_t {
    texture_t* tex;
    uint32 layer;
    uint32 level;
  };

public:
  gl_state() noexcept;

public:
  void init(const init_data_t& data) noexcept;

public:
  [[nodiscard]] buffer_t create_buffer(r_buffer_type type, r_buffer_flag flags, size_t size,
                                       weak_ref<r_buffer_data> data);
  void destroy_buffer(const buffer_t& buffer) noexcept;
  bool bind_buffer(GLuint id, GLenum type) noexcept;
  void update_buffer(const buffer_t& buffer, const void* data,
                     size_t size, size_t offset) noexcept;
  void* map_buffer(buffer_t& buffer, size_t offset, size_t len);
  void unmap_buffer(buffer_t& buffer, void* ptr);

  [[nodiscard]] vao_t create_vao() noexcept;
  void bind_vao(GLuint id) noexcept;
  void destroy_vao(const vao_t& vao) noexcept;

  [[nodiscard]] shader_t create_shader(r_shader_type type, std::string_view src);
  void destroy_shader(const shader_t& shader) noexcept;

  [[nodiscard]] program_t create_program(shader_t const* const* shaders, uint32 count,
                                         r_primitive primitive);
  void query_program_uniforms(const program_t& program, rp_uniform_query_vec& unif);
  void destroy_program(const program_t& program) noexcept;
  bool bind_program(GLuint id) noexcept;
  void push_uniform(uint32 loc, r_attrib_type type, const void* data) noexcept;

  [[nodiscard]] texture_t create_texture(r_texture_type type, r_texture_format format,
                                         r_texture_sampler sampler, r_texture_address addressing,
                                         uvec3 extent, uint32 layers, uint32 levels);
  void destroy_texture(const texture_t& tex) noexcept;
  void bind_texture(GLuint tex, GLenum type, uint32 index) noexcept;
  void update_texture_data(const texture_t& tex, const void* data,
                           r_texture_format format, r_image_alignment alignment,
                           uvec3 offset, uint32 layer, uint32 level) noexcept;
  void update_texture_sampler(texture_t& tex, r_texture_sampler sampler) noexcept;
  void update_texture_addressing(texture_t& tex, r_texture_address addressing) noexcept;
  void gen_texture_mipmaps(const texture_t& tex);

  [[nodiscard]] framebuffer_t create_framebuffer(uint32 w, uint32 h, rp_test_buffer_flag buffers,
                                                 r_texture_format format);
  [[nodiscard]] framebuffer_t create_framebuffer(uint32 w, uint32 h, rp_test_buffer_flag buffers,
                                                 const fbo_attachment_t* attachments,
                                                 uint32 att_count);
  void destroy_framebuffer(const framebuffer_t& fbo) noexcept;
  void bind_framebuffer(GLuint id, fbo_binding binding) noexcept;
  void prepare_draw_target(GLuint fb, r_clear_flag flags,
                           const uvec4& vp, const color4& col) noexcept;

  void bind_attributes(const r_attrib_descriptor* attrs, uint32 count, size_t stride) noexcept;

public:
  const auto& tex_limits() const { return _tex_limits; }

public:
  [[nodiscard]] static GLenum buffer_type_cast(r_buffer_type type) noexcept;
  [[nodiscard]] static GLenum attrib_underlying_type_cast(r_attrib_type type) noexcept;
  [[nodiscard]] static GLenum primitive_cast(r_primitive type) noexcept;
  [[nodiscard]] static GLenum shader_type_cast(r_shader_type type) noexcept;
  [[nodiscard]] static GLenum fbo_attachment_cast(rp_test_buffer_flag flag) noexcept;
  [[nodiscard]] static GLenum texture_type_cast(r_texture_type type, bool is_array) noexcept;
  [[nodiscard]] static GLenum texture_format_cast(r_texture_format format) noexcept;
  [[nodiscard]] static GLenum texture_format_symbolic_cast(r_texture_format format) noexcept;
  [[nodiscard]] static GLenum texture_format_underlying_cast(r_texture_format format) noexcept;
  [[nodiscard]] static GLenum texture_sampler_cast(r_texture_sampler samp, bool mips) noexcept;
  [[nodiscard]] static GLenum texture_addressing_cast(r_texture_address address) noexcept;
  [[nodiscard]] static GLbitfield clear_bit_cast(r_clear_flag flags) noexcept;

private:
  GLenum& _buffer_pos(GLenum type);

private:
  GLuint _bound_buffers[BUFFER_TYPE_COUNT];
  GLuint _bound_fbos[FBO_BIND_COUNT];
  GLuint _bound_vao;
  GLuint _bound_program;

  struct {
    uint32 max_dim;
    uint32 max_dim3d;
    uint32 max_layers;
  } _tex_limits;
  std::vector<std::pair<GLuint, GLenum>> _bound_texs; // id, type
  size_t _active_tex;
};

} // namespace ntf
