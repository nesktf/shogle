#include "shogle.hpp"

#include "imgui.h"

struct TestScene : public ntf::TaskedScene<TestScene> {
  ntf::ResPool<ntf::Shader, ntf::Spritesheet, ntf::ModelRes> pool;

  ntf::uptr<ntf::Sprite> rin;
  ntf::uptr<ntf::Sprite> cirno;
  ntf::uptr<ntf::Sprite> sheet;
  ntf::uptr<ntf::Model> cirno_fumo;

  TestScene() {
    pool.direct_load<ntf::Shader>({
      {.id="generic_2d", .path="res/shaders/generic_2d"},
      {.id="generic_3d", .path="res/shaders/generic_3d"}
    });
    pool.direct_load<ntf::Spritesheet>({
      .id="2hus", .path="_temp/2hus.json"
    });
    pool.direct_load<ntf::ModelRes>({
      .id="cirno_fumo", .path="_temp/models/cirno_fumo/cirno_fumo.obj"
    });
    
    sheet = ntf::make_uptr<ntf::Sprite>(
      pool.get<ntf::Spritesheet>("2hus"),
      pool.get<ntf::Shader>("generic_2d")
    );
    sheet->pos = ntf::vec2{0.0f, 0.0f};
    sheet->scale *= 200.0f;

    rin = ntf::make_uptr<ntf::Sprite>(
      pool.get<ntf::Spritesheet>("2hus"),
      "rin_dance",
      pool.get<ntf::Shader>("generic_2d")
    );
    rin->use_screen_space = true;
    rin->pos = ntf::vec2{700.0f, 100.0f};
    rin->scale *= 200.0f;

    cirno = ntf::make_uptr<ntf::Sprite>(
      pool.get<ntf::Spritesheet>("2hus"),
      "cirno_fall",
      pool.get<ntf::Shader>("generic_2d")
    );
    cirno->use_screen_space = true;
    cirno->pos = ntf::vec2{100.0f, 100.0f};
    cirno->scale *= 200.0f;

    cirno_fumo = ntf::make_uptr<ntf::Model>(
      pool.get<ntf::ModelRes>("cirno_fumo"),
      pool.get<ntf::Shader>("generic_3d")
    );
    cirno_fumo->pos = ntf::vec3{0.0f, -0.25f, -1.0f};
    cirno_fumo->scale = ntf::vec3{0.015f};
    cirno_fumo->rot = ntf::vec3{0.0f, 90.0f, 0.0f};

    float t = 0.0f;
    float rate = 1.0f/12.0f;

    rin->add_task([t, rate](ntf::Sprite* obj, float dt) mutable {
      t+=dt;
      if(t>rate){
        t = 0.0f;
        obj->next_index();
      }
      return false;
    });
    cirno->add_task([t, rate](ntf::Sprite* obj, float dt) mutable {
      t+=dt;
      if(t>rate){
        t = 0.0f;
        obj->next_index();
      }
      return false;
    });
    cirno_fumo->add_task([t](ntf::Model* obj, float dt) mutable {
      t+=dt;
      float b_scale = 0.015f/2.0f;
      float ang_speed = 5.0f;
      float jum_speed = 10.0f;
      obj->rot.y += ang_speed*dt;
      obj->scale.y = b_scale + (b_scale*glm::abs(glm::sin(jum_speed*t)));
      return false;
    });

    this->add_task([](auto, float dt) {
      auto& in = ntf::InputHandler::instance();
      auto& cam = ntf::Shogle::instance().cam3D_default;
      
      auto view = cam.view();

      ntf::vec3 pos = view.pos;
      float speed = 1.0f;

      if (in.is_key_pressed(ntf::KEY_UP)) {
        pos.y += speed*dt;
      } else if (in.is_key_pressed(ntf::KEY_DOWN)) {
        pos.y -= speed*dt;
      }

      if (in.is_key_pressed(ntf::KEY_LEFT)) {
        pos.x -= speed*dt;
      } else if (in.is_key_pressed(ntf::KEY_RIGHT)) {
        pos.x += speed*dt;
      }

      cam.set_pos(pos);

      return false;
    });

    this->add_task([](auto, float dt) {
      auto& in = ntf::InputHandler::instance();
      auto& cam = ntf::Shogle::instance().cam2D_default;

      auto view = cam.view();

      ntf::vec2 center = view.center;
      float speed = 300.0f;

      if (in.is_key_pressed(ntf::KEY_W)) {
        center.y -= speed*dt;
      } else if (in.is_key_pressed(ntf::KEY_S)) {
        center.y += speed*dt;
      }

      if (in.is_key_pressed(ntf::KEY_A)) {
        center.x -= speed*dt;
      } else if (in.is_key_pressed(ntf::KEY_D)) {
        center.x += speed*dt;
      }

      cam.set_center(center);

      float zoom = view.zoom.x;
      speed = 1.0f;

      if (in.is_key_pressed(ntf::KEY_J)) {
        zoom += speed*dt;
      } else if (in.is_key_pressed(ntf::KEY_K)) {
        zoom -= speed*dt;
      }

      cam.set_zoom(zoom);

      float rot = view.rot;
      speed = 10.0f;

      if (in.is_key_pressed(ntf::KEY_L)) {
        rot += speed*dt;
      }

      cam.set_rot(rot);

      return false;
    });
  };

