#include "patches.h"

#include "alignment.h"
#include "animation.h"
#include "dma.h"
#include "object.h"
#include "pause.h"
#include "play_state.h"
#include "player.h"
#include "save.h"
#include "sched.h"
#include "segmented_address.h"

#include "assets/objects/gameplay_keep/player_anim_headers.h"

// @recomp Patched to wait for the RCP before overwriting gameplay_keep in place.
RECOMP_PATCH u32 Player_InitPauseDrawData(PlayState* play, u8* segment, SkelAnime* skelAnime) {
  s16 linkObjectId = gLinkObjectIds[gSaveContext.save.linkAge];
  u32 size;
  void* ptr;

  // @recomp Fix bad read, previous frame's list draws compass arrow from memory that is about
  // to be replaced.
  Sched_FlushTaskQueue();

  size = gObjectTable[OBJECT_GAMEPLAY_KEEP].vromEnd - gObjectTable[OBJECT_GAMEPLAY_KEEP].vromStart;
  ptr = PAUSE_PLAYER_SEGMENT_GAMEPLAY_KEEP_START(segment);
  DmaMgr_RequestSync(ptr, gObjectTable[OBJECT_GAMEPLAY_KEEP].vromStart, size);

  size = gObjectTable[linkObjectId].vromEnd - gObjectTable[linkObjectId].vromStart;
  ptr = PAUSE_PLAYER_SEGMENT_LINK_OBJECT(segment);
  DmaMgr_RequestSync(ptr, gObjectTable[linkObjectId].vromStart, size);

  ptr = (void*)ALIGN16((uintptr_t)ptr + size);

  gSegments[4] = OS_K0_TO_PHYSICAL(PAUSE_PLAYER_SEGMENT_GAMEPLAY_KEEP_START(segment));
  gSegments[6] = OS_K0_TO_PHYSICAL(PAUSE_PLAYER_SEGMENT_LINK_OBJECT(segment));

  SkelAnime_InitLink(play, skelAnime, gPlayerSkelHeaders[gSaveContext.save.linkAge],
      &gPlayerAnim_link_normal_wait, 9, ptr, ptr, PLAYER_LIMB_MAX);

  return PAUSE_PLAYER_SEGMENT_TOTAL_SIZE(size);
}

// @recomp Patched to wait for RCP before reloading every object over the pause menu's copy.
RECOMP_PATCH void Object_ReloadAll(ObjectContext* objectCtx) {
  s32 i;
  s32 id;
  u32 size;

  // @recomp The menu's last frame still draws Link and the item icons out of memory
  // that is about to be replaced.
  Sched_FlushTaskQueue();

  for (i = 0; i < objectCtx->numEntries; i++) {
    id = objectCtx->slots[i].id;
    size = gObjectTable[id].vromEnd - gObjectTable[id].vromStart;
    DmaMgr_RequestSync(objectCtx->slots[i].segment, gObjectTable[id].vromStart, size);
  }
}
