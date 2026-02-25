#include <shogle/math/transform.hpp>
#include <shogle/render/data.hpp>
#include <shogle/render/opengl.hpp>
#include <shogle/render/window.hpp>

#include <chimatools/chimatools.hpp>

#define TINYOBJLOADER_IMPLEMENTATION
#include "./tiny_obj_loader.h"

namespace {

using namespace shogle::numdefs;

constexpr char vert_src[] = R"glsl(
#version 440 core

layout (location = 0) in vec3 att_pos;
layout (location = 1) in vec3 att_norm;
layout (location = 2) in vec2 att_uvs;

layout (location = 0) out vec3 frag_norm;
layout (location = 1) out vec2 frag_uvs;

uniform mat4 u_proj;
uniform mat4 u_model;

void main() {
  gl_Position = u_proj*u_model*vec4(att_pos, 1.0f);
  frag_norm = att_norm;
  frag_uvs = att_uvs;
}  
)glsl";

constexpr char frag_src[] = R"glsl(
#version 440 core

layout (location = 0) in vec3 frag_norm;
layout (location = 1) in vec2 frag_uvs;

layout (location = 0) out vec4 out_color;

uniform sampler2D u_tex;
  
void main() {
  out_color = texture(u_tex, frag_uvs);
}
)glsl";

struct model_data {
  std::vector<shogle::vec3> positions;
  std::vector<shogle::vec3> normals;
  std::vector<shogle::vec2> uvs;
  chima::image diffuse;
};

shogle::expected<model_data, std::string> load_model(chima::context_view chima) {
  tinyobj::ObjReaderConfig reader_config;
  reader_config.mtl_search_path = RES_FOLDER "/cirno_fumo";
  static const char file_path[] = RES_FOLDER "/cirno_fumo/cirno_fumo.obj";
  shogle::logger::info("Model path: {}", file_path);

  tinyobj::ObjReader reader;
  if (!reader.ParseFromFile(file_path, reader_config)) {
    if (!reader.Error().empty()) {
      return {shogle::unexpect, reader.Error()};
    } else {
      return {shogle::unexpect, "Can't load model"};
    }
  }

  const auto& mat = reader.GetMaterials()[0];
  const auto& attr = reader.GetAttrib();
  const auto& mesh = reader.GetShapes()[0].mesh;
  std::vector<shogle::vec3> pos_data;
  std::vector<shogle::vec3> norm_data;
  std::vector<shogle::vec2> uv_data;

  u32 face_offset = 0;
  for (u32 face : mesh.num_face_vertices) {
    for (u32 vertex = 0; vertex < face; ++vertex) {
      tinyobj::index_t idx = mesh.indices[face_offset + vertex];
      assert(idx.vertex_index >= 0);
      assert(idx.normal_index >= 0);
      assert(idx.texcoord_index >= 0);
      pos_data.emplace_back(attr.vertices[3 * idx.vertex_index + 0],
                            attr.vertices[3 * idx.vertex_index + 1],
                            attr.vertices[3 * idx.vertex_index + 2]);
      norm_data.emplace_back(attr.normals[3 * idx.normal_index + 0],
                             attr.normals[3 * idx.normal_index + 1],
                             attr.normals[3 * idx.normal_index + 2]);
      uv_data.emplace_back(attr.texcoords[2 * idx.texcoord_index + 0],
                           attr.texcoords[2 * idx.texcoord_index + 1]);
    }
    face_offset += face;
  }
  const auto diffuse_path =
    fmt::format("{}/{}", reader_config.mtl_search_path, mat.diffuse_texname);
  shogle::logger::info("Diffuse path: {}", diffuse_path);
  chima::image diffuse(chima, CHIMA_DEPTH_8U, diffuse_path.c_str());

  return {shogle::in_place, std::move(pos_data), std::move(norm_data), std::move(uv_data),
          diffuse};
}

