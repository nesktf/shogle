#pragma once

#include "./context.hpp"
#include "../stl/expected.hpp"

namespace ntf {

class gl_context;

class gl_state {
public:
  static constexpr uint32 MAX_FBO_ATTACHMENTS = 8;
  static constexpr uint32 MAX_TEXTURE_LEVEL = 7;

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
    weak_ref<r_attributes> layout;
  };

  struct texture_t {
    GLuint id{0};
    GLenum type;
    GLenum format;
    GLenum sampler[2]; // MIN, MAG
    GLenum addressing;
    uvec3 dim;
    uint32 level;
    uint32 array_size;
    uint32 attached_fbos;
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
  };

public:
  gl_state(gl_context& ctx) noexcept;

public:
  void init() noexcept;

public:
  [[nodiscard]] buffer_t create_buffer(r_buffer_type type, const void* data, size_t size);
  void destroy_buffer(const buffer_t& buffer) noexcept;
  void bind_buffer(const buffer_t& buffer) noexcept;
  void update_buffer(const buffer_t& buffer, const void* data,
                     size_t size, size_t offset) noexcept;

  [[nodiscard]] vao_t create_vao() noexcept;
  void bind_vao(const vao_t& vao) noexcept;
  void destroy_vao(const vao_t& vao) noexcept;

  [[nodiscard]] shader_t create_shader(r_shader_type type, std::string_view src);
  void destroy_shader(const shader_t& shader) noexcept;

  [[nodiscard]] program_t create_program(const shader_t* shaders, uint32 count);
  void destroy_program(const program_t& program) noexcept;
  void bind_program(const program_t& program) noexcept;


  [[nodiscard]] texture_t create_texture(r_texture_type type, r_texture_format format,
                                         r_texture_sampler sampler, r_texture_address addressing,
                                         uvec3 dim, uint32 array_size, uint32 mipmaps);
  void destroy_texture(const texture_t& tex) noexcept;
  void bind_texture(const texture_t& tex, uint32 index) noexcept;
  void update_texture_data(const texture_t& tex, const void* data, r_texture_format format,
                           uvec3 offset, uint32 index, uint32 level, bool gen_mipmaps) noexcept;
  void update_texture_sampler(texture_t& tex, r_texture_sampler sampler) noexcept;
  void update_texture_addressing(texture_t& tex, r_texture_address addressing) noexcept;

  [[nodiscard]] framebuffer_t create_framebuffer(uint32 w, uint32 h, r_clear_flag buffers,
                                                 r_texture_format format);
  [[nodiscard]] framebuffer_t create_framebuffer(uint32 w, uint32 h, r_clear_flag buffers,
                                                 texture_t** attachments, uint32* levels,
                                                 uint32 att_count);
  void destroy_framebuffer(const framebuffer_t& fbo) noexcept;
  void bind_framebuffer(const framebuffer_t& fbo, fbo_binding binding) noexcept;

public:
  [[nodiscard]] static GLenum buffer_type_cast(r_buffer_type type) noexcept;
  [[nodiscard]] static GLenum attrib_underlying_type_cast(r_attrib_type type) noexcept;
  [[nodiscard]] static GLenum primitive_cast(r_primitive type) noexcept;
  [[nodiscard]] static GLenum shader_type_cast(r_shader_type type) noexcept;
  [[nodiscard]] static GLenum fbo_attachment_cast(r_clear_flag flag) noexcept;
  [[nodiscard]] static GLenum texture_type_cast(r_texture_type type) noexcept;
  [[nodiscard]] static GLenum texture_format_cast(r_texture_format format) noexcept;
  [[nodiscard]] static GLenum texture_format_underlying_cast(r_texture_format format) noexcept;
  [[nodiscard]] static GLenum texture_sampler_cast(r_texture_sampler samp, bool mips) noexcept;
  [[nodiscard]] static GLenum texture_addressing_cast(r_texture_address address) noexcept;

private:
  GLenum& _buffer_pos(GLenum type);

private:
  gl_context& _ctx;
  GLuint _bound_buffers[BUFFER_TYPE_COUNT];
  GLuint _bound_fbos[FBO_BIND_COUNT];
  struct {
    uint32 max_dim;
    uint32 max_dim3d;
    uint32 max_layers;
  } _tex_limits;
  size_t _active_tex;
  std::vector<std::pair<GLuint, GLenum>> _bound_texs; // id, type
  GLuint _bound_vao;
  GLuint _bound_program;
};

class gl_context : public r_platform_context {
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
  void start_frame();
  void enqueue(r_draw_cmd cmd);
  void end_frame();

public:
  void viewport(uint32 x, uint32 y, uint32 w, uint32 h);
  void viewport(uint32 w, uint32 h);
  void viewport(uvec2 pos, uvec2 size);
  void viewport(uvec2 size);

  void clear_color(float32 r, float32 g, float32 b, float32 a);
  void clear_color(float32 r, float32 g, float32 b);
  void clear_color(color4 color);
  void clear_color(color3 color);

  void clear_flags(r_clear_flag clear);

public:
  std::string_view name_str() const;
  std::string_view vendor_str() const;
  std::string_view version_str() const;
  std::pair<uint32, uint32> version() const { return std::make_pair(_major, _minor); }

  r_api api_type() const override { return r_api::opengl; }

private:
  GLAPIENTRY static void _debug_callback(GLenum src, GLenum type, GLuint id, GLenum severity,
                                         GLsizei len, const GLchar* msg, const void* user_ptr);

private:
  gl_state _state;
  uint32 _major, _minor; // Core only
  GLADloadproc _proc_fun;
  uvec4 _viewport;
  gl_state::vao_t _vao;

  res_container<gl_state::buffer_t, r_context::buff_handle> _buffers;
  res_container<gl_state::texture_t, r_context::tex_handle> _textures;
  res_container<gl_state::shader_t, r_context::shad_handle> _shaders;
  res_container<gl_state::program_t, r_context::pip_handle> _programs;
  res_container<gl_state::framebuffer_t, r_context::fb_handle> _framebuffers;

public:
  NTF_DECLARE_NO_MOVE_NO_COPY(gl_context);
};

} // namespace ntf
