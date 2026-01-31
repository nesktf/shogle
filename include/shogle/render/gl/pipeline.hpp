#pragma once

#include <shogle/render/gl/common.hpp>

namespace shogle {

class gl_shader {
public:
  enum shader_stage : gldefs::GLenum {
    STAGE_VERTEX = 0x8B32,    // GL_VERTEX_SHADER
    STAGE_FRAGMENT = 0x8B30,  // GL_FRAGMENT_SHADER
    STAGE_GEOMETRY = 0x8DD9,  // GL_GEOMETRY_SHADER
    STAGE_TESS_EVAL = 0x8E87, // GL_TESS_EVALUATION_SHADER
    STAGE_TESS_CTRL = 0x8E88, // GL_TESS_CONTROL_SHADER
    STAGE_COMPUTE = 0x91B9,   // GL_COMPUTE_SHADER
  };

  enum stages_bits : gldefs::GLenum {
    STAGE_VERTEX_BIT = 0x00000001,    // GL_VERTEX_SHADER_BIT
    STAGE_FRAGMENT_BIT = 0x00000002,  // GL_FRAGMENT_SHADER_BIT
    STAGE_GEOMETRY_BIT = 0x00000004,  // GL_GEOMETRY_SHADER_BIT
    STAGE_TESS_CTRL_BIT = 0x00000008, // GL_TESS_CONTROL_SHADER_BIT
    STAGE_TESS_EVAL_BIT = 0x00000010, // GL_TESS_EVALUATION_SHADER_BIT
    STAGE_COMPUTE_BIT = 0x00000020,   // GL_COMPUTE_SHADER_BIT
    STAGE_ALL_BITS = 0xFFFFFFFF,      // GL_ALL_STAGE_BITS
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

  static void destroy(gl_context& gl, gl_shader& shader);

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
    PRIMITIVE_POINTS = 0x0000,         // GL_POINTS
    PRIMITIVE_LINES = 0x0001,          // GL_LINES
    PRIMITIVE_LINE_LOOP = 0x0002,      // GL_LINE_LOOP
    PRIMITIVE_LINE_STRIP = 0x0003,     // GL_LINE_STRIP
    PRIMITIVE_TRIANGLES = 0x0004,      // GL_TRIANGLES
    PRIMITIVE_TRIANGLE_STRIP = 0x0005, // GL_TRIANGLE_STRIP
    PRIMITIVE_TRIANGLE_FAN = 0x0006,   // GL_TRIANGLE_FAN
  };

  enum polygon_mode : gldefs::GLenum {
    POLY_MODE_POINT = 0x1B00, // GL_POINT
    POLY_MODE_LINE = 0x1B01,  // GL_LINE
    POLY_MODE_FILL = 0x1B02,  // GL_FILL
  };

  enum uniform_type : gldefs::GLenum {
    UNIFORM_F32 = 0x1406,       // GL_FLOAT
    UNIFORM_F32VEC2 = 0x8B50,   // GL_FLOAT_VEC2
    UNIFORM_F32VEC3 = 0x8B51,   // GL_FLOAT_VEC3
    UNIFORM_F32VEC4 = 0x8B52,   // GL_FLOAT_VEC4
    UNIFORM_F32MAT2 = 0x8B5A,   // GL_FLOAT_MAT2
    UNIFORM_F32MAT3 = 0x8B5B,   // GL_FLOAT_MAT3
    UNIFORM_F32MAT4 = 0x8B5C,   // GL_FLOAT_MAT4
    UNIFORM_F32MAT2x3 = 0x8B65, // GL_FLOAT_MAT2x3
    UNIFORM_F32MAT2x4 = 0x8B66, // GL_FLOAT_MAT2x4
    UNIFORM_F32MAT3x2 = 0x8B67, // GL_FLOAT_MAT3x2
    UNIFORM_F32MAT3x4 = 0x8B68, // GL_FLOAT_MAT3x4
    UNIFORM_F32MAT4x2 = 0x8B69, // GL_FLOAT_MAT4x2
    UNIFORM_F32MAT4x3 = 0x8B6A, // GL_FLOAT_MAT4x3

    UNIFORM_F64 = 0x140A,       // GL_DOUBLE
    UNIFORM_F64VEC2 = 0x8FFC,   // GL_DOUBLE_VEC2
    UNIFORM_F64VEC3 = 0x8FFD,   // GL_DOUBLE_VEC3
    UNIFORM_F64VEC4 = 0x8FFE,   // GL_DOUBLE_VEC4
    UNIFORM_F64MAT2 = 0x8F46,   // GL_DOUBLE_MAT2
    UNIFORM_F64MAT3 = 0x8F47,   // GL_DOUBLE_MAT3
    UNIFORM_F64MAT4 = 0x8F48,   // GL_DOUBLE_MAT4
    UNIFORM_F64MAT2x3 = 0x8F49, // GL_DOUBLE_MAT2x3
    UNIFORM_F64MAT2x4 = 0x8F4A, // GL_DOUBLE_MAT2x4
    UNIFORM_F64MAT3x2 = 0x8F4B, // GL_DOUBLE_MAT3x2
    UNIFORM_F64MAT3x4 = 0x8F4C, // GL_DOUBLE_MAT3x4
    UNIFORM_F64MAT4x2 = 0x8F4D, // GL_DOUBLE_MAT4x2
    UNIFORM_F64MAT4x3 = 0x8F4E, // GL_DOUBLE_MAT4x3

