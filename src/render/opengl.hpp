#pragma once

#include "./context.hpp"
#include "../stl/expected.hpp"

namespace ntf {

class gl_context;

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
    weak_ref<r_context::vertex_attributes_t> layout;
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
    uvec2 dim;
    GLuint sd_rbo;
    GLuint color_rbo;
    GLbitfield clear_flags;
    uvec4 viewport;
    color4 color;
  };

  enum fbo_binding {
    FBO_BIND_READ = 0,
    FBO_BIND_WRITE,

    FBO_BIND_COUNT,
    FBO_BIND_BOTH,
  };

  struct init_data_t {
    GLDEBUGPROC dbg;
    uvec4 vport;
    // color4 clear_color;
    // r_clear_flag flags;
  };

public:
  gl_state(gl_context& ctx) noexcept;

public:
  void init(const init_data_t& data) noexcept;

public:
  [[nodiscard]] buffer_t create_buffer(r_buffer_type type, const void* data, size_t size);
  void destroy_buffer(const buffer_t& buffer) noexcept;
  bool bind_buffer(GLuint id, GLenum type) noexcept;
  void update_buffer(const buffer_t& buffer, const void* data,
                     size_t size, size_t offset) noexcept;

  [[nodiscard]] vao_t create_vao() noexcept;
  void bind_vao(GLuint id) noexcept;
  void destroy_vao(const vao_t& vao) noexcept;

  [[nodiscard]] shader_t create_shader(r_shader_type type, std::string_view src);
  void destroy_shader(const shader_t& shader) noexcept;

  [[nodiscard]] program_t create_program(shader_t const* const* shaders, uint32 count,
                                         r_primitive primitive);
  void destroy_program(const program_t& program) noexcept;
  bool bind_program(GLuint id) noexcept;
  void push_uniform(uint32 loc, r_attrib_type type, const void* data) noexcept;
  r_uniform uniform_location(GLuint program, std::string_view name) noexcept;


  [[nodiscard]] texture_t create_texture(r_texture_type type, r_texture_format format,
                                         r_texture_sampler sampler, r_texture_address addressing,
                                         uvec3 extent, uint32 layers, uint32 levels,
                                         const void* data, bool genmips);
  void destroy_texture(const texture_t& tex) noexcept;
  void bind_texture(GLuint tex, GLenum type, uint32 index) noexcept;
  void update_texture_data(const texture_t& tex, const void* data, r_texture_format format,
                           uvec3 offset, uint32 layer, uint32 level, bool gen_mipmaps) noexcept;
  void update_texture_sampler(texture_t& tex, r_texture_sampler sampler) noexcept;
  void update_texture_addressing(texture_t& tex, r_texture_address addressing) noexcept;

  [[nodiscard]] framebuffer_t create_framebuffer(uint32 w, uint32 h, r_clear_flag buffers,
                                                 r_texture_format format);
  [[nodiscard]] framebuffer_t create_framebuffer(uint32 w, uint32 h, r_clear_flag buffers,
                                                 texture_t** attachments, const uint32* levels,
                                                 uint32 att_count);
  void destroy_framebuffer(const framebuffer_t& fbo) noexcept;
  void bind_framebuffer(GLuint id, fbo_binding binding) noexcept;
  void prepare_draw_target(const framebuffer_t* fbo) noexcept;

  void update_viewport(uvec4 vp) noexcept;
  void update_color(color4 col) noexcept;
  void update_flags(r_clear_flag flags) noexcept;
  void bind_attributes(const r_attrib_descriptor* attrs, uint32 count, size_t stride) noexcept;

