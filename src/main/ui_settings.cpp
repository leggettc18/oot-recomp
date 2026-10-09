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
  std::atomic<ultramodern::renderer::AspectRatio> g_orig_aspect_ratio { ultramodern::renderer::AspectRatio::Expand };

  void push_to_runtime() {
    ultramodern::renderer::GraphicsConfig graphics_config = ultramodern::renderer::get_graphics_config();
    const bool prerendered = g_prerendered_room.load(std::memory_order_relaxed);
    if (prerendered) {
      g_orig_aspect_ratio.store(graphics_config.ar_option, std::memory_order_relaxed);
    }
    const ultramodern::renderer::AspectRatio orig_ratio = g_orig_aspect_ratio.load(std::memory_order_relaxed);
    graphics_config.ar_option = !prerendered ? orig_ratio : ultramodern::renderer::AspectRatio::Original;
    ultramodern::renderer::set_graphics_config(graphics_config);
    auto tmp = recompui::config::get_graphics_config();
    tmp.set_option_value(recompui::config::graphics::options::ar_option, (uint32_t)graphics_config.ar_option);
    tmp.update_option_disabled(recompui::config::graphics::options::ar_option, prerendered);
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
