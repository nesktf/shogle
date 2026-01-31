#pragma once

#include <shogle/render/gl/common.hpp>

namespace shogle {

class gl_shader {
public:
  using context_type = gl_context;

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
  public:
    graphics_set(std::array<gldefs::GLhandle, 5u> shader_set, u32 shader_count,
                 gl_shader::stages_bits active_stages) noexcept :
        _shader_set(shader_set), _shader_count(shader_count), _active_stages(active_stages) {}

  public:
    span<const gldefs::GLhandle> stages() const noexcept {
      return {_shader_set.data(), _shader_count};
    }

    gl_shader::stages_bits active_stages() const noexcept { return _active_stages; }

  private:
    std::array<gldefs::GLhandle, 5u> _shader_set;
    u32 _shader_count;
    gl_shader::stages_bits _active_stages;
  };

private:
  struct create_t {};

public:
  gl_shader(create_t, gldefs::GLhandle id, shader_stage stage);
  gl_shader(gl_context& gl, std::string_view src, shader_stage stage);

public:
  static gl_s_expect<gl_shader> create(gl_context& gl, std::string_view src, shader_stage stage);

  static void destroy(gl_context& gl, gl_shader& shader) noexcept;

public:
  gldefs::GLhandle id() const;
  shader_stage stage() const;

private:
  gldefs::GLhandle _id;
  shader_stage _stage;
};

static_assert(::shogle::meta::renderer_object_type<gl_shader>);

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
public:
  enum stencil_test : gldefs::GLenum {
    TEST_NEVER = 0x0200,     // GL_NEVER
    TEST_LESS = 0x0201,      // GL_LESS
    TEST_EQUAL = 0x0202,     // GL_EQUAL
    TEST_LEQUAL = 0x0203,    // GL_LEQUAL
    TEST_GREATER = 0x0204,   // GL_GREATER
    TEST_NOT_EQUAL = 0x0205, // GL_NOTEQUAL
    TEST_GEQUAL = 0x0206,    // GL_GEQUAL
    TEST_ALWAYS = 0x0207,    // GL_ALWAYS
  };

  enum stencil_action : gldefs::GLenum {
    STENCIL_SET_ZERO = 0x0000,  // GL_ZERO
    STENCIL_KEEP = 0x1E00,      // GL_KEEP,
    STENCIL_REPLACE = 0x1E01,   // GL_REPLACE
    STENCIL_INCR = 0x1E02,      // GL_INCR
    STENCIL_DECR = 0x1E03,      // GL_DECR
    STENCIL_INCR_WRAP = 0x8507, // GL_INCR_WRAP
    STENCiL_DECR_WRAP = 0x8508, // GL_DECR_WRAP
    STENCIL_INVERT = 0x150A,    // GL_INVERT
  };

public:
  static constexpr inline gl_stencil_test_props make_default(bool enabled) {
    return {
      .enable = enabled,
      .test = TEST_NEVER,
      .test_ref = 0x00000000,
      .test_mask = 0x00000000,
      .on_stencil_fail = STENCIL_KEEP,
      .on_depth_fail = STENCIL_KEEP,
      .on_pass = STENCIL_KEEP,
      .stencil_mask = 0x00000000,
    };
  }

public:
  bool enable;
  stencil_test test;
  i32 test_ref;
  u32 test_mask;
  stencil_action on_stencil_fail;
  stencil_action on_depth_fail;
  stencil_action on_pass;
  u32 stencil_mask;
};

struct gl_depth_test_props {
public:
  enum depth_test : gldefs::GLenum {
    TEST_NEVER = 0x0200,     // GL_NEVER
    TEST_LESS = 0x0201,      // GL_LESS
    TEST_EQUAL = 0x0202,     // GL_EQUAL
    TEST_LEQUAL = 0x0203,    // GL_LEQUAL
    TEST_GREATER = 0x0204,   // GL_GREATER
    TEST_NOT_EQUAL = 0x0205, // GL_NOTEQUAL
    TEST_GEQUAL = 0x0206,    // GL_GEQUAL
    TEST_ALWAYS = 0x0207,    // GL_ALWAYS
  };

  enum depth_mask : gldefs::GLenum {
    MASK_WRITE_DISABLE = 0x0000, // GL_FALSE
    MASK_WRITE_ENABLE = 0x0001,  // GL_TRUE
  };

public:
  static constexpr inline gl_depth_test_props make_default(bool enabled) {
    return {
      .enable = enabled,
      .near = 0.01f,
      .far = 1.f,
      .test = TEST_LESS,
      .mask = MASK_WRITE_ENABLE,
    };
  }

public:
  bool enable;
  f64 near;
  f64 far;
  depth_test test;
  depth_mask mask;
};

