#include <cstdio>
#include "recomp.h"

namespace {
  bool warned_write_io = false;
  bool warned_disk_init = false;
}

extern "C" void osEPiWriteIo_recomp(uint8_t* rdram, recomp_context* ctx) {
  (void)rdram;
  (void)ctx;
  if (!warned_write_io) {
    warned_write_io = true;
    std::fprintf(stderr, "[shim] osEPiWriteIo was called. This is 64DD code that should be unreachable.\n");
  }
}

extern "C" void osLeoDiskInit_recomp(uint8_t* rdram, recomp_context* ctx) {
  (void)rdram;
  (void)ctx;
  if (!warned_disk_init) {
    warned_disk_init = true;
    std::fprintf(stderr, "[shim] osLeoDeskInit was called. This is 64DD code that should be unreachable.\n");
  }
}
