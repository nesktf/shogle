#include <shogle/render/gl/render.hpp>

#include <shogle/core/log.hpp>
#include <shogle/core/error.hpp>

namespace ntf::shogle::gl {

void init(GLADloadproc proc) {
  if (!gladLoadGLLoader(proc)) {
    throw ntf::error{"[gl::init] Failed to load GLAD"};
  }
}

void terminate() {}

void set_viewport_size(vec2sz sz) {
  glViewport(0, 0, sz.w, sz.h);
}

void clear_viewport(vec4 color, clear flag) {
  GLbitfield mask = GL_COLOR_BUFFER_BIT;
  if ((flag & clear::depth) != clear::none) {
    mask |= GL_DEPTH_BUFFER_BIT;
  }
  if ((flag & clear::stencil) != clear::none) {
    mask |= GL_STENCIL_BUFFER_BIT;
  }
  glClearColor(color.r, color.g, color.b, color.a);
  glClear(mask);
}


void set_stencil_test(bool flag) {
  if (flag) {
    glEnable(GL_STENCIL_TEST);
  } else {
    glDisable(GL_STENCIL_TEST);
  }
}

void set_depth_test(bool flag) {
  if (flag) {
    glEnable(GL_DEPTH_TEST);
  } else {
    glDisable(GL_DEPTH_TEST);
  }
}

void set_blending(bool flag) {
  if (flag) {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);  
  } else {
    glDisable(GL_BLEND);
  }
}

void set_depth_fun(depth_fun fun) {
  switch (fun) {
    case depth_fun::less: {
      glDepthFunc(GL_LESS);
      break;
    }
    case depth_fun::lequal: {
      glDepthFunc(GL_LEQUAL);
      break;
    }
  }
}

} // namespace ntf::shogle::gl
