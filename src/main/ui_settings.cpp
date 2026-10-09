#include "recomp.h"
#include "recompui/recompui.h"
#include "recompui/renderer.h"
#include "librecomp/helpers.hpp"
#include "../patches/input.h"
#include "../patches/graphics.h"
#include "../patches/sound.h"
#include "ultramodern/ultramodern.hpp"
#include "ultramodern/config.hpp"
#include "recompui/config.h"
#include "ui_settings.h"

namespace {
  oot::ui::Settings g_live;
  std::atomic<bool> g_prerendered_room { false };

  void push_to_runtime() {
    ultramodern::renderer::GraphicsConfig graphics_config = ultramodern::renderer::get_graphics_config();
    const bool prerendered = g_prerendered_room.load(std::memory_order_relaxed);
    const ultramodern::renderer::AspectRatio orig_ratio = (ultramodern::renderer::AspectRatio)std::get<uint32_t>(recompui::config::get_graphics_config().get_option_value(recompui::config::graphics::options::ar_option));
    graphics_config.ar_option = !prerendered ? orig_ratio : ultramodern::renderer::AspectRatio::Original;
    ultramodern::renderer::set_graphics_config(graphics_config);
  }
}

namespace oot::ui {
  bool Settings::operator==(const Settings& other) const {
    return aspect == other.aspect;
  }

  bool prerendered_room() {
    return g_prerendered_room.load(std::memory_order_relaxed);
  }

  void set_prerendered_room(bool on, int cam_setting) {
    constexpr int STEADY_FRAMES = 4;

    static bool applied = false;
    static bool pending = false;
    static int pending_frames = 0;

    if (on == applied) {
      pending_frames = 0;
      return;
    }
    if (on != pending) {
      pending = on;
      pending_frames = 0;
    }
    if (++pending_frames < STEADY_FRAMES) {
      return;
    }

    applied = on;
    pending_frames = 0;
    g_prerendered_room.store(on, std::memory_order_relaxed);
    std::fprintf(stderr, "[gfx] prerendered room %s (camera setting %d): the view is %s\n",
        on ? "entered" : "left", cam_setting,
        on ? "forced to the console's 4:3" : "back to the settings");
    push_to_runtime();
  }
}
