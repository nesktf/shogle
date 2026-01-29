#pragma once

#include <shogle/render/gl/common.hpp>

namespace shogle {

class gl_shader {
public:
  enum shader_stage : gldefs::GLenum {
    SHADER_VERTEX = 0x8B32,    // GL_VERTEX_SHADER
    SHADER_FRAGMENT = 0x8B30,  // GL_FRAGMENT_SHADER
    SHADER_GEOMETRY = 0x8DD9,  // GL_GEOMETRY_SHADER
    SHADER_TESS_EVAL = 0x8E87, // GL_TESS_EVALUATION_SHADER
    SHADER_TESS_CTRL = 0x8E88, // GL_TESS_CONTROL_SHADER
    SHADER_COMPUTE = 0x91B9,   // GL_COMPUTE_SHADER
  };

  enum stages_bits : gldefs::GLenum {
    SHADER_VERTEX_BIT = 0x00000001,    // GL_VERTEX_SHADER_BIT
    SHADER_FRAGMENT_BIT = 0x00000002,  // GL_FRAGMENT_SHADER_BIT
    SHADER_GEOMETRY_BIT = 0x00000004,  // GL_GEOMETRY_SHADER_BIT
    SHADER_TESS_CTRL_BIT = 0x00000008, // GL_TESS_CONTROL_SHADER_BIT
    SHADER_TESS_EVAL_BIT = 0x00000010, // GL_TESS_EVALUATION_SHADER_BIT
    SHADER_COMPUTE_BIT = 0x00000020,   // GL_COMPUTE_SHADER_BIT
    SHADER_ALL_BITS = 0xFFFFFFFF,      // GL_ALL_SHADER_BITS
  };

  struct graphics_set {
    std::array<gldefs::GLhandle, 5u> shaders;
    u32 shader_count;
    gl_shader::stages_bits active_stages;
  };

private:
  struct create_t {};

public:
  gl_shader(create_t, gldefs::GLhandle id, shader_stage stage);
  gl_shader(gl_context& gl, std::string_view src, shader_stage stage);

public:
  static gl_s_expect<gl_shader> create(gl_context& gl, std::string_view src, shader_stage stage);
  void destroy();
  void rebind_context(gl_context& gl);

public:
  gldefs::GLhandle id() const;
  shader_stage stage() const;

private:
  gldefs::GLhandle _id;
  shader_stage _stage;
};

class gl_shader_builder {
public:
  static constexpr u32 MAP_SIZE = 10;
  using shader_map = std::array<gldefs::GLhandle, MAP_SIZE>;

public:
  gl_shader_builder() noexcept;

public:
  gl_shader_builder& add_shader(const gl_shader& shader);
  gldefs::GLhandle get_shader(gl_shader::shader_stage stage);
  ntf::optional<gl_shader::graphics_set> build();

private:
  shader_map _shaders;
};

struct gl_stencil_test_props {
  enum stencil_func : gldefs::GLenum {

  };

  enum stencil_action : gldefs::GLenum {

  };

  bool enable;
  stencil_func func;
  i32 func_ref;
  u32 func_mask;
  stencil_action on_stencil_fail;
  stencil_action on_depth_fail;
  stencil_action on_pass;
  u32 mask;
};

struct gl_depth_test_props {
  enum depth_func : gldefs::GLenum {

  };

  bool enable;
  f64 near;
  f64 far;
  depth_func func;
};

struct gl_blending_props {
  enum blending_mode : gldefs::GLenum {

  };

  enum blending_factor : gldefs::GLenum {

  };

  bool enable;
  blending_mode mode;
  blending_factor src_fac;
  blending_factor dst_fac;
  color4 color;
};

struct gl_culling_props {
  enum cull_mode : gldefs::GLenum {

  };

  enum cull_face : gldefs::GLenum {

  };

  bool enable;
  cull_mode mode;
  cull_face face;
};

class gl_pipeline {
public:
  enum primitive_mode : gldefs::GLenum {

  };

  enum polygon_mode : gldefs::GLenum {

  };

  enum uniform_type : gldefs::GLenum {

  };

  static constexpr size_t MAX_UNIFORM_NAME_SIZE = 128;

  struct uniform_props {
    char name[MAX_UNIFORM_NAME_SIZE];
    size_t size;
    u32 name_len;
    uniform_type type;
  };

private:
  struct create_t {};

public:
  gl_pipeline(create_t, gl_context& gl, gldefs::GLhandle program, primitive_mode primitive,
              polygon_mode poly_mode);

  gl_pipeline(gl_context& gl, const gl_shader::graphics_set& shaders, primitive_mode primitive,
              polygon_mode poly_mode, f32 poly_width);

public:
  static gl_s_expect<gl_pipeline> create(gl_context& gl, const gl_shader::graphics_set& shaders,
                                         primitive_mode primitive, polygon_mode poly_mode,
                                         f32 poly_width);

  void destroy();
  void rebind_context(gl_context& gl);

public:
  ntf::optional<u32> uniform_location(const char* name);

  template<typename Cont>
  void query_uniforms(Cont&& cont) {
    const u32 count = uniform_count();
    for (u32 i = 0; i < count; ++i) {
      cont.emplace_back(_query_uniform_index(i));
    }
  }

public:
  gl_pipeline& reset_props();

  gl_pipeline& set_primitive(primitive_mode primitive);
  gl_pipeline& set_poly_mode(polygon_mode poly_mode);
  gl_pipeline& set_poly_width(f32 poly_width);

  gl_pipeline& set_stencil_test(const gl_stencil_test_props& stencil);
  gl_pipeline& set_depth_test(const gl_depth_test_props& depth);
  gl_pipeline& set_blending(const gl_blending_props& blending);
  gl_pipeline& set_culling(const gl_culling_props& culling);

public:
  gldefs::GLhandle program() const;
  gl_shader::stages_bits stages() const;

  gldefs::GLenum primitive() const;
  gldefs::GLenum poly_mode() const;
  f32 poly_width() const;

  const gl_stencil_test_props& stencil_test() const;
  const gl_depth_test_props& depth_test() const;
  const gl_blending_props& blending() const;
  const gl_culling_props& culling() const;

  u32 uniform_count() const;

private:
  uniform_props _query_uniform_index(u32 idx);

private:
  gl_stencil_test_props _stencil;
  gl_depth_test_props _depth;
  gl_blending_props _blending;
  gl_culling_props _culling;
  gl_shader::stages_bits _stages;
  gldefs::GLhandle _program;
  primitive_mode _primitive;
  polygon_mode _poly_mode;
  f32 _poly_width;
};

} // namespace shogle
