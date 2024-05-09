#include <shogle.hpp>

using namespace ntf;

struct test_imgui : public scene {
  uptr<dynamic_sprite> rin;
  uptr<dynamic_sprite> cirno;
  uptr<dynamic_model> cirno_fumo;

  uptr<sprite> sheet;

  render::shader generic_2d {"res/shaders/generic_2d"};
  render::shader generic_3d {"res/shaders/generic_3d"};

  render::spritesheet toohus {"_temp/2hus.json"};

  render::model cino {"_temp/models/cirno_fumo/cirno_fumo.obj"};

  void on_create(shogle_state& state) override {
    sheet = make_uptr<dynamic_sprite>(
      toohus["__sheet"],
      &generic_2d,
      &state.cam_2d
    );
    set_pos(*sheet, vec2{0.0f, 0.0f});
    scale(*sheet, 200.0f);

    rin = make_uptr<dynamic_sprite>(
      toohus["rin_dance"],
      &generic_2d,
      &state.cam_2d
    );
    rin->toggle_screen_space(true);
    set_pos(*rin, vec2{700.0f, 100.0f});
    scale(*rin, 200.0f);

    cirno = make_uptr<dynamic_sprite>(
      toohus["cirno_fall"],
      &generic_2d,
      &state.cam_2d
    );
    cirno->toggle_screen_space(true);
    set_pos(*cirno, vec2{100.0f, 100.0f});
    scale(*cirno, 200.0f);

    cirno_fumo = make_uptr<dynamic_model>(
      &cino,
      &generic_3d,
      &state.cam_3d
    );
    set_pos(*cirno_fumo, vec3{0.0f, -0.25f, -1.0f});
    set_rotation(*cirno_fumo, PI*0.5f, {0.0f, 1.0f, 0.0f});
    set_scale(*cirno_fumo, vec3{0.015f});

    float t = 0.0f;
    float rate = 1.0f/12.0f;

    rin->add_task([t, rate](dynamic_sprite* obj, float dt) mutable {
      t+=dt;
      if(t>rate){
        t = 0.0f;
        obj->next_index();
      }
      return false;
    });
    cirno->add_task([t, rate](dynamic_sprite* obj, float dt) mutable {
      t+=dt;
      if(t>rate){
        t = 0.0f;
        obj->next_index();
      }
      return false;
    });
    cirno_fumo->add_task([t](dynamic_model* obj, float dt) mutable {
      t+=dt;
      float b_scale = 0.015f/2.0f;
      float ang_speed = 5.0f;
      float jum_speed = 10.0f;
      rotate(*obj, ang_speed*dt, vec3{0.0f, 1.0f, 0.0f});
      obj->scale.y = b_scale + (b_scale*glm::abs(glm::sin(jum_speed*t)));
      return false;
    });
    state.input.subscribe(key::ESCAPE, key::PRESS, [&state]() {
      shogle_close_window(state);
    });
  };

  void update_cameras(shogle_state& state, float dt) {
    auto& in {state.input};
    auto& cam3d {state.cam_3d};
    auto& cam2d {state.cam_2d};
    
    float speed3d = 1.0f;
    auto pos3d = cam3d.pos();

    if (in.is_key_pressed(key::UP)) {
      pos3d.y += speed3d*dt;
    } else if (in.is_key_pressed(key::DOWN)) {
      pos3d.y -= speed3d*dt;
    }

    if (in.is_key_pressed(key::LEFT)) {
      pos3d.x -= speed3d*dt;
    } else if (in.is_key_pressed(key::RIGHT)) {
      pos3d.x += speed3d*dt;
    }
    cam3d.set_pos(pos3d);
    cam3d.update();

    float speed2d_pos = 300.0f;
    float speed2d_zoom = 1.0f;
    float speed2d_rot = 10.0f;

    auto center = cam2d.center();
    auto zoom = cam2d.zoom();
    auto rot = cam2d.rot();

    if (in.is_key_pressed(key::W)) {
      center.y -= speed2d_pos*dt;
    } else if (in.is_key_pressed(key::S)) {
      center.y += speed2d_pos*dt;
    }

    if (in.is_key_pressed(key::A)) {
      center.x -= speed2d_pos*dt;
    } else if (in.is_key_pressed(key::D)) {
      center.x += speed2d_pos*dt;
    }


    if (in.is_key_pressed(key::J)) {
      zoom += speed2d_zoom*dt;
    } else if (in.is_key_pressed(key::K)) {
      zoom -= speed2d_zoom*dt;
    }

    if (in.is_key_pressed(key::L)) {
      rot += speed2d_rot*dt;
    }

    cam2d.set_center(center);
    cam2d.set_zoom(zoom);
    cam2d.set_rot(rot);
    cam2d.update();
  }

  void update(shogle_state& state, float dt) override {
    update_cameras(state, dt);

    cirno_fumo->update(dt);

    sheet->update(dt);
    rin->update(dt);
    cirno->update(dt);
  };

  void draw(shogle_state& state) override {
    render::gl::depth_test(true);
    cirno_fumo->draw();

    render::gl::depth_test(false);
    sheet->draw();
    rin->draw();
    cirno->draw();

    ui_draw(state);
  }

  void ui_draw(shogle_state& state) {
    auto center = state.cam_2d.center();
    auto zoom = state.cam_2d.zoom();
    auto rot = state.cam_2d.rot();

    auto pos = state.cam_3d.pos();

    static float vals[90];
    static int vals_offset = 0;

    vals[vals_offset] = cirno_fumo->scale.y;
    vals_offset = (vals_offset + 1) % IM_ARRAYSIZE(vals);
    
    ImGui::Begin("scene_state");{
      ImGui::SeparatorText("2D Camera");

      if(ImGui::BeginTable("Camera2D", 2)) {
        ImGui::TableNextColumn();
        ImGui::Text("X: %f", center.x);
        ImGui::TableNextColumn();
        ImGui::Text("Y: %f", center.y);

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("ZoomX: %f", zoom.x);
        ImGui::TableNextColumn();
        ImGui::Text("ZoomY: %f", zoom.y);

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("Rot: %f rad", rot);

        ImGui::EndTable();
      }

      ImGui::SeparatorText("3D Camera");

      if(ImGui::BeginTable("Camera3D", 3)) {

        ImGui::TableNextColumn();
        ImGui::Text("X: %f", pos.x);
        ImGui::TableNextColumn();
        ImGui::Text("Y: %f", pos.y);
        ImGui::TableNextColumn();
        ImGui::Text("Z: %f", pos.z);

        ImGui::EndTable();
      }

      ImGui::SeparatorText("Objects");
      ImGui::Text("Rin index: %u", (uint)rin->index());
      ImGui::Text("Cirno index: %u", (uint)cirno->index());

      ImGui::PlotLines("cirno_fumo y scale", vals, IM_ARRAYSIZE(vals), vals_offset, NULL, FLT_MAX, FLT_MAX, ImVec2(0, 80.0f));
    }ImGui::End();
  }

  static uptr<scene> create(void) { return make_uptr<test_imgui>(); }
};

int main(void) {
  Log::set_level(LogLevel::LOG_VERBOSE);

  auto shogle = shogle_create(800, 600, shogle_gen_title("test_imgui"));
  shogle_main_loop(shogle, test_imgui::create);

  return EXIT_SUCCESS;
}

