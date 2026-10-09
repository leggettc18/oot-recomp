#ifndef __ZELDA_CONFIG_H__
#define __ZELDA_CONFIG_H__

#include <filesystem>
#include <string>
#include <string_view>
#include "ultramodern/config.hpp"

namespace oot {
  inline const std::u8string program_id = u8"OotRecompiled";
  inline const std::string program_name = "Ocarina of Time: Recompiled";
}

namespace zelda64 {
  namespace configkeys {
    namespace general {
      inline const std::string camera_invert_mode = "camera_invert_mode";
      inline const std::string analog_camera_invert_mode = "analog_camera_invert_mode";
      inline const std::string analog_cam_mode = "analog_cam_mode";
    }
    namespace sound {
      inline const std::string bgm_volume = "bgm_volume";
    }
    namespace graphics {
      inline const std::string cutscene_aspect_ratio_mode = "cutscene_aspect_ratio_mode";
    }
  }
    // TODO: Move loading configs to the runtime once we have a way to allow per-project customization.
    void load_config();
    void save_config();
    
    void reset_input_bindings();
    void reset_cont_input_bindings();
    void reset_kb_input_bindings();
    // void reset_single_input_binding(recomp::InputDevice device, recomp::GameInput input);

    std::filesystem::path get_app_folder_path();
    
    bool get_debug_mode_enabled();
    void set_debug_mode_enabled(bool enabled);

    void init_config();
    
    enum class AutosaveMode {
        On,
        Off,
        OptionCount
    };

    NLOHMANN_JSON_SERIALIZE_ENUM(zelda64::AutosaveMode, {
        {zelda64::AutosaveMode::On, "On"},
        {zelda64::AutosaveMode::Off, "Off"}
    });

    enum class TargetingMode {
        Switch,
        Hold,
        OptionCount
    };

    NLOHMANN_JSON_SERIALIZE_ENUM(zelda64::TargetingMode, {
        {zelda64::TargetingMode::Switch, "Switch"},
        {zelda64::TargetingMode::Hold, "Hold"}
    });

    TargetingMode get_targeting_mode();
    void set_targeting_mode(TargetingMode mode);

    enum class CameraInvertMode {
        InvertNone,
        InvertX,
        InvertY,
        InvertBoth,
        OptionCount
    };

    NLOHMANN_JSON_SERIALIZE_ENUM(zelda64::CameraInvertMode, {
        {zelda64::CameraInvertMode::InvertNone, "InvertNone"},
        {zelda64::CameraInvertMode::InvertX, "InvertX"},
        {zelda64::CameraInvertMode::InvertY, "InvertY"},
        {zelda64::CameraInvertMode::InvertBoth, "InvertBoth"}
    });

    CameraInvertMode get_camera_invert_mode();
    void set_camera_invert_mode(CameraInvertMode mode);

    CameraInvertMode get_analog_camera_invert_mode();
    void set_analog_camera_invert_mode(CameraInvertMode mode);

    enum class AnalogCamMode {
        On,
        Off,
		OptionCount
    };

    NLOHMANN_JSON_SERIALIZE_ENUM(zelda64::AnalogCamMode, {
        {zelda64::AnalogCamMode::On, "On"},
        {zelda64::AnalogCamMode::Off, "Off"}
    });

    AutosaveMode get_autosave_mode();
    void set_autosave_mode(AutosaveMode mode);

    AnalogCamMode get_analog_cam_mode();
    void set_analog_cam_mode(AnalogCamMode mode);

    enum class CutsceneAspectRatioMode {
      Original,
      Clamp16x9,
      Full,
      OptionCount
    };

    CutsceneAspectRatioMode get_cutscene_aspect_ratio_mode();

    void open_quit_game_prompt();
};

#endif