struct fumo_vert_layout {
public:
  static constexpr size_t attribute_count = 3u;

public:
  fumo_vert_layout(size_t nverts) noexcept :
      _pos_stride(nverts * sizeof(shogle::vec3)), _norm_stride(nverts * sizeof(shogle::vec3)),
      _uv_stride(nverts * sizeof(shogle::vec2)) {}

public:
  shogle::vertex_attrib_array<attribute_count> attributes() const noexcept {
    return std::to_array<shogle::vertex_attribute>({
      {.location = 0,
       .type = shogle::attribute_type::vec3,
       .offset = pos_offset(),
       .stride = sizeof(shogle::vec3)},
      {.location = 1,
       .type = shogle::attribute_type::vec3,
       .offset = norm_offset(),
       .stride = sizeof(shogle::vec3)},
      {.location = 2,
       .type = shogle::attribute_type::vec2,
       .offset = uv_offset(),
       .stride = sizeof(shogle::vec2)},
    });
  }

public:
  size_t buffer_stride() const noexcept { return _pos_stride + _norm_stride + _uv_stride; }

  size_t pos_stride() const noexcept { return _pos_stride; }

  size_t pos_offset() const noexcept { return 0; }

  size_t norm_stride() const noexcept { return _norm_stride; }

  size_t norm_offset() const noexcept { return _pos_stride; }

  size_t uv_stride() const noexcept { return _uv_stride; }

  size_t uv_offset() const noexcept { return _pos_stride + _norm_stride; }

private:
  size_t _pos_stride, _norm_stride, _uv_stride;
};

} // namespace

