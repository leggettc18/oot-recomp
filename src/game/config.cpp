#include "zelda_config.h"
#include "recompui/recompui.h"
#include "recompui/config.h"
#include "recompinput/recompinput.h"
#include "zelda_sound.h"
#include "zelda_support.h"
#include "ultramodern/config.hpp"
#include "librecomp/files.hpp"
#include "librecomp/config.hpp"
#include "util/file.h"
#include <filesystem>
#include <fstream>
#include <iomanip>

#if defined(_WIN32)
#include <Shlobj.j>
#elif defined(__linux__)
#include <unistd.h>
#include <pwd.h>
#elif defined(__APPLE__)
#include "apple/rt64_apple.h"
#endif

static void add_general_options(recomp::config::Config &config) {
  using EnumOptionVector = const std::vector<recomp::config::ConfigOptionEnumOption>;

  static EnumOptionVector analog_cam_mode_options = {
    {zelda64::AnalogCamMode::Off, "Off", "Off"},
    {zelda64::AnalogCamMode::On, "On", "On"},
  };
  config.add_enum_option(
    zelda64::configkeys::general::analog_cam_mode,
    "Analog Camera",
    "Enables the analog camera.",
    analog_cam_mode_options,
    zelda64::AnalogCamMode::Off
  );

  static EnumOptionVector camera_invert_mode_options = {
    {zelda64::CameraInvertMode::InvertNone, "InvertNone", "None"},
    {zelda64::CameraInvertMode::InvertX, "InvertX", "Invert X"},
    {zelda64::CameraInvertMode::InvertY, "InvertY", "Invert Y"},
    {zelda64::CameraInvertMode::InvertBoth, "InvertBoth", "Invert Both"}
  };
  config.add_enum_option(
    zelda64::configkeys::general::camera_invert_mode,
    "Invert Camera",
    "Inverts the camera controls.",
    camera_invert_mode_options,
    zelda64::CameraInvertMode::InvertX
  );
}

template <typename T = uint32_t>
T get_general_config_enum_value(const std::string& option_id) {
  return static_cast<T>(std::get<uint32_t>(recompui::config::get_general_config().get_option_value(option_id)));
}

template <typename T = uint32_t>
T get_general_config_number_value(const std::string& option_id) {
  return static_cast<T>(std::get<double>(recompui::config::get_general_config().get_option_value(option_id)));
}

zelda64::CameraInvertMode zelda64::get_camera_invert_mode() {
  return get_general_config_enum_value<zelda64::CameraInvertMode>(zelda64::configkeys::general::camera_invert_mode);
}

zelda64::CameraInvertMode zelda64::get_analog_camera_invert_mode() {
  return get_general_config_enum_value<zelda64::CameraInvertMode>(zelda64::configkeys::general::analog_camera_invert_mode);
}

zelda64::AnalogCamMode zelda64::get_analog_cam_mode() {
  return get_general_config_enum_value<zelda64::AnalogCamMode>(zelda64::configkeys::general::analog_cam_mode);
}

static void add_sound_options(recomp::config::Config &config) {
  config.add_percent_number_option(
      zelda64::configkeys::sound::bgm_volume,
      "Background Music Volume",
      "Controls the overall volume of background music.",
      100.0f
  );
}

static void add_graphics_options(recomp::config::Config &config) {
  using EnumOptionVector = const std::vector<recomp::config::ConfigOptionEnumOption>;
  static EnumOptionVector cutscene_aspect_ratio_mode_options = {
    {zelda64::CutsceneAspectRatioMode::Original, "Original", "Original"},
    {zelda64::CutsceneAspectRatioMode::Clamp16x9, "Clamp16x9", "16:9"},
    {zelda64::CutsceneAspectRatioMode::Full, "Full", "Full"},
  };
  config.add_enum_option(
    zelda64::configkeys::graphics::cutscene_aspect_ratio_mode,
    "Cutscene Aspect Ratio",
    "Sets the aspect ratio limit for cutscenes.",
    cutscene_aspect_ratio_mode_options,
    zelda64::CutsceneAspectRatioMode::Clamp16x9
  );
}

template <typename T = uint32_t>
T get_graphics_config_enum_value(const std::string& option_id) {
    return static_cast<T>(std::get<uint32_t>(recompui::config::get_graphics_config().get_option_value(option_id)));
}

zelda64::CutsceneAspectRatioMode zelda64::get_cutscene_aspect_ratio_mode() {
  return get_graphics_config_enum_value<zelda64::CutsceneAspectRatioMode>(zelda64::configkeys::graphics::cutscene_aspect_ratio_mode);
}

template <typename T = uint32_t>
T get_sound_config_number_value(const std::string& option_id) {
  return static_cast<T>(std::get<double>(recompui::config::get_general_config().get_option_value(option_id)));
}