public:
  [[nodiscard]] static GLenum buffer_type_cast(r_buffer_type type) noexcept;
  [[nodiscard]] static GLenum attrib_underlying_type_cast(r_attrib_type type) noexcept;
  [[nodiscard]] static GLenum primitive_cast(r_primitive type) noexcept;
  [[nodiscard]] static GLenum shader_type_cast(r_shader_type type) noexcept;
  [[nodiscard]] static GLenum fbo_attachment_cast(r_clear_flag flag) noexcept;
  [[nodiscard]] static GLenum texture_type_cast(r_texture_type type, bool is_array) noexcept;
  [[nodiscard]] static GLenum texture_format_cast(r_texture_format format) noexcept;
  [[nodiscard]] static GLenum texture_format_symbolic_cast(r_texture_format format) noexcept;
  [[nodiscard]] static GLenum texture_format_underlying_cast(r_texture_format format) noexcept;
  [[nodiscard]] static GLenum texture_sampler_cast(r_texture_sampler samp, bool mips) noexcept;
  [[nodiscard]] static GLenum texture_addressing_cast(r_texture_address address) noexcept;

private:
  GLenum& buffer_pos(GLenum type);

private:
  gl_context& _ctx;
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

  struct {
    uvec4 vport;
    color4 color;
    GLbitfield flag;
  } _swpch;
};

class gl_context final : public r_platform_context {
private:
  template<typename T, typename H>
  class res_container {
  public:
    res_container() = default;

  public:
    template<typename Fun>
    void clear(Fun&& f) {
      for (auto& res : _res) {
        f(res);
      }
      _res.clear();
      _free = {};
    }

    H acquire() {
      if (!_free.empty()) {
        H pos = H{_free.front()};
        _free.pop();
        return pos;
      }
      _res.emplace_back(T{});
      return H{_res.size()-1};
    }

    void push(H pos) {
      NTF_ASSERT(validate(pos));
      _free.push(pos.value());
    }

    T& get(H pos) {
      NTF_ASSERT(validate(pos));
      return _res[pos.value()];
    }

    const T& get(H pos) const {
      NTF_ASSERT(validate(pos));
      return _res[pos.value()];
    }

    bool validate(H pos) const {
      return pos.value() < _res.size();
    }

  private:
    std::vector<T> _res;
    std::queue<r_handle_value> _free;
  };

public:
  gl_context(r_window& win, uint32 major, uint32 minor);

public:
  r_buffer_handle create_buffer(const r_context::buffer_create_t& data) override;
  void update_buffer(r_buffer_handle buf, const r_context::buffer_update_t& data) override;
  void destroy_buffer(r_buffer_handle buff) override;

  r_texture_handle create_texture(const r_context::tex_create_t& data) override;
  void update_texture(r_texture_handle tex, const r_context::tex_update_t& data) override;
  void destroy_texture(r_texture_handle tex) override;

  r_framebuffer_handle create_framebuffer(const r_context::fb_create_t& data) override;
  void destroy_framebuffer(r_framebuffer_handle fb) override;

  r_shader_handle create_shader(const r_context::shader_create_t& data) override;
  void destroy_shader(r_shader_handle shader) override;

  r_pipeline_handle create_pipeline(const r_context::pipeline_create_t& data) override;
  r_uniform pipeline_uniform(r_pipeline_handle pipeline, std::string_view name) override;
  void destroy_pipeline(r_pipeline_handle pipeline) override;

  void update_swapchain(const r_context::swapchain_update_t& data) override;
  void submit(const r_context::command_queue& cmds) override;

public:
  r_api api_type() const override { return r_api::opengl; }
  std::string_view name_str() const override { return _name_str; }

private:
  GLAPIENTRY static void debug_callback(GLenum src, GLenum type, GLuint id, GLenum severity,
                                         GLsizei len, const GLchar* msg, const void* user_ptr);

private:
  gl_state _state;
  uint32 _major, _minor; // Core only
  GLADloadproc _proc_fun;
  std::string _name_str;
  gl_state::vao_t _vao;

  res_container<gl_state::buffer_t, r_buffer_handle> _buffers;
  res_container<gl_state::texture_t, r_texture_handle> _textures;
  res_container<gl_state::shader_t, r_shader_handle> _shaders;
  res_container<gl_state::program_t, r_pipeline_handle> _programs;
  res_container<gl_state::framebuffer_t, r_framebuffer_handle> _framebuffers;

public:
  NTF_DECLARE_NO_MOVE_NO_COPY(gl_context);
};

} // namespace ntf