    UNIFORM_I32 = 0x1404,     // GL_INT
    UNIFORM_I32VEC2 = 0x8B53, // GL_INT_VEC2
    UNIFORM_I32VEC3 = 0x8B54, // GL_INT_VEC3
    UNIFORM_I32VEC4 = 0x8B55, // GL_INT_VEC4

    UNIFORM_U32 = 0x1405, // GL_UNSIGNED_INT
    UNIFORM_U32VEC2 = GL_UNSIGNED_INT_VEC2,
    UNIFORM_U32VEC3 = GL_UNSIGNED_INT_VEC3,
    UNIFORM_U32VEC4 = GL_UNSIGNED_INT_VEC4,

    UNIFORM_BOOL = GL_BOOL,
    UNIFORM_BOOLVEC2 = GL_BOOL_VEC2,
    UNIFORM_BOOLVEC3 = GL_BOOL_VEC3,
    UNIFORM_BOOLVEC4 = GL_BOOL_VEC4,

    UNIFORM_SAMPLER1D = GL_SAMPLER_1D,
    UNIFORM_SAMPLER1D_ARRAY = GL_SAMPLER_1D_ARRAY,
    UNIFORM_SAMPLER2D = GL_SAMPLER_2D,
    UNIFORM_SAMPLER2D_ARRAY = GL_SAMPLER_2D_ARRAY,
    UNIFORM_SAMPLER2DMS = GL_SAMPLER_2D_MULTISAMPLE,
    UNIFORM_SAMPLER2DMS_ARRAY = GL_SAMPLER_2D_MULTISAMPLE_ARRAY,
    UNIFORM_SAMPLER3D = GL_SAMPLER_3D,
    UNIFORM_SAMPLER_CUBE = GL_SAMPLER_CUBE,
    UNIFORM_SAMPLER_BUFFER = GL_SAMPLER_BUFFER,

    UNIFORM_ISAMPLER1D = 0x8DC9,       // GL_INT_SAMPLER_1D
    UNIFORM_ISAMPLER1D_ARRAY = 0x8DCE, // GL_INT_SAMPLER_1D_ARRAY
    UNIFORM_ISAMPLER2D = 0x8DCA,       // GL_INT_SAMPLER_2D
    UNIFORM_ISAMPLER2D_ARRAY = 0x8DCF, // GL_INT_SAMPLER_2D_ARRAY
    UNIFORM_ISAMPLER2DMS = GL_INT_SAMPLER_2D_MULTISAMPLE,
    UNIFORM_ISAMPLER2DMS_ARRAY = GL_INT_SAMPLER_2D_MULTISAMPLE_ARRAY,
    UNIFORM_ISAMPLER3D = GL_INT_SAMPLER_3D,
    UNIFORM_ISAMPLER_CUBE = GL_INT_SAMPLER_CUBE,
    UNIFORM_ISAMPLER_BUFFER = GL_INT_SAMPLER_BUFFER,

    UNIFORM_USAMPLER1D = GL_UNSIGNED_INT_SAMPLER_1D,
    UNIFORM_USAMPLER1D_ARRAY = GL_UNSIGNED_INT_SAMPLER_1D_ARRAY,
    UNIFORM_USAMPLER2D = GL_UNSIGNED_INT_SAMPLER_2D,
    UNIFORM_USAMPLER2D_ARRAY = GL_UNSIGNED_INT_SAMPLER_2D_ARRAY,
    UNIFORM_USAMPLER2DMS = GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE,
    UNIFORM_USAMPLER2DMS_ARRAY = GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY,
    UNIFORM_USAMPLER3D = GL_UNSIGNED_INT_SAMPLER_3D,
    UNIFORM_USAMPLER_CUBE = GL_UNSIGNED_INT_SAMPLER_CUBE,
    UNIFORM_USAMPLER_BUFFER = GL_UNSIGNED_INT_SAMPLER_BUFFER,
  };

  static constexpr size_t MAX_UNIFORM_NAME_SIZE = 128;

  struct uniform_props {
    char name[MAX_UNIFORM_NAME_SIZE];
    size_t name_len;
    size_t size;
    uniform_type type;
  };

private:
  struct create_t {};

public:
  gl_pipeline(create_t, gldefs::GLhandle program, gl_shader::stages_bits stages,
              primitive_mode primitive, polygon_mode poly_mode);

  gl_pipeline(gl_context& gl, const gl_shader::graphics_set& shaders, primitive_mode primitive,
              polygon_mode poly_mode, f32 poly_width);

public:
  static gl_s_expect<gl_pipeline> create(gl_context& gl, const gl_shader::graphics_set& shaders,
                                         primitive_mode primitive, polygon_mode poly_mode,
                                         f32 poly_width);

  static void destroy(gl_context& gl, gl_pipeline& pipeline);

public:
  ntf::optional<u32> uniform_location(gl_context& gl, const char* name);
  u32 uniform_count(gl_context& gl) const;
  void query_uniform_index(gl_context& gl, uniform_props& props, u32 idx);

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

  primitive_mode primitive() const;
  polygon_mode poly_mode() const;
  f32 poly_width() const;

  const gl_stencil_test_props& stencil_test() const;
  const gl_depth_test_props& depth_test() const;
  const gl_blending_props& blending() const;
  const gl_culling_props& culling() const;

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