struct gl_blending_props {
public:
  enum blending_mode : gldefs::GLenum {
    MODE_ADD = 0x8006,         // GL_FUNC_ADD
    MODE_MIN = 0x8007,         // GL_MIN
    MODE_MAX = 0x8008,         // GL_MAX
    MODE_SUB = 0x800A,         // GL_FUNC_SUBSTRACT
    MODE_REVERSE_SUB = 0x800B, // GL_FUNC_REVERSE_SUBTRACT
  };

  enum blending_factor : gldefs::GLenum {
    FAC_ZERO = 0x0000, // GL_ZERO
    FAC_ONE = 0x0001,  // GL_ONE

    FAC_SRC_COLOR = 0x0300,     // GL_SRC_COLOR
    FAC_INV_SRC_COLOR = 0x0301, // GL_ONE_MINUS_SRC_COLOR
    FAC_DST_COLOR = 0x0306,     // GL_DST_COLOR
    FAC_INV_DST_COLOR = 0x0307, // GL_ONE_MINUS_DST_COLOR

    FAC_SRC_ALPHA = 0x0302,     // GL_SRC_ALPHA
    FAC_INV_SRC_ALPHA = 0x0303, // GL_ONE_MINUS_SRC_ALPHA
    FAC_SRC_ALPHA_SAT = 0x0308, // GL_SRC_ALPHA_SATURATE
    FAC_DST_ALPHA = 0x0304,     // GL_DST_ALPHA
    FAC_INV_DST_ALPHA = 0x0305, // GL_ONE_MINUS_DST_ALPHA

    FAC_CONST_COLOR = 0x8001,     // GL_CONSTANT_COLOR
    FAC_INV_CONST_COLOR = 0x8002, // GL_ONE_MINUS_CONSTANT_COLOR

    FAC_CONST_ALPHA = 0x8003,     // GL_CONSTANT_ALPHA
    FAC_INV_CONST_ALPHA = 0x8004, // GL_ONE_MINUS_CONSTANT_ALPHA

    FAC_SRC1_COLOR = 0x88F9,     // GL_SRC1_COLOR
    FAC_INV_SRC1_COLOR = 0x88FA, // GL_ONE_MINUS_SRC1_COLOR

    FAC_SRC1_ALPHA = 0x8589,     // GL_SRC1_ALPHA
    FAC_INV_SRC1_ALPHA = 0x88FB, // GL_ONE_MINUS_SRC1_ALPHA
  };

public:
  static constexpr inline gl_blending_props make_default(bool enabled) {
    return {
      .enable = enabled,
      .mode = MODE_ADD,
      .src_color = FAC_SRC_ALPHA,
      .dst_color = FAC_INV_SRC_ALPHA,
      .src_alpha = FAC_ONE,
      .dst_alpha = FAC_ZERO,
      .color = {1.f, 1.f, 1.f, 1.f},
    };
  }

public:
  bool enable;
  blending_mode mode;
  blending_factor src_color;
  blending_factor dst_color;
  blending_factor src_alpha;
  blending_factor dst_alpha;
  color4 color;
};

struct gl_culling_props {
public:
  enum cull_mode : gldefs::GLenum {
    CULL_FRONT = 0x0404,      // GL_FRONT
    CULL_BACK = 0x0405,       // GL_BACK
    CULL_FRONT_BACK = 0x0408, // GL_FRONT_AND_BACK
  };

  enum cull_face : gldefs::GLenum {
    FACE_CLOCKWISE = 0x0900,        // GL_CW
    FACE_COUNTERCLOCKWISE = 0x0901, // GL_CCW
  };

public:
  static constexpr inline gl_culling_props make_default(bool enabled) {
    return {
      .enable = enabled,
      .mode = CULL_BACK,
      .face = FACE_COUNTERCLOCKWISE,
    };
  }

public:
  bool enable;
  cull_mode mode;
  cull_face face;
};

class gl_graphics_pipeline {
public:
  using context_type = gl_context;

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
    UNIFORM_NONE = 0x0000,

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

    UNIFORM_U32 = 0x1405,     // GL_UNSIGNED_INT
    UNIFORM_U32VEC2 = 0x8DC6, // GL_UNSIGNED_INT_VEC2
    UNIFORM_U32VEC3 = 0x8DC7, // GL_UNSIGNED_INT_VEC3
    UNIFORM_U32VEC4 = 0x8DC8, // GL_UNSIGNED_INT_VEC4

    UNIFORM_BOOL = 0x8B56,     // GL_BOOL
    UNIFORM_BOOLVEC2 = 0x8B57, // GL_BOOL_VEC2
    UNIFORM_BOOLVEC3 = 0x8B58, // GL_BOOL_VEC3
    UNIFORM_BOOLVEC4 = 0x8B59, // GL_BOOL_VEC4