int zelda64::get_bgm_volume() {
  return get_sound_config_number_value<int>(zelda64::configkeys::sound::bgm_volume);
}

static void set_control_defaults() {
  using namespace recompinput;

    // Left shoulder -> C Down | Item Button
    set_default_mapping_for_controller(
        GameInput::C_DOWN,
        { 
            InputField::controller_analog(SDL_GameControllerAxis::SDL_CONTROLLER_AXIS_RIGHTY, true),
            InputField::controller_digital(SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_LEFTSHOULDER)
        }
    );

    // Right shoulder -> C Up | first person
    set_default_mapping_for_controller(
        GameInput::C_UP,
        { 
            InputField::controller_analog(SDL_GameControllerAxis::SDL_CONTROLLER_AXIS_RIGHTY, false),
            InputField::controller_digital(SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_RIGHTSHOULDER)
        }
    );

    // North button -> C Left | Item Button
    set_default_mapping_for_controller(
        GameInput::C_LEFT,
        { 
            InputField::controller_analog(SDL_GameControllerAxis::SDL_CONTROLLER_AXIS_RIGHTX, false),
            InputField::controller_digital(SDL_CONTROLLER_BUTTON_NORTH)
        }
    );

    // East button -> C Right | Item Button
    set_default_mapping_for_controller(
        GameInput::C_RIGHT,
        { 
            InputField::controller_analog(SDL_GameControllerAxis::SDL_CONTROLLER_AXIS_RIGHTX, true),
            InputField::controller_digital(SDL_CONTROLLER_BUTTON_EAST)
        }
    );

    // R3 -> L | Unused in OoT but can be used in mods
    set_default_mapping_for_controller(GameInput::L, { InputField::controller_digital(SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_RIGHTSTICK) });
}

static void set_control_descriptions() {
  recompinput::set_game_input_description(recompinput::GameInput::Y_AXIS_POS, "Used to move.");
  recompinput::set_game_input_description(recompinput::GameInput::Y_AXIS_NEG, "Used to move.");
  recompinput::set_game_input_description(recompinput::GameInput::X_AXIS_NEG, "Used to move.");
  recompinput::set_game_input_description(recompinput::GameInput::X_AXIS_POS, "Used to move.");
  recompinput::set_game_input_description(recompinput::GameInput::A, "Action Button.");
  recompinput::set_game_input_description(recompinput::GameInput::B, "Sword Attack Button.");
  recompinput::set_game_input_description(recompinput::GameInput::Z, "Lock on Button.");
  recompinput::set_game_input_description(recompinput::GameInput::L, "Minimap Toggle Button.");
  recompinput::set_game_input_description(recompinput::GameInput::R, "Shield Button.");
  recompinput::set_game_input_description(recompinput::GameInput::START, "Pause Button.");
  recompinput::set_game_input_description(recompinput::GameInput::C_UP, "Used to enter first-person mode, and talk to Navi.");
  recompinput::set_game_input_description(recompinput::GameInput::C_DOWN, "Item Button.");
  recompinput::set_game_input_description(recompinput::GameInput::C_LEFT, "Item Button.");
  recompinput::set_game_input_description(recompinput::GameInput::C_RIGHT, "Item Button.");
  recompinput::set_game_input_description(recompinput::GameInput::DPAD_UP, "Unused. Mods may use it for additional features.");
  recompinput::set_game_input_description(recompinput::GameInput::DPAD_DOWN, "Unused. Mods may use it for additional features.");
  recompinput::set_game_input_description(recompinput::GameInput::DPAD_LEFT, "Unused. Mods may use it for additional features.");
  recompinput::set_game_input_description(recompinput::GameInput::DPAD_RIGHT, "Unused. Mods may use it for additional features.");
}

void zelda64::init_config() {
  std::filesystem::path recomp_dir = recompui::file::get_app_folder_path();

  if (!recomp_dir.empty()) {
    std::filesystem::create_directories(recomp_dir);
  }

  recompui::config::GeneralTabOptions general_options{};
  general_options.has_rumble_strength = true;
  general_options.has_gyro_sensitivity = false;
  general_options.has_mouse_sensitivity = false;

  auto &general_config = recompui::config::create_general_tab(general_options);
  add_general_options(general_config);

  auto &graphics_config = recompui::config::create_graphics_tab();
  add_graphics_options(graphics_config);

  set_control_defaults();
  set_control_descriptions();
  recompui::config::create_controls_tab();

  auto &sound_config = recompui::config::create_sound_tab();
  add_sound_options(sound_config);

  recompui::config::create_mods_tab();
  recompui::config::finalize();
}
