#include <shogle.hpp>

using namespace ntf;

struct test_imgui : public scene {
  res::pool<render::shader, render::spritesheet, render::model> pool;

  uptr<dynamic_sprite> rin;
  uptr<dynamic_sprite> cirno;
  uptr<dynamic_model> cirno_fumo;

  uptr<sprite> sheet;

  test_imgui(shogle_state& state) : 
    pool(state.loader) {
    pool.direct_request<render::shader>({
      {.id="generic_2d", .path="res/shaders/generic_2d"},
      {.id="generic_3d", .path="res/shaders/generic_3d"}
    });
    pool.direct_request<render::spritesheet>({
      {.id="2hus", .path="_temp/2hus.json"}
    });
    pool.direct_request<render::model>({
      {.id="cirno_fumo", .path="_temp/models/cirno_fumo/cirno_fumo.obj"}
    });
  }

  void on_create(shogle_state& state) override {
    sheet = make_uptr<dynamic_sprite>(
      pool.get<render::spritesheet>("2hus")->get_sprite("__sheet"),
      pool.get<render::shader>("generic_2d"),
      state.cam_2d
    );
    sprite::move(*sheet, vec2{0.0f, 0.0f});
    sprite::scale(*sheet, sheet->corrected_scale(200.0f));

    rin = make_uptr<dynamic_sprite>(
      pool.get<render::spritesheet>("2hus")->get_sprite("rin_dance"),
      pool.get<render::shader>("generic_2d"),
      state.cam_2d
    );
    sprite::toggle_screen_space(*rin, true);
    sprite::move(*rin, vec2{700.0f, 100.0f});
    sprite::scale(*rin, rin->corrected_scale(200.0f));

    cirno = make_uptr<dynamic_sprite>(
      pool.get<render::spritesheet>("2hus")->get_sprite("cirno_fall"),
      pool.get<render::shader>("generic_2d"),
      state.cam_2d
    );
    sprite::toggle_screen_space(*cirno, true);
    sprite::move(*cirno, vec2{100.0f, 100.0f});
    sprite::scale(*cirno, cirno->corrected_scale(200.0f));

    cirno_fumo = make_uptr<dynamic_model>(
      pool.get<render::model>("cirno_fumo"),
      pool.get<render::shader>("generic_3d"),
      state.cam_3d
    );
    model::move(*cirno_fumo, vec3{0.0f, -0.25f, -1.0f});
    model::rotate(*cirno_fumo, vec3{0.0f, 90.0, 0.0f});
    model::scale(*cirno_fumo, vec3{0.015f});

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
      model::rotate(*obj, ang_speed*t, vec3{0.0f, 1.0f, 0.0f});
      obj->_scale.y = b_scale + (b_scale*glm::abs(glm::sin(jum_speed*t)));
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
    gl::depth_test(true);
    cirno_fumo->draw();

    gl::depth_test(false);
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

    vals[vals_offset] = cirno_fumo->_scale.y;
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

  static uptr<scene> create(shogle_state& state) { return make_uptr<test_imgui>(state); }
};

int main(int argc, char* argv[]) {
  Log::set_level(LogLevel::LOG_VERBOSE);

  std::string sett_path {SHOGLE_RESOURCES};
  sett_path += "/script/default_settings.lua";
  settings sett{argc, argv, sett_path.c_str()};

  auto shogle = shogle_create(sett.w_width, sett.w_height, sett.w_title);
  gl::blend(true);
  shogle_start(shogle, test_imgui::create);

  return EXIT_SUCCESS;
}

