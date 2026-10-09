#include "ovl_patches.hpp"
#include "../../RecompiledPatches/recomp_overlays.inl"

#include "librecomp/overlays.hpp"
#include "librecomp/game.hpp"

extern "C" {
  extern const char oot_patches_bin[];
  extern const size_t oot_patches_bin_size;
}

void zelda64::register_oot_patches() {
  recomp::overlays::register_base_exports(export_table);
  recomp::overlays::register_base_events(event_names);
  recomp::overlays::register_manual_patch_symbols(manual_patch_symbols);
}
