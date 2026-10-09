#include "game/game_state.h"

#include <atomic>
#include <chrono>
#include <cstdio>
#include <deque>
#include <mutex>

namespace {
  int64_t now_ms() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count();
  }
}

namespace oot::game_state {

uint32_t g_dropped_frames_value = 0;

uint32_t dropped_frames() {
  return g_dropped_frames_value;
}

void note_dropped_frame(uint32_t which, int32_t overrun) {
  ++g_dropped_frames_value;
  static int64_t last_said_ms = -1000000;
  const int64_t now = now_ms();
  if (now - last_said_ms < 1000) {
    return;
  }
  last_said_ms = now;

  const char* names[] = { "opaque", "translucent", "overlay", "work" };
  char pools[64];
  int used = 0;
  for (int bit = 0; bit < 4 && used < static_cast<int>(sizeof(pools)) - 1; ++bit) {
    if (which & (1u << bit)) {
      used += std::snprintf(pools + used, sizeof(pools) - static_cast<size_t>(used), "%s%s", used ? ", " : "", names[bit]);
    }
  }
  std::fprintf(stderr, "[gfx] frame dropped: the %s list overflowed by %d bytes (%u dropped so far)\n", used ? pools : "(none?)", overrun, g_dropped_frames_value);
}
} // namespace oot::game_state