int main() {
  shogle::logger::set_level(shogle::logger::LEVEL_VERBOSE);

  chima::context chima;
  chima.set_flip_y(true);
  auto cirno = load_model(chima);
  if (!cirno) {
    shogle::logger::error("Failed to load model: {}", cirno.error());
    return EXIT_FAILURE;
  }
  chima::scoped_resource diffuse_defer(chima, cirno->diffuse);
  const auto [diffuse_w, diffuse_h] = cirno->diffuse.extent();
  const size_t nverts = cirno->positions.size();
  fumo_vert_layout fumo_layout(nverts);

  f32 win_w = 800;
  f32 win_h = 600;

  const auto glfw = shogle::glfw_win::initialize_lib();
  const auto hints = shogle::glfw_gl_hints::make_default(4, 4);
  shogle::glfw_win win((u32)win_w, (u32)win_h, "test", hints);
  shogle::gl_context gl(win);

  bool pause = false;
  win.set_viewport_callback([&](auto, const shogle::extent2d& vp) {
    win_w = (f32)vp.width;
    win_h = (f32)vp.height;
  });
  win.set_key_input_callback([&](auto, const shogle::glfw_key_data& key) {
    if (key.key == GLFW_KEY_SPACE && key.action == GLFW_PRESS) {
      pause = !pause;
    }
  });

  // Important: We are using a SoA vertex layout
  shogle::gl_buffer vertices(gl, fumo_layout.buffer_stride());
  shogle::gl_defer vertices_defer(gl, vertices);
  shogle::gl_layout_builder layout_builder;
  auto layout = layout_builder.set_vertex_buffer(vertices).build(gl, fumo_layout).value();
  const shogle::gl_defer layout_defer(gl, layout);
  layout.vertex_buffer()
    ->upload_data(gl, cirno->positions.data(), fumo_layout.pos_stride(), fumo_layout.pos_offset())
    .value();
  layout.vertex_buffer()
    ->upload_data(gl, cirno->normals.data(), fumo_layout.norm_stride(), fumo_layout.norm_offset())
    .value();
  layout.vertex_buffer()
    ->upload_data(gl, cirno->uvs.data(), fumo_layout.uv_stride(), fumo_layout.uv_offset())
    .value();

  shogle::gl_texture_builder tex_builder;
  auto tex = tex_builder.set_type(shogle::gl_texture::TEX_TYPE_2D)
               .set_format(shogle::gl_texture::TEX_FORMAT_RGB8)
               .set_extent(shogle::extent2d(diffuse_w, diffuse_h))
               .set_min_sampler(shogle::gl_texture::SAMPLER_MIN_LINEAR)
               .set_mag_sampler(shogle::gl_texture::SAMPLER_MAG_LINEAR)
               .build(gl)
               .value();
  shogle::gl_defer tex_scope(gl, tex);
  const shogle::gl_texture::image_data diffuse_data{
    .data = cirno->diffuse.data(),
    .extent = {diffuse_w, diffuse_h, 1},
    .format = shogle::gl_texture::PIXEL_FORMAT_RGB,
    .datatype = shogle::gl_texture::PIXEL_TYPE_U8,
    .alignment = shogle::gl_texture::ALIGN_4BYTES,
  };
  tex.upload_image(gl, diffuse_data).value();
  tex.generate_mipmaps(gl);

  shogle::gl_shader vert_shader(gl, vert_src, sizeof(vert_src), shogle::gl_shader::STAGE_VERTEX);
  const shogle::gl_defer vert_shader_defer(gl, vert_shader);
  shogle::gl_shader frag_shader(gl, frag_src, sizeof(frag_src), shogle::gl_shader::STAGE_FRAGMENT);
  const shogle::gl_defer frag_shader_defer(gl, frag_shader);

  shogle::gl_pipeline_builder pipeline_builder;
  auto pipeline = pipeline_builder.set_primitive(shogle::gl_pipeline::PRIMITIVE_TRIANGLES)
                    .set_polygon_mode(shogle::gl_pipeline::POLY_MODE_FILL)
                    .set_polygon_width(1.f)
                    .add_shader(vert_shader)
                    .add_shader(frag_shader)
                    .set_depth_test(shogle::gl_depth_test_props::make_default(true))
                    .build(gl)
                    .value();
  const shogle::gl_defer pipeline_defer(gl, pipeline);
  const auto u_model = pipeline.uniform_location(gl, "u_model").value();
  const auto u_proj = pipeline.uniform_location(gl, "u_proj").value();
  const auto u_tex = pipeline.uniform_location(gl, "u_tex").value();

  shogle::gl_clear_builder clear_builder;
  const auto frame_clear = clear_builder.set_clear_color(.3f, .3f, .3f, 1.f)
                             .set_clear_flag(shogle::gl_clear_opts::CLEAR_COLOR)
                             .set_clear_flag(shogle::gl_clear_opts::CLEAR_DEPTH)
                             .build();

  f32 t = 0.f;
  const f32 fumo_scale = 0.025f;
  shogle::gl_cmd_builder cmd_builder;
  shogle::render_loop(win, [&](f64 dt) {
    if (win.poll_key(GLFW_KEY_ESCAPE) == GLFW_PRESS) {
      win.close();
    }
    if (!pause) {
      t += (f32)dt;
    }

    shogle::mat4 model(1.f);
    const f32 yscale =
      fumo_scale * .5f + fumo_scale * .5f * std::abs(std::sin(3 * shogle::math::pi<f32> * t));
    model = shogle::math::translate(model, shogle::vec3(0.f, -.25f, -.75f));
    model =
      shogle::math::rotate(model, t * 1.2f * shogle::math::pi<f32>, shogle::vec3(0.f, 1.f, 0.f));
    model = shogle::math::scale(model, shogle::vec3(fumo_scale, yscale, fumo_scale));
    const auto proj = shogle::math::perspective(shogle::math::rad(90.f), win_w / win_h, .1f, 10.f);

    gl.start_frame(frame_clear);
    cmd_builder.reset();
    const auto cmd = cmd_builder.set_vertex_layout(layout)
                       .set_pipeline(pipeline)
                       .set_draw_count(nverts)
                       .add_texture(tex, 0)
                       .add_uniform(proj, u_proj)
                       .add_uniform(model, u_model)
                       .add_uniform(0, u_tex)
                       .build();
    gl.submit_immediate_command(cmd);
    gl.end_frame();
  });

  return EXIT_SUCCESS;
}
