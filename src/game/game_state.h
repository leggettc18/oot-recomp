#pragma once

#include <cstdint>

namespace oot::game_state {
  uint32_t dropped_frames();
  void note_dropped_frame(uint32_t which, int32_t overrun);
}
