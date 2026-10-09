#ifndef OOT_RECOMP_GFX_POOL_H
#define OOT_RECOMP_GFX_POOL_H

#include "gfx.h"

#define GFXPOOL_HEAD_MAGIC 0x1234
#define GFXPOOL_TAIL_MAGIC 0x5678

typedef struct {
  Gfx polyOpaBuffer[0x10000];
  Gfx polyXluBuffer[0x4000];
  Gfx overlayBuffer[0x1000];
  Gfx workBuffer[0x400];
} BiggerGfxPool;

extern BiggerGfxPool gBiggerGfxPools[2];

#endif // OOT_RECOMP_GFX_POOL_H
