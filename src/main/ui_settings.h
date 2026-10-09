#pragma once

#include <cstdint>
#include <string>

namespace oot::ui {
  struct Settings {
    int aspect = 0;
    bool operator==(const Settings& other) const;
    bool operator!=(const Settings& other) const { return !(*this == other); }
  };

  const Settings& settings();

  void load_settings(const std::string& path);
  bool apply(const Settings& next);
  void clamp(Settings& s);

  void set_prerendered_room(bool on, int cam_setting);
  bool prerendered_room();
} // namespace oot::ui