  void update(float dt) override {
  auto& shogle = ntf::Shogle::instance();
    do_tasks(this, dt);

    shogle.enable_depth_test(true);
    cirno_fumo->udraw(dt);

    shogle.enable_depth_test(false);
    sheet->udraw(dt);
    rin->udraw(dt);
    cirno->udraw(dt);
  };

  void ui_draw(void) override {
    auto& eng = ntf::Shogle::instance();
    auto view2d = eng.cam2D_default.view();
    auto view3d = eng.cam3D_default.view();

    static float vals[90];
    static int vals_offset = 0;

    vals[vals_offset] = cirno_fumo->scale.y;
    vals_offset = (vals_offset + 1) % IM_ARRAYSIZE(vals);
    
    ImGui::Begin("scene_state");{
      ImGui::SeparatorText("2D Camera");

      if(ImGui::BeginTable("Camera2D", 2)) {
        ImGui::TableNextColumn();
        ImGui::Text("X: %f", view2d.center.x);
        ImGui::TableNextColumn();
        ImGui::Text("Y: %f", view2d.center.y);

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("ZoomX: %f", view2d.zoom.x);
        ImGui::TableNextColumn();
        ImGui::Text("ZoomY: %f", view2d.zoom.y);

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("Rot: %f rad", view2d.rot);

        ImGui::EndTable();
      }

      ImGui::SeparatorText("3D Camera");

      if(ImGui::BeginTable("Camera3D", 3)) {

        ImGui::TableNextColumn();
        ImGui::Text("X: %f", view3d.pos.x);
        ImGui::TableNextColumn();
        ImGui::Text("Y: %f", view3d.pos.y);
        ImGui::TableNextColumn();
        ImGui::Text("Z: %f", view3d.pos.z);

        ImGui::EndTable();
      }

      ImGui::SeparatorText("Objects");
      ImGui::Text("Rin index: %u", (uint)rin->index());
      ImGui::Text("Cirno index: %u", (uint)cirno->index());

      ImGui::PlotLines("cirno_fumo y scale", vals, IM_ARRAYSIZE(vals), vals_offset, NULL, FLT_MAX, FLT_MAX, ImVec2(0, 80.0f));
    }ImGui::End();
  }
};

int main(int argc, char* argv[]) {
  auto& shogle = ntf::Shogle::instance();
  ntf::Log::set_level(ntf::LogLevel::LOG_VERBOSE);

  if (shogle.init(ntf::Settings{argc, argv, "script/default_settings.lua"})) {
    shogle.enable_blend(true);
    shogle.start(TestScene::create);
    return EXIT_SUCCESS;
  }

  return EXIT_FAILURE;
}

