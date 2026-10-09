#ifndef __ZELDA_GAME_H__
#define __ZELDA_GAME_H__

#include <cstdint>
#include <span>
#include <vector>
#include "recomp.h"

namespace zelda64 {
    void quicksave_save();
    void quicksave_load();
    std::vector<uint8_t> decompress_oot(std::span<const uint8_t> compressed_rom);
};

#endif