    UNIFORM_SAMPLER1D = 0x8B5D,         // GL_SAMPLER_1D
    UNIFORM_SAMPLER1D_ARRAY = 0x8DC0,   // GL_SAMPLER_1D_ARRAY
    UNIFORM_SAMPLER2D = 0x8B5E,         // GL_SAMPLER_2D
    UNIFORM_SAMPLER2D_ARRAY = 0x8DC1,   // GL_SAMPLER_2D_ARRAY
    UNIFORM_SAMPLER2DMS = 0x9108,       // GL_SAMPLER_2D_MULTISAMPLE
    UNIFORM_SAMPLER2DMS_ARRAY = 0x910B, // GL_SAMPLER_2D_MULTISAMPLE_ARRAY
    UNIFORM_SAMPLER3D = 0x8B5F,         // GL_SAMPLER_3D
    UNIFORM_SAMPLER_CUBE = 0x8B60,      // GL_SAMPLER_CUBE
    UNIFORM_SAMPLER_BUFFER = 0x8DC2,    // GL_SAMPLER_BUFFER

    UNIFORM_ISAMPLER1D = 0x8DC9,         // GL_INT_SAMPLER_1D
    UNIFORM_ISAMPLER1D_ARRAY = 0x8DCE,   // GL_INT_SAMPLER_1D_ARRAY
    UNIFORM_ISAMPLER2D = 0x8DCA,         // GL_INT_SAMPLER_2D
    UNIFORM_ISAMPLER2D_ARRAY = 0x8DCF,   // GL_INT_SAMPLER_2D_ARRAY
    UNIFORM_ISAMPLER2DMS = 0x9109,       // GL_INT_SAMPLER_2D_MULTISAMPLE
    UNIFORM_ISAMPLER2DMS_ARRAY = 0x910C, // GL_INT_SAMPLER_2D_MULTISAMPLE_ARRAY
    UNIFORM_ISAMPLER3D = 0x8DCB,         // GL_INT_SAMPLER_3D
    UNIFORM_ISAMPLER_CUBE = 0x8DCC,      // GL_INT_SAMPLER_CUBE
    UNIFORM_ISAMPLER_BUFFER = 0x8DD0,    // GL_INT_SAMPLER_BUFFER

    UNIFORM_USAMPLER1D = 0x8DD1,         // GL_UNSIGNED_INT_SAMPLER_1D
    UNIFORM_USAMPLER1D_ARRAY = 0x8DD6,   // GL_UNSIGNED_INT_SAMPLER_1D_ARRAY
    UNIFORM_USAMPLER2D = 0x8DD2,         // GL_UNSIGNED_INT_SAMPLER_2D
    UNIFORM_USAMPLER2D_ARRAY = 0x8DD7,   // GL_UNSIGNED_INT_SAMPLER_2D_ARRAY
    UNIFORM_USAMPLER2DMS = 0x910A,       // GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE
    UNIFORM_USAMPLER2DMS_ARRAY = 0x910D, // GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY
    UNIFORM_USAMPLER3D = 0x8DD3,         // GL_UNSIGNED_INT_SAMPLER_3D
    UNIFORM_USAMPLER_CUBE = 0x8DD4,      // GL_UNSIGNED_INT_SAMPLER_CUBE
    UNIFORM_USAMPLER_BUFFER = 0x8DD8,    // GL_UNSIGNED_INT_SAMPLER_BUFFER
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
  gl_graphics_pipeline(create_t, gldefs::GLhandle program, gl_shader::stages_bits stages,
                       primitive_mode primitive, polygon_mode poly_mode);

  gl_graphics_pipeline(gl_context& gl, const gl_shader::graphics_set& shaders,
                       primitive_mode primitive, polygon_mode poly_mode);

public:
  static gl_s_expect<gl_graphics_pipeline> create(gl_context& gl,
                                                  const gl_shader::graphics_set& shaders,
                                                  primitive_mode primitive,
                                                  polygon_mode poly_mode);

  static void destroy(gl_context& gl, gl_graphics_pipeline& pipeline) noexcept;

public:
  ntf::optional<u32> uniform_location(gl_context& gl, const char* name);
  u32 uniform_count(gl_context& gl) const;
  uniform_type query_uniform_index(gl_context& gl, uniform_props& props, u32 idx);

public:
  gl_graphics_pipeline& reset_props();

  gl_graphics_pipeline& set_primitive(primitive_mode primitive);
  gl_graphics_pipeline& set_poly_mode(polygon_mode poly_mode);
  gl_graphics_pipeline& set_poly_width(f32 poly_width);

  gl_graphics_pipeline& set_stencil_test(const gl_stencil_test_props& stencil);
  gl_graphics_pipeline& set_depth_test(const gl_depth_test_props& depth);
  gl_graphics_pipeline& set_blending(const gl_blending_props& blending);
  gl_graphics_pipeline& set_culling(const gl_culling_props& culling);

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

static_assert(::shogle::meta::renderer_object_type<gl_graphics_pipeline>);

} // namespace shogle
